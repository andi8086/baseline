#include "int86.h"
#include "conio.h"
#include "vesa.h"
#include "dev.h"
#include "bda.h"
#include "c_printf.h"
#include "blkio.h"
#include "system.h"
#include "mem.h"


int main(void);
uint16_t read_far16(uint16_t seg, uint16_t offs);

uint8_t boot_drive;


void _cstart(void)
{
        __asm {
                mov ax, 70h
                mov ds, ax
                mov es, ax
                mov ss, ax
                mov sp, 0FFFFh
                mov boot_drive, dl
        }
        main();
}


const char *hello_msg = "\r\nBaseline, v0.1\r\n"
                        "(C)2025 by Andreas J. Reichel\r\n";



int main(void)
{
        equipment_t *eqp;
        int res;
        uint16_t mode, fblo, fbhi;
        char bufr[128];
        uint8_t drive = 0;
        uint8_t drive_counter = 0;
        blk_dev_t *bdev;
        char drv_letter;
        uint8_t blkdev_counter;
        char *vbr;
        bpb_dos200_t *bpb;


        res = dev_init();

        printf("%s\r\n", hello_msg);

        sys_get_equipment(&eqp);

        /* Initialize block IO and block devices */

        blkio_init(boot_drive);

        drive = 0;
        blkdev_counter = 0;

        for (drive = 0; drive < eqp->num_floppies; drive++) {
                bdev = blkio_get_dev(drive);
                /* init BIOS int13 driver for the drive */
                blkdrv_int13_init((blk_drv_t *)&bdev->drv_int13,
                                  (void *)&drive);
                bdev->has_parttable = 0;
                bdev->type = BLK_DEV_PHYSICAL;
                c_snprintf(bdev->name, 8, "FD%u", drive);
                drv_letter++;
                blkdev_counter++;
        }

        for (drive = 0x80; drive < 0x80 + eqp->num_hdds - 1; drive++) {
                bdev = blkio_get_dev(blkdev_counter);
                /* init BIOS int13 driver for the drive */
                blkdrv_int13_init((blk_drv_t *)&bdev->drv_int13,
                                  (void *)&drive);
                bdev->has_parttable = 1;
                bdev->type = BLK_DEV_PHYSICAL;
                c_snprintf(bdev->name, 8, "HD%u", drive - 0x80);
                drv_letter++;
                blkdev_counter++;
        }

        blkio_set_max(blkdev_counter - 1);

        /* Initialize drive table */
        drv_letter = 'A';

        for (drive = 0; drive < blkdev_counter; drive++) {
                bdev = blkio_get_dev(drive);

                if (!bdev) {
                        /* should not happen */
                        break;
                }
                if (!bdev->has_parttable) {
                        /* no partition table, filesystem is on
                           drive itself */
                        vbr = blkio_read_vbr(bdev, 0);
                        if (!vbr) {
                                /* could not read VBR */
                                continue;
                        }
                        /* we assume FAT12 (supported for 4 FDDs) */
                        if (!(bdev->drv_int13.drive_number == 0 ||
                            bdev->drv_int13.drive_number == 1 ||
                            bdev->drv_int13.drive_number == 2 ||
                            bdev->drv_int13.drive_number == 3)) {
                                continue;
                        }

                        bdev->fs_type = FS_TYPE_FAT12;
                        bdev->vfat.fat_bits = 12;

                        bpb = (bpb_dos200_t *)(vbr + BPB_START_OFFSET);

                        bdev->vfat.fat_start = bpb->reserved_sectors;
                        bdev->vfat.cluster_size = bpb->sectors_per_cluster;
                        bdev->vfat.fat_sectors = bpb->sectors_per_fat;
                        bdev->vfat.num_fats = bpb->num_fats;

                        bdev->vfat.root_dir_cluster = 0;
                        bdev->vfat.root_dir_lba =
                                bdev->vfat.fat_start +
                                bdev->vfat.fat_sectors * bdev->vfat.num_fats;

                        bdev->vfat.root_dir_entries = bpb->root_dir_entries;

                        drive_table[max_drive].dev = bdev;
                        drive_table[max_drive].drive_letter = drv_letter;
                        max_drive++;

                        /* round up! */
                        bdev->vfat.data_start = bdev->vfat.root_dir_lba +
                                (bpb->root_dir_entries + 15) / 16;

                        strncpy(drive_table[max_drive].current_dir,
                                "\\", 2);
                        drive_table[max_drive].current_dir_lba =
                                bdev->vfat.root_dir_lba;

                } else {


                }
        }

        puts("Boot drive is ");
        putc(boot_drive + 'A');
        puts(":\r\n");

        debug_dump_dir(&drive_table[boot_drive]);
        debug_dump_file(&drive_table[boot_drive], 28);
/*
        for (drive = 0; drive < blkdev_counter; drive++) {
                bdev = blkio_get_dev(drive);
                printf("Drive %s has "
                       "%u tracks, %u heads, %u sectors per track\r\n",
                       bdev->name, bdev->drv_int13.cmax + 1,
                       bdev->drv_int13.hmax + 1, bdev->drv_int13.smax);
        }
*/
/*        res = vesa_init();
        if (res) {
                puts("VESA init failed\r\n");
                while (1);
        }
*/

//        mode = vesa_find_mode(640, 480, 24, &fblo, &fbhi);

        // vararg_test(4, 16384u, 32768u, 65535u, 1024u);
//       puts("VESA mode found: ");
//       dump16(mode);
        puts("\r\n");
//        res = vesa_mode_set(mode);

kernel_halt:
        goto kernel_halt;

        return 0;
}

