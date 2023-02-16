
#include "kenny_audio.h"

extern int sig_two_sine_waves[FFT_MAX_NUM_PTS]; // FFT input data


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


/********************************************/

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

	xil_printf("KDEBUG: inval ptr = %x \n\r", inval);

	for (int in_idx = 0, out_idx = 0;
		 in_idx < num_vals_to_cpy*AUDIO_CHANNELS;
		 in_idx += AUDIO_CHANNELS, ++out_idx)
	{
		// Convert input data to mono (assume 2 input channels)
		cur_re_int = (inval[in_idx] + inval[in_idx+1]) / 2;
		cur_re_int = kenny_signextend_24bit(cur_re_int);
		cur_re_int = cur_re_int >> 8;
		cur_re = cur_re_int;
		cur_im = 0;
		cur_cplx.data_re = cur_re;
		cur_cplx.data_im = cur_im;
		outval[out_idx] = cur_cplx;

//		if (out_idx < 5){
//			xil_printf("outval[out_idx] = %d, %d*j \n\r", outval[out_idx].data_re, outval[out_idx].data_im);
//		}
	}
}

void kenny_convertCplxToAudio(cplx_data_t* inval, int* outval, size_t num_vals_to_cpy)
{
	cplx_data_t cur_cplx;
	int cur_re_int;
	short cur_re = 0;
	for (int in_idx = 0, out_idx = 0;
		 in_idx < num_vals_to_cpy;
		 ++in_idx, out_idx += AUDIO_CHANNELS)
	{
		cur_cplx = inval[in_idx];
		cur_re = cur_cplx.data_re;	// In testing, I find that ignoring the imaginary part is ok. But maybe it should be re + im.
		cur_re_int = (int) cur_re;

		// Write to output channels (assume 2 channels)
		outval[out_idx] = cur_re_int;
		outval[out_idx+1] = cur_re_int;
	}
}


//void kenny_updateFFT_InputData(cplx_data_t* stim_buf, int* recorded_audio_buf)
//{
//	char c = '\0';
//
//	xil_printf("Which input data would you like to use?\n\r");
//	xil_printf("0: Recorded Audio\n\r");
//	xil_printf("1: Generated Values from Xilinx\n\r");
//	xil_printf("2: Exit\n\r");
//	while (1)
//	{
//		c = XUartPs_RecvByte(XPAR_PS7_UART_1_BASEADDR);
//		if (c == '0')
//		{
//			kenny_convertAudioToCplx(recorded_audio_buf, stim_buf, sizeof(cplx_data_t)*FFT_MAX_NUM_PTS);
//			break;
//		}
//		else if (c == '1')
//		{
//		    memcpy(stim_buf, sig_two_sine_waves, sizeof(cplx_data_t)*FFT_MAX_NUM_PTS);
//			break;
//		}
//		else if (c == '2')
//		{
//			break;
//		}
//		else
//    	{
//    		xil_printf("Invalid character. Please try again.\n\r");
//    	}
//	}
//}


int kenny_guessFrequencyOfData(fft_t* p_fft_inst)
{
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


