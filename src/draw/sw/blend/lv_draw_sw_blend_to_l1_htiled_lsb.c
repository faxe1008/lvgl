/**
 * @file lv_draw_sw_blend_to_l1_htiled_lsb.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_draw_sw_blend_to_l1_htiled_lsb.h"
#if LV_USE_DRAW_SW

    #if LV_DRAW_SW_SUPPORT_L1_HTILED_LSB

        #include "lv_draw_sw_blend_private.h"

        /*********************
        *      DEFINES
        *********************/

        /*This layout has no accelerator hooks yet*/
        #define LV_BLEND_NO_HOOK(...)     LV_RESULT_INVALID

        /*********************
        *  TEMPLATE INSTANCE
        *********************/

        #define LV_BLEND_COLOR_FN       lv_draw_sw_blend_color_to_l1_htiled_lsb
        #define LV_BLEND_IMAGE_FN       lv_draw_sw_blend_image_to_l1_htiled_lsb
        #define LV_BLEND_HOOK(name)     LV_BLEND_NO_HOOK

        #define DEST_IS_HTILED          1
        #define DEST_ROW(buf, stride, y)     ((uint8_t *)(buf) + (uint32_t)(y) * (stride))
        #define DEST_PX_ADDR(row, x, y)      (&(row)[((x) + bit_ofs) >> 3])
        #define DEST_PX_SHIFT(x, y)          (((x) + bit_ofs) & 7)

        #include "lv_draw_sw_blend_to_l1.inc"

    #endif /*LV_DRAW_SW_SUPPORT_L1_HTILED_LSB*/

#endif /*LV_USE_DRAW_SW*/