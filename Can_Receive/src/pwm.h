#ifndef PWM_H_
#define PWM_H_

#include <stdint.h>

typedef enum {
    PWM_STATUS_SUCCESS = 0U,
    PWM_STATUS_ERROR = 1U
} pwm_status_t;


/**
 * @brief Khởi tạo FTM0 Channel 1 ở chế độ Edge-Aligned PWM.
 */
void PWM_FTM0_Init_Register(void);

/**
 * @brief Cập nhật duty cycle cho FTM0 Channel.
 *
 * @param instanceIdx: Số hiệu FTM instance (ví dụ: 0 cho FTM0)
 * @param channel: Số channel (ví dụ: 1 cho FTM0_CH1)
 * @param duty: Giá trị duty tính bằng số tick (tối đa là MOD)
 * @return status_t: STATUS_SUCCESS nếu thành công, STATUS_ERROR nếu sai
 */
status_t PWM_UpdateDuty_rs(uint8_t instanceIdx, uint8_t channel, uint16_t duty);

#endif /* PWM_H_ */
