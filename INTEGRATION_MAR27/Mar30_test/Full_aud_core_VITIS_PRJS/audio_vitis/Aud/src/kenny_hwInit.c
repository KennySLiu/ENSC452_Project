
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

static void BTN_InterruptHandler(void *InstancePtr) {
    // Disable GPIO interrupts
    int myBTN = 0;
    XGpio_InterruptDisable(&BTNs, BTN_INT);
    // Ignore additional button presses
    if ((XGpio_InterruptGetStatus(&BTNs) & BTN_INT) != BTN_INT) {
        return;
    }

    myBTN = read_BTN();
    if(myBTN != 0) push_btns = read_BTN();
    xil_printf("Buttons value: %d \r\n", read_BTN());
    (void) XGpio_InterruptClear(&BTNs, BTN_INT);

    // Enable GPIO interrupts
    XGpio_InterruptEnable(&BTNs, BTN_INT);
}

static void Timer_InterruptHandler(XTmrCtr *data, u8 TmrCtrNumber) {
    XTmrCtr_Stop(data, TmrCtrNumber);
    XTmrCtr_Reset(data, TmrCtrNumber);
    //Update Stuff
    //COMM_VAL = 1;
    xil_printf("Timer Interrupt Called. \r\n");
    TIMER_INTR_FLG = 1;
    XTmrCtr_Start(data, TmrCtrNumber);
}


void setup_timer() {
    XTmrCtr_SetHandler(
            &TimerInstancePtr,
            (XTmrCtr_Handler) Timer_InterruptHandler, 
            &TimerInstancePtr
    );
    //Reset Values
    XTmrCtr_SetResetValue(&TimerInstancePtr, 0, //Change with generic value
                                                //0xFFF0BDC0);
                                                //0x23C34600);
                                                //0xDC3CB9FF); 6sec
            0xFA0A1EFF
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
