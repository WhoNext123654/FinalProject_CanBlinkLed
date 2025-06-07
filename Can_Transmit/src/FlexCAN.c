#include "S32K144.h"  // Thu vien dinh nghia cac thanh ghi
#define MSG_BUF_SIZE  4  // So tu trong 1 bo dem tin nhan (2 tu header + 2 tu du lieu)

void FLEXCAN0_init(void) {
    uint32_t i = 0;
    // Bat clock cho module FlexCAN0
    PCC->PCCn[PCC_FlexCAN0_INDEX] |= PCC_PCCn_CGC_MASK;
    // bit 30{Clock gate control} = 1 enable clock cho FlexCAN0
    // Vo hieu hoa module truoc khi cau hinh
    CAN0->MCR |= CAN_MCR_MDIS_MASK;
    // CAN0->MCR |= 1UL << CAN_MCR_MDIS_SHIFT; // Hoac co the viet nhu the nay 1UL << 31
    // Chon nguon clock 8 MHz tu oscillator
     CAN0->CTRL1 &= ~CAN_CTRL1_CLKSRC_MASK;
    // CAN0->CTRL1 &= ~(1UL << 16)
    CAN0->MCR |= (CAN_MCR_FRZ_MASK | CAN_MCR_HALT_MASK);
    // Kich hoat module tro lai (vao che do freeze de cau hinh)
    CAN0->MCR &= ~CAN_MCR_MDIS_MASK;
    while (!((CAN0->MCR & CAN_MCR_FRZACK_MASK) >> CAN_MCR_FRZACK_SHIFT)) {}
    // FRZACK = 1, cho phep cau hinh module CAN vao che do freeze(xac nhan FRZACK = 1)
    // Cau hinh toc do CAN 500 kHz (tham so CAN0->CTRL1)
    CAN0->CTRL1 = 0x00DB0006;
    // Xoa toan bo bo dem tin nhan (128 words tuong ung 32 buffer * 4 words)
    for (i = 0; i < 128; i++) {
        CAN0->RAMn[i] = 0;
    }
    // Cau hinh bo loc, cho phep nhan tat ca ID tu mbf0-15
    for (i = 0; i < 16; i++) {
        CAN0->RXIMR[i] = 0xFFFFFFFF;
    }
    // khai bao toan cuc cho phep nhan tat ca cac ID(29bit)
    CAN0->RXMGMASK = 0x1FFFFFFF;
    // Cau hinh bo dem 4 nhan tin nhan voi ID chuan, chua kich hoat (CODE=4)
    CAN0->RAMn[4*MSG_BUF_SIZE + 1] = 2UL << 18; //ID_receive = 2
    // id[28-18] = 10100010001 = 1297 = 0x111 (hex)
    CAN0->RAMn[4*MSG_BUF_SIZE + 0] = 0x04000000; // CODE=4 (RX inactive)
    // Kich hoat module CAN, thoat freeze mode
    CAN0->MCR &= ~(CAN_MCR_FRZ_MASK | CAN_MCR_HALT_MASK);
    // => Xóa FRZ=0, HALT=0: Cho phép CAN hoạt động bình thường
    //CAN0->MCR = 0x0000001F;
    while ((CAN0->MCR & CAN_MCR_FRZACK_MASK) >> CAN_MCR_FRZACK_SHIFT) {}
    // => Đợi FRZACK=0: Xác nhận module CAN đã thoát chế độ freeze
    while ((CAN0->MCR & CAN_MCR_NOTRDY_MASK) >> CAN_MCR_NOTRDY_SHIFT) {}
    // => Đợi NOTRDY=0: Module CAN sẵn sàng hoạt động
}

void FLEXCAN0_transmit_msg(uint8_t buffer[]) {
    // Xoa co cu cua bo dem 0
    CAN0->IFLAG1 = 0x00000001;

    // Cau hinh ID chuan cho bo dem 0
    CAN0->RAMn[0*MSG_BUF_SIZE + 1] = (0x01 << 18); // ID= 1

    // Cau hinh bo dem 1 kich hoat truyen voi DLC=8 bytes, CODE=0xC (TX frame)
    CAN0->RAMn[0*MSG_BUF_SIZE + 0] = 0x0C400000 | (4 << CAN_WMBn_CS_DLC_SHIFT);

    // Gan du lieu gui vao RAM bo dem (gop 4 bytes dau thanh 1 word)
    uint32_t dataWord = 0;
    dataWord |= buffer[0];
    dataWord |= ((uint32_t)buffer[1] << 8);
    dataWord |= ((uint32_t)buffer[2] << 16);
    dataWord |= ((uint32_t)buffer[3] << 24);

    CAN0->RAMn[0*MSG_BUF_SIZE+2] = dataWord;  // Ghi du lieu vao word 2 cua MB1 (doan du lieu)
}


uint32_t FLEXCAN0_receive_msg(uint8_t *buffer_rx) {
//    if (buffer_rx == NULL) {
//        return 0;  // Kiểm tra con trỏ đầu vào
//    }

    /* Đọc thông tin từ MB4 */
    uint32_t RxCODE = (CAN0->RAMn[4 * 4 + 0] & 0x07000000) >> 24;  /* CODE field */
    if (RxCODE != 0x2) { // MB trong
        return 0;  // Kiểm tra mã trạng thái
    }
    // RxID = (CAN0->RAMn[4 * 4 + 1] & CAN_WMBn_ID_ID_MASK) >> CAN_WMBn_ID_ID_SHIFT; do ham ngat da ktra ID roi
    uint32_t RxLENGTH = (CAN0->RAMn[4 * 4 + 0] & CAN_WMBn_CS_DLC_MASK) >> CAN_WMBn_CS_DLC_SHIFT;
    uint32_t RxDATA = CAN0->RAMn[4 * 4 + 2];  /* Đọc 4 byte dữ liệu từ word 2 */
    for (uint32_t i = 0; i < RxLENGTH && i < 4; i++) {
            buffer_rx[i] = (RxDATA >> (i * 8)) & 0xFF; // Lưu trực tiếp vào buffer
        }
    for (uint32_t i = RxLENGTH; i < 4; i++) {
        buffer_rx[i] = 0; // Xóa byte không hợp lệ
    }

    /* Đọc TIMESTAMP từ MB4 (sửa lỗi từ MB0) */
//    uint32_t RxTIMESTAMP = (CAN0->RAMn[4 * 4 + 0] & 0x0000FFFF);

    /* Mở khóa MB và xóa cờ ngắt */
    (void)CAN0->TIMER;           /* Mở khóa MB */
    CAN0->IFLAG1 = 0x00000010;     /* Xóa cờ MB4 */
    return RxLENGTH;
}
