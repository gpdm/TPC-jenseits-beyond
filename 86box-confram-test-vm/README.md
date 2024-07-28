# 86box Test VM

To facilitate testing of `jenseits` / `beyond`, I've create
this 86box Test VM.

It's the same as featured in my (video)[https://www.youtube.com/watch?v=Nbw5klso-VY] on YouTube, and is based on an *IBM PC AT 1570* base configuration.

I've configured it to have

-  512K base memory on the mainboard
-  448K memory, starting from 512K, via "Generic PC/AT Memory Expansion" #1
- 1024K memory, starting from 1024K, via "PC/AT Memory Expansion" #2

Now, if you do the math, this leaves you with the following memory outline:

 +-------+
 | 1024K | Expansion 2 (from 1024K-2048K)
 +-------+
 |   64K | Memory Hole for System ROM (from 960K-1024K)
 +-------+
 |  448K | Expansion 1 (from 512K-960K)
 +-------+
 |  512K | Base Memory (from 0K-512K)
 +-------+

The memory hole between 960K-1024K for the System ROM (BIOS)
deemed necessary, as without it, the RAM expansion would shadow
the BIOS, and thus the emulated machine would not start.

I don't have suitable real hardware to test, so I can't tell
if the same effect happens on a real 5170.

At least in this configuration it became possible to successfully
demo the `jenseits` / `beyond` utilities.

For that purposes, two floppy disk images are provided:

* `cftest360-dos33.ima`: PC-DOS 3.3
* `cftest360-dos5.ima`: PC-DOS 5.02

Both disk are mostly identical and ready to run, booting directly
into DOS with

* `confram` enabled (maxed out to 960K, with video regions excluded)
* loading `mouse.com` using the `beyond` utility into the segments above 640K

The PC DOS 5 version additionally loads `HIMEM.SYS` to move the DOS kernel
into the UMA, freeing up some resources in the segment below 640K.

