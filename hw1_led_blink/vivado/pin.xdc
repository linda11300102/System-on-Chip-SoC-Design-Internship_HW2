## EGO-XZ7 (XC7Z020-CLG484-1) pin constraints for HW2 assignment 1
## Pin number: EGO-XZ7 Hardware Manual, Table 15

## ===== LED0 (axi_gpio_0 channel 1, output) =====
set_property PACKAGE_PIN T22 [get_ports {LED_tri_o[0]}]
set_property IOSTANDARD LVCMOS33 [get_ports {LED_tri_o[0]}]

## LED is a slow asynchronous output: no timing analysis needed
set_false_path -to [get_ports {LED_tri_o[*]}]
