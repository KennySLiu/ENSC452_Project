# Definitional proc to organize widgets for parameters.
proc init_gui { IPINST } {
  ipgui::add_param $IPINST -name "Component_Name"
  #Adding Page
  set Page_0 [ipgui::add_page $IPINST -name "Page 0"]
  ipgui::add_param $IPINST -name "AXI_DATA_WIDTH" -parent ${Page_0}
  ipgui::add_param $IPINST -name "Ratio" -parent ${Page_0}
  ipgui::add_param $IPINST -name "Thresh" -parent ${Page_0}
  ipgui::add_param $IPINST -name "ENRGY_DATA_WIDTH" -parent ${Page_0}
  ipgui::add_param $IPINST -name "NUM_FFT_PTS" -parent ${Page_0}


}

proc update_PARAM_VALUE.AXI_DATA_WIDTH { PARAM_VALUE.AXI_DATA_WIDTH } {
	# Procedure called to update AXI_DATA_WIDTH when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.AXI_DATA_WIDTH { PARAM_VALUE.AXI_DATA_WIDTH } {
	# Procedure called to validate AXI_DATA_WIDTH
	return true
}

proc update_PARAM_VALUE.ENRGY_DATA_WIDTH { PARAM_VALUE.ENRGY_DATA_WIDTH } {
	# Procedure called to update ENRGY_DATA_WIDTH when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.ENRGY_DATA_WIDTH { PARAM_VALUE.ENRGY_DATA_WIDTH } {
	# Procedure called to validate ENRGY_DATA_WIDTH
	return true
}

proc update_PARAM_VALUE.NUM_FFT_PTS { PARAM_VALUE.NUM_FFT_PTS } {
	# Procedure called to update NUM_FFT_PTS when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.NUM_FFT_PTS { PARAM_VALUE.NUM_FFT_PTS } {
	# Procedure called to validate NUM_FFT_PTS
	return true
}

proc update_PARAM_VALUE.Ratio { PARAM_VALUE.Ratio } {
	# Procedure called to update Ratio when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.Ratio { PARAM_VALUE.Ratio } {
	# Procedure called to validate Ratio
	return true
}

proc update_PARAM_VALUE.Thresh { PARAM_VALUE.Thresh } {
	# Procedure called to update Thresh when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.Thresh { PARAM_VALUE.Thresh } {
	# Procedure called to validate Thresh
	return true
}


proc update_MODELPARAM_VALUE.AXI_DATA_WIDTH { MODELPARAM_VALUE.AXI_DATA_WIDTH PARAM_VALUE.AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.AXI_DATA_WIDTH}] ${MODELPARAM_VALUE.AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.Thresh { MODELPARAM_VALUE.Thresh PARAM_VALUE.Thresh } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.Thresh}] ${MODELPARAM_VALUE.Thresh}
}

proc update_MODELPARAM_VALUE.Ratio { MODELPARAM_VALUE.Ratio PARAM_VALUE.Ratio } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.Ratio}] ${MODELPARAM_VALUE.Ratio}
}

proc update_MODELPARAM_VALUE.ENRGY_DATA_WIDTH { MODELPARAM_VALUE.ENRGY_DATA_WIDTH PARAM_VALUE.ENRGY_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.ENRGY_DATA_WIDTH}] ${MODELPARAM_VALUE.ENRGY_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.NUM_FFT_PTS { MODELPARAM_VALUE.NUM_FFT_PTS PARAM_VALUE.NUM_FFT_PTS } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.NUM_FFT_PTS}] ${MODELPARAM_VALUE.NUM_FFT_PTS}
}

