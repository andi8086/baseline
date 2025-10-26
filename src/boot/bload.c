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

const char *msg_drive_not_ready = "\r\nDrive not ready\r\n";
const char *msg_drive_invalid = "\r\nInvalid drive\r\n";
const char *msg_path_not_found = "\r\nPath not found\r\n";

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

extern uint8_t current_drive;
void to_upper(char __far *str);
int change_drive(char drive_letter);
void command_dir(char __far *param);
int command_cd(char __far *param, int virtual);
void command_cls(void);

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
                                /* but we continue anyway to have
                                   this drive letter registered */
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
                        bdev->vfat.drv = &bdev->drv_gen;

                        vfat_init_from_vbr(&bdev->vfat, vbr);

                        drive_table[max_drive].dev = bdev;
                        drive_table[max_drive].drive_letter = drv_letter;
                        strncpy(drive_table[max_drive].current_dir,
                                "\\", 2);
                        drive_table[max_drive].current_dir_cluster = 0;
                        drive_table[max_drive].drive_letter = drv_letter;
                        drv_letter++;
                        max_drive++;
                } else {


                }
        }

//        debug_dump_dir(&drive_table[boot_drive]);
//        debug_dump_file();
        
        while (1) {
                drive_entry_t *drive = &drive_table[current_drive - 1];
                char __far *s;
                char __far *dir = drive->current_dir;
                int cmd_len;
                int res;

                if (strlen(dir) > 1) {
                        dir++;
                }

                printf("\r\n%c:\\%.*s>", drive->drive_letter,
                        strlen(dir)-1, dir);

                buffered_input(input_buffer);
                to_upper(input_buffer);
                s = strtok(input_buffer, " ");
                if (!s) {
                        continue;
                }
                cmd_len = strlen(s);

                if (cmd_len == 3 && strncmp(s, "DIR", 3) == 0) {
                        s = strtok(NULL, " ");
                        command_dir(s);                      
                } else
                if (cmd_len == 2 && strncmp(s, "CD", 2) == 0) {
                        s = strtok(NULL, " ");
                        res = command_cd(s, 0);
                        if (res == 1) {
                                printf(msg_drive_not_ready);
                        } else if (res == 2) {
                                printf(msg_drive_invalid);
                        } else if (res == 3) {
                                printf(msg_path_not_found);
                        }
                } else
                if (cmd_len == 3 && strncmp(s, "CLS", 3) == 0) {
                        command_cls();
                } else
                if (cmd_len == 2 && s[1] == ':' &&
                    (s[0] >= 'A' && s[0] <= 'Z')) {
                        res = change_drive(s[0]);
                        if (res == 1) {
                                printf(msg_drive_not_ready);
                        } else if (res == 2) {
                                printf(msg_drive_invalid);
                        }
                } else {
                        printf("\r\n?");
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


int change_drive(char drive_letter)
{
        int lw = 0;
        for (lw = 0; lw < max_drive; lw++) {
                if (drive_table[lw].drive_letter != drive_letter) {
                        continue;
                } 
                if (blkio_change_drive(
                        &drive_table[lw]) == 0) {
                        current_drive = lw + 1;
                        return 0;
                }
                /* Drive not ready */
                return 1;
        }
        /* Unknown drive letter */
        return 2;
}


void command_cls(void)
{
        __asm {
                "push bx"
                "push dx"
                "mov ah, 6"
                "mov al, 0"
                "mov bh, 7"
                "mov ch, 0"
                "mov cl, 0"
                "mov dh, 49"
                "mov dl, 79"
                "int 10h"
                "mov ah, 2"
                "mov bh, 0"
                "mov dx, 0"
                "int 10h"
                "pop dx"
                "pop bx"
        }
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


#define SCANPATH_DIR  1
#define SCANPATH_FILE 2

/* Cluster 1 never used, this is an error */
#define SCANPATH_ERR 1

#undef SCANPATH_DEBUG

unsigned long scan_path(char __far *path, int flags, char __far *path_buffer)
{
        extern uint8_t __far *dta;
        drive_entry_t __far *drive;
        fcb_t fcb;
        vfat_dir_entry_t __far *e;
        char __far *d, __far *s, __far *p, __far *p_old;
        int count, scan_absolute;
        unsigned long old_dir_cluster;
        int len;
        unsigned long res;
        char __far *pdst = path_buffer;

        /* TODO: implement drive letter support */
        drive = &drive_table[current_drive - 1];
        e = (vfat_dir_entry_t __far *)dta; 
              
        /* check if first char is a backslash */
        if (*path == '\\') {
                scan_absolute = 1;
                while (*path == '\\') path++;
                /* strangely specified root directory */
                if (!*path) {
                        if (pdst) {
                                *(pdst++) = '\\';
                                *pdst = 0;
                        }
                        /* FIXME: this will not always be zero! */
                        return 0;
                }
        } else {
                scan_absolute = 0;
        }

#ifdef SCANPATH_DEBUG
        /*************** DEBUG ****************/
        printf("\r\nScan absolute: %d\r\n", scan_absolute);
        printf("Input path: %s\r\n", path);
        printf("Current dir: %s\r\n", drive->current_dir);
        /*************** DEBUG ****************/
#endif

        old_dir_cluster = drive->current_dir_cluster; 

        if (scan_absolute) {
                /* start scanning from root directory */
                drive->current_dir_cluster =
                        drive->dev->vfat.root_dir_cluster;
                if (path_buffer) {
                        str_append(pdst++, "\\");
                }
        } else {
                if (path_buffer) {
                        len = strlen(drive->current_dir);
                        strncpy(pdst, drive->current_dir, len);
                        pdst += len;
                        if (*(pdst - 1) != '\\') {
                                *(pdst++) = '\\';
                        }
                        *pdst = 0; // temporary
                }
        }

#ifdef SCANPATH_DEBUG
        /**************** DEBUG *****************/
        if (path_buffer) {
                printf("\r\nStart-Buffer: %s\r\n", path_buffer);
        }
        /**************** DEBUG *****************/
#endif

        p = strtok(path, "\\");

        while (p) {
                memset(&fcb, 0, sizeof(fcb_t));
                fcb.drive_id = current_drive;
                /* set up file name in FCB */
                to_upper(p);
                fcb_set_filename(&fcb, p);
                
                /* search the entered name */
                if (vfat_dir_search(&drive->dev->vfat,
                    drive->current_dir_cluster, &fcb) != 0) {
                        goto file_not_found;
                }

                p_old = p;
                p = strtok(NULL, "\\");

                if (flags & SCANPATH_DIR) {
                        /* check last level according to flags */
                        if (!(e->attrib & FATTR_DIR)) {
                                goto invalid_path;
                        }
                }

                drive->current_dir_cluster =
                        (unsigned long)e->start_cluster_hi * 65536 +
                        e->start_cluster;

                if (pdst) {
                        if (strncmp(p_old, "..", 2) == 0) {
                                pdst--;
                                while (*(pdst-1) != '\\') {
                                        pdst--;
                                        *pdst = 0;
                                }
                                continue;
                        }
                        if (strncmp(p_old, ".", 1) == 0) {
                                continue;
                        }
                        len = strlen(p_old);
                        strncpy(pdst, p_old, len);
                        pdst += len;
                        *(pdst++) = '\\';
                }
        } 

        if (pdst) {
                *pdst = 0;
        }

        res = (unsigned long)e->start_cluster_hi * 65536 + e->start_cluster;
#ifdef SCANPATH_DEBUG
        printf("path scan compete\r\n");
        printf("cluster is %lu\r\n", res);
#endif
        return res;
file_not_found:
invalid_path:
#ifdef SCANPATH_DEBUG
        printf("invalid path\r\n");
#endif
        /* restore old directory */
        drive->current_dir_cluster = old_dir_cluster;
        /* return invalid cluster */
        return SCANPATH_ERR;
}


/* A virtual CD is a CD that does not alter the current
   path string, but works on cluster level

returns: 0: success
         1: drive not ready
         2: drive invalid
         3: path not found
*/


int command_cd(char __far *param, int virtual)
{
        extern uint8_t __far *dta;
        drive_entry_t *drive;
        fcb_t fcb;
        vfat_dir_entry_t __far *e;
        char __far *d, __far *s;
        int count;
        char path_buffer[MAX_PATH]; 
        unsigned long res;
        char current_drive_letter;

 
        drive = &drive_table[current_drive - 1];
        current_drive_letter = 'A' + current_drive - 1;

        if (!virtual) {
                printf("\r\n");
                if (!param) {
                        printf("\r\n%c:%s\r\n",
                               current_drive_letter,
                               _MK_FP(_SEG_DS(), drive->current_dir));
                        return 0;
                }
        }

        if (!virtual && strlen(param) >= 2 && param[1] == ':') {
                /* drive leter specified, temporarily change drive */
                res = change_drive(param[0]);
                if (res == 1) {
                        return 1;
                } else if (res == 2) {
                        return 2;
                }
                drive = &drive_table[current_drive - 1];
                /* here drive has changed, increment param */
                param += 2;
                if (param[0] == 0) {
                        printf("\r\n%c:%s\r\n",
                               param[-2],
                               _MK_FP(_SEG_DS(), drive->current_dir));

                        change_drive(current_drive_letter); 

                        return 0;
                } 
        }

        if (strlen(param) == 1 && *param == '\\') {
                drive->current_dir_cluster = 0;

                if (!virtual) {
                        drive->current_dir[0] = '\\';
                        drive->current_dir[1] = 0;
                        change_drive(current_drive_letter); 
                }
                return 0;
        }

        /* directory specified, scan for it */
        path_buffer[0] = 0;
        if (!virtual) {
                res = scan_path(param, SCANPATH_DIR, path_buffer);
        } else {
                res = scan_path(param, SCANPATH_DIR, NULL);
        }

        if (res != 1) {
                if (!virtual) {
                        memcpy(drive->current_dir,
                               _MK_FP(_SEG_DS(), path_buffer),
                               strlen(_MK_FP(_SEG_DS(), path_buffer)) + 1);
                }
#ifdef PATHSCAN_DEBUG
                printf("\r\nReturned: %s\r\n", _MK_FP(_SEG_DS(), path_buffer));
#endif
                drive->current_dir_cluster = res;
        } else {
                if (!virtual) {
                        change_drive(current_drive_letter); 
                }
                return 3;
        }

        if (!virtual) {
                change_drive(current_drive_letter); 
        }

        return 0;
}


void command_dir(char __far *param)
{
        fcb_t fcb;
 
        drive_entry_t *drive;
        unsigned long bytes_used = 0;
        extern uint8_t __far *dta;
        vfat_dir_entry_t __far *e;
        uint16_t count = 0;
        char saved_drive_letter = 'A' + current_drive - 1;
        int res, path_specified;
        char __far *tmp;
        char __far *root = "\\";
        unsigned long saved_dir_cluster;


        /* first check if we have a drive specification */
        if (param && strlen(param) >= 2 && param[1] == ':') {
                res = change_drive(param[0]); 
                if (res == 1) {
                        printf(msg_drive_not_ready);
                        return;
                } else if (res == 2) {
                        printf(msg_drive_invalid);
                        return;
                }
                param += 2;
        } 


        /* 2nd check if a path is specified, i.e. if the string
           contains any backslash */
        path_specified = 0;
        tmp = param;
        while (*tmp) {
                if (*tmp == '\\') {
                        path_specified = 1;
                        break;
                }
                tmp++;
        }

        memset(&fcb, 0, sizeof(fcb_t));
        fcb.drive_id = current_drive;
        drive = &drive_table[current_drive - 1];
        saved_dir_cluster = drive->current_dir_cluster;

        tmp = param;

        if (path_specified) {
                /* goto end of string and scan backwards until
                   backslash */
                tmp = param + strlen(param) - 1;
                while (*tmp != '\\' && tmp >= param) {
                        tmp--;
                }

                if (strlen(tmp) > 0) {
                        *tmp = 0;
                        tmp++;
                } else {
                        tmp = 0;
                }

                if (*param == 0) {
                        param = root;
                }

#ifdef DIR_DEBUG
                printf("path is: %s\r\n", param);
                printf("file is: %s\r\n", tmp);
                goto debug_done;
#endif 
                /* Now we have separated the file name (if any)
                   from the directory and first CD into the directory */

                /* do a virtual directory change */
                res = command_cd(param, 1);
                if (res) {
                        switch (res) {
                        case 1: printf(msg_drive_not_ready); break;
                        case 2: printf(msg_drive_invalid); break;
                        case 3: printf(msg_path_not_found); break;
                        default: break;
                        }
                        goto dir_done;
                }
        }

        if (param && strlen(tmp) > 0) {
                to_upper(tmp);
                fcb_set_filename(&fcb, tmp);
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
        printf("%10u Files %13lu Bytes\r\n", count, bytes_used);
        printf("%30lu Bytes free\r\n", vfat_free_space(&drive->dev->vfat));
#ifdef DIR_DEBUG
debug_done:
#endif
dir_done:
        change_drive(saved_drive_letter);
        drive = &drive_table[current_drive - 1];
        drive->current_dir_cluster = saved_dir_cluster;
}
