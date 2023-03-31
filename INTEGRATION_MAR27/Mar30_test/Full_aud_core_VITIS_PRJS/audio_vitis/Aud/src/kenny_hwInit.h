
#ifndef __KENNY_HWINIT_H__
#define __KENNY_HWINIT_H__

#include <stdlib.h>
#include "xaxidma.h"
#include "xscugic.h"
#include "dma_accel.h"

#define INIT_FAILED (-1)
#define INIT_SUCCESS (0)

int kenny_init_intc(XScuGic * p_intc_inst, int intc_device_id);


#endif
