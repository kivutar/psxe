</$objtype/mkfile

# Core-only build for 9front/native ports.
# Excludes frontend/ (SDL UI/audio loop) and builds reusable emulator core.
LIB=libpsxe_core.a
CFLAGS=$CFLAGS -I. -Ipsx

HFILES=\
	psx/bus.h\
	psx/bus_init.h\
	psx/config.h\
	psx/cpu.h\
	psx/cpu_debug.h\
	psx/exe.h\
	psx/log.h\
	psx/psx.h\
	psx/input/guncon.h\
	psx/input/sda.h\
	psx/dev/bios.h\
	psx/dev/dma.h\
	psx/dev/exp1.h\
	psx/dev/exp2.h\
	psx/dev/gpu.h\
	psx/dev/ic.h\
	psx/dev/input.h\
	psx/dev/mc1.h\
	psx/dev/mc2.h\
	psx/dev/mc3.h\
	psx/dev/mcd.h\
	psx/dev/mdec.h\
	psx/dev/pad.h\
	psx/dev/ram.h\
	psx/dev/scratchpad.h\
	psx/dev/spu.h\
	psx/dev/timer.h\
	psx/dev/xa.h\
	psx/dev/cdrom/cdrom.h\
	psx/dev/cdrom/cue.h\
	psx/dev/cdrom/disc.h\
	psx/dev/cdrom/list.h\
	psx/dev/cdrom/queue.h\

OFILES=\
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

default:V: $LIB

%.$O: psx/%.c
	$CC $CFLAGS -o $target $prereq
%.$O: psx/input/%.c
	$CC $CFLAGS -o $target $prereq
%.$O: psx/dev/%.c
	$CC $CFLAGS -o $target $prereq
%.$O: psx/dev/cdrom/%.c
	$CC $CFLAGS -o $target $prereq

</sys/src/cmd/mksyslib
