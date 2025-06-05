#include "S32K144.h"
#include <FlexCan.h>

/**
 * Initialize the FLEXCAN0 module for 500 kbps communication.
 */
void FLEXCAN0_init(void) {

    PCC->PCCn[PCC_FlexCAN0_INDEX] |= PCC_PCCn_CGC_MASK;

    CAN0->MCR |= CAN_MCR_MDIS_MASK;
    CAN0->CTRL1 &= ~CAN_CTRL1_CLKSRC_MASK;
    CAN0->MCR |= (CAN_MCR_FRZ_MASK | CAN_MCR_HALT_MASK);

    CAN0->MCR &= ~CAN_MCR_MDIS_MASK;
    while (!((CAN0->MCR & CAN_MCR_FRZACK_MASK) >> CAN_MCR_FRZACK_SHIFT)) {}

    CAN0->CTRL1 = 0x00DB0006;  // Set CAN baud rate to 500 kbps (based on 8 MHz clock)

    uint32_t i = 0;
    for (i = 0; i < 128; i++) CAN0->RAMn[i] = 0;

    for (i = 0; i < 16; i++)  CAN0->RXIMR[i] = 0xFFFFFFFF;

    CAN0->RXMGMASK = 0x1FFFFFFF;
    CAN0->RAMn[RX_MAILBOX * MSG_BUF_SIZE + 1] = RX_MSG_ID << 18;
    CAN0->RAMn[RX_MAILBOX * MSG_BUF_SIZE + 0] = 0x04000000; // CODE=4 (RX inactive)

    CAN0->MCR &= ~(CAN_MCR_FRZ_MASK | CAN_MCR_HALT_MASK);
    while ((CAN0->MCR & CAN_MCR_FRZACK_MASK) >> CAN_MCR_FRZACK_SHIFT) {}
    while ((CAN0->MCR & CAN_MCR_NOTRDY_MASK) >> CAN_MCR_NOTRDY_SHIFT) {}
}

/*
 * Transmit a 4-byte CAN message using FLEXCAN0 TX MailBox.
 *
 * @param buffer Pointer to 4-byte array containing the message data.
 */
void FLEXCAN0_transmit_msg(uint8_t *buffer) {

    CAN0->IFLAG1 = (1UL << TX_MAILBOX);
    CAN0->RAMn[TX_MAILBOX * MSG_BUF_SIZE + 1] = (TX_MSG_ID << 18);

    CAN0->RAMn[TX_MAILBOX * MSG_BUF_SIZE + 0] = 0x0C400000 |
    		(DLC << CAN_WMBn_CS_DLC_SHIFT);

    uint32_t dataWord = 0;
    dataWord |= buffer[0];
    dataWord |= ((uint32_t)buffer[1] << 8);
    dataWord |= ((uint32_t)buffer[2] << 16);
    dataWord |= ((uint32_t)buffer[3] << 24);

    CAN0->RAMn[TX_MAILBOX * MSG_BUF_SIZE+2] = dataWord;
}

/**
 * Receive a CAN message from FLEXCAN0 MailBox and store it in a buffer.
 *
 * @param buffer_rx Pointer to a buffer to store received data
 * @return uint32_t Number of bytes received (0 if no valid message).
 */
uint32_t FLEXCAN0_receive_msg(uint8_t *buffer_rx) {

    uint32_t RxCODE = (CAN0->RAMn[RX_MAILBOX * MSG_BUF_SIZE + 0] & 0x07000000) >> 24;

    if (RxCODE != 0x2) {
        return 0;
    }
    uint32_t RxLENGTH = (CAN0->RAMn[RX_MAILBOX * MSG_BUF_SIZE + 0] &
    		CAN_WMBn_CS_DLC_MASK) >> CAN_WMBn_CS_DLC_SHIFT;
    uint32_t RxDATA = CAN0->RAMn[RX_MAILBOX * MSG_BUF_SIZE + 2];

    for (uint32_t i = 0; i < RxLENGTH && i < DLC; i++) {
            buffer_rx[i] = (RxDATA >> (i * 8)) & 0xFF;
        }
    for (uint32_t i = RxLENGTH; i < DLC; i++) {
        buffer_rx[i] = 0;
    }

    (void)CAN0->TIMER;
    CAN0->IFLAG1 = (1UL << RX_MAILBOX);
    return RxLENGTH;
}
