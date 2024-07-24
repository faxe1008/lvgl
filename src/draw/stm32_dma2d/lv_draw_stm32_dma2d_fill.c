/**
 * @file lv_draw_stm32_dma2d.c
 *
 */

/*********************
 *      INCLUDES
 *********************/

#include "lv_draw_stm32_dma2d.h"

#if LV_USE_DRAW_STM32_DMA2D


/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_draw_stm32_dma2d_fill(lv_draw_unit_t * draw_unit, const lv_draw_fill_dsc_t * dsc, const lv_area_t * coords)
{
    lv_draw_stm32_dma2d_unit_t * u = (lv_draw_stm32_dma2d_unit_t *)draw_unit;

    lv_area_t clip_area;
    if(!_lv_area_intersect(&clip_area, coords, draw_unit->clip_area)) {
        /*Fully clipped, nothing to do*/
        return;
    }

    // FIXME: check if another DMA is in progress and wait for it

    int32_t draw_width = lv_area_get_width(coords);
    int32_t draw_height = lv_area_get_height(coords);
    dma2d_color_format_t draw_color_format = lv_cf_to_dma2d(dsc->base.layer->color_format);

    if(dsc->opa >= LV_OPA_MAX) {
        DMA2D->CR = 0x3UL << DMA2D_CR_MODE_Pos; // Register-to-memory (no FG nor BG, only output stage active)

        DMA2D->OPFCCR = draw_color_format;
#if defined(DMA2D_OPFCCR_RBS_Pos)
        DMA2D->OPFCCR |= (RBS_BIT << DMA2D_OPFCCR_RBS_Pos);
#endif
        DMA2D->OMAR = (uint32_t)(u->target_buffer + (dest_stride * coords->y1) + coords->x1);
        DMA2D->OOR = dest_stride - draw_width;  // out buffer offset
        // Note: unlike FGCOLR and BGCOLR, OCOLR bits must match DMA2D_OUTPUT_COLOR, alpha can be specified
#if RBS_BIT
        // FIXME: swap red/blue bits
#else
        DMA2D->OCOLR = lv_color_to_u32(dsc->color);
#endif
    }
    else {
        DMA2D->CR = 0x2UL << DMA2D_CR_MODE_Pos; // Memory-to-memory with blending (FG and BG fetch with PFC and blending)

        DMA2D->FGPFCCR = DMA2D_A8;
        DMA2D->FGPFCCR |= (opa << DMA2D_FGPFCCR_ALPHA_Pos);
        // Alpha Mode 1: Replace original foreground image alpha channel value by FGPFCCR.ALPHA
        DMA2D->FGPFCCR |= (0x1UL << DMA2D_FGPFCCR_AM_Pos);
        //DMA2D->FGPFCCR |= (RBS_BIT << DMA2D_FGPFCCR_RBS_Pos);

        // Note: in Alpha Mode 1 FGMAR and FGOR are not used to supply foreground A8 bytes,
        // those bytes are replaced by constant ALPHA defined in FGPFCCR
        DMA2D->FGMAR = (uint32_t)u->target_buffer;
        DMA2D->FGOR = dest_stride;
        DMA2D->FGCOLR = lv_color_to_u32(dsc->color) & 0x00ffffff; // swap FGCOLR R/B bits if FGPFCCR.RBS (RBS_BIT) bit is set

        DMA2D->BGPFCCR = draw_color_format;
#if defined(DMA2D_BGPFCCR_RBS_Pos)
        DMA2D->BGPFCCR |= (RBS_BIT << DMA2D_BGPFCCR_RBS_Pos);
#endif
        DMA2D->BGMAR = (uint32_t)(u->target_buffer + (dest_stride * coords->y1) + coords->x1);
        DMA2D->BGOR = dest_stride - draw_width;
        DMA2D->BGCOLR = 0;  // used in A4 and A8 modes only

        // FIXME: clean cache?

        DMA2D->OPFCCR = draw_color_format;
#if defined(DMA2D_OPFCCR_RBS_Pos)
        DMA2D->OPFCCR |= (RBS_BIT << DMA2D_OPFCCR_RBS_Pos);
#endif
        DMA2D->OMAR = DMA2D->BGMAR;
        DMA2D->OOR = DMA2D->BGOR;
        DMA2D->OCOLR = 0;
    }
    // PL - pixel per lines (14 bit), NL - number of lines (16 bit)
    DMA2D->NLR = (draw_width << DMA2D_NLR_PL_Pos) | (draw_height << DMA2D_NLR_NL_Pos);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif /*LV_USE_DRAW_VG_LITE*/
