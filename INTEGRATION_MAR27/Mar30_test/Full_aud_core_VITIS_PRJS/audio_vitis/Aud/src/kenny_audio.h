
#ifndef __KENNY_AUDIO_H__
#define __KENNY_AUDIO_H__

#include <stdio.h>
#include <stdlib.h>
#include <k_aud_cmprs_configurator.h>
#include <eq_config.h>
#include <gain_config.h>
#include "platform.h"
#include "xuartps_hw.h"
#include "fft.h"
#include "cplx_data.h"
#include "adventures_with_ip.h"

#define SAMPLE_SLEEP_USEC (21)

#define AUDIO_SAMPLE_RATE (48000)
#define AUDIO_CHANNELS (2)

#define EQ_MAX_NUM_FREQ_BUCKETS (16)

/// NOTE: As of March 31, we don't support stride factor of 1.
#define STFT_STRIDE_FACTOR (2)

#define KENNY_AUDIO_MAX_SAMPLES (FFT_MAX_NUM_PTS/STFT_STRIDE_FACTOR * AUDIO_CHANNELS)
//#define MAX_RECORD_SEC (4)
//#define KENNY_AUDIO_MAX_SAMPLES (AUDIO_SAMPLE_RATE * MAX_RECORD_SEC * AUDIO_CHANNELS)
//#define KENNY_FFTDATA_SZ ((KENNY_AUDIO_MAX_SAMPLES/AUDIO_CHANNELS) * STFT_STRIDE_FACTOR)



// REGISTERS
#define CMPRS_THRESHOLD_REG     (K_AUD_CMPRS_CONFIGURATOR_S00_AXI_SLV_REG0_OFFSET)
#define CMPRS_RATIO_REG         (K_AUD_CMPRS_CONFIGURATOR_S00_AXI_SLV_REG1_OFFSET)
#define CMPRS_NUM_FFT_PTS_REG   (K_AUD_CMPRS_CONFIGURATOR_S00_AXI_SLV_REG2_OFFSET)
#define CMPRS_BYPASS_REG        (K_AUD_CMPRS_CONFIGURATOR_S00_AXI_SLV_REG3_OFFSET)
#define EQ_READY_REG            (EQ_CONFIG_S00_AXI_SLV_REG0_OFFSET)
#define EQ_NUM_PTS_REG          (EQ_CONFIG_S00_AXI_SLV_REG1_OFFSET)
#define EQ_NUM_BKTS_REG         (EQ_CONFIG_S00_AXI_SLV_REG2_OFFSET)
#define EQ_VECTOR_START_ADDR    (EQ_CONFIG_S00_AXI_SLV_REG3_OFFSET)
#define GAIN_OUTPUT_GAIN_REG    (GAIN_CONFIG_S00_AXI_SLV_REG0_OFFSET)


//int EQ_cur_num_freq_buckets;

typedef struct kenny_stft_settings
{
    int num_fft_pts;
    int doublebuff_idx;
    int windows[2][FFT_MAX_NUM_PTS];
    float STFT_window_func[FFT_MAX_NUM_PTS];
} stft_settings_t;

typedef struct kenny_compressor_settings
{
    int     ratio;
    int     threshold_energy;
    int     bypass;
    //int     num_pts;
    stft_settings_t* p_stft_settings;
} compressor_settings_t;

typedef struct kenny_gain_settings
{
    float   output_gain;
    int     bypass;
} gain_settings_t;

typedef struct kenny_eq_settings
{
    int EQ_cur_num_freq_buckets;
    float parametric_eq_vect[EQ_MAX_NUM_FREQ_BUCKETS];
    char bypass;
    stft_settings_t* p_stft_settings;
} eq_settings_t;


eq_settings_t               eq_settings;
gain_settings_t             gain_settings;
compressor_settings_t       compressor_settings;


/****************************/
/****************************/
/****************************/

void audio_stream();

int32_t float_to_fixed_point(float value);

/******************/
void kenny_stft_print(stft_settings_t *p_stft_settings);
void kenny_stft_init(stft_settings_t *p_stft_settings);
void kenny_stft_update_window_func(stft_settings_t *p_stft_settings, int new_num_fft_pts);

void kenny_stft_run_fwd(
                stft_settings_t *p_stft_settings, 
                fft_t* p_fft_inst_FWD,
                int* inval,
                cplx_data_t* input_buf,
                cplx_data_t* outval,
                cplx_data_t* output_buf);

void kenny_stft_run_fwd_and_inv(
                stft_settings_t *p_stft_settings, 
                fft_t* p_fft_inst_FWD,
                int* KENNY_AUDIO_IN_MEM_PTR,
                cplx_data_t* input_buf,
                int* KENNY_AUDIO_OUT_MEM_PTR,
                cplx_data_t* output_buf);

void kenny_stft_run_inv(
                stft_settings_t *p_stft_settings, 
                fft_t* p_fft_inst_INV,
                cplx_data_t* KENNY_FFTDATA_MEM_PTR,
                cplx_data_t* input_buf,
                int* KENNY_AUDIO_MEM_PTR,
                cplx_data_t* output_buf);

/******************/
void kenny_eq_init(eq_settings_t *p_eq_settings, stft_settings_t *p_stft_settings);
void kenny_eq_print(eq_settings_t *eq_settings);
void kenny_eq_update_interactive(eq_settings_t *eq_settings);
void kenny_eq_update_hardware(eq_settings_t *eq_settings);

/******************/
void kenny_gain_init(gain_settings_t *p_gain_settings);
void kenny_gain_print(gain_settings_t *p_gain_settings);
void kenny_gain_update_interactive(gain_settings_t *p_gain_settings);
void kenny_gain_update_hardware(gain_settings_t *p_gain_settings);

/******************/
void kenny_compressor_init(
                compressor_settings_t *p_compressor_settings,
                stft_settings_t *p_stft_settings);
void kenny_compressor_update_interactive(compressor_settings_t *p_compressor_settings);
void kenny_compressor_update_hardware(compressor_settings_t *p_compressor_settings);
void kenny_compressor_print(compressor_settings_t *p_compressor_settings);

/******************/
void kenny_update_num_fft_pts(eq_settings_t *p_eq_settings, stft_settings_t *p_stft_settings, int new_num_fft_pts);

/******************/
void kenny_PlaybackAudioFromMem(const int* KENNY_AUDIO_MEM_PTR);
void kenny_RecordAudioIntoMem(const int* KENNY_AUDIO_MEM_PTR);
unsigned int kenny_cplx_get_magnitude_squared(cplx_data_t input);
void kenny_apply_filter(int num_fft_pts, float filter[num_fft_pts], cplx_data_t* fft_data);

int kenny_guessFrequencyOfData(fft_t* p_fft_inst);
void kenny_convertAudioToCplx(int* inval, cplx_data_t* outval, size_t num_vals_to_cpy);
void kenny_convertCplxToAudio(cplx_data_t* inval, int* outval, size_t num_vals_to_cpy);


#endif
