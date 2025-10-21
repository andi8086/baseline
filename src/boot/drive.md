# We have three different drive number schemes

BIOS drive numbers go 0, 1, 2, ... for floppies,
0x80, 0x81, 0x82, ... for HDDs

FCB drive numbers go from 1, 2, 3, 4, 5 continuously,
where 0 means the current drive.

Internal drive numbers are an index into the drive table
and map to the drive letters, so each drive letter has
an internal drive number.



## Blkdev driver layer

The blkdev driver layer generates it's BIOS calls with
the 

The int13 driver is initialized with the BIOS drive number
as parameter.

It is stored in `blkdrv_int13_t->drive_number`.

`blkbuffer_search` is called with this drive number so
it is expected that the cache entry has the BIOS drive number stored.


