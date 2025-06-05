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
    #define BTN_GPIO        PTC
    #define BTN1_PIN        13U
    #define BTN2_PIN        12U
    #define BTN_PORT        PORTC
    #define BTN_PORT_IRQn   PORTC_IRQn

#else
    #define LED_PORT        PORTC
    #define GPIO_PORT       PTC
    #define PCC_INDEX       PCC_PORTC_INDEX
    #define LED0            0U
    #define LED1            1U
#endif



uint8_t buffer[4] = {0};
uint8_t buffer_rx[4] = {0};
uint32_t RxLENGTH = 0;
typedef enum {
    LED0_CHANGE_REQUESTED = 0x00U,
    LED1_CHANGE_REQUESTED = 0x01U
} can_commands_list;

uint8_t ledRequested = LED0_CHANGE_REQUESTED;

volatile uint8_t speed = 0;
volatile uint32_t last_valid_press_time_for_sequence = 0;
volatile uint32_t last_raw_interrupt_time = 0; // Cho việc chống nhiễu (debounce)

// Hằng số 
#define MULTI_PRESS_TIMEOUT_MS  500U // 0.5 giây
#define DEBOUNCE_PERIOD_MS      50U  // 50 mili giây

void LPIT0_Init(void)
{
    PCC->PCCn[PCC_LPIT_INDEX] |= PCC_PCCn_CGC_MASK; // Bật clock LPIT0
    LPIT0->MCR = 0x00000001; // Enable module, debug mode run

    LPIT0->TMR[0].TVAL = 8000 - 1; // 8 MHz / 8000 = 1 kHz => 1 ms
    LPIT0->TMR[0].TCTRL = 0x00000021; // Enable timer, periodic, interrupt enabled

    LPIT0->MIER = 0x00000001; // Enable interrupt channel 0
    INT_SYS_EnableIRQ(LPIT0_Ch0_IRQn);
}


volatile uint32_t g_millis = 0;

void LPIT0_Ch0_IRQHandler(void)
{
    LPIT0->MSR |= LPIT_MSR_TIF0_MASK; // Clear flag
    g_millis++;
}

uint32_t Sys_GetMillis(void)
{
    return g_millis;
}
volatile uint32_t current_time;
volatile bool g_send_flag = false;
volatile uint8_t g_speed_value_to_send = 0;

void buttonISR(void)
{

    if ((PINS_DRV_GetPortIntFlag(BTN_PORT) & (1 << BTN1_PIN)) != 0)
    {

        current_time = Sys_GetMillis();
        if ((current_time - last_raw_interrupt_time) < DEBOUNCE_PERIOD_MS) {
            PINS_DRV_ClearPinIntFlagCmd(BTN_PORT, BTN1_PIN);
            return;
        }
        last_raw_interrupt_time = current_time;

        if ((speed == 0) | ((current_time - last_valid_press_time_for_sequence) < MULTI_PRESS_TIMEOUT_MS)) {
            speed++;
            PINS_DRV_TogglePins(GPIO_PORT, (1 << LED1));
            if (speed > 3) {
            	speed = 3;
            }
        }
        last_valid_press_time_for_sequence = current_time; // Cập nhật thời gian cho lần nhấn này
        g_speed_value_to_send = speed;
        g_send_flag = true;
        // Xóa cờ ngắt cho nút đã được xử lý
        PINS_DRV_ClearPinIntFlagCmd(BTN_PORT, BTN1_PIN);
    }
}

void BoardInit(void)
{
    CLOCK_DRV_Init(&clockMan1_InitConfig0);
    PINS_DRV_Init(NUM_OF_CONFIGURED_PINS0, g_pin_mux_InitConfigArr0);
}

/*
 * @brief Function which configures the LEDs and Buttons
 */
void GPIOInit(void)
{
	/* Output direction for LEDs */
	    PINS_DRV_SetPinsDirection(GPIO_PORT, (1 << LED1) | (1 << LED0));

	    /* Set Output value LEDs */
	    PINS_DRV_ClearPins(GPIO_PORT, (1 << LED1) | (1 << LED0));
	/* Setup button pin */
	    PINS_DRV_SetPinsDirection(BTN_GPIO, ~((1 << BTN1_PIN)));
    /* Setup button pins interrupt */
    PINS_DRV_SetPinIntSel(BTN_PORT, BTN1_PIN, PORT_INT_RISING_EDGE);
    /* Install buttons ISR */
    INT_SYS_InstallHandler(BTN_PORT_IRQn, &buttonISR, NULL);

    /* Enable buttons interrupt */
    INT_SYS_EnableIRQ(BTN_PORT_IRQn);
}


volatile int exit_code = 0;


int main(void)
{
    /* Do the initializations required for this application */
    BoardInit();
    GPIOInit();
    LPIT0_Init();
    FLEXCAN0_init();


    while(1)
    {
    	if (g_send_flag && (g_millis > current_time + MULTI_PRESS_TIMEOUT_MS))
    	{
    	    uint8_t tx_buf[4] = {g_speed_value_to_send, 0, 0, 0};
    	    FLEXCAN0_transmit_msg(tx_buf);
    	    g_send_flag = false;
    	    speed = 0;
    	}

    }

    for(;;) {
      if(exit_code != 0) {
        break;
      }
    }
    return exit_code;
}
