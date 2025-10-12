; BOOT-SECTOR, compatible to IBM PC 8088 with 64K memory
;
; originally IBM PC had 64K memory, so that only memory
; below 0x8000 was utilized by the operating system.
;
; the boot sector was loaded to linear address 0x7C00
; putting it into 0x7C00-0x7DFF and one could use
; the are from 0x7E00-0x7FFF for stack (512 Bytes)

IBM_BOOTADDR     equ 0x7C00
IBM_CKSUM_ADDR   equ IBM_BOOTADDR+0x1FE
IBM_STACK        equ 0x7FFF
BYTES_PER_SECTOR equ 512

CLUSTER_SIZE     equ 1  ; Sectors per cluster
RESERVED_SECS    equ 1  ; Reserved sectors for
                        ; boot record
NUM_FATS         equ 2
ROOT_DIR_ENTRIES equ 240
LOGICAL_SECTORS  equ 2880
MEDIA_DESCRIPTOR equ 0xF0
SECS_PER_FAT     equ 9
SECS_PER_TRACK   equ 18
NUM_HEADS        equ 2

BLOAD_SECS       equ 16 ; bload.sys has maximally 8 KB

bits 16
cpu 8086

section .text start=IBM_BOOTADDR
start:  jmp short entry
        nop
bpb:
        db "BASELINE"
        ;------------ since DOS 2.0 -------------
        dw BYTES_PER_SECTOR
        db CLUSTER_SIZE
rsecs:  dw RESERVED_SECS
nfats:  db NUM_FATS
rde:    dw ROOT_DIR_ENTRIES
        dw LOGICAL_SECTORS
        db MEDIA_DESCRIPTOR
fatsecs:dw SECS_PER_FAT
        ;------------ since DOS 3.31 ------------
spt:    dw SECS_PER_TRACK
nheads: dw NUM_HEADS
        dd 0                    ; hidden sectors
        dd LOGICAL_SECTORS
        ;------------ since DOS 4.0 -------------
drv:    db 0                    ; physical drive num
        db 0                    ; flags
        db 0                    ; ext boot sig
        dd 0x0C340EFA           ; serial num
        db "BOOTVOLUME0"        ; boot volume name
        db "FAT12   "           ; FS ID

entry:
        cli
        cld
        xor ax, ax
        mov ds, ax
        mov es, ax
        xor ax, ax
        mov ss, ax
        mov sp, IBM_STACK
        mov byte [drv], dl
        sti

        ; put out smiley face to signal we are there
        mov al, dl
        inc al
        call putc

        ; check first directory entry to be our boot file
        ; BLOAD.SYS
        mov di, 0x700
        xor ax, ax
        mov al, byte [nfats]
        mul word [fatsecs]
        add ax, word [rsecs]
        mov bp, ax              ; hack, save ax with bp
        push bp
        call read_lba
        pop bp
        jc boot_error

        mov si, 0x700
        mov di, bootfile
        mov cx, 11
        repe cmpsb
        jne boot_error

        ; BLOAD.SYS is first file, so read it
        mov bx, word [rde]
        mov cl, 4             ; 16 dir entries per sector
        shr bx, cl            ;    hence divide by 16
        mov ax, bp
        add ax, bx
        mov di, 0x700
        mov cx, BLOAD_SECS
load_bload:
        push cx
        call read_lba
        pop cx
        jc boot_error
        loop load_bload
        mov dl, byte [drv]

        jmp 0x70:0              ; jump to bload.sys

boot_error:
        mov si, booterror
        call puts
        jmp $

puts:
        lodsb
        test al, al
        jz ends
        call putc
        jmp puts
ends:   ret
putc:
        mov bx, 7
        mov ah, 0eh
        int 10h
        ret

read_lba:  ; input: ax = LBA [0-2779] for 1.44M
           ; es:di = buffer
        mov bl, byte [spt]
        div bl
        inc ah
        mov byte [bpb], ah      ; we reuse this as 'sector'

        ; now if we have 1 head, then [h] = 0, [c] = al
        ; if we have 2 heads, then [h] = al AND 1, [c] = al SHR 1

        mov cl, byte [nheads]
        dec cl
        shr al, cl
        mov byte [bpb + 1], al  ; cylinder
        mov byte [bpb + 2], 0   ; head
        adc byte [bpb + 2], 0

        mov cl, 3
read_sector: ; dst = es:di
        mov byte [bpb + 3], cl   ; retry count
        clc
        mov ah, 2
        mov al, 1
        mov ch, byte [bpb + 1]
        mov cl, byte [bpb] ; sector - bits 6,7 are cyl_hi == 0
        mov dh, byte [bpb + 2] ; head
        mov dl, byte [drv]
        mov bx, di
        int 13h
        jc  read_error
        add di, 512
        ret
read_error:
        mov cl, byte [bpb + 3]
        loop read_sector
        ret

bootfile: db "BLOAD   SYS"
booterror:db "Error", 00

section .cksum start=IBM_CKSUM_ADDR
        db 0x55, 0xAA
