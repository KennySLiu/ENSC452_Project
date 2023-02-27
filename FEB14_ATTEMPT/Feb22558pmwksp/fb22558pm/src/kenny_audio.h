
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
#define KENNY_FFTDATA_SZ ((KENNY_AUDIO_MAX_SAMPLES/AUDIO_CHANNELS) * STFT_STRIDE_FACTOR)
//#define KENNY_AUDIO_MAX_SAMPLES (8192)

#define EQ_MAX_NUM_FREQ_BUCKETS (16)
#define STFT_STRIDE_FACTOR (2)
int EQ_cur_num_freq_buckets;



typedef struct kenny_stft_settings
{
    int num_fft_windows;
    int num_fft_pts;
	float STFT_window_func[FFT_MAX_NUM_PTS];
} stft_settings_t;

typedef struct kenny_compressor_settings
{
	float ratio;
	unsigned int threshold_energy;
	char bypass;
    stft_settings_t* p_stft_settings;
} compressor_settings_t;

typedef struct kenny_eq_settings
{
	int EQ_cur_num_freq_buckets;
	float parametric_eq_vect[EQ_MAX_NUM_FREQ_BUCKETS];
	char bypass;
    stft_settings_t* p_stft_settings;
} eq_settings_t;

/****************************/
/****************************/
/****************************/

void audio_stream();

void kenny_stft_init(stft_settings_t *p_stft_settings);
void kenny_stft_update_window(stft_settings_t *p_stft_settings, int new_num_fft_pts);
void kenny_stft_convert_window_to_audiodata(
    stft_settings_t *p_stft_settings, 
    cplx_data_t* inval, 
    int* outval, 
    size_t num_vals_to_cpy);
void kenny_stft_run_fwd_fft(
    stft_settings_t *p_stft_settings, 
    fft_t* p_fft_inst_FWD,
    int* inval,
    cplx_data_t* input_buf,
    cplx_data_t* outval,
    cplx_data_t* output_buf);

void kenny_eq_init(eq_settings_t *p_eq_settings, stft_settings_t *p_stft_settings);
void kenny_eq_print(eq_settings_t *eq_settings);
void kenny_eq_update_buckets(eq_settings_t *eq_settings, int new_num_fft_pts);
void kenny_eq_update_interactive(eq_settings_t *eq_settings);
void kenny_eq_run(eq_settings_t *eq_settings, cplx_data_t KENNY_FFTDATA_MEM_PTR[KENNY_FFTDATA_SZ]);

void kenny_update_num_fft_pts(eq_settings_t *p_eq_settings, stft_settings_t *p_stft_settings, int new_num_fft_pts);

void kenny_PlaybackAudioFromMem(const int* KENNY_AUDIO_MEM_PTR);
void kenny_RecordAudioIntoMem(const int* KENNY_AUDIO_MEM_PTR);
void kenny_apply_filter(int num_fft_pts, float filter[num_fft_pts], cplx_data_t* fft_data);

int kenny_guessFrequencyOfData(fft_t* p_fft_inst);
void kenny_convertAudioToCplx(int* inval, cplx_data_t* outval, size_t num_vals_to_cpy);
void kenny_convertCplxToAudio(cplx_data_t* inval, int* outval, float *STFT_window_func, size_t num_vals_to_cpy);


#endif
