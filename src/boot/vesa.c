#include "vesa.h"
#include "conio.h"
#include "int86.h"

#include "mem.h"

/*
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
*/

#define DIFF(a, b) ((a) >= (b) ? (a) - (b) : ((b) - (a)));


uint16_t vesa_find_mode(int16_t x, int16_t y, uint8_t bpp,
                        uint16_t *fblo, uint16_t *fbhi)
{
        vesa_buffer_t *ctrl = (vesa_buffer_t *)0xC000;
        vbe_mode_info_t *inf = (vbe_mode_info_t *)0xD000;
        regs86_t rin, rout;
        uint16_t fb0, fb1;

        uint16_t far *modes;
        int i;
        uint16_t best = 0x13;
        uint32_t pixdiff, bestpixdiff = DIFF(320 * 200, x * y);
        uint32_t depthdiff, bestdepthdiff =
                8 >= bpp ? 8 - bpp : (bpp - 8) * 2;
        fb0 = 0;
        fb1 = 0;
        *fblo = 0;
        *fbhi = 0;
        strncpy((void far *)ctrl->id, (void far *)"VBE2", 4);
        rin._es = _SEG_DS();
        rin._ax = 0x4F00;
        rin._di = (uint16_t)ctrl;
        vid_int86(&rin, &rout);

        if (rout._ax != 0x004F) {
                return best;
        }

        modes = (uint16_t far *)
                MK_FAR(ctrl->mode_pointer_hi, ctrl->mode_pointer_lo);

        for (i = 0; modes[i] != 0xFFFF; i++) {
                rin._es = _SEG_DS();
                rin._cx = modes[i];
                rin._di = (uint16_t)inf;
                rin._ax = 0x4F01;
                vid_int86(&rin, &rout);
                if (rout._ax != 0x004F) {
                        continue;
                }

                /* check if this is a graphics mode with linear
                   frame buffer */
                if ((inf->attributes & 0x80) != 0x80) {
                        continue;
                }

                /* check if packed pixel or direct color mode */
                if (inf->memory_model != 4 && inf->memory_model != 6) {
                        continue;
                }

                /* check fo exact match */
                if (x == inf->width && y == inf->height &&
                    bpp == inf->bpp) {
                        *fblo = inf->framebuffer[0];
                        *fbhi = inf->framebuffer[1];
                        return modes[i];
                }

                /* compare to closest match and remember */
                pixdiff = DIFF(inf->width * inf->height, x * y);
                depthdiff = (inf->bpp >= bpp) ?
                            inf->bpp - bpp : (bpp - inf->bpp) * 2;
                if (bestpixdiff > pixdiff ||
                    (bestpixdiff == pixdiff && bestdepthdiff > depthdiff)) {
                        best = modes[i];
                        bestpixdiff = pixdiff;
                        bestdepthdiff = depthdiff;
                        fb0 = inf->framebuffer[0];
                        fb1 = inf->framebuffer[1];
                }
        }
        if (x == 640 && y == 480 && bpp == 1) return 0x11;

        puts("Linear frame buffer at ");
        *fblo = fb0;
        *fbhi = fb1;
        return best;
}


int vesa_mode_set(uint16_t mode)
{
        regs86_t rin, rout;
        rin._ax = 0x4F02;
        rin._bx = mode | VESA_REQUEST_LFB;
        vid_int86(&rin, &rout);
        if (rout._ax != 0x004F) {
                return 1;
        }
        return 0;
}
