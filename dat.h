#ifndef DAT_H
#define DAT_H

#include "psx/p9.h"
#include <draw.h>
#include <thread.h>
#include <keyboard.h>

#include "eui.h"
#include "psx/log.h"
#include "psx/psx.h"
#include "psx/dev/cdrom/cdrom.h"
#include "psx/dev/gpu.h"
#include "psx/dev/pad.h"
#include "psx/dev/timer.h"
#include "psx/input/sda.h"

typedef struct Emu Emu;

struct Emu {
	psx_t *psx;
	psx_pad_t *pad;
	u64int prevkeys;
};

enum {
	Vwdx = 640,
	Vwdy = 480,
};

enum {
	KeyCross = 1 << 0,
	KeySquare = 1 << 1,
	KeyTriangle = 1 << 2,
	KeyCircle = 1 << 3,
	KeyStart = 1 << 4,
	KeySelect = 1 << 5,
	KeyUp = 1 << 6,
	KeyDown = 1 << 7,
	KeyLeft = 1 << 8,
	KeyRight = 1 << 9,
	KeyL1 = 1 << 10,
	KeyR1 = 1 << 11,
	KeyL2 = 1 << 12,
	KeyR2 = 1 << 13,
	KeyL3 = 1 << 14,
	KeyR3 = 1 << 15,
	KeyAnalog = 1 << 16,
};

extern Emu emu;

/* Frontend glue uses these directly. Keep explicit declarations local to psxe UI. */
void psx_pad_button_press(psx_pad_t*, int, uint32_t);
void psx_pad_button_release(psx_pad_t*, int, uint32_t);
void psx_pad_attach_joy(psx_pad_t*, int, psx_input_t*);
int psx_pad_attach_mcd(psx_pad_t*, int, const char*);
psx_spu_t* psx_spu_create(void);
void psx_spu_init(psx_spu_t*, psx_ic_t*);
void psx_spu_destroy(psx_spu_t*);

#endif
