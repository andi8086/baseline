#include "vesa.h"
#include "conio.h"
#include "int86.h"

#include "mem.h"


char vesa_buffer[512];

const vesa_mode_t mode_table[] = {
        { 0x101, 640, 480, 8, "640x480x8" },
        { 0x111, 640, 480, 16, "640x480x16" },
        { 0x112, 640, 480, 24, "640x480x24" },
        { 0x103, 800, 600, 8, "800x600x8" },
        { 0x114, 800, 600, 16, "800x600x16" },
        { 0x115, 800, 600, 24, "800x600x24" },
        { 0x105, 1024, 768, 8, "1024x768x8" },
        { 0x117, 1024, 768, 16, "1024x768x16" },
        { 0x118, 1024, 768, 24, "1024x768x24" },
        { 0x107, 1280, 1024, 8, "1280x1024x8" },
        { 0x11A, 1280, 1024, 16, "1280x1024x16" },
        { 0x11B, 1280, 1024, 24, "1280x1024x24" },
        { 0 }
};

vesa_mode_t *m = (vesa_mode_t *)&mode_table[0];

int vesa_init(void)
{
        uint16_t far *mp;
        uint16_t mp_seg, mp_offs;
        uint16_t val;
        regs86_t regs_in, regs_out;
        vesa_buffer_t *vbuf;

        ((vesa_buffer_t *)vesa_buffer)->id[0] = 'V';
        ((vesa_buffer_t *)vesa_buffer)->id[1] = 'B';
        ((vesa_buffer_t *)vesa_buffer)->id[2] = 'E';
        ((vesa_buffer_t *)vesa_buffer)->id[3] = '2';

        vbuf = (vesa_buffer_t *)vesa_buffer;
        regs_in._es = _SEG_ES();
        regs_in._ax = 0x4F00;
        regs_in._di = (uintptr_t)vbuf;
        vid_int86(&regs_in, &regs_out);

        if (regs_out._ax != 0x004F) {
                return 1;
        }

        if (vbuf->id[0] != 'V' ||
            vbuf->id[1] != 'E' ||
            vbuf->id[2] != 'S' ||
            vbuf->id[3] != 'A') {
                return 1;
        }

        dump16(vbuf->mode_pointer_hi);
        putc(':');
        dump16(vbuf->mode_pointer_lo);
        puts("\r\n");

        mp_seg = vbuf->mode_pointer_hi;
        mp_offs = vbuf->mode_pointer_lo;

        mp = (uint16_t far *)MK_FAR(mp_seg, mp_offs);

        val = *mp;
        while (val != 0xFFFF) {
                m = &mode_table[0];
                while (m->mode) {
                        if (m->mode == val) {
                                puts(m->mode_str);
                                puts("\r\n");
                                break;
                        }
                        m++;
                }
                mp++;
                val = *mp;
        }

        return 0;
}


