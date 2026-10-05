#include "bsp_encoder.h"
#include "board_config.h"

typedef struct
{
    TIM_HandleTypeDef *htim;
    uint8_t            bits;
} enc_hw_t;

/* Extend this table when more wheels are wired:
 *   { &htim2, 32U }, { &htim3, 16U }, { &htim4, 16U }, { &htim5, 32U }       */
static const enc_hw_t s_enc[BOARD_NUM_WHEELS] =
{
    { &htim3, 16U }
};

#define ENC_SMS_MODE3   (TIM_SMCR_SMS_1 | TIM_SMCR_SMS_0)   /* SMS = 0b011 */

status_t bsp_encoder_start(uint8_t wheel)
{
    status_t st = ST_OK;

    if (wheel >= BOARD_NUM_WHEELS)
    {
        st = ST_ERR_PARAM;
    }
    else if (HAL_TIM_Encoder_Start(s_enc[wheel].htim, TIM_CHANNEL_ALL) != HAL_OK)
    {
        st = ST_ERR_HW;
    }
    else
    {
        /* nothing more to do */
    }
    return st;
}

status_t bsp_encoder_read(uint8_t wheel, uint32_t *raw)
{
    status_t st = ST_OK;

    if ((wheel >= BOARD_NUM_WHEELS) || (raw == NULL))
    {
        st = ST_ERR_PARAM;
    }
    else
    {
        *raw = __HAL_TIM_GET_COUNTER(s_enc[wheel].htim);
    }
    return st;
}

uint8_t bsp_encoder_bits(uint8_t wheel)
{
    return (wheel < BOARD_NUM_WHEELS) ? s_enc[wheel].bits : 0U;
}

status_t bsp_encoder_check(uint8_t wheel)
{
    status_t st = ST_OK;

    if (wheel >= BOARD_NUM_WHEELS)
    {
        st = ST_ERR_PARAM;
    }
    else
    {
        const TIM_TypeDef *t   = s_enc[wheel].htim->Instance;
        const uint32_t     arr = (s_enc[wheel].bits == 16U) ? 0xFFFFU : 0xFFFFFFFFU;

        if ((t->CR1 & TIM_CR1_CEN) == 0U)
        {
            st = ST_ERR_STATE;
        }
        else if ((t->SMCR & TIM_SMCR_SMS) != ENC_SMS_MODE3)
        {
            st = ST_ERR_CONFIG;
        }
        else if (t->ARR != arr)
        {
            st = ST_ERR_CONFIG;
        }
        else
        {
            /* all good */
        }
    }
    return st;
}
