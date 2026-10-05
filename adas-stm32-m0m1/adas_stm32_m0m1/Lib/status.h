/**
 * @file    status.h
 * @brief   Common status codes used by every module of the project.
 */
#ifndef STATUS_H
#define STATUS_H

typedef enum
{
    ST_OK = 0,          /* success                                    */
    ST_ERR_PARAM,       /* NULL pointer or argument out of range      */
    ST_ERR_STATE,       /* peripheral/module not in the right state   */
    ST_ERR_CONFIG,      /* wrong configuration (mode, ARR, ...)       */
    ST_ERR_HW,          /* HAL/hardware reported an error             */
    ST_WARN_OVERSPEED   /* result valid but delta is suspiciously big */
} status_t;

#endif /* STATUS_H */
