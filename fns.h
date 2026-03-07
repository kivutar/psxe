#ifndef FNS_H
#define FNS_H

typedef struct psx_gpu_t psx_gpu_t;

int	audioout(void);
void	bindkeys(void);
void	blitframe(void);
void	flush(void);
void	process_inputs(void);
void	psxe_gpu_vblank_event_cb(psx_gpu_t*);
void	usage(void);

#endif
