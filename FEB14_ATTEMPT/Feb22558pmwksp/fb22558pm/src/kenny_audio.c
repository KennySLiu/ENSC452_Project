
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

void kenny_init_eq(float parametric_eq_vect[EQ_MAX_NUM_FREQ_BUCKETS])
{
	for (int i = 0; i < EQ_MAX_NUM_FREQ_BUCKETS; ++i)
	{
		parametric_eq_vect[i] = 1.0;
	}
}

void kenny_print_eq(float parametric_eq_vect[EQ_cur_num_freq_buckets])
{
	for (int eq_idx = 0; eq_idx < EQ_cur_num_freq_buckets; ++eq_idx)
	{
		printf("%1.1f\t", parametric_eq_vect[eq_idx]);
	}
	printf("\r\n");
}

void kenny_update_eq(float parametric_eq_vect[EQ_MAX_NUM_FREQ_BUCKETS])
{
	xil_printf("What would you like to do?\r\n");
	xil_printf("1: Print current EQ settings\r\n");
	xil_printf("2: Modify EQ settings\r\n");
	char c;

	while(1)
	{
		c = XUartPs_RecvByte(XPAR_PS7_UART_1_BASEADDR);

		if (c == '1') {
			kenny_print_eq(parametric_eq_vect);
			break;
		}
		else if (c == '2') {
			xil_printf("Please enter the new values for the EQ. (ones, tenths)");
			for (int eq_idx = 0; eq_idx < EQ_cur_num_freq_buckets; ++eq_idx)
			{
				float cur_eq_val = 0;
				for (int dec_ctr = 0; dec_ctr < 2; ++dec_ctr)
				{
					float divider = (dec_ctr == 0 ? 1 : 10);
					c = XUartPs_RecvByte(XPAR_PS7_UART_1_BASEADDR);
					switch (c){
						case '0':
							cur_eq_val += 0;
							break;
						case '1':
							cur_eq_val += 1.0/divider;
							break;
						case '2':
							cur_eq_val += 2.0/divider;
							break;
						case '3':
							cur_eq_val += 3.0/divider;
							break;
						case '4':
							cur_eq_val += 4.0/divider;
							break;
						case '5':
							cur_eq_val += 5.0/divider;
							break;
						case '6':
							cur_eq_val += 6.0/divider;
							break;
						case '7':
							cur_eq_val += 7.0/divider;
							break;
						case '8':
							cur_eq_val += 8.0/divider;
							break;
						case '9':
							cur_eq_val += 9.0/divider;
							break;
					}
				}
				parametric_eq_vect[eq_idx] = cur_eq_val;
			}
			for (int eq_idx = EQ_cur_num_freq_buckets; eq_idx < EQ_MAX_NUM_FREQ_BUCKETS; ++eq_idx)
			{
				parametric_eq_vect[eq_idx] = 1.0;
			}
			xil_printf("Done. Your new EQ vector is:\r\n");
			kenny_print_eq(parametric_eq_vect);
			xil_printf("Where %d are valid values.\r\n", EQ_cur_num_freq_buckets);
			break;
		}
		else
		{
			xil_printf("Invalid entry. Please try again \r\n");
		}
	}
}

void kenny_PlaybackAudioFromMem(const int* KENNY_AUDIO_MEM_PTR)
{
	u32  in_left, in_right;
	int * cur_ptr = KENNY_AUDIO_MEM_PTR;
	u32 num_samples_played = 0;

	while (1)
	{
		if (!XUartPs_IsReceiveData(UART_BASEADDR)){
			// Read audio data from memory
			in_left  = *(cur_ptr++);
			in_right = *(cur_ptr++);
			num_samples_played += 2;

			// Write audio data to audio codec
			Xil_Out32(I2S_DATA_TX_L_REG, in_left);
			Xil_Out32(I2S_DATA_TX_R_REG, in_right);

			usleep(SAMPLE_SLEEP_USEC);

			if (num_samples_played >= KENNY_AUDIO_MAX_SAMPLES-1){
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
// HELPER DATA FUNCTIONS:
int kenny_signextend_24bit_to_int(int inval){
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

	//xil_printf("KDEBUG: inval ptr = %x \n\r", inval);

	for (int in_idx = 0, out_idx = 0;
		 in_idx < num_vals_to_cpy*AUDIO_CHANNELS;
		 in_idx += AUDIO_CHANNELS, ++out_idx)
	{
		// Convert input data to mono (assume 2 input channels)
		//cur_re_int = (inval[in_idx] + inval[in_idx+1]) / 2;
		cur_re_int = inval[in_idx];
		cur_re_int = kenny_signextend_24bit_to_int(cur_re_int);
		cur_re_int = cur_re_int >> 8;
		cur_re = cur_re_int;
		cur_im = 0;
		cur_cplx.data_re = cur_re;
		cur_cplx.data_im = cur_im;
		outval[out_idx] = cur_cplx;
	}
}

// THIS FUNCTION ASSUMES the input is already representable in 24 bits.
int kenny_convert_short_to_24bit(short inval){
	int retval = 0;
	retval = 0x00ffffff & inval; // Mask out the higher bits.
	retval = retval << 12;		// scale it up so it's audible
	return retval;
}

void kenny_convertCplxToAudio(cplx_data_t* inval, int* outval, float *STFT_window_func, size_t num_vals_to_cpy)
{
	cplx_data_t cur_cplx;
	int cur_re_int;
	short cur_re = 0;
	//xil_printf("KDEBUG: outval ptr = %x \n\r", outval);
	for (int in_idx = 0, out_idx = 0;
		 in_idx < num_vals_to_cpy;
		 ++in_idx, out_idx += AUDIO_CHANNELS)
	{
		cur_cplx = inval[in_idx];
		cur_re = cur_cplx.data_re;	// In testing, I find that ignoring the imaginary part is ok. But maybe it should be re + im.
		cur_re_int = kenny_convert_short_to_24bit(cur_re);

		// Write to output channels (assume 2 channels)
		outval[out_idx] 	+= cur_re_int * STFT_window_func[in_idx];
		outval[out_idx+1] 	+= cur_re_int * STFT_window_func[in_idx];
	}
}



// NOTE: The filter should be symmetric.
void kenny_apply_filter(int num_fft_pts, float filter[num_fft_pts], cplx_data_t* fft_data)
{
	for (int i = 0 ; i < num_fft_pts; ++i)
	{
		fft_data[i].data_re *= filter[i];
		fft_data[i].data_im *= filter[i];
	}
}

/*************************************/

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


