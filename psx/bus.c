#include "p9.h"

#include "bus.h"
#include "bus_init.h"
#include "dev/cdrom/cdrom.h"
#include "dev/bios.h"
#include "dev/ram.h"
#include "dev/dma.h"
#include "dev/exp1.h"
#include "dev/exp2.h"
#include "dev/mc1.h"
#include "dev/mc2.h"
#include "dev/mc3.h"
#include "dev/ic.h"
#include "dev/scratchpad.h"
#include "dev/gpu.h"
#include "log.h"

/*
 * 6c struggles with the full SPU header in this TU; bus only needs
 * the MMIO prefix fields and read/write entry points.
 */
struct psx_spu_t {
    uint32_t bus_delay;
    uint32_t io_base, io_size;
};
uint32_t psx_spu_read32(struct psx_spu_t*, uint32_t);
uint16_t psx_spu_read16(struct psx_spu_t*, uint32_t);
uint8_t psx_spu_read8(struct psx_spu_t*, uint32_t);
void psx_spu_write32(struct psx_spu_t*, uint32_t, uint32_t);
void psx_spu_write16(struct psx_spu_t*, uint32_t, uint16_t);
void psx_spu_write8(struct psx_spu_t*, uint32_t, uint8_t);

struct psx_timer_t {
    uint32_t bus_delay;
    uint32_t io_base, io_size;
};
uint32_t psx_timer_read32(struct psx_timer_t*, uint32_t);
uint16_t psx_timer_read16(struct psx_timer_t*, uint32_t);
uint8_t psx_timer_read8(struct psx_timer_t*, uint32_t);
void psx_timer_write32(struct psx_timer_t*, uint32_t, uint32_t);
void psx_timer_write16(struct psx_timer_t*, uint32_t, uint16_t);
void psx_timer_write8(struct psx_timer_t*, uint32_t, uint8_t);

struct psx_pad_t {
    uint32_t bus_delay;
    uint32_t io_base, io_size;
};
uint32_t psx_pad_read32(struct psx_pad_t*, uint32_t);
uint16_t psx_pad_read16(struct psx_pad_t*, uint32_t);
uint8_t psx_pad_read8(struct psx_pad_t*, uint32_t);
void psx_pad_write32(struct psx_pad_t*, uint32_t, uint32_t);
void psx_pad_write16(struct psx_pad_t*, uint32_t, uint16_t);
void psx_pad_write8(struct psx_pad_t*, uint32_t, uint8_t);

struct psx_mdec_t {
    uint32_t bus_delay;
    uint32_t io_base, io_size;
};
uint32_t psx_mdec_read32(struct psx_mdec_t*, uint32_t);
uint16_t psx_mdec_read16(struct psx_mdec_t*, uint32_t);
uint8_t psx_mdec_read8(struct psx_mdec_t*, uint32_t);
void psx_mdec_write32(struct psx_mdec_t*, uint32_t, uint32_t);
void psx_mdec_write16(struct psx_mdec_t*, uint32_t, uint16_t);
void psx_mdec_write8(struct psx_mdec_t*, uint32_t, uint8_t);

#define RANGE(v, s, e) ((v >= s) && (v < e))

const uint32_t g_psx_bus_region_mask_table[] = {
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0x7fffffff, 0x1fffffff, 0xffffffff, 0xffffffff
};

psx_bus_t* psx_bus_create(void) {
    return (psx_bus_t*)malloc(sizeof(psx_bus_t));
}

// Does nothing for now
void psx_bus_init(psx_bus_t* bus) {
    USED(bus);
}

void psx_bus_destroy(psx_bus_t* bus) {
    free(bus);
}

#define HANDLE_READ_OP(dev, fn) \
    if (RANGE(addr, bus->dev->io_base, (bus->dev->io_base + bus->dev->io_size))) { \
        bus->access_cycles = bus->dev->bus_delay; \
        return fn(bus->dev, addr - bus->dev->io_base); \
    }
#define HANDLE_WRITE_OP(dev, fn) \
    if (RANGE(addr, bus->dev->io_base, (bus->dev->io_base + bus->dev->io_size))) { \
        bus->access_cycles = bus->dev->bus_delay; \
        fn(bus->dev, addr - bus->dev->io_base, value); \
        return; \
    }

uint32_t psx_bus_read32(psx_bus_t* bus, uint32_t addr) {
    uint32_t vaddr = addr;

    addr &= g_psx_bus_region_mask_table[addr >> 29];

    if (addr & 0x3) {
        log_fatal("Unaligned 32-bit read from %08x:%08x", vaddr, addr);
    }

    HANDLE_READ_OP(bios, psx_bios_read32);
    HANDLE_READ_OP(ram, psx_ram_read32);
    HANDLE_READ_OP(dma, psx_dma_read32);
    HANDLE_READ_OP(exp1, psx_exp1_read32);
    HANDLE_READ_OP(exp2, psx_exp2_read32);
    HANDLE_READ_OP(mc1, psx_mc1_read32);
    HANDLE_READ_OP(mc2, psx_mc2_read32);
    HANDLE_READ_OP(mc3, psx_mc3_read32);
    HANDLE_READ_OP(ic, psx_ic_read32);
    HANDLE_READ_OP(scratchpad, psx_scratchpad_read32);
    HANDLE_READ_OP(gpu, psx_gpu_read32);
    HANDLE_READ_OP(spu, psx_spu_read32);
    HANDLE_READ_OP(timer, psx_timer_read32);
    HANDLE_READ_OP(cdrom, psx_cdrom_read32);
    HANDLE_READ_OP(pad, psx_pad_read32);
    HANDLE_READ_OP(mdec, psx_mdec_read32);

    log_fatal("Unhandled 32-bit read from %08x:%08x", vaddr, addr);

    //exit(1);

    return 0x00000000;
}

static uint16_t sio_ctrl;

uint16_t psx_bus_read16(psx_bus_t* bus, uint32_t addr) {
    bus->access_cycles = 2;

    uint32_t vaddr = addr;

    addr &= g_psx_bus_region_mask_table[addr >> 29];

    if (addr & 0x1) {
        log_fatal("Unaligned 16-bit read from %08x:%08x", vaddr, addr);
    }

    HANDLE_READ_OP(bios, psx_bios_read16);
    HANDLE_READ_OP(ram, psx_ram_read16);
    HANDLE_READ_OP(dma, psx_dma_read16);
    HANDLE_READ_OP(exp1, psx_exp1_read16);
    HANDLE_READ_OP(exp2, psx_exp2_read16);
    HANDLE_READ_OP(mc1, psx_mc1_read16);
    HANDLE_READ_OP(mc2, psx_mc2_read16);
    HANDLE_READ_OP(mc3, psx_mc3_read16);
    HANDLE_READ_OP(ic, psx_ic_read16);
    HANDLE_READ_OP(scratchpad, psx_scratchpad_read16);
    HANDLE_READ_OP(gpu, psx_gpu_read16);
    HANDLE_READ_OP(spu, psx_spu_read16);
    HANDLE_READ_OP(timer, psx_timer_read16);
    HANDLE_READ_OP(cdrom, psx_cdrom_read16);
    HANDLE_READ_OP(pad, psx_pad_read16);
    HANDLE_READ_OP(mdec, psx_mdec_read16);

    if (addr == 0x1f80105a)
        return sio_ctrl;

    if (addr == 0x1f801054)
        return 0x05;

    if (addr == 0x1f400004)
        return 0xc8;

    if (addr == 0x1f400006)
        return 0x1fe0;

    print("Unhandled 16-bit read from %08x:%08x\n", vaddr, addr);

    // exit(1);

    return 0x0000;
}

uint8_t psx_bus_read8(psx_bus_t* bus, uint32_t addr) {
    bus->access_cycles = 2;

    // uint32_t vaddr = addr;

    addr &= g_psx_bus_region_mask_table[addr >> 29];

    HANDLE_READ_OP(bios, psx_bios_read8);
    HANDLE_READ_OP(ram, psx_ram_read8);
    HANDLE_READ_OP(dma, psx_dma_read8);
    HANDLE_READ_OP(exp1, psx_exp1_read8);
    HANDLE_READ_OP(exp2, psx_exp2_read8);
    HANDLE_READ_OP(mc1, psx_mc1_read8);
    HANDLE_READ_OP(mc2, psx_mc2_read8);
    HANDLE_READ_OP(mc3, psx_mc3_read8);
    HANDLE_READ_OP(ic, psx_ic_read8);
    HANDLE_READ_OP(scratchpad, psx_scratchpad_read8);
    HANDLE_READ_OP(gpu, psx_gpu_read8);
    HANDLE_READ_OP(spu, psx_spu_read8);
    HANDLE_READ_OP(timer, psx_timer_read8);
    HANDLE_READ_OP(cdrom, psx_cdrom_read8);
    HANDLE_READ_OP(pad, psx_pad_read8);
    HANDLE_READ_OP(mdec, psx_mdec_read8);

    // printf("Unhandled 8-bit read from %08x:%08x\n", vaddr, addr);

    //exit(1);

    return 0x00;
}

void psx_bus_write32(psx_bus_t* bus, uint32_t addr, uint32_t value) {
    bus->access_cycles = 0;

    uint32_t vaddr = addr;

    addr &= g_psx_bus_region_mask_table[addr >> 29];

    if (addr & 0x3) {
        log_fatal("Unaligned 32-bit write to %08x:%08x (%08x)", vaddr, addr, value);
    }

    HANDLE_WRITE_OP(bios, psx_bios_write32);
    HANDLE_WRITE_OP(ram, psx_ram_write32);
    HANDLE_WRITE_OP(dma, psx_dma_write32);
    HANDLE_WRITE_OP(exp1, psx_exp1_write32);
    HANDLE_WRITE_OP(exp2, psx_exp2_write32);
    HANDLE_WRITE_OP(mc1, psx_mc1_write32);
    HANDLE_WRITE_OP(mc2, psx_mc2_write32);
    HANDLE_WRITE_OP(mc3, psx_mc3_write32);
    HANDLE_WRITE_OP(ic, psx_ic_write32);
    HANDLE_WRITE_OP(scratchpad, psx_scratchpad_write32);
    HANDLE_WRITE_OP(gpu, psx_gpu_write32);
    HANDLE_WRITE_OP(spu, psx_spu_write32);
    HANDLE_WRITE_OP(timer, psx_timer_write32);
    HANDLE_WRITE_OP(cdrom, psx_cdrom_write32);
    HANDLE_WRITE_OP(pad, psx_pad_write32);
    HANDLE_WRITE_OP(mdec, psx_mdec_write32);

    print("Unhandled 32-bit write to %08x:%08x (%08x)\n", vaddr, addr, value);

    //exit(1);
}


void psx_bus_write16(psx_bus_t* bus, uint32_t addr, uint32_t value) {
    bus->access_cycles = 0;

    uint32_t vaddr = addr;

    addr &= g_psx_bus_region_mask_table[addr >> 29];

    if (addr & 0x1) {
        log_fatal("Unaligned 16-bit write to %08x:%08x (%04x)", vaddr, addr, value);
    }

    HANDLE_WRITE_OP(bios, psx_bios_write16);
    HANDLE_WRITE_OP(ram, psx_ram_write16);
    HANDLE_WRITE_OP(dma, psx_dma_write16);
    HANDLE_WRITE_OP(exp1, psx_exp1_write16);
    HANDLE_WRITE_OP(exp2, psx_exp2_write16);
    HANDLE_WRITE_OP(mc1, psx_mc1_write16);
    HANDLE_WRITE_OP(mc2, psx_mc2_write16);
    HANDLE_WRITE_OP(mc3, psx_mc3_write16);
    HANDLE_WRITE_OP(ic, psx_ic_write16);
    HANDLE_WRITE_OP(scratchpad, psx_scratchpad_write16);
    HANDLE_WRITE_OP(gpu, psx_gpu_write16);
    HANDLE_WRITE_OP(spu, psx_spu_write16);
    HANDLE_WRITE_OP(timer, psx_timer_write16);
    HANDLE_WRITE_OP(cdrom, psx_cdrom_write16);
    HANDLE_WRITE_OP(pad, psx_pad_write16);
    HANDLE_WRITE_OP(mdec, psx_mdec_write16);

    // if (addr == 0x1f80105a) { sio_ctrl = value; return; }

    print("Unhandled 16-bit write to %08x:%08x (%04x)\n", vaddr, addr, value);

    //exit(1);
}

void psx_bus_write8(psx_bus_t* bus, uint32_t addr, uint32_t value) {
    bus->access_cycles = 0;

    uint32_t vaddr = addr;

    addr &= g_psx_bus_region_mask_table[addr >> 29];

    HANDLE_WRITE_OP(bios, psx_bios_write8);
    HANDLE_WRITE_OP(ram, psx_ram_write8);
    HANDLE_WRITE_OP(dma, psx_dma_write8);
    HANDLE_WRITE_OP(exp1, psx_exp1_write8);
    HANDLE_WRITE_OP(exp2, psx_exp2_write8);
    HANDLE_WRITE_OP(mc1, psx_mc1_write8);
    HANDLE_WRITE_OP(mc2, psx_mc2_write8);
    HANDLE_WRITE_OP(mc3, psx_mc3_write8);
    HANDLE_WRITE_OP(ic, psx_ic_write8);
    HANDLE_WRITE_OP(scratchpad, psx_scratchpad_write8);
    HANDLE_WRITE_OP(gpu, psx_gpu_write8);
    HANDLE_WRITE_OP(spu, psx_spu_write8);
    HANDLE_WRITE_OP(timer, psx_timer_write8);
    HANDLE_WRITE_OP(cdrom, psx_cdrom_write8);
    HANDLE_WRITE_OP(pad, psx_pad_write8);
    HANDLE_WRITE_OP(mdec, psx_mdec_write8);

    print("Unhandled 8-bit write to %08x:%08x (%02x)\n", vaddr, addr, value);

    //exit(1);
}

void psx_bus_init_bios(psx_bus_t* bus, psx_bios_t* bios) {
    bus->bios = bios;
}

void psx_bus_init_ram(psx_bus_t* bus, psx_ram_t* ram) {
    bus->ram = ram;
}

void psx_bus_init_dma(psx_bus_t* bus, psx_dma_t* dma) {
    bus->dma = dma;
}

void psx_bus_init_exp1(psx_bus_t* bus, psx_exp1_t* exp1) {
    bus->exp1 = exp1;
}

void psx_bus_init_exp2(psx_bus_t* bus, psx_exp2_t* exp2) {
    bus->exp2 = exp2;
}

void psx_bus_init_mc1(psx_bus_t* bus, psx_mc1_t* mc1) {
    bus->mc1 = mc1;
}

void psx_bus_init_mc2(psx_bus_t* bus, psx_mc2_t* mc2) {
    bus->mc2 = mc2;
}

void psx_bus_init_mc3(psx_bus_t* bus, psx_mc3_t* mc3) {
    bus->mc3 = mc3;
}

void psx_bus_init_ic(psx_bus_t* bus, psx_ic_t* ic) {
    bus->ic = ic;
}

void psx_bus_init_scratchpad(psx_bus_t* bus, psx_scratchpad_t* scratchpad) {
    bus->scratchpad = scratchpad;
}

void psx_bus_init_gpu(psx_bus_t* bus, psx_gpu_t* gpu) {
    bus->gpu = gpu;
}

void psx_bus_init_spu(psx_bus_t* bus, struct psx_spu_t* spu) {
    bus->spu = spu;
}

void psx_bus_init_timer(psx_bus_t* bus, struct psx_timer_t* timer) {
    bus->timer = timer;
}

void psx_bus_init_cdrom(psx_bus_t* bus, psx_cdrom_t* cdrom) {
    bus->cdrom = cdrom;
}

void psx_bus_init_pad(psx_bus_t* bus, struct psx_pad_t* pad) {
    bus->pad = pad;
}

void psx_bus_init_mdec(psx_bus_t* bus, struct psx_mdec_t* mdec) {
    bus->mdec = mdec;
}

uint32_t psx_bus_get_access_cycles(psx_bus_t* bus) {
    uint32_t cycles = bus->access_cycles;

    bus->access_cycles = 0;

    return cycles;
}

#undef HANDLE_READ_OP
#undef HANDLE_WRITE_OP
