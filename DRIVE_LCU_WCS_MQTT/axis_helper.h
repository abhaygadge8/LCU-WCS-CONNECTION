#ifndef AXIS_HELPER_H
#define AXIS_HELPER_H

#include "ini.h"
#include<stddef.h>

/* Axis identifier */
typedef enum
{
    AXIS_PAN  = 2,
    AXIS_TILT = 1,
    AXIS_BOTH = 3
} Axis_t;

/* -------------------------------------------------------
 * Get axis configuration from config.ini
 * ------------------------------------------------------- */
static inline AXIS_CONFIG *GetAxisCfg(Axis_t axis)
{
    switch (axis)
    {
        case AXIS_TILT:
            return &axis1_cfg;   /* PAN */

        case AXIS_PAN:
            return &axis2_cfg;   /* TILT */

        default:
            return NULL;         /* Invalid axis */
    }
}

#endif /* AXIS_HELPER_H */
