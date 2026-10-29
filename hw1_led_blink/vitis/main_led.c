/*
 * HW2 assignment 1: LED0 on for 1 s, off for 1 s
 *
 * Hardware (Vivado):
 *   axi_gpio_0 @ 0x41200000, GPIO channel 1 = 1-bit output -> LED0 (T22)
 */
#include "xil_io.h"
#include "sleep.h"

#define LED_BASE  0x41200000U   /* axi_gpio_0 base address (Address Editor) */
#define GPIO_DATA 0x0U          /* data register:      1 = LED on            */
#define GPIO_TRI  0x4U          /* direction register: 0 = output            */

int main(void)
{
    Xil_Out32(LED_BASE + GPIO_TRI, 0x0);      /* set channel 1 as output */

    while (1) {
        Xil_Out32(LED_BASE + GPIO_DATA, 0x1); /* LED0 on  */
        sleep(1);
        Xil_Out32(LED_BASE + GPIO_DATA, 0x0); /* LED0 off */
        sleep(1);
    }
    return 0;
}
