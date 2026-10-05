#include "wheel.h"
#include "bsp_encoder.h"

status_t wheel_init(wheel_t *w, uint8_t idx, const wheel_cfg_t *cfg)
{
    status_t  st;
    wheel_cfg_t c;

    if ((w == NULL) || (cfg == NULL))
    {
        return ST_ERR_PARAM;
    }

    c = *cfg;
    c.enc.counter_bits = bsp_encoder_bits(idx);   /* single source of truth */
    c.mot.wheel_idx    = idx;
    w->idx             = idx;

    st = enc_init(&w->enc, &c.enc);
    if (st == ST_OK)
    {
        st = mot_init(&w->mot, &c.mot);
    }
    if (st == ST_OK)
    {
        st = wheel_reset_encoder(w);
    }
    return st;
}

status_t wheel_step(wheel_t *w)
{
    status_t st;
    uint32_t raw = 0U;

    if (w == NULL)
    {
        return ST_ERR_PARAM;
    }

    st = bsp_encoder_check(w->idx);
    if (st == ST_OK)
    {
        st = bsp_encoder_read(w->idx, &raw);
    }
    if (st == ST_OK)
    {
        st = enc_update(&w->enc, raw);   /* may return ST_WARN_OVERSPEED */
    }

    if ((st == ST_OK) || (st == ST_WARN_OVERSPEED))
    {
        mot_update(&w->mot);
    }
    else
    {
        mot_stop(&w->mot);               /* lost feedback -> never keep driving */
    }
    return st;
}

void wheel_set_duty(wheel_t *w, int16_t permille)
{
    if (w != NULL)
    {
        mot_set_target(&w->mot, permille);
    }
}

void wheel_stop(wheel_t *w)
{
    if (w != NULL)
    {
        mot_stop(&w->mot);
    }
}

status_t wheel_reset_encoder(wheel_t *w)
{
    uint32_t raw = 0U;
    status_t st;

    if (w == NULL)
    {
        return ST_ERR_PARAM;
    }
    st = bsp_encoder_read(w->idx, &raw);
    if (st == ST_OK)
    {
        enc_reset(&w->enc, raw);
    }
    return st;
}
