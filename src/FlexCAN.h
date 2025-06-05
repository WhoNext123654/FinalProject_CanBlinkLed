#ifndef FLEXCAN_H_
#define FLEXCAN_H_

#include <stdint.h>

#define MSG_BUF_SIZE  4
#define TX_MAILBOX  (0UL) // MB0
#define TX_MSG_ID   (1UL) // 0x01
#define RX_MAILBOX  (4UL) // MB4
#define RX_MSG_ID   (2UL) // 0x02
#define DLC 		(4UL) // 4 bytes

void FLEXCAN0_init(void);
void FLEXCAN0_transmit_msg(uint8_t *buffer);
uint32_t FLEXCAN0_receive_msg(uint8_t *buffer);

#endif /* FLEXCAN_H_ */
