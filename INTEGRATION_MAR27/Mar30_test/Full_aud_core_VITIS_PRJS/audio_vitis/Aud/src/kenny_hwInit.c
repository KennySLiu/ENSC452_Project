
#include "kenny_hwInit.h"


int read_SW(void) {
    return XGpio_DiscreteRead(&SWs, SLIDE_SWT_CH);
}

int read_BTN(void) {
    return XGpio_DiscreteRead(&BTNs, PUSH_BTN_CH);
}

int read_LED(void) {
    return XGpio_DiscreteRead(&LEDs, LED_CHANNEL);
}

static void SW_InterruptHandler(void *InstancePtr) {
    // Disable GPIO interrupts
    XGpio_InterruptDisable(&SWs, SW_INT);
    // Ignore additional button presses
    if ((XGpio_InterruptGetStatus(&SWs) & SW_INT) != SW_INT) {
        return;
    }

    xil_printf("Switches value: %d \r\n", read_SW());
    (void) XGpio_InterruptClear(&SWs, SW_INT);

    // Enable GPIO interrupts
    XGpio_InterruptEnable(&SWs, SW_INT);
}

static void update_aud_system(int up) {
    int update_EQ = 0;
    int update_COMP = 0;
    int update_GAIN = 0;

    float INCREASE_AMOUNT = 0;
    int MAX_AMOUNT = 0;
    int MIN_AMOUNT = 0;

    if (SELECTION_STATE >= 0 && SELECTION_STATE <= 2) {
        update_COMP = 1;
        if (SELECTION_STATE == 0) {
            INCREASE_AMOUNT = 5000000;
            MAX_AMOUNT = 200000000;
            MIN_AMOUNT = 0;

            compressor_settings.threshold_energy += (up ? INCREASE_AMOUNT : -INCREASE_AMOUNT);
            if (compressor_settings.threshold_energy > MAX_AMOUNT){
                compressor_settings.threshold_energy = MAX_AMOUNT;
            }
            else if (compressor_settings.threshold_energy < MIN_AMOUNT){
                compressor_settings.threshold_energy = MIN_AMOUNT;
            }
        }
        else if (SELECTION_STATE == 1) {
            INCREASE_AMOUNT = 5;
            MAX_AMOUNT = 200;
            MIN_AMOUNT = 5;

            compressor_settings.ratio += (up ? INCREASE_AMOUNT : -INCREASE_AMOUNT);
            if (compressor_settings.ratio > MAX_AMOUNT){
                compressor_settings.ratio = MAX_AMOUNT;
            }
            else if (compressor_settings.ratio < MIN_AMOUNT){
                compressor_settings.ratio = MIN_AMOUNT;
            }
        } 
        else {
            compressor_settings.bypass = !compressor_settings.bypass;
        }
    }
    else if (SELECTION_STATE <= 4)
    {
        update_GAIN = 1;

        if (SELECTION_STATE == 3) {
            INCREASE_AMOUNT = 0.25;
            MAX_AMOUNT = 5;
            MIN_AMOUNT = 0;

            gain_settings.output_gain += (up ? INCREASE_AMOUNT : -INCREASE_AMOUNT);
            if (gain_settings.output_gain > MAX_AMOUNT){
                gain_settings.output_gain = MAX_AMOUNT;
            }
            else if (gain_settings.output_gain < MIN_AMOUNT){
                gain_settings.output_gain = MIN_AMOUNT;
            }
        }
        else if (SELECTION_STATE == 4) {
            gain_settings.bypass = !gain_settings.bypass;
        }
    }
    else if (SELECTION_STATE <= 11)
    {
        update_EQ = 1;
        if (SELECTION_STATE >= 5 && SELECTION_STATE <= 10)
        {
            INCREASE_AMOUNT = 1.0;
            MAX_AMOUNT = 8;
            MIN_AMOUNT = 0;

            int idx = SELECTION_STATE-5 + 3;
            eq_settings.parametric_eq_vect[idx] += (up ? INCREASE_AMOUNT : -INCREASE_AMOUNT);
            if (eq_settings.parametric_eq_vect[idx] > MAX_AMOUNT){
                eq_settings.parametric_eq_vect[idx] = MAX_AMOUNT;
            }
            else if (eq_settings.parametric_eq_vect[idx] < MIN_AMOUNT){
                eq_settings.parametric_eq_vect[idx] = MIN_AMOUNT;
            }
        }
        else if (SELECTION_STATE == 11) {
            eq_settings.bypass = !eq_settings.bypass;
        }
    }

    if (update_COMP) {
        kenny_compressor_update_hardware(&compressor_settings);
        return;
    } else if (update_EQ) {
        kenny_eq_update_hardware(&eq_settings);
        return;
    } else if (update_GAIN) {
        kenny_gain_update_hardware(&gain_settings);
        return;
    }

}

static void BTN_InterruptHandler(void *InstancePtr) {
    // Disable GPIO interrupts
    int myBTN = 0;
    static int btn_cnt = 0;
    XGpio_InterruptDisable(&BTNs, BTN_INT);
    // Ignore additional button presses
    if ((XGpio_InterruptGetStatus(&BTNs) & BTN_INT) != BTN_INT) {
        return;
    }
    
    myBTN = read_BTN();
    if(myBTN != 0) myBTN = read_BTN();
    xil_printf("Buttons value: %d \r\n", myBTN);
    (void) XGpio_InterruptClear(&BTNs, BTN_INT);


    //if (btn_cnt != 1) {
    //    btn_cnt = 1;
    //    myBTN = 0;
    //} else {
    //    btn_cnt = 0;
    //}


    // 1, 2, 4, 8, 16 = mid, down, l, r, up

    switch (myBTN) {
        case 1:
            // Center button. do nothing
            break;

        case 2: 
            // Down.
            update_aud_system(0);
            break;

        case 4:
            // Left.
            if (SELECTION_STATE > 0)
                SELECTION_STATE -= 1;
            break;

        case 8:
            // Right.
            if (SELECTION_STATE < 11)
                SELECTION_STATE += 1;
            break;

        case 16:
            // Up.
            update_aud_system(1);
            break;

        default:
            break;
    }
    
    XGpio_DiscreteWrite(&LEDs, LED_CHANNEL, SELECTION_STATE);

    // Enable GPIO interrupts
    XGpio_InterruptEnable(&BTNs, BTN_INT);
}

static void Timer_InterruptHandler(XTmrCtr *data, u8 TmrCtrNumber) {
    XTmrCtr_Stop(data, TmrCtrNumber);
    XTmrCtr_Reset(data, TmrCtrNumber);

    #ifdef __DO_TIMING__
    float total_time_usec;
    XTime_GetTime(&startcycles);
    #endif

    u32  in_left, in_right;

    //Update Stuff
    #ifdef __DEBUGGING__
    printf("HELLO From the timer handler. num_fft_pts = %d, audio_in_read_ctr = %d, audio_out_read_ctr = %d\r\n", 
            num_fft_pts,
            audio_in_read_ctr,
            audio_out_read_ctr
    );
    printf("audio_in_read_ctr = %d, aud_in_ptr = %d, aud_out_ptr = %d\r\n",
            &audio_in_read_ctr,
            cur_audio_in_ptr,
            cur_audio_out_ptr
    );
    #endif

    #ifdef __PURE_STREAMING__
    in_left = Xil_In32(I2S_DATA_RX_L_REG);
    in_right = Xil_In32(I2S_DATA_RX_R_REG);
    Xil_Out32(I2S_DATA_TX_L_REG, in_left);
    Xil_Out32(I2S_DATA_TX_R_REG, in_right);
    #else
    //////////////////////////////////
    // READ AUDIO IN

    in_left = Xil_In32(I2S_DATA_RX_L_REG);
    in_right = Xil_In32(I2S_DATA_RX_R_REG);
    audio_in_read_ctr++;
    *(cur_audio_in_ptr++) = in_left;
    *(cur_audio_in_ptr++) = in_right;

    // Perform an STFT window
    if (audio_in_read_ctr == num_fft_pts/STFT_STRIDE_FACTOR)
    {
        audio_in_read_ctr = 0;

        int cur_idx = aud_in_idx;
        int next_idx = (cur_idx+1)%3;
        int prev_idx = (cur_idx+2)%3;

        // The datacopying cannot happen here because it's slow. Handle that in main.

        // Swap the buffer indices.
        aud_in_idx = next_idx;
        cur_audio_in_ptr = AUDIO_IN_MEM_PTRS[aud_in_idx];
        FFTDATA_READY = 1;
    }

    //////////////////////////////////
    // PLAY AUDIO OUT
    in_left  = *(cur_audio_out_ptr++);
    in_right = *(cur_audio_out_ptr++);
    audio_out_read_ctr++;
    Xil_Out32(I2S_DATA_TX_L_REG, in_left);
    Xil_Out32(I2S_DATA_TX_R_REG, in_right);

    if (audio_out_read_ctr == num_fft_pts/STFT_STRIDE_FACTOR)
    {
        audio_out_read_ctr = 0;
        aud_out_idx = !aud_out_idx;
        cur_audio_out_ptr = AUDIO_OUT_MEM_PTR[aud_out_idx];
    }
    #endif


    #ifdef __DO_TIMING__
    XTime_GetTime(&endcycles);
    totalcycles = 2 * (endcycles-startcycles);
    total_time_usec = ((float) totalcycles) * 1000000 / 2 / COUNTS_PER_SECOND;
    printf("\n\n");
    printf("INTRRUPT: The total time was %f usec\r\n.", total_time_usec);
    #endif

    XTmrCtr_Start(data, TmrCtrNumber);
}


void setup_timer() {
    XTmrCtr_SetHandler(
            &TimerInstancePtr,
            (XTmrCtr_Handler) Timer_InterruptHandler, 
            &TimerInstancePtr
    );
    //Reset Values

    // On April 1st I tested the timer by using XTime_GetTime(). It has a period of 10ns.
    // Therefore, the time per sample = 100,000,000/AUD_SAMPLE_RATE.
    XTmrCtr_SetResetValue(
            &TimerInstancePtr, 
            0,
            0xFFFFf8f4        // Random crap.
            //0xFFFFf8a4      //1883 , corresponding to 48khz with 2us spent in the interrupt.
            //0xFFFFF3CA      //3125, corresponding to 32khz
            //0xFFFFDF7C  // 2083, corresponding to 48khz
    );

    //Interrupt Mode and Auto reload
    XTmrCtr_SetOptions(&TimerInstancePtr,
        XPAR_AXI_TIMER_0_DEVICE_ID, 
        (XTC_INT_MODE_OPTION | XTC_AUTO_RELOAD_OPTION)
    );
}



static int SetUpInterruptSystem(XScuGic *XScuGicInstancePtr) {
    //Enable GPIO interrupts
    XGpio_InterruptEnable(&SWs, SW_INT);
    XGpio_InterruptGlobalEnable(&SWs);
    XGpio_InterruptEnable(&BTNs, BTN_INT);
    XGpio_InterruptGlobalEnable(&BTNs);

    Xil_ExceptionInit();
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
            (Xil_ExceptionHandler) XScuGic_InterruptHandler,
            XScuGicInstancePtr);
    Xil_ExceptionEnable()
    ;
    return XST_SUCCESS;
}


int kenny_init_intc(
    XScuGic * p_intc_inst, 
    int intc_device_id
) {
    TIMER_INTR_FLG = 0;
	XScuGic_Config* cfg_ptr;
	int          status;

	// Look up hardware configuration for device
	cfg_ptr = XScuGic_LookupConfig(intc_device_id);
	if (!cfg_ptr)
	{
		xil_printf("ERROR! No hardware configuration found for Interrupt Controller with device id %d.\r\n", intc_device_id);
		return INIT_FAILED;
	}
	// Initialize driver
	status = XScuGic_CfgInitialize(p_intc_inst, cfg_ptr, cfg_ptr->CpuBaseAddress);
	if (status != XST_SUCCESS)
	{
		xil_printf("ERROR! Initialization of Interrupt Controller failed with %d.\r\n", status);
		return INIT_FAILED;
	}




    /*********** ABDULS STUFF BELOW *************/


    status = SetUpInterruptSystem(p_intc_inst);
    if (status != XST_SUCCESS) {
        return XST_FAILURE;
    }
    // Set interrupt priorities and trigger type
    XScuGic_SetPriorityTriggerType(p_intc_inst,
            XPAR_FABRIC_GPIO_SWITCHES_IP2INTC_IRPT_INTR, 0xB0, 0x3
    );    //SWs, Lowest, rising
    XScuGic_SetPriorityTriggerType(p_intc_inst,
            XPAR_FABRIC_GPIO_BUTTONS_IP2INTC_IRPT_INTR, 0xA0, 0x3
    );    //BTNs, Highest, rising
    XScuGic_SetPriorityTriggerType(p_intc_inst,
            XPAR_FABRIC_AXI_TIMER_0_INTERRUPT_INTR, 0xA8, 0x3
    );    //TIMER, Middle, rising

    /*Connect device driver handlers that will be called when an interrupt for the device occurs, the device driver handler performs the specific interrupt processing for the device*/
    //connecting SW
    status = XScuGic_Connect(p_intc_inst,
            XPAR_FABRIC_GPIO_SWITCHES_IP2INTC_IRPT_INTR, 
            (Xil_ExceptionHandler) SW_InterruptHandler,
            (void *) &SWs
    );
    if (status != XST_SUCCESS) {
        return XST_FAILURE;
    }
    //connecting BTN
    status = XScuGic_Connect(p_intc_inst,
            XPAR_FABRIC_GPIO_BUTTONS_IP2INTC_IRPT_INTR, 
            (Xil_ExceptionHandler) BTN_InterruptHandler,
            (void *) &BTNs
    );
    if (status != XST_SUCCESS) {
        return XST_FAILURE;
    }

    //connecting timer
    status = XScuGic_Connect(p_intc_inst,
            XPAR_FABRIC_AXI_TIMER_0_INTERRUPT_INTR, 
            (Xil_ExceptionHandler) XTmrCtr_InterruptHandler,
            (void *) &TimerInstancePtr
    );
    if (status != XST_SUCCESS) {
        return XST_FAILURE;
    }

    //Enable GPIO interrupts
    XGpio_InterruptEnable(&SWs, SW_INT);
    XGpio_InterruptGlobalEnable(&SWs);
    XGpio_InterruptEnable(&BTNs, BTN_INT);
    XGpio_InterruptGlobalEnable(&BTNs);

    //Enabling Interrupts inside the controller
    XScuGic_Enable(p_intc_inst, XPAR_FABRIC_GPIO_SWITCHES_IP2INTC_IRPT_INTR);
    XScuGic_Enable(p_intc_inst, XPAR_FABRIC_GPIO_BUTTONS_IP2INTC_IRPT_INTR);
    XScuGic_Enable(p_intc_inst, XPAR_FABRIC_AXI_TIMER_0_INTERRUPT_INTR);

	return INIT_SUCCESS;
}



// ABDUL'S ORIGINAL INTERRUPT SETUP FUNCTION
///*Setup all interrupts of the system*/
//int ScuGicInterrupt_Init() {
//    int Status = 0;
//    GicConfig = XScuGic_LookupConfig(XPAR_PS7_SCUGIC_0_DEVICE_ID);
//    if (NULL == GicConfig) {
//        return XST_FAILURE;
//    }
//
//    Status = XScuGic_CfgInitialize(&InterruptController, GicConfig,
//            GicConfig->CpuBaseAddress);
//    if (Status != XST_SUCCESS) {
//        return XST_FAILURE;
//    }
//    Status = SetUpInterruptSystem(&InterruptController);
//    if (Status != XST_SUCCESS) {
//        return XST_FAILURE;
//    }
//    // Set interrupt priorities and trigger type
//    XScuGic_SetPriorityTriggerType(&InterruptController,
//            XPAR_FABRIC_GPIO_SWITCHES_IP2INTC_IRPT_INTR, 0xB0, 0x3
//    );    //SWs, Lowest, rising
//    XScuGic_SetPriorityTriggerType(&InterruptController,
//            XPAR_FABRIC_GPIO_BUTTONS_IP2INTC_IRPT_INTR, 0xA0, 0x3
//    );    //BTNs, Highest, rising
//    XScuGic_SetPriorityTriggerType(&InterruptController,
//            XPAR_FABRIC_AXI_TIMER_0_INTERRUPT_INTR, 0xA8, 0x3
//    );    //TIMER, Middle, rising
//
//    /*Connect device driver handlers that will be called when an interrupt for the device occurs, the device driver handler performs the specific interrupt processing for the device*/
//    //connecting SW
//    Status = XScuGic_Connect(&InterruptController,
//            XPAR_FABRIC_GPIO_SWITCHES_IP2INTC_IRPT_INTR, 
//            (Xil_ExceptionHandler) SW_InterruptHandler,
//            (void *) &SWs
//    );
//    if (Status != XST_SUCCESS) {
//        return XST_FAILURE;
//    }
//    //connecting BTN
//    Status = XScuGic_Connect(&InterruptController,
//            XPAR_FABRIC_GPIO_BUTTONS_IP2INTC_IRPT_INTR, 
//            (Xil_ExceptionHandler) BTN_InterruptHandler,
//            (void *) &BTNs
//    );
//    if (Status != XST_SUCCESS) {
//        return XST_FAILURE;
//    }
//
//    //connecting timer
//    Status = XScuGic_Connect(&InterruptController,
//            XPAR_FABRIC_AXI_TIMER_0_INTERRUPT_INTR, 
//            (Xil_ExceptionHandler) XTmrCtr_InterruptHandler,
//            (void *) &TimerInstancePtr
//    );
//    if (Status != XST_SUCCESS) {
//        return XST_FAILURE;
//    }
//
//    //Enable GPIO interrupts
//    XGpio_InterruptEnable(&SWs, SW_INT);
//    XGpio_InterruptGlobalEnable(&SWs);
//    XGpio_InterruptEnable(&BTNs, BTN_INT);
//    XGpio_InterruptGlobalEnable(&BTNs);
//
//    //Enabling Interrupts inside the controller
//    XScuGic_Enable(&InterruptController, XPAR_FABRIC_GPIO_SWITCHES_IP2INTC_IRPT_INTR);
//    XScuGic_Enable(&InterruptController, XPAR_FABRIC_GPIO_BUTTONS_IP2INTC_IRPT_INTR);
//    XScuGic_Enable(&InterruptController, XPAR_FABRIC_AXI_TIMER_0_INTERRUPT_INTR);
//
//    return XST_SUCCESS;
//}
//
