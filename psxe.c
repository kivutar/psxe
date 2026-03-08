#include "dat.h"
#include "fns.h"

Emu emu;
static int audfd = -1;
static s16int audbuf[735 * 2];
static int needflush;

static int
fileexists(char *path)
{
	int fd;

	fd = open(path, OREAD);
	if(fd < 0)
		return 0;
	close(fd);
	return 1;
}

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
	u32int *dst, *drow, *drow2;
	u16int *s16, *row16;
	uchar *s8, *row8;
	void *src;
	int fmt, sw, sh, src_w, src_h, dw, dh, scale2x, dx, dy, x, y;

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

	/* Scale 240-line modes by 2x (nearest-neighbor) to avoid size jumps. */
	scale2x = (sh == 240);
	src_w = sw;
	src_h = sh;
	dw = sw;
	dh = sh;

	if(scale2x){
		dw = sw * 2;
		dh = sh * 2;
	}

	if(dw > Vwdx)
		dw = Vwdx;
	if(dh > Vwdy)
		dh = Vwdy;

	if(scale2x){
		dw &= ~1;
		dh &= ~1;
		src_w = dw / 2;
		src_h = dh / 2;
	}else{
		src_w = dw;
		src_h = dh;
	}

	if(src_w <= 0 || src_h <= 0)
		return;

	dx = (Vwdx - dw) / 2;
	dy = (Vwdy - dh) / 2;

	memset(pic, 0, Vwdx * Vwdy * 4);

	dst = (u32int*)pic;

	if(fmt != 0){
		s8 = (uchar*)src;
		for(y = 0; y < src_h; y++){
			row8 = s8 + (y * PSX_GPU_FB_STRIDE);
			if(scale2x){
				drow = dst + ((dy + (y * 2)) * Vwdx) + dx;
				drow2 = drow + Vwdx;
			}else{
				drow = dst + ((dy + y) * Vwdx) + dx;
				drow2 = nil;
			}
			for(x = 0; x < src_w; x++){
				u32int c, b, g, r;

				b = row8[(x * 3) + 0];
				g = row8[(x * 3) + 1];
				r = row8[(x * 3) + 2];

				c = 0xff000000 | (r << 16) | (g << 8) | b;
				if(scale2x){
					drow[(x * 2) + 0] = c;
					drow[(x * 2) + 1] = c;
					drow2[(x * 2) + 0] = c;
					drow2[(x * 2) + 1] = c;
				}else{
					drow[x] = c;
				}
			}
		}
		return;
	}

	s16 = (u16int*)src;
	for(y = 0; y < src_h; y++){
		row16 = s16 + (y * PSX_GPU_FB_WIDTH);
		if(scale2x){
			drow = dst + ((dy + (y * 2)) * Vwdx) + dx;
			drow2 = drow + Vwdx;
		}else{
			drow = dst + ((dy + y) * Vwdx) + dx;
			drow2 = nil;
		}
		for(x = 0; x < src_w; x++){
			u32int c;

			c = bgr555toxrgb32(row16[x]);
			if(scale2x){
				drow[(x * 2) + 0] = c;
				drow[(x * 2) + 1] = c;
				drow2[(x * 2) + 0] = c;
				drow2[(x * 2) + 1] = c;
			}else{
				drow[x] = c;
			}
		}
	}
}

int
audioout(void)
{
	psx_cdrom_t *cdrom;
	psx_spu_t *spu;
	int i;
	int nsamp;

	if(emu.psx == nil)
		return -1;
	if(audfd < 0){
		audfd = open("/dev/audio", OWRITE);
		if(audfd < 0)
			return -1;
	}

	cdrom = psx_get_cdrom(emu.psx);
	spu = psx_get_spu(emu.psx);
	nsamp = nelem(audbuf) / 2;

	memset(audbuf, 0, sizeof audbuf);
	psx_cdrom_get_audio_samples(cdrom, audbuf, sizeof audbuf);
	psx_spu_update_cdda_buffer(spu, cdrom->cdda_buf);

	for(i = 0; i < nsamp; i++){
		u32int sample;
		int l, r;

		sample = psx_spu_get_sample(spu);

		l = audbuf[i * 2 + 0] + (s16int)(sample & 0xffff);
		r = audbuf[i * 2 + 1] + (s16int)(sample >> 16);

		if(l > 32767)
			l = 32767;
		else if(l < -32768)
			l = -32768;
		if(r > 32767)
			r = 32767;
		else if(r < -32768)
			r = -32768;

		audbuf[i * 2 + 0] = l;
		audbuf[i * 2 + 1] = r;
	}

	if(warp10)
		return 0;

	if(write(audfd, audbuf, sizeof audbuf) < 0)
		return -1;

	return 0;
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
	USED(gpu);
	needflush = 1;
	psxe_gpu_vblank_timer_event_cb(gpu);
}

void
usage(void)
{
	print("usage: %s [-x scale] [-e expansion] bios [disc]\n", argv0);
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
		print("warning: cannot open disc %s\n", disc);

	input = psx_input_create();
	psx_input_init(input);

	controller = psxi_sda_create();
	psxi_sda_init(controller, SDA_MODEL_DIGITAL);
	psxi_sda_init_input(controller, input);

	psx_pad_attach_joy(emu.pad, 0, input);
	if(fileexists("slot1.mcd"))
		psx_pad_attach_mcd(emu.pad, 0, "slot1.mcd");
	else
		print("warning: slot1.mcd not found, memory card 1 disabled\n");
	if(fileexists("slot2.mcd"))
		psx_pad_attach_mcd(emu.pad, 1, "slot2.mcd");
	else
		print("warning: slot2.mcd not found, memory card 2 disabled\n");

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
		if(needflush){
			needflush = 0;
			blitframe();
			flush();
		}
	}
}
