#include "int86.h"
#include "conio.h"
#include "vesa.h"
#include "dev.h"
#include "bda.h"
#include "c_printf.h"
#include "blkio.h"
#include "system.h"


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

        res = dev_init();

        puts((char *)hello_msg);


        sys_get_equipment(&eqp);

        /* Initialize block IO and block devices */

        blkio_init();

//        drv_letter = 'A';
        drive = 0;
        blkdev_counter = 0;

        for (drive = 0; drive < eqp->num_floppies; drive++) {
                bdev = blkio_get_dev(drive);
                /* init BIOS int13 driver for the drive */
                blkdrv_int13_init((blk_drv_t *)&bdev->drv_int13,
                                  (void *)&drive);
                bdev->has_parttable = 0;
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
                c_snprintf(bdev->name, 8, "HD%u", drive - 0x80);
                drv_letter++;
                blkdev_counter++;
        }

        blkio_set_max(blkdev_counter - 1);

        /* Initialize logical drives */

        for (drive = 0; drive < blkdev_counter; drive++) {
                bdev = blkio_get_dev(drive);

                if (!bdev) {
                        /* should not happen */
                        break;
                }
                if (!bdev->has_parttable) {
                        /* no partition table, filesystem is on
                           drive itself */
                        res = blkio_read_vbr(bdev, 0);
                        if (res) {
                                /* could not read VBR */
                        }
                } else {


                }
        }

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
        puts("Boot drive is ");
        putc(boot_drive + 'A');
        puts(":\r\n");
//       puts("VESA mode found: ");
//       dump16(mode);
        puts("\r\n");
//        res = vesa_mode_set(mode);

kernel_halt:
        goto kernel_halt;

        return 0;
}

