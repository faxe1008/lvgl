/**
 * @file lv_draw_sw_blend_to_l1_vtiled_msb.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_draw_sw_blend_to_l1_vtiled_msb.h"
#if LV_USE_DRAW_SW

    #if LV_DRAW_SW_SUPPORT_L1_VTILED_MSB

        #include "lv_draw_sw_blend_private.h"

        /*********************
        *      DEFINES
        *********************/

        /*This layout has no accelerator hooks yet*/
        #define LV_BLEND_NO_HOOK(...)     LV_RESULT_INVALID

        /*********************
        *  TEMPLATE INSTANCE
        *********************/

        #define LV_BLEND_COLOR_FN       lv_draw_sw_blend_color_to_l1_vtiled_msb
        #define LV_BLEND_IMAGE_FN       lv_draw_sw_blend_image_to_l1_vtiled_msb
        #define LV_BLEND_HOOK(name)     LV_BLEND_NO_HOOK

        #define DEST_IS_HTILED          0
        #define DEST_ROW(buf, stride, y)     ((uint8_t *)(buf) + (uint32_t)((((y1_abs) + (y)) >> 3) - (y1_abs >> 3)) * (stride))
        #define DEST_PX_ADDR(row, x, y)      (&(row)[(x)])
        #define DEST_PX_SHIFT(x, y)          (7 - (((y1_abs) + (y)) & 7))

        #include "lv_draw_sw_blend_to_l1.inc"

    #endif /*LV_DRAW_SW_SUPPORT_L1_VTILED_MSB*/

#endif /*LV_USE_DRAW_SW*/