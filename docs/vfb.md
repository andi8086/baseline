# Virtual frame buffer

The graphics memory is mapped onto a linear virtual frame buffer,
which is 5 MB in size for 1280x1024x32.

The `vcon` virtual console is the best example where we need a
scrollable sub-area of this linear frame buffer.

We have a separate memory area to store the contents of the virtual console,
called the `console buffer`. This console buffer's width is a multiple
of one character's width, i.e. 8 pixels for a font size of 8x16.

One important property of the buffer is the `pitch`, i.e. how many bytes
does one line of the virtual screen, that is represented by the console buffer
have.

For the linear frame buffer, the pitch is provided by a multiboot2 information
tag, together with the frame buffer. In our case it might be 5120 bytes, i.e.
if we want to go from one line to the next, we have to add 5120 to the
byte offset or 1280 to the DWORD offset of the pixel.

To display one scan line of the virtual console's buffer, we start at the
top left coordinate of the virtual screen on the linear frame buffer.


```
                ^
                | y0
                v
<-------------->,------------------------------,
      x0        |           ^                  |
                |        ym |                  |
                .    xm     v  wm              .
                .<--------->,-------,          .
                |           |_______| hm       |
                '------------------------------'
```

Following algorithm is used:

1. calculate the offset in the linear frame buffer of x0,y0, which is
   `*((uint32_t *)x0 + y0 * pitch/4)`
2. draw one scan line of the virtual screen
3. now we have incremented x0 by vpitch/4, with vpitch < pitch.
   We simpliy add (pitch-vpitch) and land at (x0,y0+1) to draw
   the next virtual scan line.

If we have to scroll the virtual screen, we just scroll the virtual
buffer and then redraw it onto the linear frame buffer.

If we just update a small portion of the virtual screen, we only replace
the corresponding contents on the linear frame buffer.

So if we modify `(xm,ym)-(xm+wm,ym+hm)` inside the virtual screen,
we do:

1. calculate the offset in the linear frame buffer of x0,y0, which is
    `*((uint32_t *)x0 + xm + y0 * pitch/4)`
2. copy `wm` DWORDs from virtual buffer to linear buffer
3. increment by `vpitch/4 - wm` in the virtual buffer and
   increment by `pitch/4 - vpitch/4 - wm` in the linear buffer
4. loop back to 2. for `hm` times.




