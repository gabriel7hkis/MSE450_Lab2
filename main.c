/*
 * Author: Tom Liang
 * Date: Feb 8, 2019
 * Objective: configure GPIO interrupt and SysTick interrupt.
 * Use SysTick_Handler() to blink the blue LED when SW1 is released.
 * When SW1 is pressed, GPIO handler should light up the red LED.
 */

#include "SysTickInts.h"
#include "PLL.h"
#include "tm4c123gh6pm.h"
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"

void PortF_Init(void);
void disable_interrupts(void);
void enable_interrupts(void);
void wait_for_interrupts(void);
//void GPIO_handler(void);

unsigned long TOGGLE_COUNT = 1000;


volatile unsigned long count = 0;
volatile unsigned long In, Out;


/* main */
int main(void){
  PLL_Init();                 // bus clock at 80 MHz
  PortF_Init();
  count = 0;

  SysTick_Init(80000);        // initialize SysTick timer
  enable_interrupts();




  while(1){                   // i nterrupts every 1ms
      wait_for_interrupts();
  }


}


/* Initialize PortF GPIOs */
void PortF_Init(void) {
    SYSCTL_RCGC2_R |= 0x00000020;           // activate clock for PortF
    while ((SYSCTL_PRGPIO_R & 0x00000020) == 0)
    {};                          // wait until PortF is ready
    GPIO_PORTF_LOCK_R = 0x4C4F434B;         // unlock GPIO PortF
    GPIO_PORTF_CR_R = 0x1F;                 // allow changes to PF4-0
    GPIO_PORTF_AMSEL_R = 0x00;              // disable analog on PortF
    GPIO_PORTF_PCTL_R = 0x00000000;         // use PF4-0 as GPIO
    GPIO_PORTF_DIR_R = 0x0E;                // PF4,PF0 in, PF3-1 out
    GPIO_PORTF_AFSEL_R = 0x00;              // disable alt function on PF
    GPIO_PORTF_PUR_R = 0x11;                // enable pull-up on PF0,PF4
    GPIO_PORTF_DEN_R = 0x1F;                // enable digital I/O on PF4-0

    //Interrupt

    //Setting up GPIO_IEV for PORT_F

    GPIO_PORTF_IS_R &= 0xEF;  // Interrupt Sense
    GPIO_PORTF_IBE_R &= 0xEF; // Interrupt Both Edges
    GPIO_PORTF_IEV_R &= 0xEF; // Edge detection
    GPIO_PORTF_IM_R |= 0x10;  // Masking
    GPIO_PORTF_ICR_R = 0x10; // clear
    NVIC_EN0_R = 0x40000000;
    enable_interrupts();

}


/* Disable interrupts by setting the I bit in the PRIMASK system register */
void disable_interrupts(void) {
    __asm("    CPSID  I\n"
          "    BX     LR");
}


/* Enable interrupts by clearing the I bit in the PRIMASK system register */
void enable_interrupts(void) {
    __asm("    CPSIE  I\n"
          "    BX     LR");
}


/* Enter low-power mode while waiting for interrupts */
void wait_for_interrupts(void) {
    __asm("    WFI\n"
          "    BX     LR");
}


/* Interrupt service routine for SysTick Interrupt */
// Executed every 12.5ns*(period)
void SysTick_Handler(void){
    count++;
    if (count == TOGGLE_COUNT-1) {
        count = 0;
        //GPIO_PORTF_DATA_R &= ~0x02; // clear PF1
        GPIO_PORTF_DATA_R ^= 0x04; // toggle PF2
    }

}
void GPIO_handler(void){

    //GPIO_PORTF_IEV_R = 0x00;
    GPIO_PORTF_ICR_R = 0x10;
    //GPIO_PORTF_DATA_R &= ~0x04; // clear PF1
    GPIO_PORTF_DATA_R ^= 0x02; //RED LED
}
