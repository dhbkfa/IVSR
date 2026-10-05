#include "bsp_pwm.h"
#include "board_config.h"

#define PWM_FULL_PERMILLE   (1000U)

typedef struct
{
    TIM_HandleTypeDef *htim;
    uint32_t           channel;
} pwm_hw_t;

/* Extend this table when more wheels are wired (M3: TIM1 CH1..CH4). */
static const pwm_hw_t s_pwm[BOARD_NUM_WHEELS] =
{
    { &htim1, TIM_CHANNEL_1 }
};

status_t bsp_pwm_start(uint8_t wheel)
{
    status_t st = ST_OK;

    if (wheel >= BOARD_NUM_WHEELS)
    {
        st = ST_ERR_PARAM;
    }
    else
    {
        __HAL_TIM_SET_COMPARE(s_pwm[wheel].htim, s_pwm[wheel].channel, 0U);
        if (HAL_TIM_PWM_Start(s_pwm[wheel].htim, s_pwm[wheel].channel) != HAL_OK)
        {
            st = ST_ERR_HW;
        }
    }
    return st;
}

status_t bsp_pwm_set_permille(uint8_t wheel, uint16_t permille)
{
    status_t st = ST_OK;

    if (wheel >= BOARD_NUM_WHEELS)
    {
        st = ST_ERR_PARAM;
    }
    else
    {
        const uint32_t p      = (permille > PWM_FULL_PERMILLE) ? PWM_FULL_PERMILLE : permille;
        const uint32_t period = __HAL_TIM_GET_AUTORELOAD(s_pwm[wheel].htim) + 1U;
        /* 16-bit timers (TIM1/9/10/11): p * period <= 1000 * 65536 fits in 32 bit */
        const uint32_t cmp    = (p * period) / PWM_FULL_PERMILLE;
        __HAL_TIM_SET_COMPARE(s_pwm[wheel].htim, s_pwm[wheel].channel, cmp);
    }
    return st;
}
