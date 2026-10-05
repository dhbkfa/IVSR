#include "bsp_gpio.h"
#include "board_config.h"

typedef struct
{
    GPIO_TypeDef *enr_port;
    uint16_t      enr_pin;
    GPIO_TypeDef *enl_port;
    uint16_t      enl_pin;
} gate_hw_t;

/* Extend this table when more wheels are wired (M3). */
static const gate_hw_t s_gate[BOARD_NUM_WHEELS] =
{
    { BOARD_W0_ENR_PORT, BOARD_W0_ENR_PIN, BOARD_W0_ENL_PORT, BOARD_W0_ENL_PIN }
};

static void gate_write(const gate_hw_t *g, bool enr, bool enl)
{
    HAL_GPIO_WritePin(g->enr_port, g->enr_pin, enr ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(g->enl_port, g->enl_pin, enl ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void bsp_gpio_init(void)
{
    uint8_t i;
    bsp_bridge_enable(false);
    for (i = 0U; i < BOARD_NUM_WHEELS; i++)
    {
        gate_write(&s_gate[i], false, false);
    }
}

void bsp_led_toggle(void)
{
    HAL_GPIO_TogglePin(BOARD_LED_PORT, BOARD_LED_PIN);
}

void bsp_bridge_enable(bool enable)
{
    HAL_GPIO_WritePin(BOARD_BRIDGE_EN_PORT, BOARD_BRIDGE_EN_PIN,
                      enable ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

status_t bsp_bridge_set_dir(uint8_t wheel, bridge_dir_t dir)
{
    status_t st = ST_OK;

    if (wheel >= BOARD_NUM_WHEELS)
    {
        st = ST_ERR_PARAM;
    }
    else
    {
        switch (dir)
        {
            case BRIDGE_DIR_FWD:
                gate_write(&s_gate[wheel], true, false);
                break;
            case BRIDGE_DIR_REV:
                gate_write(&s_gate[wheel], false, true);
                break;
            case BRIDGE_DIR_BRAKE:
            default:
                gate_write(&s_gate[wheel], false, false);
                break;
        }
    }
    return st;
}
