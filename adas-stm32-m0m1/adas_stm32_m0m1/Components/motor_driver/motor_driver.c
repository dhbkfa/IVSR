#include "motor_driver.h"
#include "bsp_gpio.h"
#include "bsp_pwm.h"
#include "board_config.h"

#define MOT_FULL (1000)

static int8_t sign_of(int32_t v)
{
    int8_t s = 0;

    if (v > 0)
    {
        s = 1;
    }
    else if (v < 0)
    {
        s = -1;
    }
    else
    {
        /* zero */
    }
    return s;
}

static int32_t abs_i32(int32_t v)
{
    return (v < 0) ? -v : v;
}

static int32_t clamp_i32(int32_t v, int32_t lo, int32_t hi)
{
    int32_t r = v;

    if (r < lo)
    {
        r = lo;
    }
    if (r > hi)
    {
        r = hi;
    }
    return r;
}

static bool cfg_valid(const mot_cfg_t *c)
{
    return (c->wheel_idx < BOARD_NUM_WHEELS) &&
           (c->max_duty_permille >= 1) && (c->max_duty_permille <= MOT_FULL) &&
           (c->deadband_permille >= 0) && (c->deadband_permille < c->max_duty_permille) &&
           (c->ramp_permille_per_tick >= 1);
}

static void apply_hw(mot_t *m, int8_t want, int32_t eff_permille)
{
    if (want != m->cur_dir)
    {
        /* Always remove the PWM before touching the direction gates. */
        (void)bsp_pwm_set_permille(m->cfg.wheel_idx, 0U);
        if (want > 0)
        {
            (void)bsp_bridge_set_dir(m->cfg.wheel_idx, BRIDGE_DIR_FWD);
        }
        else if (want < 0)
        {
            (void)bsp_bridge_set_dir(m->cfg.wheel_idx, BRIDGE_DIR_REV);
        }
        else
        {
            (void)bsp_bridge_set_dir(m->cfg.wheel_idx, BRIDGE_DIR_BRAKE);
        }
        m->cur_dir = want;
    }
    (void)bsp_pwm_set_permille(m->cfg.wheel_idx, (uint16_t)eff_permille);
}

status_t mot_init(mot_t *m, const mot_cfg_t *cfg)
{
    status_t st = ST_OK;

    if ((m == NULL) || (cfg == NULL))
    {
        st = ST_ERR_PARAM;
    }
    else if (!cfg_valid(cfg))
    {
        st = ST_ERR_CONFIG;
    }
    else
    {
        m->cfg           = *cfg;
        m->target        = 0;
        m->output        = 0;
        m->cur_dir       = 1;     /* force apply_hw() to program the brake state */
        m->deadtime_left = 0U;
        apply_hw(m, 0, 0);
    }
    return st;
}

void mot_set_target(mot_t *m, int16_t permille)
{
    if (m != NULL)
    {
        m->target = (int16_t)clamp_i32(permille, -MOT_FULL, MOT_FULL);
    }
}

void mot_stop(mot_t *m)
{
    if (m != NULL)
    {
        m->target        = 0;
        m->output        = 0;
        m->deadtime_left = 0U;
        apply_hw(m, 0, 0);
    }
}

void mot_update(mot_t *m)
{
    int32_t out;
    int32_t eff = 0;
    int32_t lim;
    int32_t tgt;
    int8_t  want;

    if (m == NULL)
    {
        return;
    }

    lim = (int32_t)m->cfg.max_duty_permille;
    tgt = clamp_i32((int32_t)m->target, -lim, lim);
    out = (int32_t)m->output;

    /* 1) slew-rate limit */
    if (tgt > out)
    {
        out = clamp_i32(out + (int32_t)m->cfg.ramp_permille_per_tick, -lim, tgt);
    }
    else if (tgt < out)
    {
        out = clamp_i32(out - (int32_t)m->cfg.ramp_permille_per_tick, tgt, lim);
    }
    else
    {
        /* already at target */
    }

    /* 2) safe direction change: hold zero for deadtime_ticks */
    if (m->deadtime_left > 0U)
    {
        m->deadtime_left--;
        out = 0;
    }
    else if ((out != 0) && (m->cur_dir != 0) && (sign_of(out) != m->cur_dir))
    {
        out = 0;
        m->deadtime_left = m->cfg.deadtime_ticks;
    }
    else if ((out == 0) && ((int32_t)m->output != 0))
    {
        m->deadtime_left = m->cfg.deadtime_ticks;
    }
    else
    {
        /* normal operation */
    }

    /* 3) dead-band compensation: map |out| in (0..lim] to (db..lim] */
    want = sign_of(out);
    if (want != 0)
    {
        const int32_t db = (int32_t)m->cfg.deadband_permille;
        eff = db + ((abs_i32(out) * (lim - db)) / lim);
    }

    apply_hw(m, want, eff);
    m->output = (int16_t)out;
}

status_t mot_set_limit(mot_t *m, int16_t max_duty_permille)
{
    status_t st = ST_OK;

    if (m == NULL)
    {
        st = ST_ERR_PARAM;
    }
    else if ((max_duty_permille < 1) || (max_duty_permille > MOT_FULL) ||
             (max_duty_permille <= m->cfg.deadband_permille))
    {
        st = ST_ERR_CONFIG;
    }
    else
    {
        m->cfg.max_duty_permille = max_duty_permille;
    }
    return st;
}

status_t mot_set_deadband(mot_t *m, int16_t deadband_permille)
{
    status_t st = ST_OK;

    if (m == NULL)
    {
        st = ST_ERR_PARAM;
    }
    else if ((deadband_permille < 0) || (deadband_permille >= m->cfg.max_duty_permille))
    {
        st = ST_ERR_CONFIG;
    }
    else
    {
        m->cfg.deadband_permille = deadband_permille;
    }
    return st;
}

int16_t mot_get_output(const mot_t *m)
{
    return (m != NULL) ? m->output : 0;
}
