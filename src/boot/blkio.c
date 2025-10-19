#include "blkio.h"
#include "int86.h"
#include "mem.h"
#include <stdint.h>
#include <stddef.h>

#include "conio.h"

#define DEBUG 0


static blk_dev_t blk_devs[BLK_DEV_MAX];

drive_entry_t drive_table[MAX_DRIVES];
uint8_t max_drive = 0;

static uint8_t blkio_maxdev = BLK_DEV_MAX;


int blkdrv_int13_init(struct blk_drv *b, void *);
int blkdrv_int13_read(struct blk_drv *b, uint16_t seg_buffer,
                      uint16_t offs_buffer,
                      unsigned long addr, unsigned long size);
int blkdrv_int13_write(struct blk_drv *b, uint16_t seg_buffer,
                       uint16_t offs_buffer,
                       unsigned long addr, unsigned long size);


void bzero(void *p, unsigned long size)
{
        uint8_t *dst = (uint8_t *)p;
        unsigned long i;

        for (i = 0; i < size; i++) {
                dst[i] = 0;
        }
}


void blkio_init(void)
{
        int i;

        for (i = 0; i < BLK_DEV_MAX; i++) {
                bzero(&blk_devs[i], sizeof(blk_dev_t));
        }
}


/* Int 13 extended information */
typedef struct {
        uint16_t buffer_size;
        uint16_t info_flags;
        uint16_t cylinders_lo;
        uint16_t cylinders_hi;
        uint16_t heads_lo;
        uint16_t heads_hi;
        uint16_t sectors_lo;
        uint16_t sectors_hi;
        uint16_t total_sectors[4];
        uint16_t bytes_per_sector;
} geom_desc_t;


geom_desc_t disk_geo;


int blkdrv_int13_init(struct blk_drv *b, void *p)
{
        blk_drv_int13_t *bd = (blk_drv_int13_t *)b;
        uint8_t drive = *(uint8_t *)p;
        regs86_t rin, rout;
        int int13_extended;

        bd->header.size = sizeof(blk_drv_int13_t);
        bd->drive_number = drive;

        bd->header.init = &blkdrv_int13_init;
        bd->header.read = &blkdrv_int13_read;
        bd->header.write = &blkdrv_int13_write;

        /* get drive geometry from BIOS */

        /*
                In the ATA interface, the maximum CHS values are

                65535/15/255,

                whereas for BIOS INT 0x13, they are

                1023/255/63

                Together they form 1023/15/63, which gives

                1024 * 16 * 63 * 512 bytes = 528482304 bytes
                                           = 504 MB (exactly)

                However, if int13 supports the use of a virtual
                geometry and maps this to LBA numbers directly
                controllling ATA-2+ via LBA, then the limit is

                1024 * 256 * 63 * 512 = 8455716864 bytes
                                      = 7.9 GB

        */

        /* Check if int13 extension is available */

        rin._es = _SEG_DS();
        rin._ax = 0x4100;
        rin._dx = 0x0080;
        rin._bx = 0x55AA;
        disk_int86(&rin, &rout);
        if ((rout._flags & FLAGS_CARRY) == 0) {
                int13_extended = 1;
        } else {
                int13_extended = 0;
        }

        if (drive >= 0x80 && int13_extended) {
                        /* Use extended function 48 to get geo */
                disk_geo.buffer_size = 0x1A;
                rin._ax = 0x4800;
                rin._dx = drive;
                rin._si = (uint16_t)&disk_geo;
                disk_int86(&rin, &rout);
                bd->hmax = disk_geo.heads_lo;
                bd->cmax = disk_geo.cylinders_lo;
                bd->smax = disk_geo.sectors_lo;
                bd->ext_bios = 1;
        } else {
                /* use old function 8 to get geo (504 MB max) */
                rin._ax = 0x0800;
                rin._dx = drive;
                disk_int86(&rin, &rout);
                if (rout._flags & FLAGS_CARRY) {
                        /* function returned with error */
                        return 1;
                }
                bd->hmax = rout._dx >> 8;
                bd->cmax = ((rout._cx >> 5) & 3) * 256 +
                                (rout._cx >> 8);
                bd->smax = rout._cx & 63;
                bd->ext_bios = 0;
        }

        return 0;
}


void div32(unsigned long num, uint16_t divisor,
           uint16_t *erg, uint8_t *rem)
{
        *erg = 0;

        while (num >= divisor) {
                num -= divisor;
                (*erg)++;
        }

        *rem = (uint8_t)num;

}


void lba_to_chs(blk_drv_int13_t *d, unsigned long lba,
                uint16_t *cyl, uint8_t *head, uint8_t *sec)
{
        uint8_t n_heads = d->hmax + 1;
        uint8_t secs_p_track = d->smax;
        uint16_t tmp;

        // LBA = (cyl * (hmax + 1) + head) * smax + sector - 1

        // *sec = (lba % secs_p_track) + 1;
        // tmp = lba / secs_p_track;
        div32(lba, secs_p_track, &tmp, sec);
        *sec += 1;

        *head = tmp % n_heads;
        *cyl = tmp / n_heads;
}


/* FIXME: chqange size param to uint8_t */
int blkdrv_int13_read(struct blk_drv *b, uint16_t seg_buffer,
                      uint16_t offs_buffer,
                      unsigned long addr, unsigned long size)
{
        blk_drv_int13_t *drv = (blk_drv_int13_t *)b;
        uint16_t cyl;
        uint8_t sec;
        uint8_t head;
        regs86_t rin, rout;

        if (DEBUG) {
                printf("buffer = %lp\r\n",
                       ((unsigned long)seg_buffer << 16) | offs_buffer);
                printf("addr = %lu\r\n", addr);
        }

        lba_to_chs(drv, addr, &cyl, &head, &sec);

        if (DEBUG) {
                printf("cyl = %u, head = %u, sec = %u\r\n",
                       cyl, head, sec);
        }

        rin._ax = 0x0200 | (size & 0xFF);
        rin._dx = ((uint16_t)head << 8) | drv->drive_number;
        rin._cx = ((uint16_t)cyl << 8) |
                  (((uint16_t)cyl >> 8) << 6) | (sec & 63);
        rin._bx = offs_buffer;
        rin._es = seg_buffer;

        if (DEBUG) {
                printf("AX = %04x, BX = %04x, CX = %04x, DX = %04x, "
                       "ES = %04x\r\n",
                       rin._ax, rin._bx, rin._cx, rin._dx, rin._es);
        }

        disk_int86(&rin, &rout);

        if (rout._flags & FLAGS_CARRY) {
                return 1;
        }

        return 0;
}

int blkdrv_int13_write(struct blk_drv *b, uint16_t seg_buffer,
                       uint16_t offs_buffer,
                       unsigned long addr, unsigned long size)
{
        (void)b;
        (void)seg_buffer;
        (void)offs_buffer;
        (void)addr;
        (void)size;

        /* call int 13 */
        return 0;
}


blk_dev_t *blkio_get_dev(uint8_t dev)
{
        if (dev > blkio_maxdev) {
                return NULL;
        }
        return &blk_devs[dev];
}


void blkio_set_max(uint8_t maxdev)
{
        blkio_maxdev = maxdev;
}


uint8_t sector_buffer[512];


void *blkio_read_vbr(blk_dev_t *bdev, unsigned long addr)
{
        int res;
        bpb_dos200_t *bpb = (bpb_dos200_t *)&sector_buffer[BPB_START_OFFSET];

        res = bdev->drv_gen.read(&bdev->drv_gen,
                                 _SEG_DS(), (uint16_t)sector_buffer,
                                 addr, 1);
        if (res || *(uint16_t *)&sector_buffer[510] != 0xAA55) {
                /* error */

                /* TODO: use bios default values for drive */

                return NULL;
        }

        /* seems to be a valid boot sector, read PBP */
        if (DEBUG) {
                printf("Bytes per sector: %u\r\n", bpb->bytes_per_sector);
                printf("Sectors per cluster: %u\r\n", bpb->sectors_per_cluster);
                printf("Reserved Sectors: %u\r\n", bpb->reserved_sectors);
                printf("Number of FATs: %u\r\n", bpb->num_fats);
                printf("Root dir entries: %u\r\n", bpb->root_dir_entries);
                printf("Number of Sectors: %u\r\n", bpb->num_sectors);
                printf("Media type: %x\r\n", bpb->media_type);
                printf("Sectors per FAT: %u\r\n", bpb->sectors_per_fat);
                printf("Sectors per track: %u\r\n", bpb->sectors_per_track);
                printf("Number of heads: %u\r\n", bpb->num_heads);
        }

        bdev->drv_int13.hmax = bpb->num_heads - 1;
        bdev->drv_int13.smax = bpb->sectors_per_track;

        /* FAT relevant stuff */

        /* FIXME: for 1.2 M drives, we need double stepping in some
           cases, how can we detect that? */

        return sector_buffer;
}


typedef struct {
        char name[8];
        char ext[3];
        uint8_t attrib;
        uint8_t res0;
        uint8_t creat_time[5];
        uint16_t access_time;
        uint16_t start_cluster_hi;
        uint8_t write_time[4];
        uint16_t start_cluster;
        uint16_t file_size_lo;
        uint16_t file_size_hi;
} vfat_dir_entry_t;


void debug_dump_dir(drive_entry_t *drive)
{
        blk_drv_t *b;
        int entry;
        vfat_dir_entry_t *dire;
        uint8_t hour, min, sec, sec_100;
        uint8_t year, mon, day;

        b = &drive->dev->drv_gen;

        b->read(b, _SEG_DS(), (uint16_t)sector_buffer,
                drive->dev->vfat.root_dir_lba, 1);

        dire = (vfat_dir_entry_t *)sector_buffer;

        for (entry = 0; entry < 16; entry++) {
                if (dire->name[0] != 0x20 && dire->name[0] != 0xF6 &&
                    dire->name[0] != 0 && dire->name[0] != 0xE5) {
                        printf("%.8s %.3s %8u\r\n", dire->name,
                               dire->ext, *(unsigned long int *)&dire->file_size_lo);
                }
                dire++;
        }

}
