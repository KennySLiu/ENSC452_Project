
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
#include <math.h>
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
//extern int EQ_cur_num_freq_buckets;

// Function prototypes
void which_fft_param(int* cur_num_fft_pts, fft_t* p_fft_inst_FWD, fft_t* p_fft_inst_INV);


// Main entry point
int main()
{
	int cur_num_fft_pts = INIT_NUM_FFT_PTS;
//	EQ_cur_num_freq_buckets = (int) log2f(INIT_NUM_FFT_PTS) - 1;
//	float parametric_eq_vect[EQ_MAX_NUM_FREQ_BUCKETS] = {1.0};
//	float STFT_window_func[FFT_MAX_NUM_PTS] = {0};
	eq_settings_t			eq_settings;
	compressor_settings_t 	compressor_settings;
	stft_settings_t         stft_settings;

	// Hardware stuff:
	XScuGic intc_inst;

	// Local variables
	//int 		* 	KENNY_AUDIO_IN_MEM_PTR = malloc(sizeof(int) * (KENNY_AUDIO_MAX_SAMPLES));
	//cplx_data_t * 	KENNY_FFTDATA_MEM_PTR = malloc(sizeof(cplx_data_t) * KENNY_FFTDATA_SZ);
	//int 		* 	KENNY_AUDIO_OUT_MEM_PTR = malloc(sizeof(int) * (KENNY_AUDIO_MAX_SAMPLES));
	int 		KENNY_AUDIO_IN_MEM_PTR  [KENNY_AUDIO_MAX_SAMPLES];
	cplx_data_t KENNY_FFTDATA_MEM_PTR   [KENNY_FFTDATA_SZ];
	int 		KENNY_AUDIO_OUT_MEM_PTR [KENNY_AUDIO_MAX_SAMPLES];

	char         c;
	fft_t*       p_fft_inst_FWD;
	fft_t* 	     p_fft_inst_INV;
	cplx_data_t* input_buf;
	cplx_data_t* intermediate_buf;
	cplx_data_t* result_buf;

	// Setup UART and enable caches
    init_platform();
	xil_printf("\r\n\r\n\r\n\r\n\r\n\r\n\r\n");
	xil_printf("Entering Main \r\n");
	xil_printf("AUDIO IN MEM PTR = %x to %x\r\n",
				KENNY_AUDIO_IN_MEM_PTR, &(KENNY_AUDIO_IN_MEM_PTR[KENNY_AUDIO_MAX_SAMPLES-1])
				);
	xil_printf("FFT PTR = %x to %x\r\n",
				KENNY_FFTDATA_MEM_PTR, &(KENNY_FFTDATA_MEM_PTR[KENNY_FFTDATA_SZ-1])
				);
	xil_printf("AUDIO OUT MEM PTR = %x to %x\r\n",
				KENNY_AUDIO_OUT_MEM_PTR, &(KENNY_AUDIO_OUT_MEM_PTR[KENNY_AUDIO_MAX_SAMPLES-1])
				);

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

	kenny_stft_init(&stft_settings);
	kenny_eq_init(&eq_settings, &stft_settings);
	kenny_compressor_init(&compressor_settings, &stft_settings);

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


	p_fft_inst_INV = fft_create
    (
		XPAR_GPIO_1_DEVICE_ID,
		XPAR_AXIDMA_1_DEVICE_ID,
		&intc_inst,
		XPAR_FABRIC_AXI_DMA_1_S2MM_INTROUT_INTR,
		XPAR_FABRIC_AXI_DMA_1_MM2S_INTROUT_INTR
    );
    if (p_fft_inst_INV == NULL)
    {
		xil_printf("ERROR! Failed to create INV FFT instance.\n\r");
		return -1;
    }
	fft_set_fwd_inv(p_fft_inst_INV, FFT_INVERSE);

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
    result_buf = (cplx_data_t*) malloc(sizeof(cplx_data_t)*FFT_MAX_NUM_PTS);
    if (result_buf == NULL){
		xil_printf("ERROR! Failed to allocate memory for the result buffer.\n\r");
		return -1;
    }

    // Fill stimulus buffer with some signal
    memcpy(input_buf, sig_two_sine_waves, sizeof(cplx_data_t)*FFT_MAX_NUM_PTS);

    // Main control loop
    while (1)
    {
    	// Get command
    	xil_printf("What would you like to do?\n\r");
    	xil_printf("0: FFT Configuration submenu\n\r");
    	xil_printf("1: EQ Configuration submenu\n\r");
    	xil_printf("2: Compressor Configuration submenu\n\r");
    	xil_printf("3: Print STFT stuff\n\r");
    	xil_printf("7/8: Perform FFT / IFFT using current parameters\n\r");
    	xil_printf("9: Run audio system (EQ and compressor)\n\r");
    	//xil_printf("3: Print current INPUT to be used for the FFT operation\n\r");
    	//xil_printf("4: Print current INTERMEDIATE data of FFT operation\n\r");
    	xil_printf("S: Stream Pure Audio\n\r");
    	xil_printf("R/P: Record/Play Audio to/from memory\n\r");
    	//xil_printf("D: Detect Frequency from FFT output\n\r");
    	xil_printf("Q: Quit\n\r");
    	c = XUartPs_RecvByte(XPAR_PS7_UART_1_BASEADDR);

    	if (c == '0')
    	{
    		which_fft_param(&cur_num_fft_pts, p_fft_inst_FWD, p_fft_inst_INV);
    		kenny_update_num_fft_pts(&eq_settings, &stft_settings, cur_num_fft_pts);
    	}
    	else if (c == '1')
    	{
    		kenny_eq_update_interactive(&eq_settings);
    	}
    	else if (c == '2')
    	{
            kenny_compressor_update_interactive(&compressor_settings);
    	}
    	else if (c == '3')
    	{
            kenny_stft_print(&stft_settings);
    	}
    	/*******************************************************/
    	// FFT & FILTERING STUFF
    	else if (c == '7') // Run FFT
		{
            kenny_stft_run_fwd(
                &stft_settings, 
                p_fft_inst_FWD,
                KENNY_AUDIO_IN_MEM_PTR,
                input_buf,
                KENNY_FFTDATA_MEM_PTR,
                intermediate_buf
            );

			xil_printf("FFT complete!\n\r\n\r");
		}
    	else if (c == '8') // Run IFFT
		{
            kenny_stft_run_inv(
                &stft_settings, 
                p_fft_inst_INV,
                KENNY_FFTDATA_MEM_PTR,
                intermediate_buf,
                KENNY_AUDIO_OUT_MEM_PTR,
                result_buf
            );

			xil_printf("Inverse FFT complete!\n\r\n\r");
		}
    	else if (c == '9')		// Run audio system stuff (EQ and Compressor)
    	{
			xil_printf("Press 'd' to enable debug prints, or anything else to disable debug prints.\n\r");
    	    c = XUartPs_RecvByte(XPAR_PS7_UART_1_BASEADDR);

            if (c == 'd') {
    		    kenny_eq_run(&eq_settings, KENNY_FFTDATA_MEM_PTR, 1);
    		    kenny_compressor_run(&compressor_settings, KENNY_FFTDATA_MEM_PTR, 1);
            } else {
    		    kenny_eq_run(&eq_settings, KENNY_FFTDATA_MEM_PTR, 0);
    		    kenny_compressor_run(&compressor_settings, KENNY_FFTDATA_MEM_PTR, 0);
            }
    	}
    	/*******************************************************/
    	else if (c == ',')	// Debugging - print part of the FFT/Audio data.
    	{
        	c = XUartPs_RecvByte(XPAR_PS7_UART_1_BASEADDR);

        	if (c == ',')
        	{
				char str[25];
				cplx_data_t tmp;
				int idx = 0;
				int i = 0;
				for (int jj = 0; jj < cur_num_fft_pts; ++jj)
				{
					idx = i*cur_num_fft_pts + jj;
					tmp = KENNY_FFTDATA_MEM_PTR[idx];

					cplx_data_get_string(str, tmp);
					xil_printf("KDEBUG: fftdata[%d] = %s\n\r", idx, str);
				}
        	}
        	else if (c == '.')
        	{
				int tmp;
				int idx = 0;
				int i = 0;
				for (int jj = 0; jj < cur_num_fft_pts; ++jj)
				{
					idx = i*cur_num_fft_pts + jj;
					tmp = KENNY_AUDIO_OUT_MEM_PTR[idx];

					xil_printf("KDEBUG: audiodata[%d] = %d\n\r", idx, tmp);
				}
        	}
        	else{
        		xil_printf("Invalid input. Try again\r\n");
        	}
    	}
    	/*******************************************************/
    	else if (c == '4')
    	{
    		fft_set_stim_buf(p_fft_inst_FWD, input_buf);
    		fft_print_stim_buf(p_fft_inst_FWD);
    	}
    	else if (c == '5')
    	{
    		fft_print_result_buf(p_fft_inst_FWD, -1);
    	}
    	else if (c == '6')
    	{
    		fft_print_result_buf(p_fft_inst_INV, -1);
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
    		kenny_RecordAudioIntoMem(KENNY_AUDIO_IN_MEM_PTR);
		}
    	else if (c == 'p')
    	{
			xil_printf("Press 'o' to play the ORIGINAL (nonprocessed) sound, or anything else to play processed sound.\n\r");
    	    c = XUartPs_RecvByte(XPAR_PS7_UART_1_BASEADDR);

			xil_printf("PLAYING BACK RECORDED AUDIO\r\n");
			xil_printf("Press 'q' to stop playback early and return to the main menu\r\n");
            if (c == 'o') {
			    kenny_PlaybackAudioFromMem(KENNY_AUDIO_IN_MEM_PTR);
            } else {
			    kenny_PlaybackAudioFromMem(KENNY_AUDIO_OUT_MEM_PTR);
            }
    	}
    	else if (c == 'd')
    	{
    		int guessed_freq = kenny_guessFrequencyOfData(p_fft_inst_FWD);
    		xil_printf("The frequency is around: %d \r\n", guessed_freq);
    	}
    	else if (c == 'q')
    	{
    		xil_printf("Are you sure you want to quit? (YES)\r\n");
        	c = XUartPs_RecvByte(XPAR_PS7_UART_1_BASEADDR);
        	if ( c == 'y' )
        	{
            	c = XUartPs_RecvByte(XPAR_PS7_UART_1_BASEADDR);
            	if (c == 'e')
            	{
                	c = XUartPs_RecvByte(XPAR_PS7_UART_1_BASEADDR);
                	if (c == 's')
                	{
                		break;
                	}
            	}
        	}
    	}
    	else
    	{
    		xil_printf("Invalid character. Please try again.\n\r");
    	}
    }

    free(input_buf);
    free(intermediate_buf);
    free(result_buf);
    //free(KENNY_FFTDATA_MEM_PTR);
    //free(KENNY_AUDIO_IN_MEM_PTR);
    //free(KENNY_AUDIO_OUT_MEM_PTR);
    fft_destroy(p_fft_inst_FWD);
    fft_destroy(p_fft_inst_INV);

    return 0;

}

void which_fft_param(int *cur_num_fft_pts, fft_t* p_fft_inst_FWD, fft_t* p_fft_inst_INV)
{
	// Local variables
	char c;

	while (1)
	{
	    xil_printf("Okay, which FFT parameter would you like to change?\n\r");
	    xil_printf("p: Print current settings\n\r");
	    xil_printf("0: Point length\n\r");
	    //xil_printf("1: Direction\n\r");
	    xil_printf("2: Scaling schedule\n\r");
	    xil_printf("q: Quit back to main menu\n\r");

		c = XUartPs_RecvByte(XPAR_PS7_UART_1_BASEADDR);

    	if (c == 'p')
    	{
    		xil_printf("---------------------------------------------\n\r");
    		fft_print_params(p_fft_inst_FWD);
    		xil_printf("---------------------------------------------\n\r");
    		fft_print_params(p_fft_inst_INV);
    		xil_printf("---------------------------------------------\n\r");
    	}
		else if (c == '0')
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
					*cur_num_fft_pts = 16;
					break;
				}
				else if (c == '1')
				{
					xil_printf("Okay, setting the core to perform a 32-point FFT.\n\r");
					*cur_num_fft_pts = 32;
					break;
				}
				else if (c == '2')
				{
					xil_printf("Okay, setting the core to perform a 64-point FFT.\n\r");
					*cur_num_fft_pts = 64;
					break;
				}
				else if (c == '3')
				{
					xil_printf("Okay, setting the core to perform a 128-point FFT.\n\r");
					*cur_num_fft_pts = 128;
					break;
				}
				else if (c == '4')
				{
					xil_printf("Okay, setting the core to perform a 256-point FFT.\n\r");
					*cur_num_fft_pts = 256;
					break;
				}
				else if (c == '5')
				{
					xil_printf("Okay, setting the core to perform a 512-point FFT.\n\r");
					*cur_num_fft_pts = 512;
					break;
				}
				else if (c == '6')
				{
					xil_printf("Okay, setting the core to perform a 1024-point FFT.\n\r");
					*cur_num_fft_pts = 1024;
					break;
				}
				else if (c == '7')
				{
					xil_printf("Okay, setting the core to perform a 2048-point FFT.\n\r");
					*cur_num_fft_pts = 2048;
					break;
				}
				else if (c == '8')
				{
					xil_printf("Okay, setting the core to perform a 4096-point FFT.\n\r");
					*cur_num_fft_pts = 4096;
					break;
				}
				else if (c == '9')
				{
					xil_printf("Okay, setting the core to perform a 8192-point FFT.\n\r");
					*cur_num_fft_pts = 8192;
					break;
				}
				else
		    		xil_printf("Invalid character. Please try again.\n\r");
			}
    		fft_set_num_pts(p_fft_inst_FWD, *cur_num_fft_pts);
    		fft_set_num_pts(p_fft_inst_INV, *cur_num_fft_pts);
		}
		/*
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
		*/
		else if (c == '2')
		{
			fft_t* target_fft = NULL;
			xil_printf("Would you like to apply this to the forward (f) or reverse (r) FFT, or BOTH (b)?\n\r");
			while (1)
			{
				c = XUartPs_RecvByte(XPAR_PS7_UART_1_BASEADDR);
				if (c == 'f'){
					target_fft = p_fft_inst_FWD;
					break;
				}
				else if (c == 'r'){
					target_fft = p_fft_inst_INV;
					break;
				}
				else if (c == 'b'){
					target_fft = NULL;
					break;
				}
				else{
					xil_printf("Invalid entry, try again.\n\r");
				}
			}

			int new_sched = 0;
			int lshift_amount = 0;

			xil_printf("What would you like the scaling schedule to become? Enter in hex:\n\r");
			for (int input_num = 0; input_num < 3; ++input_num)
			{
				c = XUartPs_RecvByte(XPAR_PS7_UART_1_BASEADDR);

				lshift_amount = (4 * (3 - input_num - 1));
				switch (c){
					case '0':
						new_sched += 0;
						break;
					case '1':
						new_sched += 1 << lshift_amount;
						break;
					case '2':
						new_sched += 2 << lshift_amount;
						break;
					case '3':
						new_sched += 3 << lshift_amount;
						break;
					case '4':
						new_sched += 4 << lshift_amount;
						break;
					case '5':
						new_sched += 5 << lshift_amount;
						break;
					case '6':
						new_sched += 6 << lshift_amount;
						break;
					case '7':
						new_sched += 7 << lshift_amount;
						break;
					case '8':
						new_sched += 8 << lshift_amount;
						break;
					case '9':
						new_sched += 9 << lshift_amount;
						break;
					case 'a':
						new_sched += 10 << lshift_amount;
						break;
					case 'b':
						new_sched += 11 << lshift_amount;
						break;
					case 'c':
						new_sched += 12 << lshift_amount;
						break;
					case 'd':
						new_sched += 13 << lshift_amount;
						break;
					case 'e':
						new_sched += 14 << lshift_amount;
						break;
					case 'f':
						new_sched += 15 << lshift_amount;
						break;
					default:
						input_num = -1;
						xil_printf("INVALID ENTRY. Please try again. Enter in hex:\n\r");
						break;
				}
			}
			xil_printf("New scaling schedule: 0x%x\n\r", new_sched);

			if (target_fft != NULL) {
				fft_set_scale_sch(target_fft, new_sched);
			} else {
				fft_set_scale_sch(p_fft_inst_FWD, new_sched);
				fft_set_scale_sch(p_fft_inst_INV, new_sched);
			}
		}
		else if (c == 'q')
		{
			return;
		}
		else
		{
			xil_printf("Invalid character. Please try again.\n\r");
	        xil_printf("p: Print current settings\n\r");
	        xil_printf("0: Point length\n\r");
	        //xil_printf("1: Direction\n\r");
	        xil_printf("2: Scaling schedule\n\r");
	        xil_printf("q: Exit\n\r");
		}
	}
}

