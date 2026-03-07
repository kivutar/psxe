#ifndef PSX_BUS_INIT_H
#define PSX_BUS_INIT_H

#include "p9.h"

struct psx_bios_t;
struct psx_ram_t;
struct psx_dma_t;
struct psx_exp1_t;
struct psx_exp2_t;
struct psx_mc1_t;
struct psx_mc2_t;
struct psx_mc3_t;
struct psx_ic_t;
struct psx_scratchpad_t;
struct psx_gpu_t;
struct psx_spu_t;
struct psx_timer_t;
struct psx_cdrom_t;
struct psx_pad_t;
struct psx_mdec_t;

typedef struct psx_bios_t psx_bios_t;
typedef struct psx_ram_t psx_ram_t;
typedef struct psx_dma_t psx_dma_t;
typedef struct psx_exp1_t psx_exp1_t;
typedef struct psx_exp2_t psx_exp2_t;
typedef struct psx_mc1_t psx_mc1_t;
typedef struct psx_mc2_t psx_mc2_t;
typedef struct psx_mc3_t psx_mc3_t;
typedef struct psx_ic_t psx_ic_t;
typedef struct psx_scratchpad_t psx_scratchpad_t;
typedef struct psx_gpu_t psx_gpu_t;
typedef struct psx_spu_t psx_spu_t;
typedef struct psx_timer_t psx_timer_t;
typedef struct psx_cdrom_t psx_cdrom_t;
typedef struct psx_pad_t psx_pad_t;
typedef struct psx_mdec_t psx_mdec_t;

typedef struct psx_bus_t psx_bus_t;

struct psx_bus_t {
    struct psx_bios_t* bios;
    struct psx_ram_t* ram;
    struct psx_dma_t* dma;
    struct psx_exp1_t* exp1;
    struct psx_exp2_t* exp2;
    struct psx_mc1_t* mc1;
    struct psx_mc2_t* mc2;
    struct psx_mc3_t* mc3;
    struct psx_ic_t* ic;
    struct psx_scratchpad_t* scratchpad;
    struct psx_gpu_t* gpu;
    struct psx_spu_t* spu;
    struct psx_timer_t* timer;
    struct psx_cdrom_t* cdrom;
    struct psx_pad_t* pad;
    struct psx_mdec_t* mdec;

    uint32_t access_cycles;
};

void psx_bus_init_bios(psx_bus_t*, struct psx_bios_t*);
void psx_bus_init_ram(psx_bus_t*, struct psx_ram_t*);
void psx_bus_init_dma(psx_bus_t*, struct psx_dma_t*);
void psx_bus_init_exp1(psx_bus_t*, struct psx_exp1_t*);
void psx_bus_init_exp2(psx_bus_t*, struct psx_exp2_t*);
void psx_bus_init_mc1(psx_bus_t*, struct psx_mc1_t*);
void psx_bus_init_mc2(psx_bus_t*, struct psx_mc2_t*);
void psx_bus_init_mc3(psx_bus_t*, struct psx_mc3_t*);
void psx_bus_init_ic(psx_bus_t*, struct psx_ic_t*);
void psx_bus_init_scratchpad(psx_bus_t*, struct psx_scratchpad_t*);
void psx_bus_init_gpu(psx_bus_t*, struct psx_gpu_t*);
void psx_bus_init_spu(psx_bus_t*, struct psx_spu_t*);
void psx_bus_init_timer(psx_bus_t*, struct psx_timer_t*);
void psx_bus_init_cdrom(psx_bus_t*, struct psx_cdrom_t*);
void psx_bus_init_pad(psx_bus_t*, struct psx_pad_t*);
void psx_bus_init_mdec(psx_bus_t*, struct psx_mdec_t*);

#endif
