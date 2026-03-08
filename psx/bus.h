#ifndef PSX_BUS_H
#define PSX_BUS_H

#include "p9.h"
#include "bus_init.h"

psx_bus_t* psx_bus_create(void);
void psx_bus_init(psx_bus_t*);
uint32_t psx_bus_read32(psx_bus_t*, uint32_t);
uint16_t psx_bus_read16(psx_bus_t*, uint32_t);
uint8_t psx_bus_read8(psx_bus_t*, uint32_t);
void psx_bus_write32(psx_bus_t*, uint32_t, uint32_t);
void psx_bus_write16(psx_bus_t*, uint32_t, uint32_t);
void psx_bus_write8(psx_bus_t*, uint32_t, uint32_t);
uint32_t psx_bus_get_access_cycles(psx_bus_t*);
void psx_bus_destroy(psx_bus_t*);

#endif
