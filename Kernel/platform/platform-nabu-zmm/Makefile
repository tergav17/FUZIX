CROSS_CCOPTS += -I../../dev/ -I../../dev/net/

CSRCS = devtty.c
CSRCS += devices.c main.c wiznet.c

DISCSRCS = discard.c

ASRCS = sbcv2.S crt0.S
ASRCS += tricks.S commonmem.S

DISCARD_DSRCS = ../../dev/tinydisk_discard.c ../../dev/tinyide_discard.c
DISCARD_DSRCS += ../../dev/ds1302_discard.c
DSRCS = ../../dev/tinydisk.c ../../dev/tinyide.c ../../dev/mbr.c
DSRCS += ../../dev/propio2.c ../../dev/ds1302.c
DSRCS += ../../dev/devfd.c
NSRCS = ../../dev/net/net_w5x00.c ../../dev/net/net_native.c

DASRCS = ../../dev/rbcfd9266_hw.S ../../dev/ds1302_rbc.S ../../dev/tinyide_ppide_rbc.S

COBJS = $(CSRCS:.c=.o)
AOBJS = $(ASRCS:.S=.o)
NOBJS = $(patsubst ../../dev/net/%.c,%.o, $(NSRCS))
DISCOBJS = $(DISCSRCS:.c=.o)
DISCARD_DOBJS = $(patsubst ../../dev/%.c,%.o, $(DISCARD_DSRCS))
DOBJS = $(patsubst ../../dev/%.c,%.o, $(DSRCS))
DAOBJS = $(patsubst ../../dev/%.S,%.o, $(DASRCS))

OBJS  = $(COBJS) $(AOBJS) $(NOBJS) $(DISCOBJS) $(DOBJS) $(DISCARD_DOBJS) $(DAOBJS)

JUNK =  *.lst *.asm *.Sym *.rst *.lst

all:	$(OBJS)

$(COBJS): %.o: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DISCOBJS): %.o: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(DOBJS): %.o: ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DISCARD_DOBJS): %.o: ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(NOBJS): %.o: ../../dev/net/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(AOBJS): %.o: %.S
	$(CROSS_AS) $(ASOPTS) $<

$(DAOBJS): %.o: ../../dev/%.S
	$(CROSS_AS) $(ASOPTS) $<

clean:
	rm -f $(OBJS) $(JUNK)  core *~

#
#	Attach the kernel to the boot block
#
image:
	$(CROSS_LD) -b -C 0x0100 -S 0xF400 -f CLDBbXSs -o fuzix.bin \
	crt0.o commonmem.o sbcv2.o ../../start.o \
	../../version.o ../../cpu-z80u/lowlevel-z80u.o \
	../../usermem.o tricks.o main.o discard.o \
	../../timer.o ../../kdata.o devices.o ../../devio.o \
	../../filesys.o ../../blk512.o ../../process.o \
	../../inode.o ../../syscall_exec.o ../../syscall_exec16.o \
	../../syscall_fs.o ../../syscall_fs2.o ../../syscall_fs3.o \
	../../syscall_proc.o ../../syscall_other.o ../../syscall_net.o \
	../../network.o ../../tty.o ../../mm.o ../../mm/memalloc_none.o \
	../../swap.o ../../mm/bankfixed.o ../../devsys.o \
	devtty.o ../../dev/tinydisk.o ../../dev/tinydisk_discard.o \
	../../dev/tinyide.o ../../dev/tinyide_discard.o \
	../../dev/propio2.o ../../dev/rbcfd9266_hw.o \
	../../dev/net/net_native.o ../../dev/net/net_w5x00.o wiznet.o \
	../../dev/ds1302_rbc.o ../../dev/ds1302.o ../../dev/ds1302_discard.o \
	../../dev/tinyide_ppide_rbc.o ../../dev/devfd.o \
	/opt/fcc/lib/z80/libz80.a -m fuzix.tmpmap
	perl -lpe '$$_=hex' fuzix.tmpmap | paste -d" " - fuzix.tmpmap  |  sort -n |  cut -d" " -f 2- >../../fuzix.map
	../../tools/pack85 <../../fuzix.map fuzix.bin ../../fuzix.bin
	cp boot-romwbw.bin fuzix.romwbw
	dd if=../../fuzix.bin bs=256 skip=1 seek=4 of=fuzix.romwbw
#
#	Compile up the boot block
#
bootblock:
	asz80 boot-romwbw.S
	ldz80 -b boot-romwbw.o -o boot-romwbw.tmp
	# Chop off the leading 61440 bytes we don't want
	dd if=boot-romwbw.tmp of=boot-romwbw.bin bs=512 count=2 skip=120

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage:
	# Make a blank disk image with partition
	dd if=$(FUZIX_ROOT)/Standalone/filesystem-src/parttab.20M of=$(IMAGES)/disk.img bs=20152320 conv=sync
	# Add the file system
	dd if=$(IMAGES)/filesys8.img of=$(IMAGES)/disk.img bs=512 seek=2048 conv=notrunc
	# Add the kernel
	dd if=fuzix.romwbw of=$(IMAGES)/disk.img bs=512 seek=2 conv=notrunc
	# Make an emulator image of it
	cat $(FUZIX_ROOT)/Standalone/filesystem-src/idehdr.20M $(IMAGES)/disk.img > $(IMAGES)/emu-ide.img
