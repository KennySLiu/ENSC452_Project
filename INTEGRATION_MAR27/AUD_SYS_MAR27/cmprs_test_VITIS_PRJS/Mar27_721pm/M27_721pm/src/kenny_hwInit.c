
#include "kenny_hwInit.h"


int kenny_init_intc(XScuGic * p_intc_inst, int intc_device_id)
{
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
	return INIT_SUCCESS;
}
