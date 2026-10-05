#include "encoder.h"

#define ENC_TWO_PI      (6.28318531f)
#define ENC_DEG_PER_REV (360.0f)

static int32_t sat_i64_to_i32(int64_t v)
{
    int32_t r;

    if (v > (int64_t)INT32_MAX)
    {
        r = INT32_MAX;
    }
    else if (v < (int64_t)(-INT32_MAX))
    {
        r = -INT32_MAX;
    }
    else
    {
        r = (int32_t)v;
    }
    return r;
}

status_t enc_init(enc_t *e, const enc_cfg_t *cfg)
{
    status_t st = ST_OK;

    if ((e == NULL) || (cfg == NULL))
    {
        st = ST_ERR_PARAM;
    }
    else if (((cfg->counter_bits != 16U) && (cfg->counter_bits != 32U)) ||
             ((cfg->direction != 1) && (cfg->direction != -1)) ||
             (cfg->counts_per_rev <= 0.0f))
    {
        st = ST_ERR_CONFIG;
    }
    else
    {
        e->cfg          = *cfg;
        e->last_raw     = 0U;
        e->total_counts = 0;
        e->delta_counts = 0;
        e->initialized  = false;
    }
    return st;
}

status_t enc_update(enc_t *e, uint32_t raw)
{
    status_t st = ST_OK;

    if (e == NULL)
    {
        st = ST_ERR_PARAM;
    }
    else if (!e->initialized)
    {
        e->last_raw     = raw;
        e->delta_counts = 0;
        e->initialized  = true;
    }
    else
    {
        /* Modular subtraction handles counter wrap in both directions. */
        const uint64_t full = (uint64_t)1U << e->cfg.counter_bits;
        const uint64_t mask = full - 1U;
        const uint64_t diff = ((uint64_t)raw - (uint64_t)e->last_raw) & mask;
        int64_t        d    = (int64_t)diff;
        int64_t        sum;

        if (diff >= (full >> 1U))
        {
            d -= (int64_t)full;          /* interpret as negative */
        }
        if (e->cfg.direction < 0)
        {
            d = -d;
        }
        if ((d > (int64_t)(full >> 2U)) || (d < -(int64_t)(full >> 2U)))
        {
            st = ST_WARN_OVERSPEED;
        }

        e->delta_counts = sat_i64_to_i32(d);
        sum = (int64_t)e->total_counts + (int64_t)e->delta_counts;
        e->total_counts = sat_i64_to_i32(sum);
        e->last_raw     = raw;
    }
    return st;
}

void enc_reset(enc_t *e, uint32_t raw_now)
{
    if (e != NULL)
    {
        e->total_counts = 0;
        e->delta_counts = 0;
        e->last_raw     = raw_now;
        e->initialized  = true;
    }
}

status_t enc_set_counts_per_rev(enc_t *e, float cpr)
{
    status_t st = ST_OK;

    if (e == NULL)
    {
        st = ST_ERR_PARAM;
    }
    else if (cpr <= 0.0f)
    {
        st = ST_ERR_CONFIG;
    }
    else
    {
        e->cfg.counts_per_rev = cpr;
    }
    return st;
}

int32_t enc_get_count(const enc_t *e)
{
    return (e != NULL) ? e->total_counts : 0;
}

int32_t enc_get_delta(const enc_t *e)
{
    return (e != NULL) ? e->delta_counts : 0;
}

float enc_get_angle_deg(const enc_t *e)
{
    float a = 0.0f;

    if (e != NULL)
    {
        a = ((float)e->total_counts * ENC_DEG_PER_REV) / e->cfg.counts_per_rev;
    }
    return a;
}

float enc_get_speed_rad_s(const enc_t *e, float dt_s)
{
    float w = 0.0f;

    if ((e != NULL) && (dt_s > 0.0f))
    {
        w = (((float)e->delta_counts / e->cfg.counts_per_rev) * ENC_TWO_PI) / dt_s;
    }
    return w;
}
