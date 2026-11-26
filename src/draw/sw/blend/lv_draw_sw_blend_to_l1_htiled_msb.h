/**
 * @file lv_draw_sw_blend_to_l1_htiled_msb.h
 *
 */

#ifndef LV_DRAW_SW_BLEND_TO_L1_HTILED_MSB_H
#define LV_DRAW_SW_BLEND_TO_L1_HTILED_MSB_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "../lv_draw_sw.h"
#if LV_USE_DRAW_SW

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

#if LV_DRAW_SW_SUPPORT_L1_HTILED_MSB

void /* LV_ATTRIBUTE_FAST_MEM */ lv_draw_sw_blend_color_to_l1_htiled_msb(lv_draw_sw_blend_fill_dsc_t * dsc);

void /* LV_ATTRIBUTE_FAST_MEM */ lv_draw_sw_blend_image_to_l1_htiled_msb(lv_draw_sw_blend_image_dsc_t * dsc);

#endif /*LV_DRAW_SW_SUPPORT_L1_HTILED_MSB*/

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_DRAW_SW*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_DRAW_SW_BLEND_TO_L1_HTILED_MSB_H*/