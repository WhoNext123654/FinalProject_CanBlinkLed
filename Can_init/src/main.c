#include "S32K144.h"

#define LED0_PIN     12
#define LED0_PORT    PTC

// Delay đơn giản
void delay(volatile int cycles) {
    while(cycles--);
}

// Khởi tạo GPIO cho LED0 (PTC12)
void GPIO_init(void) {
    PCC->PCCn[PCC_PORTC_INDEX] |= PCC_PCCn_CGC_MASK;   // Bật clock PORTC
    PORTC->PCR[LED0_PIN] = PORT_PCR_MUX(1);            // GPIO mode
    LED0_PORT->PDDR |= (1 << LED0_PIN);                // Output mode
    LED0_PORT->PSOR |= (1 << LED0_PIN);                // Tắt LED ban đầu
}

// Khởi tạo CAN0
int can_init(void) {
    PCC->PCCn[PCC_FlexCAN0_INDEX] &= ~PCC_PCCn_CGC_MASK;
    PCC->PCCn[PCC_FlexCAN0_INDEX] |= PCC_PCCn_PCS(6) | PCC_PCCn_CGC_MASK;

    CAN0->MCR |= CAN_MCR_MDIS_MASK;
    CAN0->CTRL1 &= ~CAN_CTRL1_CLKSRC_MASK;
    CAN0->MCR &= ~CAN_MCR_MDIS_MASK;

    while (!(CAN0->MCR & CAN_MCR_FRZACK_MASK));
    CAN0->MCR |= CAN_MCR_FRZ_MASK | CAN_MCR_HALT_MASK;
    while (!(CAN0->MCR & CAN_MCR_FRZACK_MASK));

    CAN0->CTRL1 = 0x00DB0006;  // 500 kbps @ 8 MHz
    CAN0->MCR = 0x0001C000;

    for (int i = 0; i < 32; i++) {
        CAN0->RAMn[4 * i + 1] = 0;
        CAN0->RAMn[4 * i + 2] = 0;
        CAN0->RAMn[4 * i + 3] = 0;
    }

    CAN0->RXMGMASK = 0x1FFFFFFF;
    CAN0->MCR &= ~CAN_MCR_HALT_MASK;

    // Chờ thoát chế độ freeze
    int timeout = 100000;
    while ((CAN0->MCR & CAN_MCR_FRZACK_MASK) && --timeout);

    if (timeout == 0) return -1; // Lỗi timeout khi thoát freeze
    return 0; // Thành công
}

// Hàm chính
int main(void) {
    GPIO_init();           // Khởi tạo LED
    int canStatus = can_init();  // Khởi tạo CAN

    if (canStatus == 0) {
        // Nếu CAN khởi tạo thành công, nhấp nháy LED0
        while (1) {
            LED0_PORT->PTOR |= (1 << LED0_PIN); // Toggle LED
            delay(500000);
        }
    } else {
        // Nếu CAN lỗi, không làm gì cả (LED không nhấp nháy)
        while (1);
    }

    return 0;
}
