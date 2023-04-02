/*
 * adventures_with_ip.h
 *
 * Main header file.
 */

#ifndef ADVENTURES_WITH_IP_H_
#define ADVENTURES_WITH_IP_H_

/* ---------------------------------------------------------------------------- *
 * 								Header Files									*
 * ---------------------------------------------------------------------------- */
#include <stdio.h>
#include <xil_io.h>
#include <sleep.h>
#include "xiicps.h"
#include <xil_printf.h>
#include <xparameters.h>
#include "xgpio.h"
#include "xuartps.h"
#include "stdlib.h"
#include <math.h>

// Required for the buttons/switches interrupts:
#include "xil_types.h"
#include "xtmrctr.h"
#include "xil_exception.h"
#include "xscugic.h"
#include "xil_cache.h"
#include "xil_mmu.h"

// Misc
#include "kenny_audio.h"


/* ---------------------------------------------------------------------------- *
 * 							Custom IP Header Files								*
 * ---------------------------------------------------------------------------- */
#include "audio.h"
//#include "lms_pcore_addr.h"
//#include "xnco.h"



/* ---------------------------------------------------------------------------- *
 * 							Global Variables for interrupt stuff                *
 * ---------------------------------------------------------------------------- */

//#define __DEBUGGING__ (1)
//#define __PURE_STREAMING__ (1)
//#define __DO_TIMING__

/////////////////////// TIMING THE INTERRUPT
#ifdef __DO_TIMING__
#include "xtime_l.h"
int TMP_DEBUG_CTR;
XTime startcycles, endcycles, totalcycles;
#endif
///////////////////////////

#define LED_CHANNEL 1
#define PUSH_BTN_CH 1
#define SLIDE_SWT_CH 1
XGpio SWs; /* The Instance of the GPIO Driver */
XGpio BTNs; /* The Instance of the GPIO Driver */
XGpio LEDs;
XTmrCtr TimerInstancePtr;
u32 slide_sw;
u32 leds;
volatile int TIMER_INTR_FLG;

int aud_in_idx;
int *AUDIO_IN_MEM_PTRS[3];
int *cur_audio_in_ptr;
int audio_in_read_ctr;

int aud_out_idx;
int *AUDIO_OUT_MEM_PTR[2];
int *cur_audio_out_ptr;
int audio_out_read_ctr;

int num_fft_pts;
int *FFTDATA_IN_MEM_PTR;
int FFTDATA_READY;

// 3 for the compressor, 2 for gain, and... for now let's just say 6 for the EQ.
int SELECTION_STATE;

/* ---------------------------------------------------------------------------- *
 * 							Prototype Functions									*
 * ---------------------------------------------------------------------------- */
void menu();
void tonal_noise();
void audio_stream();
void lms_filter();
unsigned char gpio_init();
void nco_init(void *InstancePtr);

/* ---------------------------------------------------------------------------- *
 * 						Redefinitions from xparameters.h 						*
 * ---------------------------------------------------------------------------- */
//#define NCO_ID XPAR_NCO_0_DEVICE_ID

//#define LMS_LOC XPAR_LMS_PCORE_0_BASEADDR
//#define LMS_X LMS_LOC + x_k__Data_lms_pcore
//#define LMS_D LMS_LOC + d_k__Data_lms_pcore
//#define LMS_E LMS_LOC + e_k__Data_lms_pcore
//#define LMS_STROBE LMS_LOC + IPCore_Strobe_lms_pcore

#define UART_BASEADDR XPAR_PS7_UART_1_BASEADDR

//#define BUTTON_SWITCH_BASE XPAR_GPIO_1_BASEADDR
//#define LED_BASE XPAR_LED_CONTROLLER_0_S00_AXI_BASEADDR
//#define BUTTON_SWITCH_ID XPAR_GPIO_1_DEVICE_ID

/* ---------------------------------------------------------------------------- *
 * 							Define GPIO Channels								*
 * ---------------------------------------------------------------------------- */
//#define BUTTON_CHANNEL 1
//#define SWITCH_CHANNEL 2

/* ---------------------------------------------------------------------------- *
 * 							Audio Scaling Factor								*
 * ---------------------------------------------------------------------------- */
//#define SCALE 6

/* ---------------------------------------------------------------------------- *
 * 							Global Variables									*
 * ---------------------------------------------------------------------------- */
XIicPs Iic;
XGpio Gpio; // Gpio instance for buttons and switches
//XNco Nco;


#endif /* ADVENTURES_WITH_IP_H_ */
