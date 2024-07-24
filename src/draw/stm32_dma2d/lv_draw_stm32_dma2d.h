/**
 * @file lv_draw_stm32_dma2d.h
 *
 */

#ifndef LV_STM32_DMA2D_DRAW_H
#define LV_STM32_DMA2D_DRAW_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#include "../../lv_conf_internal.h"

#if LV_USE_DRAW_STM32_DMA2D

#include "../lv_draw.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

typedef struct {
    lv_draw_unit_t base_unit;
    lv_draw_task_t * task_act;
} lv_draw_stm32_dma2d_unit_t;

typedef enum {
    DMA2D_ARGB8888 = 0x0,
    DMA2D_RGB888 = 0x01,
    DMA2D_RGB565 = 0x02,
    DMA2D_ARGB1555 = 0x03,
    DMA2D_ARGB4444 = 0x04,
    DMA2D_A8 = 0x09,
    DMA2D_UNSUPPORTED = 0xff,
} dma2d_color_format_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

void lv_draw_buf_stm32_dma2d_init_handlers(void);

void lv_draw_stm32_dma2d_init(void);

void lv_draw_stm32_dma2d_deinit(void);

dma2d_color_format_t lv_cf_to_dma2d(lv_color_format_t cf);

void lv_draw_stm32_dma2d_fill(lv_draw_unit_t * draw_unit, const lv_draw_fill_dsc_t * dsc,
                              const lv_area_t * coords);

void lv_draw_stm32_dma2d_img(lv_draw_unit_t * draw_unit, const lv_draw_image_dsc_t * dsc,
                             const lv_area_t * coords, bool no_cache);

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_DRAW_STM32_DMA2D*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_STM32_DMA2D_DRAW_H*/
