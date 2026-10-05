#include <assert.h>
#include <stdio.h>
#include "motor_driver.h"
#include "bsp_gpio.h"

extern uint16_t g_pwm;
extern bridge_dir_t g_dir;
extern unsigned g_dir_change_with_pwm;

static mot_cfg_t cfg(int16_t lim, int16_t db, int16_t ramp, uint8_t dt)
{
    mot_cfg_t c;
    c.wheel_idx = 0U; c.max_duty_permille = lim; c.deadband_permille = db;
    c.ramp_permille_per_tick = ramp; c.deadtime_ticks = dt;
    return c;
}

int main(void)
{
    mot_t m;
    mot_cfg_t c;
    int i;

    /* init puts the bridge in BRAKE with zero PWM */
    c = cfg(300, 0, 10, 4U);
    assert(mot_init(&m, &c) == ST_OK);
    assert(g_dir == BRIDGE_DIR_BRAKE && g_pwm == 0U);

    /* ramp: +10 per tick toward 100, never exceeds the target */
    mot_set_target(&m, 100);
    mot_update(&m);  assert(g_pwm == 10U  && g_dir == BRIDGE_DIR_FWD);
    for (i = 0; i < 20; i++) { mot_update(&m); }
    assert(g_pwm == 100U);

    /* hard limit: request 1000, output stops at 300 */
    mot_set_target(&m, 1000);
    for (i = 0; i < 100; i++) { mot_update(&m); }
    assert(g_pwm == 300U);

    /* reversal: ramp down, hold zero for deadtime, then reverse; gates only change at PWM = 0 */
    mot_set_target(&m, -200);
    for (i = 0; i < 100; i++) { mot_update(&m); }
    assert(g_dir == BRIDGE_DIR_REV && g_pwm == 200U);
    assert(g_dir_change_with_pwm == 0U);

    /* dead-band compensation: limit 300, deadband 100: |out|=150 -> 100 + 150*200/300 = 200 */
    c = cfg(300, 100, 1000, 0U);
    assert(mot_init(&m, &c) == ST_OK);
    mot_set_target(&m, 150);
    mot_update(&m);
    assert(g_pwm == 200U);
    mot_set_target(&m, 300);
    mot_update(&m);
    assert(g_pwm == 300U);            /* never above the limit */

    /* emergency stop */
    mot_stop(&m);
    assert(g_pwm == 0U && g_dir == BRIDGE_DIR_BRAKE && mot_get_output(&m) == 0);

    /* config validation */
    c = cfg(300, 300, 10, 4U);  assert(mot_init(&m, &c) == ST_ERR_CONFIG);   /* deadband >= limit */
    c = cfg(0, 0, 10, 4U);      assert(mot_init(&m, &c) == ST_ERR_CONFIG);
    c = cfg(300, 0, 0, 4U);     assert(mot_init(&m, &c) == ST_ERR_CONFIG);
    c = cfg(300, 0, 10, 4U);    c.wheel_idx = 9U;
    assert(mot_init(&m, &c) == ST_ERR_CONFIG);
    assert(mot_init(NULL, &c) == ST_ERR_PARAM);

    puts("test_motor: OK");
    return 0;
}
