
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

const int SAMPLE_SLEEP_USEC = 21;

const int AUDIO_SAMPLE_RATE = 48000;
const int AUDIO_CHANNELS = 2;
const int MAX_RECORD_SEC = 2;
//const int KENNY_AUDIO_MAX_SAMPLES = AUDIO_SAMPLE_RATE * MAX_RECORD_SEC * AUDIO_CHANNELS;
const int KENNY_AUDIO_MAX_SAMPLES = 8192;


// External data
extern int sig_two_sine_waves[FFT_MAX_NUM_PTS]; // FFT input data

// Function prototypes
void which_fft_param(fft_t* p_fft_inst);


void audio_stream(){
	u32  in_left, in_right;

	while (1){
		/* If input from the terminal is 'q', then return.
		 * Else, continue streaming. */

		if(!XUartPs_IsReceiveData(UART_BASEADDR)){
			// Read audio input from codec
			in_left = Xil_In32(I2S_DATA_RX_L_REG);
			in_right = Xil_In32(I2S_DATA_RX_R_REG);
			// Write audio output to codec
			Xil_Out32(I2S_DATA_TX_L_REG, in_left);
			Xil_Out32(I2S_DATA_TX_R_REG, in_right);
		}
		else if (XUartPs_ReadReg(UART_BASEADDR, XUARTPS_FIFO_OFFSET) == 'q'){
			break;
		}
	}

} // audio_stream()

void kenny_PlaybackAudioFromMem(const int* KENNY_AUDIO_MEM_PTR)
{
	u32  in_left, in_right;
	int * cur_ptr = KENNY_AUDIO_MEM_PTR;
	u32 num_samples_recorded = 0;

	while (1)
	{
		if (!XUartPs_IsReceiveData(UART_BASEADDR)){
			// Read audio data from memory
			in_left  = *(cur_ptr++);
			in_right = *(cur_ptr++);
			num_samples_recorded += 2;

			// Write audio data to audio codec
			Xil_Out32(I2S_DATA_TX_L_REG, in_left);
			Xil_Out32(I2S_DATA_TX_R_REG, in_right);

			usleep(SAMPLE_SLEEP_USEC);

			if (num_samples_recorded >= KENNY_AUDIO_MAX_SAMPLES-1){
				break;
			}
		}
		else if (XUartPs_ReadReg(UART_BASEADDR, XUARTPS_FIFO_OFFSET) == 'q'){
			break;
		}
	}
}

void kenny_RecordAudioIntoMem(const int* KENNY_AUDIO_MEM_PTR)
{
	u32  in_left, in_right;
	int * cur_ptr = KENNY_AUDIO_MEM_PTR;
	u32 num_samples_recorded = 0;

	//memset(KENNY_AUDIO_MEM_START, 0, (KENNY_AUDIO_MEM_END - KENNY_AUDIO_MEM_START));

	/*
	for ( int* i = KENNY_AUDIO_MEM_START; i < KENNY_AUDIO_MEM_END; ++i)
	{
		*(i) = 0;
	}
	*/

	while (1){
		if (!XUartPs_IsReceiveData(UART_BASEADDR)){
			// Read audio input from codec
			in_left = Xil_In32(I2S_DATA_RX_L_REG);
			in_right = Xil_In32(I2S_DATA_RX_R_REG);
			// Save to memory
			*(cur_ptr++) = in_left;
			*(cur_ptr++) = in_right;
			num_samples_recorded += 2;

			usleep(SAMPLE_SLEEP_USEC);

			if (num_samples_recorded >= KENNY_AUDIO_MAX_SAMPLES-1){
				break;
			}
		}
		else if (XUartPs_ReadReg(UART_BASEADDR, XUARTPS_FIFO_OFFSET) == 'q'){
			break;
		}
	}
}

int kenny_signextend_24bit(int inval){
	int sign_val;
	int retval = 0;
	sign_val = (inval >> 23);
	if (sign_val == 0){
		retval = inval;
	}else{
		retval = 0xff000000 | inval;	// Set all high bits to 1.
	}
	return retval;
}

void kenny_convertAudioToCplx(int* inval, cplx_data_t* outval, size_t num_vals_to_cpy)
{
	cplx_data_t cur_cplx;
	int cur_re_int;
	short cur_re;
	short cur_im = 0;
	for (int in_idx = 0, out_idx = 0;
		 in_idx < num_vals_to_cpy;
		 in_idx += AUDIO_CHANNELS, ++out_idx)
	{
		cur_re_int = kenny_signextend_24bit(inval[in_idx]);
		cur_re_int = cur_re_int >> 8;
		cur_re = cur_re_int;
		cur_im = 0;
		cur_cplx.data_re = cur_re;
		cur_cplx.data_im = cur_im;
		outval[out_idx] = cur_cplx;

		if (out_idx < 5){
			xil_printf("outval[out_idx] = %d, %d*j \n\r", outval[out_idx].data_re, outval[out_idx].data_im);
		}
	}
}

void kenny_updateFFT_InputData(cplx_data_t* stim_buf, int* recorded_audio_buf)
{
	char c = '\0';

	xil_printf("Which input data would you like to use?\n\r");
	xil_printf("0: Recorded Audio\n\r");
	xil_printf("1: Generated Values from Xilinx\n\r");
	xil_printf("2: Exit\n\r");
	while (1)
	{
		c = XUartPs_RecvByte(XPAR_PS7_UART_1_BASEADDR);
		if (c == '0')
		{
			kenny_convertAudioToCplx(recorded_audio_buf, stim_buf, sizeof(cplx_data_t)*FFT_MAX_NUM_PTS);
			break;
		}
		else if (c == '1')
		{
		    memcpy(stim_buf, sig_two_sine_waves, sizeof(cplx_data_t)*FFT_MAX_NUM_PTS);
			break;
		}
		else if (c == '2')
		{
			break;
		}
		else
    	{
    		xil_printf("Invalid character. Please try again.\n\r");
    	}
	}
}

int kenny_guessFrequencyOfData(fft_t* p_fft_inst)
{
//	int num_pts_in_fft = fft_get_num_pts(p_fft_inst);
//	char         str[25]; // Large enough to hold 2 ints plus extra characters
//	int cur_sum = 0;
//	cplx_data_t* fft_output = fft_get_result_buf(p_fft_inst);
//	cplx_data_t cur_val;
//
//	for (int i = 0; i < num_pts_in_fft; i++)
//	{
//		cplx_data_get_string(str, fft_output[i]);
//		cur_val = fft_output[i];
//		cur_sum = abs(cur_val.data_re) + abs(cur_val.data_im);
//		xil_printf("Xk(%d) = %s. Sum = %d \n\r", i, str, cur_sum);
//	}
//	return 0;


	int num_pts_in_fft = fft_get_num_pts(p_fft_inst);
	cplx_data_t* fft_output = fft_get_result_buf(p_fft_inst);
	float freq_per_bucket = AUDIO_SAMPLE_RATE/num_pts_in_fft;
	int guessed_freq = 0;
	int max_sum = 0;
	int cur_sum = 0;
	cplx_data_t cur_val;
	for (int i = 0; i < num_pts_in_fft/2; ++i)
	{
		cur_val = fft_output[i];
		cur_sum = abs(cur_val.data_re) + abs(cur_val.data_im);
		if (cur_sum > max_sum){
			max_sum = cur_sum;
			guessed_freq = freq_per_bucket * i;
			//guessed_freq = i;
		}
	}
	return guessed_freq;
}


// Main entry point
int main()
{

	// Local variables
	int * 		KENNY_AUDIO_MEM_PTR = malloc(sizeof(int) * KENNY_AUDIO_MAX_SAMPLES);
	int          status = 0;
	char         c;
	fft_t*       p_fft_inst;
	cplx_data_t* stim_buf;
	cplx_data_t* result_buf;

	// Setup UART and enable caches
    init_platform();
	xil_printf("Entering Main\r\n");

	//Configure the IIC data structure
	IicConfig(XPAR_XIICPS_0_DEVICE_ID);

	//Configure the Audio Codec's PLL
	AudioPllConfig();

	//Configure the Line in and Line out ports.
	//Call LineInLineOutConfig() for a configuration that
	//enables the HP jack too.
	AudioConfigureJacks();

	xil_printf("ADAU1761 configured\n\r");


    // Create FFT object
    p_fft_inst = fft_create
    (
    	XPAR_GPIO_0_DEVICE_ID,
    	XPAR_AXIDMA_0_DEVICE_ID,
    	XPAR_PS7_SCUGIC_0_DEVICE_ID,
    	XPAR_FABRIC_CTRL_AXI_DMA_0_S2MM_INTROUT_INTR,
    	XPAR_FABRIC_CTRL_AXI_DMA_0_MM2S_INTROUT_INTR
    );
    if (p_fft_inst == NULL)
    {
    	xil_printf("ERROR! Failed to create FFT instance.\n\r");
    	return -1;
    }

    // Allocate data buffers
    stim_buf = (cplx_data_t*) malloc(sizeof(cplx_data_t)*FFT_MAX_NUM_PTS);
    if (stim_buf == NULL)
    {
    	xil_printf("ERROR! Failed to allocate memory for the stimulus buffer.\n\r");
    	return -1;
    }

    result_buf = (cplx_data_t*) malloc(sizeof(cplx_data_t)*FFT_MAX_NUM_PTS);
    if (result_buf == NULL)
    {
    	xil_printf("ERROR! Failed to allocate memory for the result buffer.\n\r");
    	return -1;
    }

    // Fill stimulus buffer with some signal
    memcpy(stim_buf, sig_two_sine_waves, sizeof(cplx_data_t)*FFT_MAX_NUM_PTS);

    // Main control loop
    while (1)
    {

    	// Get command
    	xil_printf("What would you like to do?\n\r");
    	xil_printf("0: Print current FFT parameters\n\r");
    	xil_printf("1: Change FFT parameters\n\r");
    	xil_printf("2: Perform FFT using current parameters\n\r");
    	xil_printf("3: Print current stimulus to be used for the FFT operation\n\r");
    	xil_printf("4: Print results of previous FFT operation\n\r");
    	xil_printf("5: Change FFT input data\n\r");
    	xil_printf("S: Stream Audio\n\r");
    	xil_printf("R: Record Audio to memory\n\r");
    	xil_printf("P: Play Audio from memory\n\r");
    	xil_printf("D: Detect Frequency from FFT output\n\r");
    	c = XUartPs_RecvByte(XPAR_PS7_UART_1_BASEADDR);

    	if (c == '0')
    	{
    		xil_printf("---------------------------------------------\n\r");
    		fft_print_params(p_fft_inst);
    		xil_printf("---------------------------------------------\n\r");
    	}
    	else if (c == '1')
    	{
    		which_fft_param(p_fft_inst);
    	}
    	else if (c == '2') // Run FFT
		{
			// Make sure the buffer is clear before we populate it (this is generally not necessary and wastes time doing memory accesses, but for proving the DMA working, we do it anyway)
			memset(result_buf, 0, sizeof(cplx_data_t)*FFT_MAX_NUM_PTS);

			status = fft(p_fft_inst, (cplx_data_t*)stim_buf, (cplx_data_t*)result_buf);
			if (status != FFT_SUCCESS)
			{
				xil_printf("ERROR! FFT failed.\n\r");
				return -1;
			}

			xil_printf("FFT complete!\n\r");
		}
    	else if (c == '3')
    	{
    		fft_set_stim_buf(p_fft_inst, stim_buf);
    		fft_print_stim_buf(p_fft_inst);
    	}
    	else if (c == '4')
    	{
    		fft_print_result_buf(p_fft_inst);
    	}
    	else if (c == '5')
    	{
    		kenny_updateFFT_InputData(stim_buf, KENNY_AUDIO_MEM_PTR);
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
    		int guessed_freq = kenny_guessFrequencyOfData(p_fft_inst);
    		xil_printf("The frequency is around: %d \r\n", guessed_freq);
    	}
    	else
    	{
    		xil_printf("Invalid character. Please try again.\n\r");
    	}

    }

    free(stim_buf);
    free(result_buf);
    fft_destroy(p_fft_inst);

    return 0;

}

void which_fft_param(fft_t* p_fft_inst)
{
	// Local variables
	char c;

	xil_printf("Okay, which parameter would you like to change?\n\r");
	xil_printf("0: Point length\n\r");
	xil_printf("1: Direction\n\r");
	xil_printf("2: Exit\n\r");
	while (1)
	{
		c = XUartPs_RecvByte(XPAR_PS7_UART_1_BASEADDR);

		if (c == '0')
		{
			xil_printf("What would you like to set the FFT point length to? Type:\n\r");
			xil_printf("0: FFT point length = 16\n\r");
			xil_printf("1: FFT point length = 32\n\r");
			xil_printf("2: FFT point length = 64\n\r");
			xil_printf("3: FFT point length = 128\n\r");
			xil_printf("4: FFT point length = 256\n\r");
			xil_printf("5: FFT point length = 512\n\r");
			xil_printf("6: FFT point length = 1024\n\r");
			xil_printf("7: FFT point length = 2048\n\r");
			xil_printf("8: FFT point length = 4096\n\r");
			xil_printf("9: FFT point length = 8192\n\r");

			c = XUartPs_RecvByte(XPAR_PS7_UART_1_BASEADDR);
			while (1)
			{
				if (c == '0')
				{
					xil_printf("Okay, setting the core to perform a 16-point FFT.\n\r");
					fft_set_num_pts(p_fft_inst, 16);
					break;
				}
				else if (c == '1')
				{
					xil_printf("Okay, setting the core to perform a 32-point FFT.\n\r");
					fft_set_num_pts(p_fft_inst, 32);
					break;
				}
				else if (c == '2')
				{
					xil_printf("Okay, setting the core to perform a 64-point FFT.\n\r");
					fft_set_num_pts(p_fft_inst, 64);
					break;
				}
				else if (c == '3')
				{
					xil_printf("Okay, setting the core to perform a 128-point FFT.\n\r");
					fft_set_num_pts(p_fft_inst, 128);
					break;
				}
				else if (c == '4')
				{
					xil_printf("Okay, setting the core to perform a 256-point FFT.\n\r");
					fft_set_num_pts(p_fft_inst, 256);
					break;
				}
				else if (c == '5')
				{
					xil_printf("Okay, setting the core to perform a 512-point FFT.\n\r");
					fft_set_num_pts(p_fft_inst, 512);
					break;
				}
				else if (c == '6')
				{
					xil_printf("Okay, setting the core to perform a 1024-point FFT.\n\r");
					fft_set_num_pts(p_fft_inst, 1024);
					break;
				}
				else if (c == '7')
				{
					xil_printf("Okay, setting the core to perform a 2048-point FFT.\n\r");
					fft_set_num_pts(p_fft_inst, 2048);
					break;
				}
				else if (c == '8')
				{
					xil_printf("Okay, setting the core to perform a 4096-point FFT.\n\r");
					fft_set_num_pts(p_fft_inst, 4096);
					break;
				}
				else if (c == '9')
				{
					xil_printf("Okay, setting the core to perform a 8192-point FFT.\n\r");
					fft_set_num_pts(p_fft_inst, 8192);
					break;
				}
				else
		    		xil_printf("Invalid character. Please try again.\n\r");
			}
			break;
		}
		else if (c == '1')
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

