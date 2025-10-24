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

uint8_t current_drive = 0;
uint8_t __far *dta;

static uint8_t blkio_maxdev = BLK_DEV_MAX;

blk_buffer_t blk_buffer[BLK_BUFFERS];

file_info_t open_files[MAX_FILES];

uint8_t sector_buffer[512];

int blkdrv_int13_init(struct blk_drv *b, void *, equipment_t *);
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


void blkio_init(uint8_t boot_drive)
{
        int i;

        for (i = 0; i < BLK_DEV_MAX; i++) {
                bzero(&blk_devs[i], sizeof(blk_dev_t));
        }

        if (boot_drive >= 0x80) {
                boot_drive -= 0x7E;
        }

        /* A = 1, B = 2, C = 3, ... */
        current_drive = boot_drive + 1;

        dta = sector_buffer;

        for (i = 0; i < BLK_BUFFERS; i++) {
                bzero(&blk_buffer[i], sizeof(blk_buffer_t));
                blk_buffer[i].drive = -1;
        }
}


blk_buffer_t *blk_buffer_get(void)
{
        /* returns a block buffer */
        int i;
        unsigned long acc0 = UINT32_MAX;
        unsigned long acc1 = UINT32_MAX;
        int acc0_idx;
        int acc1_idx;
        blk_drv_t *drv; 

        /* first look for a free one */
        for (i = 0; i < BLK_BUFFERS; i++) {
                if (blk_buffer[i].drive == -1) {
                        blk_buffer[i].acc++;
#ifdef BLKIO_DEBUG
                        ser_printf("Unused cache entry found (#%u)\r\n",
                               i);
#endif
                        return &blk_buffer[i];
                }
        }

        /* look for non-dirty buffer */
        for (i = 0; i < BLK_BUFFERS; i++) {
                if (blk_buffer[i].dirty == 0) {
                        bzero(&blk_buffer[i], sizeof(blk_buffer_t));
                        blk_buffer[i].drive = -1;
#ifdef BLKIO_DEBUG
                        ser_printf("Reusing non-dirty cache (#%u)\r\n",
                               i);
#endif
                        return &blk_buffer[i];
                }
        }

        /* get the least accessed one, by finding
           the one with the lowest and the 2nd lowest
           acc field */

        for (i = 0; i < BLK_BUFFERS; i++) {
                if (blk_buffer[i].acc < acc0) {
                        acc1 = acc0;
                        acc1_idx = acc0_idx;
                        acc0 = blk_buffer[i].acc;
                        acc0_idx = i;
                } 
        }
#ifdef BLKIO_DEBUG
        ser_printf("Dropping oldest cache #%u\r\n", acc0_idx);
#endif
        /* subtract acc1 from all acc fields */
        for (i = 0; i < BLK_BUFFERS; i++) {
                blk_buffer[i].acc -= acc1;
        }

        /* flush buffer acc0_idx if dirty */
        if (blk_buffer[acc0_idx].dirty) {
#ifdef BLKIO_DEBUG
                ser_printf("Flusing oldest cache #%u\r\n", acc0_idx);
#endif
                drv = blk_buffer[acc0_idx].drv;
                drv->write(drv, _FP_SEG(&blk_buffer[acc0_idx].buffer),
                           _FP_OFF(&blk_buffer[acc0_idx].buffer),
                           blk_buffer[acc0_idx].lba, 512);
                bzero(&blk_buffer[acc0_idx], sizeof(blk_buffer_t));
                blk_buffer[acc0_idx].drive = -1;
        }

        return &blk_buffer[acc0_idx];
}


blk_buffer_t *blk_buffer_search(int drive, unsigned long lba)
{
        /* searches if a desired block has been cached */
        int i;

        for (i = 0; i < BLK_BUFFERS; i++) {
                if (blk_buffer[i].drive == drive &&
                    blk_buffer[i].lba == lba) {
#ifdef BLKIO_DEBUG
                        ser_printf("sector is in cache #%u ", i);
                        ser_printf("(drive = %u, lba = %lu)\r\n",
                                    drive, lba);
#endif
                        return &blk_buffer[i];
                }
        }

        return NULL;
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


int blkdrv_int13_init(struct blk_drv *b, void *p, equipment_t *e)
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
        int13_extended = 0;

        if (e->cpu_type >= CPU_TYPE_80386) {

                rin._es = _SEG_DS();
                rin._ax = 0x4100;
                rin._dx = 0x0080;
                rin._bx = 0x55AA;
                disk_int86(&rin, &rout);
                if ((rout._flags & FLAGS_CARRY) == 0) {
                        int13_extended = 1;
                }
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
                        /* this might be a very old BIOS, so use BPB
                           for floppy drives */

                        /* TODO: read BPB and init with it */
                        if (drive <= 3) {
                                bd->hmax = 1;
                                bd->cmax = 39;
                                bd->smax = 9; 
                                bd->ext_bios = 0;
                                return 0;
                        }

                        return 1;
                }
                printf("After int13, 08\r\n");
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
        blk_buffer_t *cache;
        uint16_t seg_cache;
        uint16_t off_cache;

        if (DEBUG) {
                printf("buffer = %lp\r\n",
                       ((unsigned long)seg_buffer << 16) | offs_buffer);
                printf("addr = %lu\r\n", addr);
        }

        lba_to_chs(drv, addr, &cyl, &head, &sec);
#ifdef BLKIO_DEBUG
        ser_printf("blkdrv_int13_read: LBA address %lu is C/H/S %u/%u/%u\r\n",
                   addr, cyl, head, sec);
#endif
        if (DEBUG) {
                printf("cyl = %u, head = %u, sec = %u\r\n",
                       cyl, head, sec);
        }

        /* check if we have buffered that sector */
        cache = blk_buffer_search(drv->drive_number, addr);

        if (cache) {
                seg_cache = _FP_SEG(cache->buffer);
                off_cache = _FP_OFF(cache->buffer);
                goto copy_from_cache;
        }

        /* not cached yet, flush the oldest by
           requesting a new one */
        cache = blk_buffer_get();
        cache->drive = drv->drive_number;
        cache->lba = addr;
        cache->drv = b;

        seg_cache = _FP_SEG(cache->buffer);
        off_cache = _FP_OFF(cache->buffer);

        rin._ax = 0x0200 | (size & 0xFF);
        rin._dx = ((uint16_t)head << 8) | drv->drive_number;
        rin._cx = ((uint16_t)cyl << 8) |
                  (((uint16_t)cyl >> 8) << 6) | (sec & 63);
        rin._bx = off_cache;
        rin._es = seg_cache;

        if (DEBUG) {
                printf("AX = %04x, BX = %04x, CX = %04x, DX = %04x, "
                       "ES = %04x\r\n",
                       rin._ax, rin._bx, rin._cx, rin._dx, rin._es);
        }

        disk_int86(&rin, &rout);

        if (rout._flags & FLAGS_CARRY) {
                return 1;
        }
copy_from_cache:
        /* copy into destination buffer */
        memcpy(_MK_FP(seg_buffer, offs_buffer),
               _MK_FP(seg_cache, off_cache),
               512);

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


void vfat_decode_cdate(vfat_dir_entry_t __far *e, datetime_t *dt)
{
        dt->year = (e->creat_time[4] >> 1) + 1980;
        dt->mon = (e->creat_time[4] & 1) * 8 | (e->creat_time[3] >> 5);
        dt->day = (e->creat_time[3] & 31);
        dt->hour = (e->creat_time[2] >> 3);
        dt->min = ((e->creat_time[2] & 7) << 3) | (e->creat_time[1] >> 5);
        dt->sec = (e->creat_time[1] & 31) << 1;
}


void debug_dump_dir_entry(vfat_dir_entry_t __far *dire)
{
        datetime_t cdate;
        char attr_ch;

/*        if (dire->attrib == FATTR_LFN) {
                dire++;
                continue;
        }
*/
        attr_ch = ' ';
        if (dire->attrib & FATTR_SYSTEM) {
                attr_ch = 0xB0;
                if (dire->attrib & FATTR_HIDDEN) {
                        attr_ch = 0xB2;
                }       
        } else
        if (dire->attrib & FATTR_HIDDEN) {
                attr_ch = 0xB1;
        }

        vfat_decode_cdate(dire, &cdate);

        if (dire->attrib & FATTR_DIR) {
                printf("%8.8s%c%3.3s    <DIR>   ",
                dire->name, attr_ch, dire->ext);
        } else {
                printf("%8.8s%c%3.3s %10lu ", dire->name,
                attr_ch, dire->ext,
                *(unsigned long int *)&dire->file_size_lo);
        }

        printf(" %4u-%2u-%2u %02u:%02u:%02u ",
               cdate.year, cdate.mon, cdate.day,
               cdate.hour, cdate.min, cdate.sec);
        
        printf(" [%u]\r\n", dire->start_cluster);
}


/*unsigned long cluster_to_lba(vfs_vfat_t *vfat, unsigned long cluster)
{
        unsigned long lba = 0;
        unsigned long tmp = cluster - 2;
        int i;

         cannot use 32-bit multiply
        for (i = 0; i < vfat->cluster_size; i++) {
                lba += tmp;
        }

        lba += vfat->data_start;

        return lba;
}
*/

/* opening a file ALWAYS goes over the directory contents,
   hence we can always provide the parameters except if
   we open the root directory itself, or a special system file */
int vfat_file_open(vfs_vfat_t *vfat, vfat_dir_entry_t *e,
                   unsigned long dir_lba,
                   int dir_entry_rel)
{
        int i;
        unsigned long file_cluster;

        if (!e) {
                file_cluster = 2;
        } else {
                file_cluster = e->start_cluster;
        }

        for (i = 0; i < MAX_FILES; i++) {
                if (open_files[i].ref_count == 0) {
                        break;
                }
        }

        if (i == MAX_FILES) {
                return -1;
        }
/*
        open_files[i].ref_count++;

        open_files[i].vfat = vfat;
        open_files[i].dir_lba = dir_lba;
        open_files[i].rel_entry = dir_entry_rel;
        open_files[i].start_lba = cluster_to_lba(vfat, file_cluster);

        open_files[i].file_ptr_seq = 0;
        open_files[i].current_lba_seq = 0;
        open_files[i].current_rel_sector_seq = 0;

        open_files[i].file_ptr_rnd = 0;
        open_files[i].current_lba_rnd = 0;
        open_files[i].current_rel_sector_rnd = 0;
*/
        return i;
}


void vfat_file_close(int file_handle)
{
        if (open_files[file_handle].ref_count) {
                open_files[file_handle].ref_count--;
        }
}


unsigned long lba_to_cluster(vfs_vfat_t *vfat, unsigned long lba, uint8_t *rel)
{
        unsigned short shift_factor;
        unsigned long int cluster;
        unsigned long int tmp;

        if (lba < vfat->data_start) {
                return 0;
        }

        switch (vfat->cluster_size) {
        case 1: shift_factor = 0; break;
        case 2: shift_factor = 1; break;
        case 4: shift_factor = 2; break;
        case 8: shift_factor = 3; break;
        case 16: shift_factor = 4; break;
        case 32: shift_factor = 5; break;
        case 64: shift_factor = 6; break;
        case 128: shift_factor = 7; break;
        default:
                return 0;
        }


        /* data start is always cluster #2 */
        tmp = lba - vfat->data_start;
        cluster = tmp >> shift_factor;
        *rel = tmp & (vfat->cluster_size - 1);

        return cluster + 2;
}


unsigned long int cluster_to_lba(vfs_vfat_t *vfat, unsigned long cluster)
{
        uint8_t shift_factor;

        switch (vfat->cluster_size) {
        case 1: shift_factor = 0; break;
        case 2: shift_factor = 1; break;
        case 4: shift_factor = 2; break;
        case 8: shift_factor = 3; break;
        case 16: shift_factor = 4; break;
        case 32: shift_factor = 5; break;
        case 64: shift_factor = 6; break;
        case 128: shift_factor = 7; break;
        default:
                return 0;
        }

        return vfat->data_start + ((cluster - 2) << shift_factor);
}


unsigned char FAT_table[1024];


unsigned long int next_cluster(vfs_vfat_t *vfat, unsigned long int cluster)
{
        unsigned long int res;
        unsigned int c = cluster & 0xFFFF;

        unsigned int fat_offset = c + (c >> 1);
        unsigned int s = vfat->fat_start + (fat_offset / 512);
        unsigned int e = fat_offset % 512;
        unsigned int v;

        vfat->drv->read(vfat->drv, _FP_SEG(FAT_table), _FP_OFF(FAT_table),
                       s, 1);
        vfat->drv->read(vfat->drv, _FP_SEG(&FAT_table[512]),
                                  _FP_OFF(&FAT_table[512]),
                       s + 1, 1);
       
        v = *(unsigned short *)&FAT_table[e];
        v = (c & 1) ? v >> 4 : v & 0xFFF; 

        if (v >= 0xFF8) {
                /* end of chain */
                return 0;
        }

        return v;
}


unsigned long vfat_free_space(vfs_vfat_t *vfat)
{
        unsigned long int space = 0, tmp = 0;
        unsigned c;
        unsigned int old_s = 0;
        unsigned int max_cluster;
        blk_drv_int13_t *b = (blk_drv_int13_t *)vfat->drv;

        /* calculate maximum cluster number */
        max_cluster = (b->cmax + 1) *
                      (b->hmax + 1) * (b->smax);

        max_cluster -= vfat->data_start;
        max_cluster /= vfat->cluster_size;
        max_cluster++;
#ifdef BLKIO_DEBUG
        ser_printf("Max cluster is %u\r\n", max_cluster);
#endif
        for (c = 2; c <= max_cluster; c++) {

                unsigned int fat_offset = c + (c >> 1);
                unsigned int s = vfat->fat_start + (fat_offset / 512);
                unsigned int e = fat_offset % 512;
                unsigned int v;

                if (s != old_s) {
                        vfat->drv->read(vfat->drv, _FP_SEG(FAT_table), _FP_OFF(FAT_table),
                                       s, 1);
                        vfat->drv->read(vfat->drv, _FP_SEG(&FAT_table[512]),
                                                  _FP_OFF(&FAT_table[512]),
                                       s + 1, 1);
                        old_s = s;
                } 
                v = *(unsigned short *)&FAT_table[e];
                v = (c & 1) ? v >> 4 : v & 0xFFF; 

                if (v == 0) {
                        space += 1;                        
                }
        }
#ifdef BLKIO_DEBUG
        ser_printf("%u clusters free\r\n", space);
#endif
        for (c = 0; c < vfat->cluster_size; c++) {
                tmp += space;
        }
        space = tmp;
        tmp = 0;
       
        space <<= 9; 
        
        return space;
}



int dir_entry_valid(char __far *e)
{
        if (*e != 0 && *e != 0x5e && *e != 0x20 && *e != 0xF6) {
                return 1;
        }
        return 0;
}


int file_compare_wild(char __far *dst, char __far *src)
{
        /* compare 8.3 with support for '?' wild card */
        /*      a ? in src always matches dst */
        int i; 

        for (i = 0; i < 11; i++) {
                if (*src != '?') {
                        if (*src != *dst) {
                                return *src - *dst;
                        }
                } 

                dst++;
                src++;
        }

        return 0;
}


int vfat_dir_search(vfs_vfat_t *vfat, unsigned long dir_cluster,
                    fcb_t __far *fcb)
{

        vfat_dir_entry_t __far *dire;
        uint16_t entry;
        drive_entry_t *drive = &drive_table[fcb->drive_id - 1];
        blk_drv_t *b = &drive->dev->drv_gen;
        unsigned long dir_lba;
        uint8_t rel;
        int must_read = 0;

        if (dir_cluster == 0) {

                /* extract continuation point from FCB */
                dir_lba = vfat->root_dir_lba;
                if (fcb->dir_lba) {
                        entry = (fcb->dir_lba - dir_lba) << 4;
                        entry += fcb->dir_rel;
                        must_read = 1;

                        entry++;
                        if (entry >= vfat->root_dir_entries) {
                                return -1;
                        }
                } else {
                        entry = 0;
                }
                /* must be root dir, hence do not limit directory
                   length via FAT chain, but via root_dir_entries  */
 
                for (; entry < vfat->root_dir_entries; entry++) {
                        if (entry % 16 == 0 || must_read) {
                                b->read(b, _FP_SEG(dta), _FP_OFF(dta),
                                        dir_lba + entry / 16, 1);
                                must_read = 0;
                        }
                        dire = (vfat_dir_entry_t __far *)dta + entry % 16;
                        if (dir_entry_valid((char __far *)dire) &&
                            !file_compare_wild((char __far *)dire,
                                (char __far *)fcb + 1)) {
                                if (entry % 16) {
                                        /* if zero, it is already there */
                                        memcpy(dta, dire, sizeof(vfat_dir_entry_t));
                                }
                                fcb->dir_lba = dir_lba + entry / 16;
                                fcb->dir_rel = entry % 16; 

                                return 0; 
                        }

                        dire++; 
                }
                
        } else {

                if (fcb->dir_lba) {
                        dir_cluster = lba_to_cluster(vfat, fcb->dir_lba, &rel);
                        entry = fcb->dir_rel;

                        entry++;
                        /* we need the next entry */
                        if (entry > 15) {
                                entry = 0;
                                /* next sector */
                                rel++;
                                if (rel == vfat->cluster_size) {
                                        /* next cluster */
                                        rel = 0;
                                        dir_cluster =
                                                next_cluster(vfat, dir_cluster);
                                        if (!dir_cluster) {
                                                /* end of dir */
                                                return -1;
                                        }
                                }
                        }

                        must_read = 1;
                } else {
                        // dir_cluster already set
                        rel = 0;
                        entry = 0;
                }


                /* we must iterate the directory like a file with FAT chain */
                do {
                        dir_lba = cluster_to_lba(vfat, dir_cluster);
#ifdef BLKIO_DEBUG
                        ser_printf("dir: lba is %lu\r\n", dir_lba + rel);
#endif
                        for (; rel < vfat->cluster_size; rel++) {
                                for (; entry < 16; entry++) {
                                        if ((entry & 15) == 0 || must_read) {
                                                b->read(b,
                                                        _FP_SEG(dta),
                                                        _FP_OFF(dta),
                                                        dir_lba + rel +
                                                        (entry >> 4),
                                                        1);
                                                must_read = 0;
                                        }
                                        dire = (vfat_dir_entry_t __far *)dta +
                                               entry % 16;
                                        if (dir_entry_valid(
                                                (char __far *)dire) &&
                                            !file_compare_wild(
                                                (char __far *)dire,
                                                (char __far *)fcb + 1)) {
                                                fcb->dir_lba = dir_lba +
                                                        rel +
                                                     (entry >> 4);
                                                fcb->dir_rel = entry % 16; 
                                                if (entry % 16) {
                                                        /* if zero, it is already there */
                                                        memcpy(dta, dire, sizeof(vfat_dir_entry_t));
                                                }
                                                return 0;
                                        }
                                }
                                entry = 0;
                        }
                        rel = 0;
                } while (dir_cluster = next_cluster(vfat, dir_cluster));
        }
        return -1;
} 


int vfat_fopen_fcb(uint16_t fcb_seg, uint16_t fcb_offs)
{
        /* we check the current directory, if it is the root directory,
           then we use number of root dir entries from BPB, otherwise
           we use the linked list in FAT to iterate through the dir */

        /* We must match the file name in the FCB in which case
           no wildcards are allowed */
        vfat_dir_entry_t __far *dire;
        int res;

        fcb_t __far *fcb = _MK_FP(fcb_seg, fcb_offs);
        drive_entry_t *drive;
         
        fcb->current_block = 0;
        fcb->rec_size = 128;

        if (fcb->drive_id == 0) {
                /* set current drive ID, be aware that A = 1,
                   B = 2, C = 3, ... */
                fcb->drive_id = current_drive;
        }

        printf("fopen_fcb: drive_id = %u\r\n", fcb->drive_id);
        printf("           file_name = %.8s.%.3s\r\n",
               fcb->file_name,
               fcb->file_ext);
        /* search current directory for the file name,
           the directory is stored in the open_file struct,
           and the FCB itself gets a pointer to the file
           handle in the reserved area, hence we internally
           only work with the file handle and also update
           the FCB if FCB functions are used */

        drive = &drive_table[fcb->drive_id - 1];


        while (vfat_dir_search(&drive->dev->vfat,
               drive->current_dir_cluster, fcb) == 0) {
                debug_dump_dir_entry((vfat_dir_entry_t __far *)dta);
        };
        /*if (res == -1) {
                return -1;
        }; */

        dire = (vfat_dir_entry_t *)dta;
#ifdef BLKIO_DEBUG
        ser_printf("*************************\r\n");
        ser_printf("dir lba: %lu, dir_rel: %u, cluster: %u\r\n",
                   fcb->dir_lba, fcb->dir_rel, dire->start_cluster);
        ser_printf("*************************\r\n");
#endif
        fcb->dir_rel = 0;
        fcb->dir_lba = 0;
        while (vfat_dir_search(&drive->dev->vfat, 12, fcb) == 0) {
                debug_dump_dir_entry((vfat_dir_entry_t __far *)dta);
        }
        return 0; 
}


void debug_dump_file(void)
{
        blk_drv_t *b;
        vfs_vfat_t *vfat;
        unsigned long lba, tmp;
        int i;
        char *s;
        fcb_t fcb;

        fcb_t __far *x = &fcb;

        memset(x, 0, sizeof(fcb_t));
//        strncpy(x->file_name, "TEST    ", 8);
        strncpy(x->file_name, "????????", 8);
        strncpy(x->file_ext, "TXT", 3); 

        printf("%.8s.%.3s\r\n", x->file_name,
                                x->file_ext);

        vfat_fopen_fcb(_FP_SEG(x), _FP_OFF(x));

        return;

/*
        b = &drive->dev->drv_gen;
        vfat = &drive->dev->vfat;
*/
        /* calculate LBA of cluster (cluster 2 is the first of the
           data area */
/*
        lba = 0;
        tmp = file_cluster - 2;
        for (i = 0; i < vfat->cluster_size; i++) {
                lba += tmp;
        }

        lba += vfat->data_start;
        
        b->read(b, _FP_SEG(sector_buffer), _FP_OFF(sector_buffer),
                lba, 1);

        s = (char *)sector_buffer;
        for (i = 0; i < 16; i++) {
                printf("%32.32s", s);
                s += 32;
        }
*/

}
