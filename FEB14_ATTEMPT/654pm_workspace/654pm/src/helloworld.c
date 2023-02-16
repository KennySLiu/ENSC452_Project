
//////////////////////////////////////////////////////////////////////////////////
//
// Company:        Xilinx
// Engineer:       bwiec
// Create Date:    29 June 2015, 03:34:03 PM
// App Name:       DMA Accelerator Demonstration
// File Name:      helloworld.c
// Target Devices: Zynq
// Tool Versions:  2015.1
// Description:    Implementation of FFT using a hardware accelerator with DMA.
// Dependencies:
//   - xuartps_hw.h - Driver version v3.0
//   - fft.h        - Driver version v1.0
//   - cplx_data.h  - Driver version v1.0
//   - stim.h       - Driver version v1.0
// Revision History:
//   - v1.0
//     * Initial release
//     * Tested on ZC702 and Zedboard
// Additional Comments:
//   - UART baud rate is 115200
//   - GPIO is used with some additional glue logic to control the FFT core's
//     config interface for setting various parameters.
//
//////////////////////////////////////////////////////////////////////////////////

// Includes
#include <stdio.h>
#include <stdlib.h>
#include "platform.h"
#include "xuartps_hw.h"
#include "fft.h"
#include "cplx_data.h"
#include "stim.h"
#include "adventures_with_ip.h"
#include "kenny_audio.h"
#include "kenny_hwInit.h"




// External data
extern int sig_two_sine_waves[FFT_MAX_NUM_PTS]; // FFT input data

// Function prototypes
void which_fft_param(fft_t* p_fft_inst);




// Main entry point
int main()
{
	// Hardware stuff:
	XScuGic intc_inst;

	// Local variables
	int * 		KENNY_AUDIO_MEM_PTR = malloc(sizeof(int) * KENNY_AUDIO_MAX_SAMPLES);
	int          status = 0;
	char         c;
	fft_t*       p_fft_inst_FWD;
	fft_t* 	     p_fft_inst_INV;
	cplx_data_t* input_buf;
	cplx_data_t* intermediate_buf;
	cplx_data_t* result_buf;

	// Setup UART and enable caches
    init_platform();
	xil_printf("Entering Main\r\n");

	//Configure the IIC data structure
	IicConfig(XPAR_XIICPS_0_DEVICE_ID);

	//Configure the Audio Codec's PLL
	AudioPllConfig();

	kenny_init_intc(&intc_inst, XPAR_PS7_SCUGIC_0_DEVICE_ID);

	//Configure the Line in and Line out ports.
	//Call LineInLineOutConfig() for a configuration that
	//enables the HP jack too.
	AudioConfigureJacks();

	xil_printf("ADAU1761 configured\n\r");


	/***********************/
    // Create FFT objects
    p_fft_inst_FWD = fft_create
    (
    	XPAR_GPIO_0_DEVICE_ID,
    	XPAR_AXIDMA_0_DEVICE_ID,
		&intc_inst,
		XPAR_FABRIC_AXI_DMA_0_S2MM_INTROUT_INTR,
		XPAR_FABRIC_AXI_DMA_0_MM2S_INTROUT_INTR
    );
    if (p_fft_inst_FWD == NULL)
    {
    	xil_printf("ERROR! Failed to create FWD FFT instance.\n\r");
    	return -1;
    }
	fft_set_fwd_inv(p_fft_inst_FWD, FFT_FORWARD);


//    p_fft_inst_INV = fft_create
//    (
//    	XPAR_GPIO_1_DEVICE_ID,
//    	XPAR_AXIDMA_1_DEVICE_ID,
//		&intc_inst,
//		XPAR_FABRIC_AXI_DMA_1_S2MM_INTROUT_INTR,
//		XPAR_FABRIC_AXI_DMA_1_MM2S_INTROUT_INTR
//    );
//    if (p_fft_inst_INV == NULL)
//    {
//    	xil_printf("ERROR! Failed to create INV FFT instance.\n\r");
//    	return -1;
//    }
//	fft_set_fwd_inv(p_fft_inst_INV, FFT_INVERSE);

	/***********************/

    // Allocate data buffers
    input_buf = (cplx_data_t*) malloc(sizeof(cplx_data_t)*FFT_MAX_NUM_PTS);
    if (input_buf == NULL){
    	xil_printf("ERROR! Failed to allocate memory for the stimulus buffer.\n\r");
    	return -1;
    }
	intermediate_buf = (cplx_data_t*) malloc(sizeof(cplx_data_t)*FFT_MAX_NUM_PTS);
    if (intermediate_buf == NULL){
    	xil_printf("ERROR! Failed to allocate memory for the intermediate buffer.\n\r");
    	return -1;
    }
//    result_buf = (cplx_data_t*) malloc(sizeof(cplx_data_t)*FFT_MAX_NUM_PTS);
//    if (result_buf == NULL){
//    	xil_printf("ERROR! Failed to allocate memory for the result buffer.\n\r");
//    	return -1;
//    }

    // Fill stimulus buffer with some signal
    memcpy(input_buf, sig_two_sine_waves, sizeof(cplx_data_t)*FFT_MAX_NUM_PTS);

    // Main control loop
    while (1)
    {

    	// Get command
    	xil_printf("What would you like to do?\n\r");
    	xil_printf("0: Print current FFT parameters\n\r");
    	xil_printf("7/8: Perform FFT / IFFT using current parameters\n\r");
    	xil_printf("3: Print current INPUT to be used for the FFT operation\n\r");
    	xil_printf("4: Print current INTERMEDIATE data of FFT operation\n\r");
    	xil_printf("5: Change FFT input data\n\r");
    	xil_printf("S: Stream Pure Audio\n\r");
    	xil_printf("R/P: Record/Play Audio to/from memory\n\r");
    	xil_printf("D: Detect Frequency from FFT output\n\r");
    	c = XUartPs_RecvByte(XPAR_PS7_UART_1_BASEADDR);

    	if (c == '0')
    	{
    		xil_printf("---------------------------------------------\n\r");
    		fft_print_params(p_fft_inst_FWD);
    		xil_printf("---------------------------------------------\n\r");
    		fft_print_params(p_fft_inst_INV);
    		xil_printf("---------------------------------------------\n\r");
    	}
    	else if (c == '1')
    	{
    		which_fft_param(p_fft_inst_FWD);
    	}
    	else if (c == '7') // Run FFT
		{
			// Make sure the buffer is clear before we populate it (this is generally not necessary and wastes time doing memory accesses, but for proving the DMA working, we do it anyway)
			memset(intermediate_buf, 0, sizeof(cplx_data_t)*FFT_MAX_NUM_PTS);

			status = fft(p_fft_inst_FWD, (cplx_data_t*)input_buf, (cplx_data_t*)intermediate_buf);
			if (status != FFT_SUCCESS)
			{
				xil_printf("ERROR! FFT failed.\n\r");
				return -1;
			}

			xil_printf("FFT complete!\n\r\n\r");
		}
    	else if (c == '8') // Run IFFT
		{
			// Make sure the buffer is clear before we populate it (this is generally not necessary and wastes time doing memory accesses, but for proving the DMA working, we do it anyway)
			memset(result_buf, 0, sizeof(cplx_data_t)*FFT_MAX_NUM_PTS);

			status = fft(p_fft_inst_INV, (cplx_data_t*)intermediate_buf, (cplx_data_t*)result_buf);
			if (status != FFT_SUCCESS)
			{
				xil_printf("ERROR! Inverse FFT failed.\n\r");
				return -1;
			}

			xil_printf("Inverse FFT complete!\n\r\n\r");
		}
    	else if (c == '3')
    	{
    		fft_set_stim_buf(p_fft_inst_FWD, input_buf);
    		fft_print_stim_buf(p_fft_inst_FWD);
    	}
    	else if (c == '4')
    	{
    		fft_print_result_buf(p_fft_inst_FWD);
    	}
    	else if (c == '5')
    	{
    		kenny_updateFFT_InputData(input_buf, KENNY_AUDIO_MEM_PTR);
    	}
    	else if (c == 's')
    	{
			xil_printf("STREAMING AUDIO\r\n");
			xil_printf("Press 'q' to return to the main menu\r\n");
			audio_stream();
    	}
    	else if (c == 'r')
		{
    		xil_printf("RECORDING AUDIO\r\n");
    		xil_printf("Press 'q' to stop recording early and return to the main menu\r\n");
    		kenny_RecordAudioIntoMem(KENNY_AUDIO_MEM_PTR);
		}
    	else if (c == 'p')
    	{
			xil_printf("PLAYING BACK RECORDED AUDIO\r\n");
			xil_printf("Press 'q' to stop playback early and return to the main menu\r\n");
			kenny_PlaybackAudioFromMem(KENNY_AUDIO_MEM_PTR);
    	}
    	else if (c == 'd')
    	{
    		int guessed_freq = kenny_guessFrequencyOfData(p_fft_inst_FWD);
    		xil_printf("The frequency is around: %d \r\n", guessed_freq);
    	}
    	else
    	{
    		xil_printf("Invalid character. Please try again.\n\r");
    	}

    }

    free(input_buf);
    free(result_buf);
    fft_destroy(p_fft_inst_FWD);

    return 0;

}

void which_fft_param(fft_t* p_fft_inst)
{
	// Local variables
	char c;

	xil_printf("Okay, which parameter would you like to change?\n\r");
	xil_printf("1: Direction\n\r");
	xil_printf("2: Exit\n\r");
	while (1)
	{
		c = XUartPs_RecvByte(XPAR_PS7_UART_1_BASEADDR);

		if (c == '1')
		{

			xil_printf("What would you like to set the FFT direction to? Type:\n\r");
			xil_printf("0: Inverse\n\r");
			xil_printf("1: Forward\n\r");

			c = XUartPs_RecvByte(XPAR_PS7_UART_1_BASEADDR);
			while (1)
			{
				if (c == '0')
				{
					xil_printf("Okay, setting the core to perform an inverse FFT.\n\r");
					fft_set_fwd_inv(p_fft_inst, FFT_INVERSE);
					break;
				}
				else if (c == '1')
				{
					xil_printf("Okay, setting the core to perform a forward FFT.\n\r");
					fft_set_fwd_inv(p_fft_inst, FFT_FORWARD);
					break;
				}
				else
		    		xil_printf("Invalid character. Please try again.\n\r");
			}
			break;
		}
		else if (c == '2')
		{
			return;
		}
		else
		{
			xil_printf("Invalid character. Please try again.\n\r");
		}
	}
}

