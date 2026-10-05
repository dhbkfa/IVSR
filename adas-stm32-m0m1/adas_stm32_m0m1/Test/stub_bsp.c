/* Host stub of the BSP used only by the motor driver unit test. */
#include <stdint.h>
#include "bsp_gpio.h"
#include "bsp_pwm.h"

uint16_t     g_pwm;
bridge_dir_t g_dir;
unsigned     g_dir_changes;
unsigned     g_dir_change_with_pwm;   /* must stay 0: PWM must be 0 when gates change */

status_t bsp_pwm_set_permille(uint8_t w, uint16_t p) { (void)w; g_pwm = p; return ST_OK; }
status_t bsp_pwm_start(uint8_t w) { (void)w; return ST_OK; }
status_t bsp_bridge_set_dir(uint8_t w, bridge_dir_t d)
{
    (void)w;
    g_dir_changes++;
    if (g_pwm != 0U) { g_dir_change_with_pwm++; }
    g_dir = d;
    return ST_OK;
}
void bsp_gpio_init(void) {}
void bsp_led_toggle(void) {}
void bsp_bridge_enable(_Bool e) { (void)e; }
