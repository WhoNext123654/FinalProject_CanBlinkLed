#include "sdk_project_config.h"
#include <interrupt_manager.h>
#include <stdint.h>
#include <stdbool.h>
#include <FlexCan.h>
#include <pwm.h>

#define TX_MAILBOX  (1UL) // MB1
#define TX_MSG_ID   (1UL) // 0x01
#define RX_MAILBOX  (4UL) // MB4
#define RX_MSG_ID   (2UL) // 0x02


uint8_t rx_buffer[4] = {0};
uint32_t RxLENGTH = 0;
uint32_t duty_cycle = 0;
volatile int exit_code = 0;


void BoardInit(void)
{
    CLOCK_DRV_Init(&clockMan1_InitConfig0);
    PINS_DRV_Init(NUM_OF_CONFIGURED_PINS0, g_pin_mux_InitConfigArr0);
}


void Update_PWM(uint8_t mode) {
    switch (mode) {
        case 1: duty_cycle = 2000; break; // 40%
        case 2: duty_cycle = 3000; break; // 60%
        case 3: duty_cycle = 4000; break; // 80%
        default: duty_cycle = 0; break;
    }
}

int main(void)
{
    /* Do the initializations required for this application */
    BoardInit();
    FLEXCAN0_init();
    PWM_FTM0_Init_Register();

    Update_PWM(0);
    PWM_UpdateDuty_rs(0, 1, duty_cycle);

    while(1)
    {
        if (CAN0->IFLAG1 & (1 << RX_MAILBOX)) {
            RxLENGTH = FLEXCAN0_receive_msg(rx_buffer);
            if (RxLENGTH != 0){
                FLEXCAN0_transmit_msg(rx_buffer);
                Update_PWM(rx_buffer[0]);
            	PWM_UpdateDuty_rs(0, 1, duty_cycle);
            }
        }
    }

    for(;;) {
      if(exit_code != 0) {
        break;
      }
    }

    return exit_code;
}
