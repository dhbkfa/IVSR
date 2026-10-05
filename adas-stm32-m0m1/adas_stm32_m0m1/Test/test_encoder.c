#include <assert.h>
#include <stdio.h>
#include "encoder.h"

static enc_t make(uint8_t bits, int8_t dir)
{
    enc_t e;
    enc_cfg_t c = { bits, dir, 1000.0f };
    assert(enc_init(&e, &c) == ST_OK);
    return e;
}

int main(void)
{
    enc_t e;

    /* first call only synchronises */
    e = make(16U, 1);
    assert(enc_update(&e, 12345U) == ST_OK);
    assert(enc_get_count(&e) == 0 && enc_get_delta(&e) == 0);

    /* forward across the 16-bit wrap: 65530 -> 5 = +11 */
    e = make(16U, 1);
    (void)enc_update(&e, 65530U);
    assert(enc_update(&e, 5U) == ST_OK);
    assert(enc_get_delta(&e) == 11 && enc_get_count(&e) == 11);

    /* backward across the wrap: 5 -> 65530 = -11 */
    e = make(16U, 1);
    (void)enc_update(&e, 5U);
    assert(enc_update(&e, 65530U) == ST_OK);
    assert(enc_get_delta(&e) == -11 && enc_get_count(&e) == -11);

    /* direction flip */
    e = make(16U, -1);
    (void)enc_update(&e, 100U);
    (void)enc_update(&e, 110U);
    assert(enc_get_delta(&e) == -10);

    /* 32-bit wrap */
    e = make(32U, 1);
    (void)enc_update(&e, 0xFFFFFFFAU);
    assert(enc_update(&e, 5U) == ST_OK);
    assert(enc_get_delta(&e) == 11);
    (void)enc_update(&e, 0xFFFFFFFAU);
    assert(enc_get_delta(&e) == -11);

    /* accumulate many steps through several wraps */
    e = make(16U, 1);
    (void)enc_update(&e, 0U);
    {
        uint32_t raw = 0U;
        int i;
        for (i = 0; i < 1000; i++)
        {
            raw = (raw + 300U) & 0xFFFFU;
            assert(enc_update(&e, raw) == ST_OK);
        }
        assert(enc_get_count(&e) == 300000);
    }

    /* overspeed warning (> 1/4 of range) but still returns a value */
    e = make(16U, 1);
    (void)enc_update(&e, 0U);
    assert(enc_update(&e, 20000U) == ST_WARN_OVERSPEED);

    /* angle and speed */
    e = make(16U, 1);
    (void)enc_update(&e, 0U);
    (void)enc_update(&e, 500U);                       /* half a revolution */
    assert(enc_get_angle_deg(&e) > 179.99f && enc_get_angle_deg(&e) < 180.01f);
    {
        float w = enc_get_speed_rad_s(&e, 0.005f);    /* 0.5 rev in 5 ms   */
        assert(w > 627.0f && w < 629.5f);
    }

    /* reset resynchronises (no phantom jump) */
    enc_reset(&e, 777U);
    assert(enc_get_count(&e) == 0);
    (void)enc_update(&e, 780U);
    assert(enc_get_delta(&e) == 3);

    /* config validation */
    {
        enc_cfg_t bad1 = { 12U, 1, 1000.0f };
        enc_cfg_t bad2 = { 16U, 0, 1000.0f };
        enc_cfg_t bad3 = { 16U, 1, 0.0f };
        assert(enc_init(&e, &bad1) == ST_ERR_CONFIG);
        assert(enc_init(&e, &bad2) == ST_ERR_CONFIG);
        assert(enc_init(&e, &bad3) == ST_ERR_CONFIG);
        assert(enc_init(NULL, &bad1) == ST_ERR_PARAM);
    }
    puts("test_encoder: OK");
    return 0;
}
