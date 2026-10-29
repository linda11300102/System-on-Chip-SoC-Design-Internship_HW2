## EGO-XZ7 (XC7Z020-CLG484-1) pin constraints for HW2 assignment 2
## Pin numbers: EGO-XZ7 Hardware Manual, Table 15

## ===== LED (axi_gpio_0 channel 1, output) =====
set_property PACKAGE_PIN T22 [get_ports {LED_tri_o[0]}]
set_property PACKAGE_PIN T21 [get_ports {LED_tri_o[1]}]
set_property IOSTANDARD LVCMOS33 [get_ports {LED_tri_o[*]}]

## ===== Button BTNC (axi_gpio_0 channel 2, input) =====
set_property PACKAGE_PIN P16 [get_ports {BTN_tri_i[0]}]
set_property IOSTANDARD LVCMOS33 [get_ports {BTN_tri_i[0]}]

## LED / button are asynchronous, slow I/O: no timing analysis needed
set_false_path -from [get_ports {BTN_tri_i[*]}]
set_false_path -to   [get_ports {LED_tri_o[*]}]
