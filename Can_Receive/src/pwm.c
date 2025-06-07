#include "S32K144.h"  // Thu vien dinh nghia cac thanh ghi

#define FTM0_MOD_VALUE 4999        // MOD = period - 1
#define INIT_DUTY_CYCLE 2500       // 50% duty

void PWM_FTM0_Init_Register(void)
{
    /* 1. Bật Clock cho FTM0 */
    PCC->PCCn[PCC_FTM0_INDEX] |= PCC_PCCn_CGC_MASK;
    /* 2. Disable Write Protection & FTM */
    FTM0->MODE |= FTM_MODE_WPDIS_MASK;   // Tắt chế độ bảo vệ ghi
    FTM0->SC = 0;                        // Tắt bộ đếm trước khi cấu hình
    /* 3. Chọn chế độ Edge-aligned PWM cho kênh 1 */
    FTM0->CONTROLS[1].CnSC = FTM_CnSC_MSB_MASK | FTM_CnSC_ELSB_MASK;
    /* 4. Đặt giá trị MOD = chu kỳ */
    FTM0->MOD = FTM0_MOD_VALUE;
    /* 5. Đặt giá trị CnV = duty ban đầu */
    FTM0->CONTROLS[1].CnV = INIT_DUTY_CYCLE;
    /* 6. Reset CNT và CNTIN */
    FTM0->CNTIN = 0;
    FTM0->CNT = 0;
    /* 7. Chọn nguồn clock: system clock và Prescaler = 1 */
    FTM0->SC = FTM_SC_CLKS(1) | FTM_SC_PS(0);
}

typedef enum {
    PWM_STATUS_SUCCESS = 0U,
    PWM_STATUS_ERROR = 1U
} pwm_status_t;


pwm_status_t PWM_UpdateDuty_rs(uint8_t instanceIdx, uint8_t channel, uint16_t duty) {
	pwm_status_t status = PWM_STATUS_ERROR;

    // Giả sử instanceIdx 0 tương ứng với FTM0
    if (instanceIdx == 0) {
        if (channel < 8) {  // FTM có 8 channel: 0-7
            if (duty > FTM0->MOD) {
                duty = FTM0->MOD;  // Giới hạn duty không vượt quá MOD (100%)
            }
            FTM0->CONTROLS[channel].CnV = duty;
            status = PWM_STATUS_SUCCESS;
        }
    }

    return status;
}



