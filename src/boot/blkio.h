#ifndef BLKIO_H
#define BLKIO_H

#include <stdint.h>

#define BLK_DEV_MAX     26
#define MAX_DRIVES      26

/* This defines the block device I/O Layer that is responsible
   for direct block device acces through a corresponding driver
*/


#define BLK_DEV_OK      0
#define BLK_DEV_OFFLINE 1
#define BLK_DEV_ERROR   2


typedef struct blk_drv {
        int (*init)(struct blk_drv *b, void *);
        int (*read)(struct blk_drv *b, uint16_t seg_buffer,
                    uint16_t offs_buffer,
                    unsigned long addr, unsigned long size);
        int (*write)(struct blk_drv *b, uint16_t seg_buffer,
                     uint16_t offs_buffer,
                     unsigned long addr, unsigned long size);
        uint16_t size;  /* size of driver specific stuff */
} blk_drv_t;


typedef struct {
        blk_drv_t header;
        uint8_t drive_number;
        uint16_t cmax;
        uint16_t hmax;
        uint16_t smax;
        int ext_bios;
} blk_drv_int13_t;



typedef struct {
        unsigned long int fat_start;
        uint8_t fat_bits;
        uint8_t fat_sectors;
        uint8_t cluster_size;
        uint8_t num_fats;
        unsigned long root_dir_cluster;
        unsigned long root_dir_lba;
} vfs_vfat_t;



#define BLK_DEV_PHYSICAL 0
#define BLK_DEV_LOGICAL  1


#define FS_TYPE_FAT12   1
#define FS_TYPE_FAT16   4       /* DOS FAT16 up to 32M */
#define FS_TYPE_FAT16a  6       /* DOS FAT16 over 32M */
#define FS_TYPE_FAT32   0x0B    /* Win95 OSR2 FAT32, up to 2047 GB */
#define FS_TYPE_FAT32b  0x0C    /* Win95 OSR2 FAT32, LBA mapped */
                                /*      (uses ext INT13) */
#define FS_TYPE_FAT16b  0x0E    /* Win95 FAT16, LBA mapped */
#define FS_TYPE_EXTPART 0x0F    /* Extended Partition, LBA */

/* phy is only set for logical devices and is
   a link to the underlying physical device

   for logical devices, the starting address
   with respect to the physical device is set
   in phys_start */

typedef struct blk_dev {
        uint16_t status;
        char name[8];
        uint8_t has_parttable;
        uint8_t type;
        struct blk_dev *phy;
        unsigned long int phys_start;
        uint8_t fs_type;
        union {
                blk_drv_t drv_gen;
                blk_drv_int13_t drv_int13;
        };
        union {
                vfs_vfat_t vfat;
        };
} blk_dev_t;


typedef struct {
        uint16_t bytes_per_sector;
        uint8_t sectors_per_cluster;
        uint16_t reserved_sectors;
        uint8_t num_fats;
        uint16_t root_dir_entries;
        uint16_t num_sectors;
        uint8_t media_type;
        uint16_t sectors_per_fat;
        uint16_t sectors_per_track;
        uint16_t num_heads;
} bpb_dos200_t;


#define BPB_START_OFFSET 0x0B


void blkio_init(void);
blk_dev_t *blkio_get_dev(uint8_t dev);

int blkdrv_int13_init(struct blk_drv *b, void *p);

void lba_to_chs(blk_drv_int13_t *d, unsigned long lba,
                uint16_t *cyl, uint8_t *head, uint8_t *sec);

void *blkio_read_vbr(blk_dev_t *bdev, unsigned long addr);



typedef struct {
        char drive_letter;
        blk_dev_t *dev;
} drive_entry_t;

extern uint8_t max_drive;
extern drive_entry_t drive_table[MAX_DRIVES];


void debug_dump_dir(drive_entry_t *drive);

#endif
