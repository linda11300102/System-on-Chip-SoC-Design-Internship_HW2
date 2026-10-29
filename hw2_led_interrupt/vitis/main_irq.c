/*
 * HW2 assignment 2: LED0 blinks every 1 s, button press toggles LED1 via interrupt
 *
 * Hardware (Vivado):
 *   axi_gpio_0 @ 0x41200000
 *     GPIO  (ch1) 2-bit output : bit0 = LED0 (T22), bit1 = LED1 (T21)
 *     GPIO2 (ch2) 1-bit input  : BTNC (P16), Enable Interrupt
 *   ip2intc_irpt -> IRQ_F2P[0]  =  GIC interrupt ID 61
 */
#include "xil_io.h"
#include "xil_exception.h"
#include "xil_types.h"
#include "sleep.h"

/* ---------- AXI GPIO registers (PG144) ---------- */
#define GPIO_BASE   0x41200000U
#define GPIO_DATA   0x000U   /* ch1 data : LED                         */
#define GPIO_TRI    0x004U   /* ch1 dir  : 0 = output                  */
#define GPIO2_DATA  0x008U   /* ch2 data : button                      */
#define GPIO2_TRI   0x00CU   /* ch2 dir  : 1 = input                   */
#define GPIO_GIER   0x11CU   /* global interrupt enable (bit31)        */
#define GPIO_IPISR  0x120U   /* PENDING : interrupt status, write 1 clr*/
#define GPIO_IPIER  0x128U   /* MASK    : interrupt enable, bit1 = ch2 */

/* ---------- GIC registers (UG585 ch.7 / app.B) ---------- */
#define ICCICR   0xF8F00100U  /* CPU interface control                 */
#define ICCPMR   0xF8F00104U  /* priority mask                         */
#define ICCIAR   0xF8F0010CU  /* interrupt acknowledge (read = ID)     */
#define ICCEOIR  0xF8F00110U  /* end of interrupt                      */
#define ICDDCR   0xF8F01000U  /* distributor control                   */
#define ICDISER  0xF8F01100U  /* MASK    : set-enable,  1 bit per ID   */
#define ICDISPR  0xF8F01200U  /* PENDING : set-pending, 1 bit per ID   */
#define ICDIPR   0xF8F01400U  /* priority, 1 byte per ID               */
#define ICDIPTR  0xF8F01800U  /* target CPU, 1 byte per ID             */
#define ICDICFR  0xF8F01C00U  /* trigger type, 2 bits per ID           */

#define BTN_IRQ_ID  61U       /* IRQ_F2P[0] */
#define NUM_IRQ     96U

/* ---------- IRQ table : interrupt ID -> ISR ---------- */
typedef void (*isr_t)(void);
static isr_t irq_table[NUM_IRQ];

/* LED shadow: bit0 = main loop (LED0), bit1 = ISR (LED1).
 * Shared by main and ISR, so it must be volatile. */
static volatile u32 led_shadow  = 0;
static volatile u32 press_count = 0;

static void led_write(void)
{
    Xil_Out32(GPIO_BASE + GPIO_DATA, led_shadow);
}

/* ---------- ISR for the button ---------- */
static void button_isr(void)
{
    /* AXI GPIO interrupts on ANY change (press and release);
     * only act when the button is pressed (= 1). */
    if (Xil_In32(GPIO_BASE + GPIO2_DATA) & 0x1U) {
        led_shadow ^= 0x2U;          /* toggle LED1 */
        led_write();
        press_count++;
    }
    /* clear PENDING bit, otherwise the IRQ fires again forever */
    Xil_Out32(GPIO_BASE + GPIO_IPISR, 0x2U);
}

/* ---------- IRQ entry: BSP calls this on every CPU IRQ ---------- */
static void irq_dispatcher(void *unused)
{
    (void)unused;
    u32 iar = Xil_In32(ICCIAR);      /* 1. acknowledge, get ID */
    u32 id  = iar & 0x3FFU;
    if (id < NUM_IRQ && irq_table[id] != 0)
        irq_table[id]();             /* 2. look up IRQ table, call ISR */
    Xil_Out32(ICCEOIR, iar);         /* 3. tell GIC we are done */
}

/* write one byte inside a "1 byte per ID" GIC register */
static void gic_write_byte(u32 base, u32 id, u8 val)
{
    u32 addr  = base + (id & ~3U);
    u32 shift = (id & 3U) * 8U;
    u32 v = Xil_In32(addr);
    v &= ~(0xFFU << shift);
    v |= ((u32)val << shift);
    Xil_Out32(addr, v);
}

static void gic_enable_irq(u32 id, u8 priority, isr_t isr)
{
    irq_table[id] = isr;                         /* fill IRQ table   */
    gic_write_byte(ICDIPR,  id, priority);       /* priority         */
    gic_write_byte(ICDIPTR, id, 0x01);           /* send to CPU0     */

    u32 addr  = ICDICFR + (id / 16U) * 4U;       /* 01 = level high  */
    u32 shift = (id % 16U) * 2U;
    u32 v = Xil_In32(addr);
    v &= ~(0x3U << shift);
    v |=  (0x1U << shift);
    Xil_Out32(addr, v);

    Xil_Out32(ICDISER + (id / 32U) * 4U, 1U << (id % 32U));  /* unmask */
}

int main(void)
{
    /* 1. GPIO direction */
    Xil_Out32(GPIO_BASE + GPIO_TRI,  0x0);       /* ch1: LED output  */
    Xil_Out32(GPIO_BASE + GPIO2_TRI, 0x1);       /* ch2: button input*/
    led_write();

    /* 2. GIC: enable ID 61 and register ISR */
    Xil_Out32(ICDDCR, 0x0);
    gic_enable_irq(BTN_IRQ_ID, 0xA0, button_isr);
    Xil_Out32(ICDDCR, 0x1);                      /* distributor on   */
    Xil_Out32(ICCPMR, 0xF0);                     /* allow prio < F0  */
    Xil_Out32(ICCICR, 0x07);                     /* CPU interface on */

    /* 3. AXI GPIO interrupt */
    Xil_Out32(GPIO_BASE + GPIO_IPISR, Xil_In32(GPIO_BASE + GPIO_IPISR)); /* clear old */
    Xil_Out32(GPIO_BASE + GPIO_IPIER, 0x2);      /* unmask ch2       */
    Xil_Out32(GPIO_BASE + GPIO_GIER,  0x80000000U);

    /* 4. let the CPU take IRQs */
    Xil_ExceptionInit();
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,
                                 (Xil_ExceptionHandler)irq_dispatcher, NULL);
    Xil_ExceptionEnable();

    /* main loop: LED0 toggles every second.
     * The CPU spends most time inside sleep(1), yet LED1 reacts
     * instantly because the interrupt preempts main. */
    while (1) {
        Xil_ExceptionDisable();      /* critical section: led_shadow shared with ISR */
        led_shadow ^= 0x1U;          /* toggle LED0 */
        led_write();
        Xil_ExceptionEnable();
        sleep(1);
    }
    return 0;
}
