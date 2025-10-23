#include "int86.h"
#include "conio.h"
#include "vesa.h"
#include "dev.h"
#include "bda.h"
#include "c_printf.h"
#include "blkio.h"
#include "system.h"
#include "mem.h"

#include <stddef.h>


int main(void);
uint16_t read_far16(uint16_t seg, uint16_t offs);

uint8_t boot_drive;


void _cstart(void)
{
        /* TODO: init stack correctly for 64K systems */
        __asm {
                mov ax, 70h
                mov ds, ax
                mov es, ax
                mov ss, ax
                mov sp, 78FFh
                mov boot_drive, dl
        }
        main();
}


void command_dir(char __far *param);
void command_cd(char __far *param);

const char *hello_msg = "\r\nBaseline, v0.1\r\n"
                        "(C)2025 by Andreas J. Reichel\r\n";

char input_buffer[128];

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
        ser_init();

        printf("%s\r\n", _MK_FP(_FP_SEG(hello_msg),
                                _FP_OFF(hello_msg)));

        sys_get_equipment(&eqp);

        /* Initialize block IO and block devices */

        blkio_init(boot_drive);

        printf("BLKIO initialized.\r\n");

        drive = 0;
        blkdev_counter = 0;

        for (drive = 0; drive < eqp->num_floppies; drive++) {
                bdev = blkio_get_dev(drive);
                /* init BIOS int13 driver for the drive */
                blkdrv_int13_init((blk_drv_t *)&bdev->drv_int13,
                                  (void *)&drive, eqp);
                bdev->has_parttable = 0;
                bdev->type = BLK_DEV_PHYSICAL;
                c_snprintf(bdev->name, 8, "FD%u", drive);
                blkdev_counter++;
        }

        for (drive = 0x80; drive < 0x80 + eqp->num_hdds - 1; drive++) {
                bdev = blkio_get_dev(blkdev_counter);
                /* init BIOS int13 driver for the drive */
                blkdrv_int13_init((blk_drv_t *)&bdev->drv_int13,
                                  (void *)&drive, eqp);
                bdev->has_parttable = 1;
                bdev->type = BLK_DEV_PHYSICAL;
                c_snprintf(bdev->name, 8, "HD%u", drive - 0x80);
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

                        /* round up! */
                        bdev->vfat.data_start = bdev->vfat.root_dir_lba +
                                (bpb->root_dir_entries + 15) / 16;

                        bdev->vfat.drv = &bdev->drv_gen;

                        strncpy(drive_table[max_drive].current_dir,
                                "\\", 2);
                        drive_table[max_drive].current_dir_cluster = 0;

                        max_drive++;
                } else {


                }
        }

//        debug_dump_dir(&drive_table[boot_drive]);
//        debug_dump_file();
        
        while (1) {
                char __far *s;

                printf("\r\n%c>", boot_drive + 'A');
                buffered_input(input_buffer);

                s = strtok(input_buffer, " ");

                if (strlen(s) == 3 && strncmp(s, "dir", 3) == 0) {
                        s = strtok(NULL, " ");
                        command_dir(s);                      
                } else
                if (strlen(s) == 2 && strncmp(s, "cd", 2) == 0) {
                        s = strtok(NULL, " ");
                        command_cd(s);
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
//       puts("VESA mode found: ");
//       dump16(mode);
        puts("\r\n");
//        res = vesa_mode_set(mode);

kernel_halt:
        goto kernel_halt;

        return 0;
}


void to_upper(char __far *str)
{
        while (*str) {
                if (*str <= 'z' && *str >= 'a') {
                        *str -= 'a' - 'A';
                } 
                str++;
        }
}


void fcb_set_filename(fcb_t __far *fcb, char __far *name)
{
        int count = 0;
        char __far *dst = (char __far *)fcb->file_name;

        if (strlen(name) == 1 && *name == '.') {
                strncpy(fcb->file_name, ".          ", 11);
                return;
        } else
        if (strlen(name) == 2 && name[0] == '.' && name[1] == '.') {
                strncpy(fcb->file_name, "..         ", 11);
                return;
        }

        while (count < 8 && *name) {
                if (*name == '.') {
                        break;
                }
                if (*name == '*') {
                        while (count < 8) {
                                *(dst++) = '?';
                                count++; 
                        }
                        name++;
                        break;
                }
                *(dst++) = *name; 
                count++;
                name++;
        }
        if (*name && count == 8 && *name != '.') {
                while (*name && *name != '.') {
                        name++;
                }
        }
        if (*name == '.') {
                name++;
        }
        for (; count < 8; count++) {
                *(dst++) = ' ';
        }
        count = 0;
        while (count < 3 && *name) {
                if (*name == '*') {
                        while (count < 3) {
                                *(dst++) = '?';
                                count++;
                        }
                        break;
                }
                *(dst++) = *name;
                count++;
                name++;
        }

        for (; count < 3; count++) {
                *(dst++) = ' ';
        }

}


void command_cd(char __far *param)
{
        extern uint8_t current_drive;
        extern uint8_t __far *dta;
        drive_entry_t *drive;
        fcb_t fcb;
        vfat_dir_entry_t __far *e;
        char __far *d, __far *s;
        int count;
        
        drive = &drive_table[current_drive - 1];

        if (!param) {
                printf("\r\n%c:%s\r\n",
                       'A' + current_drive - 1,
                       _MK_FP(_SEG_DS(), drive->current_dir));
                return;
        }

        if (strlen(param) == 1 && *param == '\\') {
                drive->current_dir[1] = 0;
                drive->current_dir_cluster = 0;
                return;
        }

        memset(&fcb, 0, sizeof(fcb_t));
        /* to upper case */
        to_upper(param);
        fcb_set_filename(&fcb, param);
        fcb.drive_id = current_drive;
        
        /* search the entered name */
        if (vfat_dir_search(&drive->dev->vfat,
            drive->current_dir_cluster, &fcb) != 0) {
                goto dir_not_found;
        }

        e = (vfat_dir_entry_t __far *)dta; 
              
        if (e->attrib & FATTR_DIR) {
                drive->current_dir_cluster = e->start_cluster;
                /* append name of directory onto current path */
                d = drive->current_dir;
                s = e->name;
                while (*(++d));

                if (strncmp(s, "..", 2) == 0) {
                        goto remove_one_level;
                }
                if (strncmp(s, ".", 1) == 0) {
                        goto keep_dir;
                }

                for (count = 0; *s != ' ' && count < 8; count++) {
                        *(d++) = *(s++);
                } 
                s = ((vfat_dir_entry_t __far *)dta)->ext;
                if (*s != ' ') {
                        *(d++) = '.';
                        count = 0;
                        for (count = 0; *s != ' ' && count < 3; count++) {
                                *(d++) = *(s++);
                        }
                }
                *(d++) = '\\';
                *d = 0;
                return;
remove_one_level:
                d--;
                while (*(--d) != '\\');
                *(++d) = 0;
keep_dir:
                return;
        }

dir_not_found:
        printf("Dir not found\r\n"); 
}


void command_dir(char __far *param)
{
        fcb_t fcb;
 
        drive_entry_t *drive;
        unsigned long bytes_used = 0;
        extern uint8_t current_drive;
        extern uint8_t __far *dta;
        vfat_dir_entry_t __far *e;
        uint16_t count = 0;

        memset(&fcb, 0, sizeof(fcb_t));

        fcb.drive_id = current_drive;
        drive = &drive_table[fcb.drive_id - 1];


        if (param) {
                to_upper(param);
                fcb_set_filename(&fcb, param);
        } else {
                strncpy(fcb.file_name, "????????", 8);
                strncpy(fcb.file_ext, "???", 3); 
        }
        printf("\r\n");
        printf("\r\n");
        while (vfat_dir_search(&drive->dev->vfat,
                drive->current_dir_cluster, &fcb) == 0) {
                e = (vfat_dir_entry_t __far *)dta;
                bytes_used += *(unsigned long int *)&e->file_size_lo;
                debug_dump_dir_entry(e);
                count++;
        }
        printf("\r\n");
        printf("%10u Files %13lu Bytes\r\n", count, bytes_used);
        printf("%30lu Bytes free\r\n", vfat_free_space(&drive->dev->vfat));

}
