
#ifndef __KENNY_AUDIO_H__
#define __KENNY_AUDIO_H__

#include <stdio.h>
#include <stdlib.h>
#include "platform.h"
#include "xuartps_hw.h"
#include "fft.h"
#include "cplx_data.h"
#include "adventures_with_ip.h"

#define SAMPLE_SLEEP_USEC (21)

#define AUDIO_SAMPLE_RATE (48000)
#define AUDIO_CHANNELS (2)
#define MAX_RECORD_SEC (2)
#define KENNY_AUDIO_MAX_SAMPLES (AUDIO_SAMPLE_RATE * MAX_RECORD_SEC * AUDIO_CHANNELS)
//#define KENNY_AUDIO_MAX_SAMPLES (8192)


void audio_stream();
void kenny_PlaybackAudioFromMem(const int* KENNY_AUDIO_MEM_PTR);
void kenny_RecordAudioIntoMem(const int* KENNY_AUDIO_MEM_PTR);

int kenny_guessFrequencyOfData(fft_t* p_fft_inst);
//void kenny_updateFFT_InputData(cplx_data_t* stim_buf, int* recorded_audio_buf);
void kenny_convertAudioToCplx(int* inval, cplx_data_t* outval, size_t num_vals_to_cpy);
void kenny_convertCplxToAudio(cplx_data_t* inval, int* outval, size_t num_vals_to_cpy);


#endif
