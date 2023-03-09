## ZedBoard LED Constraints
#set_property PACKAGE_PIN T22 [get_ports {LEDs_out_0[0]}]
#set_property IOSTANDARD LVCMOS33 [get_ports {LEDs_out_0[0]}]
#set_property PACKAGE_PIN T21 [get_ports {LEDs_out_0[1]}]
#set_property IOSTANDARD LVCMOS33 [get_ports {LEDs_out_0[1]}]
#set_property PACKAGE_PIN U22 [get_ports {LEDs_out_0[2]}]
#set_property IOSTANDARD LVCMOS33 [get_ports {LEDs_out_0[2]}]
#set_property PACKAGE_PIN U21 [get_ports {LEDs_out_0[3]}]
#set_property IOSTANDARD LVCMOS33 [get_ports {LEDs_out_0[3]}]
#set_property PACKAGE_PIN V22 [get_ports {LEDs_out_0[4]}]
#set_property IOSTANDARD LVCMOS33 [get_ports {LEDs_out_0[4]}]
#set_property PACKAGE_PIN W22 [get_ports {LEDs_out_0[5]}]
#set_property IOSTANDARD LVCMOS33 [get_ports {LEDs_out_0[5]}]
#set_property PACKAGE_PIN U19 [get_ports {LEDs_out_0[6]}]
#set_property IOSTANDARD LVCMOS33 [get_ports {LEDs_out_0[6]}]
#set_property PACKAGE_PIN U14 [get_ports {LEDs_out_0[7]}]
#set_property IOSTANDARD LVCMOS33 [get_ports {LEDs_out_0[7]}]

# ZedBoard Audio Codec Constraints
set_property PACKAGE_PIN AA6 [get_ports BCLK_0]
set_property IOSTANDARD LVCMOS33 [get_ports BCLK_0]

set_property PACKAGE_PIN Y6 [get_ports LRCLK_0]
set_property IOSTANDARD LVCMOS33 [get_ports LRCLK_0]

set_property PACKAGE_PIN AA7 [get_ports SDATA_I_0]
set_property IOSTANDARD LVCMOS33 [get_ports SDATA_I_0]

set_property PACKAGE_PIN Y8 [get_ports SDATA_O_0]
set_property IOSTANDARD LVCMOS33 [get_ports SDATA_O_0]

#MCLK
set_property PACKAGE_PIN AB2 [get_ports FCLK_CLK1_0]
set_property IOSTANDARD LVCMOS33 [get_ports FCLK_CLK1_0]

set_property PACKAGE_PIN AB4 [get_ports IIC_0_0_scl_io]
set_property IOSTANDARD LVCMOS33 [get_ports IIC_0_0_scl_io]

set_property PACKAGE_PIN AB5 [get_ports IIC_0_0_sda_io]
set_property IOSTANDARD LVCMOS33 [get_ports IIC_0_0_sda_io]

#set_property PACKAGE_PIN AB1 [get_ports {GPIO_0_tri_io[0]}]
#set_property IOSTANDARD LVCMOS33 [get_ports {GPIO_0_tri_io[0]}]

#set_property PACKAGE_PIN Y5 [get_ports {GPIO_0_tri_io[1]}]
#set_property IOSTANDARD LVCMOS33 [get_ports {GPIO_0_tri_io[1]}]