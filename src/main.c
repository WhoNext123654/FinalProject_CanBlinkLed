#include "sdk_project_config.h"
#include <stdint.h>
#include <stdbool.h>
#include <FlexCan.h>

uint8_t rx_buffer[4] = {0};
uint32_t RxLENGTH = 0;
uint32_t duty_cycle = 0;

void BoardInit(void)
{
    CLOCK_DRV_Init(&clockMan1_InitConfig0);
    PINS_DRV_Init(NUM_OF_CONFIGURED_PINS0, g_pin_mux_InitConfigArr0);
}

volatile int exit_code = 0;
void Update_PWM(uint8_t mode) {

    switch (mode) {
        case 1: duty_cycle = 2000; break;
        case 2: duty_cycle = 3000; break;
        case 3: duty_cycle = 4000; break;
        default: duty_cycle = 0; break;
    }
}

int main(void)
{
    BoardInit();
    FLEXCAN0_init();

    uint8_t channel = pwm_pal_1_configs.pwmChannels[0].channel;
    status_t status = STATUS_SUCCESS;
    status = CLOCK_DRV_Init(&clockMan1_InitConfig0);
    DEV_ASSERT(status == STATUS_SUCCESS);
    status = PINS_DRV_Init(NUM_OF_CONFIGURED_PINS0, g_pin_mux_InitConfigArr0);
    DEV_ASSERT(status == STATUS_SUCCESS);
    status = PWM_Init(&pwm_pal_1_instance, &pwm_pal_1_configs);
    DEV_ASSERT(status == STATUS_SUCCESS);

    while(1)
    {
        status = PWM_UpdateDuty(&pwm_pal_1_instance, channel, duty_cycle);
        DEV_ASSERT(status == STATUS_SUCCESS);

    	if (CAN0->IFLAG1 & (1UL << RX_MAILBOX)) {
			RxLENGTH = FLEXCAN0_receive_msg(rx_buffer);
			if (RxLENGTH != 0){
				FLEXCAN0_transmit_msg(rx_buffer);
				Update_PWM(rx_buffer[0]);
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
