
#include "exe.h"
#include "log.h"
#include "dev/ram.h"

static int psx_readn_fd(int fd, void* buf, uint32_t size) {
    uint8_t* p = (uint8_t*)buf;
    uint32_t n = size;

    while (n) {
        long r = read(fd, p, n);

        if (r <= 0)
            return -1;

        p += r;
        n -= r;
    }

    return 0;
}

int psx_exe_load(psx_cpu_t* cpu, const char* path) {
    if (!path)
        return 0;

    int fd = open(path, OREAD);

    if (fd < 0)
        return 1;

    // Read header
    psx_exe_hdr_t hdr;
    
    if (psx_readn_fd(fd, &hdr, sizeof(psx_exe_hdr_t)) < 0) {
        close(fd);
        return 2;
    }

    // Seek to program start 
    if (seek(fd, 0x800, 0) < 0) {
        close(fd);
        return 2;
    }

    // Read to RAM directly
    uint32_t offset = hdr.ramdest & 0x7fffffff;

    if (psx_readn_fd(fd, cpu->bus->ram->buf + offset, hdr.filesz) < 0) {
        close(fd);
        return 3;
    }

    // Load initial register values
    cpu->pc = hdr.ipc;
    cpu->next_pc = cpu->pc + 4;
    cpu->r[28] = hdr.igp;

    if (hdr.ispb) {
        cpu->r[29] = hdr.ispb + hdr.ispoff;
        cpu->r[30] = cpu->r[29];
    }

    log_info("Loaded PS-X EXE file \"%s\"", path);
    log_fatal("PC=%08x SP=%08x (%08x) GP=%08x", cpu->pc, cpu->r[29], hdr.ispb, cpu->r[28]);

    close(fd);

    return 0;
}
