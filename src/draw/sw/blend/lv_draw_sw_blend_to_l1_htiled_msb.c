/**
 * @file lv_draw_sw_blend_to_l1_htiled_msb.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_draw_sw_blend_to_l1_htiled_msb.h"
#if LV_USE_DRAW_SW

    #if LV_DRAW_SW_SUPPORT_L1_HTILED_MSB

        #include "lv_draw_sw_blend_private.h"

        /*********************
        *      DEFINES
        *********************/

        /*Unset accelerator hooks fall back to the C implementation*/
        #ifndef LV_DRAW_SW_COLOR_BLEND_TO_L1
            #define LV_DRAW_SW_COLOR_BLEND_TO_L1(...)                                     LV_RESULT_INVALID
        #endif

        #ifndef LV_DRAW_SW_COLOR_BLEND_TO_L1_WITH_OPA
            #define LV_DRAW_SW_COLOR_BLEND_TO_L1_WITH_OPA(...)                            LV_RESULT_INVALID
        #endif

        #ifndef LV_DRAW_SW_COLOR_BLEND_TO_L1_WITH_MASK
            #define LV_DRAW_SW_COLOR_BLEND_TO_L1_WITH_MASK(...)                           LV_RESULT_INVALID
        #endif

        #ifndef LV_DRAW_SW_COLOR_BLEND_TO_L1_MIX_MASK_OPA
            #define LV_DRAW_SW_COLOR_BLEND_TO_L1_MIX_MASK_OPA(...)                        LV_RESULT_INVALID
        #endif

        #ifndef LV_DRAW_SW_L1_BLEND_NORMAL_TO_L1
            #define LV_DRAW_SW_L1_BLEND_NORMAL_TO_L1(...)                                 LV_RESULT_INVALID
        #endif

        #ifndef LV_DRAW_SW_L1_BLEND_NORMAL_TO_L1_WITH_OPA
            #define LV_DRAW_SW_L1_BLEND_NORMAL_TO_L1_WITH_OPA(...)                        LV_RESULT_INVALID
        #endif

        #ifndef LV_DRAW_SW_L1_BLEND_NORMAL_TO_L1_WITH_MASK
            #define LV_DRAW_SW_L1_BLEND_NORMAL_TO_L1_WITH_MASK(...)                       LV_RESULT_INVALID
        #endif

        #ifndef LV_DRAW_SW_L1_BLEND_NORMAL_TO_L1_MIX_MASK_OPA
            #define LV_DRAW_SW_L1_BLEND_NORMAL_TO_L1_MIX_MASK_OPA(...)                    LV_RESULT_INVALID
        #endif

        #ifndef LV_DRAW_SW_RGB565_BLEND_NORMAL_TO_L1
            #define LV_DRAW_SW_RGB565_BLEND_NORMAL_TO_L1(...)                             LV_RESULT_INVALID
        #endif

        #ifndef LV_DRAW_SW_RGB565_BLEND_NORMAL_TO_L1_WITH_OPA
            #define LV_DRAW_SW_RGB565_BLEND_NORMAL_TO_L1_WITH_OPA(...)                    LV_RESULT_INVALID
        #endif

        #ifndef LV_DRAW_SW_RGB565_BLEND_NORMAL_TO_L1_WITH_MASK
            #define LV_DRAW_SW_RGB565_BLEND_NORMAL_TO_L1_WITH_MASK(...)                   LV_RESULT_INVALID
        #endif

        #ifndef LV_DRAW_SW_RGB565_BLEND_NORMAL_TO_L1_MIX_MASK_OPA
            #define LV_DRAW_SW_RGB565_BLEND_NORMAL_TO_L1_MIX_MASK_OPA(...)                LV_RESULT_INVALID
        #endif

        #ifndef LV_DRAW_SW_RGB565_SWAPPED_BLEND_NORMAL_TO_L1
            #define LV_DRAW_SW_RGB565_SWAPPED_BLEND_NORMAL_TO_L1(...)                     LV_RESULT_INVALID
        #endif

        #ifndef LV_DRAW_SW_RGB565_SWAPPED_BLEND_NORMAL_TO_L1_WITH_OPA
            #define LV_DRAW_SW_RGB565_SWAPPED_BLEND_NORMAL_TO_L1_WITH_OPA(...)            LV_RESULT_INVALID
        #endif

        #ifndef LV_DRAW_SW_RGB565_SWAPPED_BLEND_NORMAL_TO_L1_WITH_MASK
            #define LV_DRAW_SW_RGB565_SWAPPED_BLEND_NORMAL_TO_L1_WITH_MASK(...)           LV_RESULT_INVALID
        #endif

        #ifndef LV_DRAW_SW_RGB565_SWAPPED_BLEND_NORMAL_TO_L1_MIX_MASK_OPA
            #define LV_DRAW_SW_RGB565_SWAPPED_BLEND_NORMAL_TO_L1_MIX_MASK_OPA(...)        LV_RESULT_INVALID
        #endif

        #ifndef LV_DRAW_SW_RGB888_BLEND_NORMAL_TO_L1
            #define LV_DRAW_SW_RGB888_BLEND_NORMAL_TO_L1(...)                             LV_RESULT_INVALID
        #endif

        #ifndef LV_DRAW_SW_RGB888_BLEND_NORMAL_TO_L1_WITH_OPA
            #define LV_DRAW_SW_RGB888_BLEND_NORMAL_TO_L1_WITH_OPA(...)                    LV_RESULT_INVALID
        #endif

        #ifndef LV_DRAW_SW_RGB888_BLEND_NORMAL_TO_L1_WITH_MASK
            #define LV_DRAW_SW_RGB888_BLEND_NORMAL_TO_L1_WITH_MASK(...)                   LV_RESULT_INVALID
        #endif

        #ifndef LV_DRAW_SW_RGB888_BLEND_NORMAL_TO_L1_MIX_MASK_OPA
            #define LV_DRAW_SW_RGB888_BLEND_NORMAL_TO_L1_MIX_MASK_OPA(...)                LV_RESULT_INVALID
        #endif

        #ifndef LV_DRAW_SW_L8_BLEND_NORMAL_TO_L1
            #define LV_DRAW_SW_L8_BLEND_NORMAL_TO_L1(...)                                 LV_RESULT_INVALID
        #endif

        #ifndef LV_DRAW_SW_L8_BLEND_NORMAL_TO_L1_WITH_OPA
            #define LV_DRAW_SW_L8_BLEND_NORMAL_TO_L1_WITH_OPA(...)                        LV_RESULT_INVALID
        #endif

        #ifndef LV_DRAW_SW_L8_BLEND_NORMAL_TO_L1_WITH_MASK
            #define LV_DRAW_SW_L8_BLEND_NORMAL_TO_L1_WITH_MASK(...)                       LV_RESULT_INVALID
        #endif

        #ifndef LV_DRAW_SW_L8_BLEND_NORMAL_TO_L1_MIX_MASK_OPA
            #define LV_DRAW_SW_L8_BLEND_NORMAL_TO_L1_MIX_MASK_OPA(...)                    LV_RESULT_INVALID
        #endif

        #ifndef LV_DRAW_SW_AL88_BLEND_NORMAL_TO_L1
            #define LV_DRAW_SW_AL88_BLEND_NORMAL_TO_L1(...)                               LV_RESULT_INVALID
        #endif

        #ifndef LV_DRAW_SW_AL88_BLEND_NORMAL_TO_L1_WITH_OPA
            #define LV_DRAW_SW_AL88_BLEND_NORMAL_TO_L1_WITH_OPA(...)                      LV_RESULT_INVALID
        #endif

        #ifndef LV_DRAW_SW_AL88_BLEND_NORMAL_TO_L1_WITH_MASK
            #define LV_DRAW_SW_AL88_BLEND_NORMAL_TO_L1_WITH_MASK(...)                     LV_RESULT_INVALID
        #endif

        #ifndef LV_DRAW_SW_AL88_BLEND_NORMAL_TO_L1_MIX_MASK_OPA
            #define LV_DRAW_SW_AL88_BLEND_NORMAL_TO_L1_MIX_MASK_OPA(...)                  LV_RESULT_INVALID
        #endif

        #ifndef LV_DRAW_SW_ARGB8888_BLEND_NORMAL_TO_L1
            #define LV_DRAW_SW_ARGB8888_BLEND_NORMAL_TO_L1(...)                           LV_RESULT_INVALID
        #endif

        #ifndef LV_DRAW_SW_ARGB8888_BLEND_NORMAL_TO_L1_WITH_OPA
            #define LV_DRAW_SW_ARGB8888_BLEND_NORMAL_TO_L1_WITH_OPA(...)                  LV_RESULT_INVALID
        #endif

        #ifndef LV_DRAW_SW_ARGB8888_BLEND_NORMAL_TO_L1_WITH_MASK
            #define LV_DRAW_SW_ARGB8888_BLEND_NORMAL_TO_L1_WITH_MASK(...)                 LV_RESULT_INVALID
        #endif

        #ifndef LV_DRAW_SW_ARGB8888_BLEND_NORMAL_TO_L1_MIX_MASK_OPA
            #define LV_DRAW_SW_ARGB8888_BLEND_NORMAL_TO_L1_MIX_MASK_OPA(...)              LV_RESULT_INVALID
        #endif

        /*********************
        *  TEMPLATE INSTANCE
        *********************/

        #define LV_BLEND_COLOR_FN       lv_draw_sw_blend_color_to_l1_htiled_msb
        #define LV_BLEND_IMAGE_FN       lv_draw_sw_blend_image_to_l1_htiled_msb
        #define LV_BLEND_HOOK(name)     LV_DRAW_SW_##name

        #define DEST_IS_HTILED          1
        #define DEST_ROW(buf, stride, y)     ((uint8_t *)(buf) + (uint32_t)(y) * (stride))
        #define DEST_PX_ADDR(row, x, y)      (&(row)[((x) + bit_ofs) >> 3])
        #define DEST_PX_SHIFT(x, y)          (7 - (((x) + bit_ofs) & 7))

        #include "lv_draw_sw_blend_to_l1.inc"

    #endif /*LV_DRAW_SW_SUPPORT_L1_HTILED_MSB*/

#endif /*LV_USE_DRAW_SW*/