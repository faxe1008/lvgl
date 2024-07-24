/**
 * @file lv_draw_stm32_dma2d.c
 *
 */

/*********************
 *      INCLUDES
 *********************/

#include "lv_draw_stm32_dma2d.h"

#if LV_USE_DRAW_STM32_DMA2D

#include LV_DMA2D_CMSIS_INCLUDE
#include "../lv_draw.h"


/*********************
 *      DEFINES
 *********************/

#define STM32_DMA2D_DRAW_UNIT_ID 5

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

static int32_t draw_dispatch(lv_draw_unit_t * draw_unit, lv_layer_t * layer);
static int32_t draw_evaluate(lv_draw_unit_t * draw_unit, lv_draw_task_t * task);
static int32_t draw_delete(lv_draw_unit_t * draw_unit);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_draw_stm32_dma2d_init(void)
{
    // Enable DMA2D clock
#if defined(STM32F4) || defined(STM32F7) || defined(STM32U5)
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA2DEN; // enable DMA2D
    // wait for hardware access to complete
    __asm volatile("DSB\n");
    volatile uint32_t temp = RCC->AHB1ENR;
    LV_UNUSED(temp);
#elif defined(STM32H7)
    RCC->AHB3ENR |= RCC_AHB3ENR_DMA2DEN;
    // wait for hardware access to complete
    __asm volatile("DSB\n");
    volatile uint32_t temp = RCC->AHB3ENR;
    LV_UNUSED(temp);
#else
# warning "LVGL can't enable the clock of DMA2D"
#endif
    // AHB master timer configuration
    DMA2D->AMTCR = 0; // AHB bus guaranteed dead time disabled

    lv_draw_stm32_dma2d_unit_t * unit = lv_draw_create_unit(sizeof(lv_draw_stm32_dma2d_unit_t));
    unit->base_unit.dispatch_cb = draw_dispatch;
    unit->base_unit.evaluate_cb = draw_evaluate;
    unit->base_unit.delete_cb = draw_delete;
}

void lv_draw_stm32_dma2d_deinit(void)
{
    // Ensure any ongoing DMA2D operations are stopped
    DMA2D->CR &= ~DMA2D_CR_START; // Stop DMA2D operations

    // Optionally, wait for the DMA2D to complete any ongoing operations
    while(DMA2D->CR & DMA2D_CR_START) {
        // Wait until the DMA2D operation has completely stopped
    }

    // Disable the DMA2D peripheral clock
#if defined(STM32F4) || defined(STM32F7) || defined(STM32U5)
    RCC->AHB1ENR &= ~RCC_AHB1ENR_DMA2DEN; // disable DMA2D
#elif defined(STM32H7)
    RCC->AHB3ENR &= ~RCC_AHB3ENR_DMA2DEN;
#else
# warning "LVGL can't disable the clock of DMA2D"
#endif

    // Optionally, reset the DMA2D configuration (if supported and needed)
    // This typically involves enabling and then disabling the reset for the peripheral
#if defined(STM32F4) || defined(STM32F7) || defined(STM32U5)
    RCC->AHB1RSTR |= RCC_AHB1RSTR_DMA2DRST;  // Assert reset
    __asm volatile("DSB\n");
    RCC->AHB1RSTR &= ~RCC_AHB1RSTR_DMA2DRST; // De-assert reset
#elif defined(STM32H7)
    RCC->AHB3RSTR |= RCC_AHB3RSTR_DMA2DRST;  // Assert reset
    __asm volatile("DSB\n");
    RCC->AHB3RSTR &= ~RCC_AHB3RSTR_DMA2DRST; // De-assert reset
#endif
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static bool lv_stm32_dma2d_is_src_cf_supported(lv_color_format_t cf)
{
    switch(cf) {
        case LV_COLOR_FORMAT_ARGB8888:
        case LV_COLOR_FORMAT_XRGB8888:
        case LV_COLOR_FORMAT_RGB565:
        case LV_COLOR_FORMAT_RGB888:
            return true;
        default:
            break;
    }
    return false;
}

static bool lv_stm32_dma2d_is_dest_cf_supported(lv_color_format_t cf)
{
    switch(cf) {
        case LV_COLOR_FORMAT_ARGB8888:
        case LV_COLOR_FORMAT_XRGB8888:
        case LV_COLOR_FORMAT_RGB565:
        case LV_COLOR_FORMAT_RGB888:
            return true;
        default:
            break;
    }
    return false;
}

dma2d_color_format_t lv_cf_to_dma2d(lv_color_format_t cf)
{
    switch(cf) {
        case LV_COLOR_FORMAT_ARGB8888:
            return DMA2D_ARGB8888;
        case LV_COLOR_FORMAT_XRGB8888:
            return DMA2D_ARGB8888;
        case LV_COLOR_FORMAT_RGB565:
            return DMA2D_RGB565;
        case LV_COLOR_FORMAT_RGB888:
            return DMA2D_RGB888;
        default:
            return DMA2D_UNSUPPORTED;
    }
}

static bool check_image_is_supported(const lv_draw_image_dsc_t * dsc)
{
    lv_image_header_t header;
    lv_result_t res = lv_image_decoder_get_info(dsc->src, &header);
    if(res != LV_RESULT_OK) {
        LV_LOG_TRACE("get image info failed");
        return false;
    }
    return lv_stm32_dma2d_is_src_cf_supported(header.cf);
}

static void draw_execute(lv_draw_stm32_dma2d_unit_t * u)
{
    lv_draw_task_t * t = u->task_act;
    lv_draw_unit_t * draw_unit = (lv_draw_unit_t *)u;

    switch(t->type) {
        case LV_DRAW_TASK_TYPE_FILL:
            lv_draw_stm32_dma2d_fill(draw_unit, t->draw_dsc, &t->area);
            break;
        case LV_DRAW_TASK_TYPE_IMAGE:
            lv_draw_stm32_dma2d_img(draw_unit, t->draw_dsc, &t->area, false);
            break;
        default:
            break;
    }
}

static int32_t draw_dispatch(lv_draw_unit_t * draw_unit, lv_layer_t * layer)
{
    lv_draw_stm32_dma2d_unit_t * u = (lv_draw_stm32_dma2d_unit_t *)draw_unit;

    /* Return immediately if it's busy with draw task. */
    if(u->task_act) {
        return 0;
    }

    /* Try to get an ready to draw. */
    lv_draw_task_t * t = lv_draw_get_next_available_task(layer, NULL, STM32_DMA2D_DRAW_UNIT_ID);

    /* Return 0 is no selection, some tasks can be supported by other units. */
    if(!t || t->preferred_draw_unit_id != STM32_DMA2D_DRAW_UNIT_ID) {
        return LV_DRAW_UNIT_IDLE;
    }

    /* Return if target buffer format is not supported. */
    if(!lv_stm32_dma2d_is_dest_cf_supported(layer->color_format)) {
        return LV_DRAW_UNIT_IDLE;
    }

    void * buf = lv_draw_layer_alloc_buf(layer);
    if(!buf) {
        return LV_DRAW_UNIT_IDLE;
    }

    t->state = LV_DRAW_TASK_STATE_IN_PROGRESS;
    u->base_unit.target_layer = layer;
    u->base_unit.clip_area = &t->clip_area;
    u->task_act = t;

    draw_execute(u);

    u->task_act->state = LV_DRAW_TASK_STATE_READY;
    u->task_act = NULL;

    /*The draw unit is free now. Request a new dispatching as it can get a new task*/
    lv_draw_dispatch_request();

    return 1;
}

static int32_t draw_evaluate(lv_draw_unit_t * draw_unit, lv_draw_task_t * task)
{
    const lv_draw_dsc_base_t * base_dsc = task->draw_dsc;
    if(!lv_stm32_dma2d_is_dest_cf_supported(base_dsc->layer->color_format)) {
        return -1;
    }

    switch(task->type) {
        case LV_DRAW_TASK_TYPE_FILL:
            break;
        case LV_DRAW_TASK_TYPE_IMAGE: {
                if(!check_image_is_supported(task->draw_dsc)) {
                    return 0;
                }
            }
            break;

        default:
            /*The draw unit is not able to draw this task. */
            return 0;
    }

    /* The draw unit is able to draw this task. */
    task->preference_score = 80;
    task->preferred_draw_unit_id = STM32_DMA2D_DRAW_UNIT_ID;
    return 1;
}

static int32_t draw_delete(lv_draw_unit_t * draw_unit)
{
    LV_UNUSED(draw_unit);
    return 1;
}

#endif /*LV_USE_DRAW_STM32_DMA2D*/
