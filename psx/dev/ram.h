#ifndef PSX_DEV_RAM_H
#define PSX_DEV_RAM_H

#include "p9.h"

#include "log.h"

typedef struct psx_mc2_t psx_mc2_t;

#define PSX_RAM_SIZE    0x800000 // 8MB window
#define PSX_RAM_BEGIN   0x00000000
//#define PSX_RAM_END     0x001fffff
#define PSX_RAM_END     0x1effffff
#define RAM_INIT_FILL   0

#define RAM_SIZE_2MB 0x200000
#define RAM_SIZE_4MB 0x400000
#define RAM_SIZE_8MB 0x800000

struct psx_ram_t {
    uint32_t bus_delay;
    uint32_t io_base, io_size;

    uintptr size;

    psx_mc2_t* mc2;

    uint8_t* buf;
};

typedef struct psx_ram_t psx_ram_t;

psx_ram_t* psx_ram_create(void);
void psx_ram_init(psx_ram_t*, psx_mc2_t*, int size);
uint32_t psx_ram_read32(psx_ram_t*, uint32_t);
uint16_t psx_ram_read16(psx_ram_t*, uint32_t);
uint8_t psx_ram_read8(psx_ram_t*, uint32_t);
void psx_ram_write32(psx_ram_t*, uint32_t, uint32_t);
void psx_ram_write16(psx_ram_t*, uint32_t, uint16_t);
void psx_ram_write8(psx_ram_t*, uint32_t, uint8_t);
void psx_ram_destroy(psx_ram_t*);

#endif
