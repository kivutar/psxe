#include "dat.h"
#include "fns.h"

Emu emu;

static u32int
bgr555toxrgb32(u16int c)
{
	u32int r, g, b;

	r = (c >> 0) & 0x1f;
	g = (c >> 5) & 0x1f;
	b = (c >> 10) & 0x1f;

	r = (r << 3) | (r >> 2);
	g = (g << 3) | (g >> 2);
	b = (b << 3) | (b >> 2);

	return 0xff000000 | (r << 16) | (g << 8) | b;
}

static void
button_edge(u64int now, u64int bit, u32int mask)
{
	if(((now ^ emu.prevkeys) & bit) == 0)
		return;

	if((now & bit) != 0)
		psx_pad_button_press(emu.pad, 0, mask);
	else
		psx_pad_button_release(emu.pad, 0, mask);
}

void
process_inputs(void)
{
	u64int now;

	now = keys;

	button_edge(now, KeyCross, PSXI_SW_SDA_CROSS);
	button_edge(now, KeySquare, PSXI_SW_SDA_SQUARE);
	button_edge(now, KeyTriangle, PSXI_SW_SDA_TRIANGLE);
	button_edge(now, KeyCircle, PSXI_SW_SDA_CIRCLE);
	button_edge(now, KeyStart, PSXI_SW_SDA_START);
	button_edge(now, KeySelect, PSXI_SW_SDA_SELECT);
	button_edge(now, KeyUp, PSXI_SW_SDA_PAD_UP);
	button_edge(now, KeyDown, PSXI_SW_SDA_PAD_DOWN);
	button_edge(now, KeyLeft, PSXI_SW_SDA_PAD_LEFT);
	button_edge(now, KeyRight, PSXI_SW_SDA_PAD_RIGHT);
	button_edge(now, KeyL1, PSXI_SW_SDA_L1);
	button_edge(now, KeyR1, PSXI_SW_SDA_R1);
	button_edge(now, KeyL2, PSXI_SW_SDA_L2);
	button_edge(now, KeyR2, PSXI_SW_SDA_R2);
	button_edge(now, KeyL3, PSXI_SW_SDA_L3);
	button_edge(now, KeyR3, PSXI_SW_SDA_R3);
	button_edge(now, KeyAnalog, PSXI_SW_SDA_ANALOG);

	emu.prevkeys = now;
}

void
bindkeys(void)
{
	regkey("cross", 'x', KeyCross);
	regkey("square", 'z', KeySquare);
	regkey("triangle", 's', KeyTriangle);
	regkey("circle", 'd', KeyCircle);
	regkey("start", '\n', KeyStart);
	regkey("select", Kshift, KeySelect);
	regkey("up", Kup, KeyUp);
	regkey("down", Kdown, KeyDown);
	regkey("left", Kleft, KeyLeft);
	regkey("right", Kright, KeyRight);
	regkey("l1", 'q', KeyL1);
	regkey("r1", 'w', KeyR1);
	regkey("l2", '1', KeyL2);
	regkey("r2", '3', KeyR2);
	regkey("l3", 'a', KeyL3);
	regkey("r3", 'f', KeyR3);
	regkey("analog", '2', KeyAnalog);
}

void
blitframe(void)
{
	u32int *dst, *drow;
	u16int *s16, *row16;
	uchar *s8, *row8;
	void *src;
	int fmt, sw, sh, dx, dy, x, y;

	if(emu.psx == nil)
		return;

	src = psx_get_display_buffer(emu.psx);
	fmt = psx_get_display_format(emu.psx);
	sw = psx_get_display_width(emu.psx);
	sh = psx_get_display_height(emu.psx);

	if(emu.psx->gpu->disp_y + sh > PSX_GPU_FB_HEIGHT)
		src = psx_get_vram(emu.psx);

	if(sw <= 0 || sh <= 0)
		return;

	if(sw > Vwdx)
		sw = Vwdx;
	if(sh > Vwdy)
		sh = Vwdy;

	dx = (Vwdx - sw) / 2;
	dy = (Vwdy - sh) / 2;

	memset(pic, 0, Vwdx * Vwdy * 4);

	dst = (u32int*)pic;

	if(fmt != 0){
		s8 = (uchar*)src;
		for(y = 0; y < sh; y++){
			row8 = s8 + (y * PSX_GPU_FB_STRIDE);
			drow = dst + ((dy + y) * Vwdx) + dx;
			for(x = 0; x < sw; x++){
				u32int b, g, r;

				b = row8[(x * 3) + 0];
				g = row8[(x * 3) + 1];
				r = row8[(x * 3) + 2];

				drow[x] = 0xff000000 | (r << 16) | (g << 8) | b;
			}
		}
		return;
	}

	s16 = (u16int*)src;
	for(y = 0; y < sh; y++){
		row16 = s16 + (y * PSX_GPU_FB_WIDTH);
		drow = dst + ((dy + y) * Vwdx) + dx;
		for(x = 0; x < sw; x++)
			drow[x] = bgr555toxrgb32(row16[x]);
	}
}

int
audioout(void)
{
	return -1;
}

void
flush(void)
{
	flushmouse(1);
	flushscreen();
	flushaudio(audioout);
}

void
psxe_gpu_vblank_event_cb(psx_gpu_t *gpu)
{
	blitframe();
	flush();
	psxe_gpu_vblank_timer_event_cb(gpu);
}

void
usage(void)
{
	fprint(2, "usage: %s [-x scale] [-e expansion] bios [disc]\n", argv0);
	exits("usage");
}

void
threadmain(int argc, char **argv)
{
	char *bios, *disc, *exp;
	psx_input_t *input;
	psxi_sda_t *controller;
	psx_gpu_t *gpu;
	int r;

	log_set_quiet(0);
	log_set_level(LOG_WARN);

	disc = nil;
	exp = nil;

	ARGBEGIN{
	case 'x':
		fixscale = strtol(EARGF(usage()), nil, 0);
		break;
	case 'e':
		exp = EARGF(usage());
		break;
	default:
		usage();
	}ARGEND

	if(argc < 1 || argc > 2)
		usage();

	bios = argv[0];
	if(argc == 2)
		disc = argv[1];

	memset(&emu, 0, sizeof emu);

	emu.psx = psx_create();
	if(emu.psx == nil)
		sysfatal("psx_create: %r");

	r = psx_init(emu.psx, bios, exp);
	if(r != 0)
		sysfatal("psx_init failed: %d", r);

	emu.pad = psx_get_pad(emu.psx);

	if(disc != nil && !psx_cdrom_open(psx_get_cdrom(emu.psx), disc))
		fprint(2, "warning: cannot open disc %s\n", disc);

	input = psx_input_create();
	psx_input_init(input);

	controller = psxi_sda_create();
	psxi_sda_init(controller, SDA_MODEL_DIGITAL);
	psxi_sda_init_input(controller, input);

	psx_pad_attach_joy(emu.pad, 0, input);
	psx_pad_attach_mcd(emu.pad, 0, "slot1.mcd");
	psx_pad_attach_mcd(emu.pad, 1, "slot2.mcd");

	gpu = psx_get_gpu(emu.psx);

	psx_gpu_set_event_callback(gpu, GPU_EVENT_VBLANK, psxe_gpu_vblank_event_cb);
	psx_gpu_set_event_callback(gpu, GPU_EVENT_HBLANK, psxe_gpu_hblank_event_cb);
	psx_gpu_set_event_callback(gpu, GPU_EVENT_VBLANK_END, psxe_gpu_vblank_end_event_cb);
	psx_gpu_set_event_callback(gpu, GPU_EVENT_HBLANK_END, psxe_gpu_hblank_end_event_cb);

	psx_gpu_set_udata(gpu, 1, psx_get_timer(emu.psx));

	initemu(Vwdx, Vwdy, 4, XRGB32, 1, nil);
	bindkeys();

	for(;;){
		if(paused){
			qlock(&pauselock);
			qunlock(&pauselock);
		}
		process_inputs();
		psx_update(emu.psx);
	}
}
