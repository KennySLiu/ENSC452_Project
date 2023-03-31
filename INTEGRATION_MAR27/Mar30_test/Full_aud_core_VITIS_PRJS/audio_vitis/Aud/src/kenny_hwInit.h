
#ifndef __KENNY_HWINIT_H__
#define __KENNY_HWINIT_H__

#include <stdlib.h>
#include "xaxidma.h"
#include "adventures_with_ip.h"
#include "xscugic.h"
#include "dma_accel.h"

#define INIT_FAILED (-1)
#define INIT_SUCCESS (0)

#define BTN_INT	XGPIO_IR_CH1_MASK
#define SW_INT XGPIO_IR_CH1_MASK

int kenny_init_intc(XScuGic * p_intc_inst, int intc_device_id);
int read_SW(void);
int read_BTN(void);
int read_LED(void);


#endif
