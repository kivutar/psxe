</$objtype/mkfile

BIN=/$objtype/bin/games
TARG=psxe
CFLAGS=$CFLAGS -I. -Ipsx
# 9front: suppress object type-signature conflicts during link.
LD=6l -S

HFILES=\
	dat.h\
	fns.h\

OFILES=\
	psxe.$O\
	eui.$O\
	bus.$O\
	config.$O\
	cpu.$O\
	exe.$O\
	log.$O\
	psx.$O\
	guncon.$O\
	sda.$O\
	bios.$O\
	dma.$O\
	exp1.$O\
	exp2.$O\
	gpu.$O\
	ic.$O\
	input.$O\
	mc1.$O\
	mc2.$O\
	mc3.$O\
	mcd.$O\
	mdec.$O\
	pad.$O\
	ram.$O\
	scratchpad.$O\
	spu.$O\
	timer.$O\
	xa.$O\
	audio.$O\
	cdrom.$O\
	cue.$O\
	disc.$O\
	impl.$O\
	list.$O\
	queue.$O\

</sys/src/cmd/mkone

%.$O: psx/%.c
	$CC $CFLAGS psx/$stem.c
%.$O: psx/input/%.c
	$CC $CFLAGS psx/input/$stem.c
%.$O: psx/dev/%.c
	$CC $CFLAGS psx/dev/$stem.c
%.$O: psx/dev/cdrom/%.c
	$CC $CFLAGS psx/dev/cdrom/$stem.c
