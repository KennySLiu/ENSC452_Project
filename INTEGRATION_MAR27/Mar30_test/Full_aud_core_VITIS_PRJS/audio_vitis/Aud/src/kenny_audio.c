
#include "kenny_audio.h"

extern int sig_two_sine_waves[FFT_MAX_NUM_PTS]; // FFT input data


int32_t float_to_fixed_point(float value)
{
    const int fract_bits = 8;
    //const int integer_bits = 16;
    const float scale_factor = 1 << fract_bits;

    int32_t fixed_pt = (int32_t)(value*scale_factor);

    return fixed_pt;
}

/********************/
/********************/
/********************/

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

/******************************/
// STFT SETTINGS
void kenny_stft_print(stft_settings_t *p_stft_settings)
{
    xil_printf("\tSTFT num pts = %d\r\n", p_stft_settings->num_fft_pts);
}

void kenny_stft_init(stft_settings_t *p_stft_settings)
{
    p_stft_settings->num_fft_pts = INIT_NUM_FFT_PTS;
    kenny_stft_update_window_func(p_stft_settings, INIT_NUM_FFT_PTS);
    p_stft_settings->doublebuff_idx = 0;
    
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < FFT_MAX_NUM_PTS; ++j)
        {
            p_stft_settings->windows[i][j].data_re = 0;
            p_stft_settings->windows[i][j].data_im = 0;
        }
    }
}

void kenny_stft_update_window_func(stft_settings_t *p_stft_settings, int new_num_fft_pts)
{
    p_stft_settings->num_fft_pts = new_num_fft_pts;

    if (STFT_STRIDE_FACTOR == 1)
    {
        for (int i = 0; i < new_num_fft_pts; ++i){
            p_stft_settings->STFT_window_func[i] = 1.0;
        }
    }
    else if (STFT_STRIDE_FACTOR == 2)
    {
        for (int i = 0; i < new_num_fft_pts/2; ++i){
            p_stft_settings->STFT_window_func[i] = 1 / (new_num_fft_pts/2.0 / (i+1));
        }
        p_stft_settings->STFT_window_func[new_num_fft_pts/2] = 1;
        for (int i = new_num_fft_pts/2+1; i < new_num_fft_pts; ++i) {
            p_stft_settings->STFT_window_func[i] = p_stft_settings->STFT_window_func[new_num_fft_pts - i - 1];
        }
    }

    #ifdef __DEBUGGING__
    for (int i = 0; i < new_num_fft_pts; ++i)
    {
        printf("STFT Window[%d] = %f\r\n", i, p_stft_settings->STFT_window_func[i]);
    }
    #endif
}

void kenny_stft_apply_window(
                stft_settings_t *p_stft_settings,
                cplx_data_t* input_buf,
                cplx_data_t* output_buf
){
    cplx_data_t in_cplx, out_cplx;
    float cur_multiplier;
    int in_re, in_im, out_re, out_im;
    for (int idx = 0; idx < p_stft_settings->num_fft_pts; ++idx)
    {
        in_cplx = input_buf[idx];
        in_re = in_cplx.data_re;
        in_im = in_cplx.data_im;
        //cur_multiplier = (p_stft_settings->STFT_window_func[idx]);
        cur_multiplier = 0.5;

        out_re = in_re*cur_multiplier;
        out_im = in_im*cur_multiplier;
        out_cplx.data_re = out_re;
        out_cplx.data_im = out_im;

        output_buf[idx] = out_cplx;

        #ifdef __DEBUGGING__
        printf("STFT Applying Window: idx = %05d, input_buf = %09d. output_buf = %09d. Multiplier = %f\r\n",
                idx, input_buf[idx], output_buf[idx], cur_multiplier
        );
        #endif
                
    }
}

void kenny_stft_combine_half_windows(
                stft_settings_t *p_stft_settings,
                cplx_data_t* secondhalf_buf,
                cplx_data_t* firsthalf_buf,
                cplx_data_t* output_buf
){
    int half_window_sz = p_stft_settings->num_fft_pts/2;
    cplx_data_t in_cplx_1, in_cplx_2, out_cplx;
    int in_1, in_2, out;

    for (int i = 0; i < half_window_sz; ++i)
    {
        in_cplx_1 = secondhalf_buf[half_window_sz-1 + i];
        in_cplx_2 = firsthalf_buf[i];

        // real part
        in_1 = in_cplx_1.data_re;
        in_2 = in_cplx_2.data_re;
        out = in_1 + in_2;
        out_cplx.data_re = out;

        // imag part
        in_1 = in_cplx_1.data_im;
        in_2 = in_cplx_2.data_im;
        out = in_1 + in_2;
        out_cplx.data_im = out;

        output_buf[i] = out_cplx;
    }
}

void kenny_stft_run_fwd_and_inv(
                stft_settings_t *p_stft_settings, 
                fft_t* p_fft_inst_FWD,
                int* KENNY_AUDIO_IN_MEM_PTR,
                cplx_data_t* fft_input_buf,
                int* KENNY_AUDIO_OUT_MEM_PTR,
                cplx_data_t* fft_output_buf
) {
    int status = 0;
    int num_fft_pts = p_stft_settings->num_fft_pts;
    int doublebuff_idx_1 = p_stft_settings->doublebuff_idx;
    int doublebuff_idx_2 = !doublebuff_idx_1;
    //int AUDIO_IDX_FACTOR = AUDIO_CHANNELS*num_fft_pts/STFT_STRIDE_FACTOR;

    memset(KENNY_AUDIO_OUT_MEM_PTR, 0, sizeof(int)*num_fft_pts/2);

    // RUN THE FFT/IFFT
    kenny_convertAudioToCplx(&(KENNY_AUDIO_IN_MEM_PTR[0]), fft_input_buf, num_fft_pts);
    // Make sure the output buffer is clear before we populate it (this is generally not necessary and wastes time doing memory accesses, but for proving the DMA working, we do it anyway)
    memset(fft_output_buf, 0, sizeof(cplx_data_t)*FFT_MAX_NUM_PTS);

    status = fft(p_fft_inst_FWD, (cplx_data_t*)fft_input_buf, (cplx_data_t*)fft_output_buf);
    if (status != FFT_SUCCESS)
    {
        xil_printf("\r\n\r\n !!!!!!!!!!!!!!ERROR!!!!!!!!!!!!! FFT failed.\n\r\n\r\n\r");
        return;
    }

    // Window the current FFT output into one of our STFT doublebuffers
    kenny_stft_apply_window(
        p_stft_settings,
        fft_output_buf,
        p_stft_settings->windows[doublebuff_idx_1]
    );

    // Combine our already-windowed buffers.
    kenny_stft_combine_half_windows(
        p_stft_settings,
        // THIS ORDERING MATTERS. Do not switch.
        p_stft_settings->windows[doublebuff_idx_2],
        p_stft_settings->windows[doublebuff_idx_1],
        fft_output_buf
    );

    kenny_convertCplxToAudio(
        fft_output_buf,
        &(KENNY_AUDIO_OUT_MEM_PTR[0]),
        num_fft_pts/2
    );

    p_stft_settings->doublebuff_idx = !p_stft_settings->doublebuff_idx;
}



/******************************/
// PARAMETRIC EQ

void kenny_eq_init(eq_settings_t *p_eq_settings, stft_settings_t *p_stft_settings)
{
    p_eq_settings->EQ_cur_num_freq_buckets = (int) log2f(INIT_NUM_FFT_PTS);
    p_eq_settings->bypass = 0;

    p_eq_settings->parametric_eq_vect[0 ] = 0.0;
    p_eq_settings->parametric_eq_vect[1 ] = 0.0;
    p_eq_settings->parametric_eq_vect[2 ] = 0.0;
    p_eq_settings->parametric_eq_vect[3 ] = 0.0;
    p_eq_settings->parametric_eq_vect[4 ] = 0.0;
    p_eq_settings->parametric_eq_vect[5 ] = 0.0;
    p_eq_settings->parametric_eq_vect[6 ] = 0.0;
    p_eq_settings->parametric_eq_vect[7 ] = 4.0;
    p_eq_settings->parametric_eq_vect[8 ] = 4.0;
    p_eq_settings->parametric_eq_vect[9 ] = 4.0;
    p_eq_settings->parametric_eq_vect[10] = 0.0;
    p_eq_settings->parametric_eq_vect[11] = 0.0;
    p_eq_settings->parametric_eq_vect[12] = 0.0;
    p_eq_settings->parametric_eq_vect[13] = 0.0;
    p_eq_settings->parametric_eq_vect[14] = 0.0;
    p_eq_settings->parametric_eq_vect[15] = 0.0;


    //for (int i = 0; i < EQ_MAX_NUM_FREQ_BUCKETS; ++i)
    //{
    //    p_eq_settings->parametric_eq_vect[i] = 1.0;
    //}
    p_eq_settings->p_stft_settings = p_stft_settings;

    kenny_eq_update_hardware(p_eq_settings);
}

void kenny_eq_print_bypass(eq_settings_t *p_eq_settings){
    xil_printf("The EQ Is %s\r\n", (p_eq_settings->bypass ? "DISABLED" : "ENABLED"));
}
void kenny_eq_print_vector(eq_settings_t *p_eq_settings){
    xil_printf("The EQ vector is:\r\n");
    for (int eq_idx = 0; eq_idx < p_eq_settings->EQ_cur_num_freq_buckets; ++eq_idx)
    {
        printf("%1.1f\t", p_eq_settings->parametric_eq_vect[eq_idx]);
    }
    printf("\r\n");
}

void kenny_eq_print(eq_settings_t *p_eq_settings)
{
    kenny_eq_print_bypass(p_eq_settings);
    kenny_eq_print_vector(p_eq_settings);
}

void kenny_eq_update_hardware(eq_settings_t *p_eq_settings)
{
    int32_t cur_num_fft_pts = (p_eq_settings->p_stft_settings)->num_fft_pts;
    int32_t cur_num_bkts = p_eq_settings->EQ_cur_num_freq_buckets;

    EQ_CONFIG_mWriteReg(
        XPAR_EQ_CONFIG_0_S00_AXI_BASEADDR, 
        EQ_NUM_PTS_REG, 
        (u32)cur_num_fft_pts
    );
    EQ_CONFIG_mWriteReg(
        XPAR_EQ_CONFIG_0_S00_AXI_BASEADDR, 
        EQ_NUM_BKTS_REG, 
        (u32)cur_num_bkts
    );

    for (int i = 0; i < cur_num_bkts; ++i)
    {
        int32_t cur_bkt_multiplier;
        if (p_eq_settings->bypass == 0)
        {
            cur_bkt_multiplier = float_to_fixed_point(p_eq_settings->parametric_eq_vect[i]);
        }
        else
        {
            cur_bkt_multiplier = float_to_fixed_point(1.0);
        }
        EQ_CONFIG_mWriteReg(
            XPAR_EQ_CONFIG_0_S00_AXI_BASEADDR, 
            EQ_VECTOR_START_ADDR + (4*i), 
            (u32) cur_bkt_multiplier
        );
    }
    EQ_CONFIG_mWriteReg(
        XPAR_EQ_CONFIG_0_S00_AXI_BASEADDR, 
        EQ_READY_REG,
        (u32) 1
    );

    printf("Updated EQ hardware with the following settings:\r\n");
    printf("    cur_num_pts = %ld\r\n", cur_num_fft_pts);
    printf("    cur_num_bkts = %ld\r\n", cur_num_bkts);
    printf("    bypass = %d\r\n", p_eq_settings->bypass);



    for (int i = 0; i < 14; ++i)
    {
        int32_t read_value = 0;
        read_value = EQ_CONFIG_mReadReg(XPAR_EQ_CONFIG_0_S00_AXI_BASEADDR, 4*i);

        printf("EQ CONFIGURATION READ: i = %d, value = %ld\r\n", i, read_value);
    }
}

void kenny_eq_update_interactive(eq_settings_t *p_eq_settings)
{
    char c;

    while(1)
    {
        xil_printf("What would you like to do?\r\n");
        xil_printf("p: Print current EQ settings\r\n");
        xil_printf("1: Modify EQ values\r\n");
        xil_printf("2: Toggle EQ Bypass\r\n");
        xil_printf("q: Quit back to main menu\r\n");
        c = XUartPs_RecvByte(XPAR_PS7_UART_1_BASEADDR);

        if (c == 'p') {
            kenny_eq_print(p_eq_settings);
        }
        else if (c == '1') {
            xil_printf("Please enter the new values for the EQ. (ones, tenths)\r\n");
            for (int eq_idx = 0; eq_idx < p_eq_settings->EQ_cur_num_freq_buckets; ++eq_idx)
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
                p_eq_settings->parametric_eq_vect[eq_idx] = cur_eq_val;
            }
            for (int eq_idx = p_eq_settings->EQ_cur_num_freq_buckets; eq_idx < EQ_MAX_NUM_FREQ_BUCKETS; ++eq_idx)
            {
                p_eq_settings->parametric_eq_vect[eq_idx] = 1.0;
            }
            kenny_eq_print(p_eq_settings);
        }
        else if (c == '2') {
            p_eq_settings->bypass = (p_eq_settings->bypass ? 0 : 1);
            kenny_eq_print_bypass(p_eq_settings);
        }
        else if (c == 'q') {
            kenny_eq_update_hardware(p_eq_settings);
            break;
        }
        else
        {
            xil_printf("Invalid entry. Please try again \r\n");
        }
    }
}

/******************************/
// GAIN FUNCTIONS
void kenny_gain_print_bypass(gain_settings_t *p_gain_settings)
{
    printf("The output gain controller is %s\r\n", (p_gain_settings->bypass ? "DISABLED" : "ENABLED"));
}
void kenny_gain_print_gain(gain_settings_t *p_gain_settings)
{
    printf("    gain = %ld\r\n", p_gain_settings->output_gain);
}

void kenny_gain_print(gain_settings_t *p_gain_settings)
{
    kenny_gain_print_bypass(p_gain_settings);
    kenny_gain_print_gain(p_gain_settings);
}

void kenny_gain_init(gain_settings_t *p_gain_settings){
    p_gain_settings->bypass = 0;
    p_gain_settings->output_gain = float_to_fixed_point(1.0);
    kenny_gain_update_hardware(p_gain_settings);
}
void kenny_gain_update_interactive(gain_settings_t *p_gain_settings){
    char c;

    while(1)
    {
        printf("Welcome to the gain configuration menu. hat would you like to do?\r\n");
        printf("p: Print current gain settings.\r\n");
        printf("1: Modify output GAIN \r\n");
        printf("2: Toggle output gain BYPASS \r\n");
        c = XUartPs_RecvByte(XPAR_PS7_UART_1_BASEADDR);

        if (c == 'p') {
            kenny_gain_print_bypass(p_gain_settings);
            kenny_gain_print_gain(p_gain_settings);
        }
        else if (c == '1') {
            char user_input[50] = {};
            float new_gain_FLOAT = 0;
            uint32_t new_gain = 0;
            printf("Please enter the gain value (as a multiple of 0.25) \r\n");
            scanf("%8s", user_input);
            new_gain_FLOAT = atof(user_input);
            new_gain = float_to_fixed_point(new_gain_FLOAT);

            p_gain_settings->output_gain = new_gain;
            kenny_gain_print_gain(p_gain_settings);
        }
        else if (c == '2') {
            p_gain_settings->bypass = !p_gain_settings->bypass;
            kenny_gain_print_bypass(p_gain_settings);
        }
        else if (c == 'q') {
            kenny_gain_update_hardware(p_gain_settings);
            break;
        }
        else {
            xil_printf("Invalid entry. Please try again\r\n");
        }
    }
}
void kenny_gain_update_hardware(gain_settings_t *p_gain_settings)
{
    int32_t output_gain_to_write = 0;
    if (p_gain_settings->bypass == 0)
    {
        output_gain_to_write = p_gain_settings->output_gain;
    } else {
        output_gain_to_write = float_to_fixed_point(1.0);
    }
    GAIN_CONFIG_mWriteReg(
        XPAR_GAIN_CONFIG_0_S00_AXI_BASEADDR,
        GAIN_OUTPUT_GAIN_REG,
        (u32)output_gain_to_write
    );

    for (int i = 0; i < 1; ++i)
    {
        int32_t read_value = 0;
        read_value = K_AUD_CMPRS_CONFIGURATOR_mReadReg(XPAR_GAIN_CONFIG_0_S00_AXI_BASEADDR, 4*i);

        printf("GAIN CONFIGURATION READ: i = %d, value = %ld\r\n", i, read_value);
    }
}


/******************************/
// COMPRESSOR FUNCTIONS

void kenny_compressor_init(
                compressor_settings_t *p_compressor_settings,
                stft_settings_t *p_stft_settings
) {
    p_compressor_settings->ratio = 10;
    p_compressor_settings->threshold_energy = 50000000;
    p_compressor_settings->bypass = 0;
    //p_compressor_settings->num_pts  = INIT_NUM_FFT_PTS;
    p_compressor_settings->p_stft_settings = p_stft_settings;
    kenny_compressor_update_hardware(p_compressor_settings);
}

void kenny_compressor_print_bypass(compressor_settings_t *p_compressor_settings) {
    xil_printf("The compressor is %s\r\n", (p_compressor_settings->bypass ? "DISABLED" : "ENABLED"));
}
void kenny_compressor_print_ratio(compressor_settings_t *p_compressor_settings) {
    printf("    compressor ratio = %d\r\n", p_compressor_settings->ratio);
}
void kenny_compressor_print_threshold(compressor_settings_t *p_compressor_settings) {
    xil_printf("    compressor threshold = %d\r\n", p_compressor_settings->threshold_energy);
}
void kenny_compressor_print_num_pts(compressor_settings_t *p_compressor_settings) {
    xil_printf("    compressor num_pts = %d\r\n", p_compressor_settings->p_stft_settings->num_fft_pts);
}
void kenny_compressor_print(compressor_settings_t *p_compressor_settings) {
    kenny_compressor_print_bypass(p_compressor_settings);
    kenny_compressor_print_ratio(p_compressor_settings);
    kenny_compressor_print_threshold(p_compressor_settings);
    kenny_compressor_print_num_pts(p_compressor_settings);
}

void kenny_compressor_update_hardware(compressor_settings_t *p_compressor_settings)
{
    K_AUD_CMPRS_CONFIGURATOR_mWriteReg(
        XPAR_K_AUD_CMPRS_CONFIGUR_0_S00_AXI_BASEADDR, 
        CMPRS_THRESHOLD_REG, 
        p_compressor_settings->threshold_energy
    );
    K_AUD_CMPRS_CONFIGURATOR_mWriteReg(
        XPAR_K_AUD_CMPRS_CONFIGUR_0_S00_AXI_BASEADDR, 
        CMPRS_RATIO_REG, 
        p_compressor_settings->ratio
    );
    K_AUD_CMPRS_CONFIGURATOR_mWriteReg(
        XPAR_K_AUD_CMPRS_CONFIGUR_0_S00_AXI_BASEADDR, 
        CMPRS_NUM_FFT_PTS_REG, 
        p_compressor_settings->p_stft_settings->num_fft_pts
    );
    K_AUD_CMPRS_CONFIGURATOR_mWriteReg(
        XPAR_K_AUD_CMPRS_CONFIGUR_0_S00_AXI_BASEADDR, 
        CMPRS_BYPASS_REG, 
        p_compressor_settings->bypass
    );


    printf("Updated COMPRESSOR hardware with the following settings:\r\n");
    printf("    threshold = %d\r\n", p_compressor_settings->threshold_energy);
    printf("    ratio = %d\r\n", p_compressor_settings->ratio);
    printf("    num_pts = %d\r\n", p_compressor_settings->p_stft_settings->num_fft_pts);
    printf("    bypass = %d\r\n", p_compressor_settings->bypass);

    for (int i = 0; i < 4; ++i)
    {
        int32_t read_value = 0;
        read_value = K_AUD_CMPRS_CONFIGURATOR_mReadReg(XPAR_K_AUD_CMPRS_CONFIGUR_0_S00_AXI_BASEADDR, 4*i);

        printf("CMPRS CONFIGURATION READ: i = %d, value = %ld\r\n", i, read_value);
    }
}

void kenny_compressor_update_interactive(compressor_settings_t *p_compressor_settings)
{
    char c;

    while(1)
    {
        xil_printf("What would you like to do?\r\n");
        xil_printf("p: Print current compressor settings\r\n");
        xil_printf("1: Modify compressor RATIO\r\n");
        xil_printf("2: Modify compressor THRESHOLD\r\n");
        xil_printf("3: Toggle compressor BYPASS\r\n");
        //xil_printf("4: Toggle compressor NUM_PTS\r\n");
        xil_printf("q: Quit back to main menu\r\n");
        c = XUartPs_RecvByte(XPAR_PS7_UART_1_BASEADDR);


        if (c == 'p') {
            kenny_compressor_print(p_compressor_settings);
        }
        else if (c == '1') {
            int new_ratio_val = 0;
            char new_ratio_CHAR[8] = {};
            xil_printf("Please enter your desired ratio. (up to 4 digits)\r\n");
            scanf("%4s", new_ratio_CHAR);
            new_ratio_val = atoi(new_ratio_CHAR);
            xil_printf("You entered: %d\r\n", new_ratio_val);
            p_compressor_settings->ratio = new_ratio_val;
            kenny_compressor_print_ratio(p_compressor_settings);
        }
        else if (c == '2')
        {
            int new_thresh_val = 0;
            char new_thresh_val_CHAR[8] = {};
            xil_printf("Please enter your desired threshold. (up to 8 digits)\r\n");
            scanf("%8s", new_thresh_val_CHAR);
            new_thresh_val = atoi(new_thresh_val_CHAR);
            xil_printf("You entered: %d\r\n", new_thresh_val);
            p_compressor_settings->threshold_energy = new_thresh_val;
            kenny_compressor_print_threshold(p_compressor_settings);
        }
        else if (c == '3')
        {
            p_compressor_settings->bypass = (p_compressor_settings->bypass ? 0 : 1);
            kenny_compressor_print_bypass(p_compressor_settings);
        }
        //else if (c == '4')
        //{
        //    int new_num_pts = 0;
        //    char new_num_pts_CHAR[8] = {};
        //    xil_printf("Please enter your desired num_pts. (up to 4 digits)\r\n");
        //    scanf("%4s", new_num_pts_CHAR);
        //    new_num_pts = atoi(new_num_pts_CHAR);
        //    xil_printf("You entered: %d\r\n", new_num_pts);
        //    p_compressor_settings->num_pts = new_num_pts;
        //    kenny_compressor_print_num_pts(p_compressor_settings);
        //}
        else if (c == 'q')
        {
            kenny_compressor_update_hardware(p_compressor_settings);
            break;
        }
        else
        {
            xil_printf("Invalid entry. Please try again \r\n");
        }
    }
}



/******************************/
// MISC:

void kenny_update_num_fft_pts(eq_settings_t *p_eq_settings, stft_settings_t *p_stft_settings, int new_num_fft_pts) {
    // Since the #FFT points can only be a power of 2, this can always be cast to an int.
    // Minus one because the FFT will be symmetric
    p_eq_settings->EQ_cur_num_freq_buckets = (int) log2f(new_num_fft_pts);
    kenny_stft_update_window_func(p_stft_settings, new_num_fft_pts);
}


/******************************/
// PLAYING/RECORDING

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
        retval = 0xff000000 | inval;    // Set all high bits to 1.
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
        #ifdef __DEBUGGING__
        printf("ConvertAUDIOtoCPLX: inval[%d] = %d\r\n",
                in_idx, inval[in_idx]
        );
        #endif

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
    retval = retval << 12;        // scale it up so it's audible
    return retval;
}

void kenny_convertCplxToAudio(cplx_data_t* inval, int* outval, size_t num_vals_to_cpy)
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

        // In testing, I find that ignoring the imaginary part is ok. But maybe it should be re + im.
        cur_re = cur_cplx.data_re;
        cur_re_int = kenny_convert_short_to_24bit(cur_re);

        #ifdef __DEBUGGING__
        printf("ConvertCplxToAudio: outval[%d] = %d\r\n",
                out_idx, cur_re_int
        );
        #endif

        // Write to output channels (assume 2 channels)
        outval[out_idx]     = cur_re_int;
        outval[out_idx+1]   = cur_re_int;
    }
}

unsigned int kenny_cplx_get_magnitude_squared(cplx_data_t input) {
    unsigned int retval = 0;
    retval += input.data_re * input.data_re;
    retval += input.data_im * input.data_im;
    return retval;
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


