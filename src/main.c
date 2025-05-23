#include "sdk_project_config.h"
#include <interrupt_manager.h>
#include <stdint.h>
#include <stdbool.h>
#include <FlexCan.h>
#define EVB

#ifdef EVB
    #define LED_PORT        PORTD
    #define GPIO_PORT       PTD
    #define PCC_INDEX       PCC_PORTD_INDEX
    #define LED0            15U
    #define LED1            16U
#else
    #define LED_PORT        PORTC
    #define GPIO_PORT       PTC
    #define PCC_INDEX       PCC_PORTC_INDEX
    #define LED0            0U
    #define LED1            1U
#endif

#define MASTER

#if defined(MASTER)
    #define TX_MAILBOX  (1UL) // MB1
    #define TX_MSG_ID   (1UL) // 0x01
    #define RX_MAILBOX  (0UL) // MB0
    #define RX_MSG_ID   (2UL) // 0x02
#elif defined(SLAVE)
    #define TX_MAILBOX  (0UL) // MB0
    #define TX_MSG_ID   (2UL) // 0x02
    #define RX_MAILBOX  (1UL) // MB1
    #define RX_MSG_ID   (1UL) // 0x01
#endif

uint8_t buffer[4] = {0};
uint8_t buffer_rx[4] = {0};
uint32_t RxLENGTH = 0;
typedef enum {
    LED0_CHANGE_REQUESTED = 0x00U,
    LED1_CHANGE_REQUESTED = 0x01U
} can_commands_list;

uint8_t ledRequested = LED0_CHANGE_REQUESTED;

void BoardInit(void)
{
    CLOCK_DRV_Init(&clockMan1_InitConfig0);
    PINS_DRV_Init(NUM_OF_CONFIGURED_PINS0, g_pin_mux_InitConfigArr0);
}

void GPIOInit(void)
{
    PINS_DRV_SetPinsDirection(GPIO_PORT, (1 << LED1) | (1 << LED0));
    PINS_DRV_ClearPins(GPIO_PORT, (1 << LED1) | (1 << LED0));
}

volatile int exit_code = 0;
//void CAN0_ORed_0_15_MB_IRQHandler(void)
//{
//    if (CAN0->IFLAG1 & (1 << RX_MAILBOX)) {
//    	FLEXCAN0_receive_msg(buffer_rx);
//		if (buffer_rx[0] == LED0_CHANGE_REQUESTED) {
//			PINS_DRV_TogglePins(GPIO_PORT, (1 << LED0));
//		}
//		else if(buffer_rx[0] == LED0_CHANGE_REQUESTED){
//			PINS_DRV_TogglePins(GPIO_PORT, (1 << LED1));
//		}
//	}
//}

int main(void)
{
    /* Do the initializations required for this application */
    BoardInit();
    GPIOInit();

    FLEXCAN0_init();
    uint8_t rx_buffer[4] = {0};

    while(1)
    {
    	if (CAN0->IFLAG1 & (1 << RX_MAILBOX)) {
			RxLENGTH = FLEXCAN0_receive_msg(rx_buffer);
			if (RxLENGTH != 0){
				if (rx_buffer[0] == LED0_CHANGE_REQUESTED) {
					PINS_DRV_TogglePins(GPIO_PORT, (1 << LED0));
				}
				else if(rx_buffer[0] == LED0_CHANGE_REQUESTED){
					PINS_DRV_TogglePins(GPIO_PORT, (1 << LED1));
				}
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
