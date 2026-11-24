/**
 * @file lv_api_map_v9_6.h
 *
 */

#ifndef LV_API_MAP_V9_6_H
#define LV_API_MAP_V9_6_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../core/lv_area.h"
#include "../draw/lv_draw.h"

/* `display` is evaluated more than once, don't pass an expression with side effects*/
#define lv_display_rotate_point_ccw(display, point)                            \
    lv_point_rotate(point, lv_display_get_rotation(display),                   \
                    lv_display_get_original_horizontal_resolution(display),    \
                    lv_display_get_original_vertical_resolution(display))

#define LV_DISPLAY_ROTATION_0   LV_ROTATION_0
#define LV_DISPLAY_ROTATION_90  LV_ROTATION_90
#define LV_DISPLAY_ROTATION_180 LV_ROTATION_180
#define LV_DISPLAY_ROTATION_270 LV_ROTATION_270
typedef lv_rotation_t lv_display_rotation_t;

#define lv_indev_set_ccw(indev) lv_indev_set_rotation_dir(indev, LV_ROTATION_DIR_CCW)
#define lv_indev_clear_ccw(indev) lv_indev_set_rotation_dir(indev, LV_ROTATION_DIR_CW)
#define lv_indev_get_ccw(indev) (lv_indev_get_rotation_dir(indev) == LV_ROTATION_DIR_CCW)

#define LV_OBJ_FLAG_SEND_DRAW_TASK_EVENTS            LV_OBJ_FLAG_SEND_DRAW_DSC_EVENTS
#define LV_PROPERTY_OBJ_FLAG_SEND_DRAW_TASK_EVENTS   LV_PROPERTY_OBJ_FLAG_SEND_DRAW_DSC_EVENTS
#define LV_PROPERTY_OBJ_SEND_DRAW_TASK_EVENTS        LV_PROPERTY_OBJ_SEND_DRAW_DSC_EVENTS
#define lv_obj_set_send_draw_task_events(obj, en)    lv_obj_set_send_draw_dsc_events(obj, en)
#define lv_obj_is_send_draw_task_events(obj)         lv_obj_is_send_draw_dsc_events(obj)

typedef lv_draw_dsc_t lv_draw_dsc_base_t;

typedef lv_draw_dsc_type_t lv_draw_task_type_t;

/* `I1` (1bpp luminance) was renamed to `L1`: it is not an indexed format, it represents a
 * luminance. The legacy layout (8 pixels per byte, row-wise, MSB first) is now explicit. */
#define LV_COLOR_FORMAT_I1            LV_COLOR_FORMAT_L1_HTILED_MSB
#define LV_COLOR_FORMAT_L1            LV_COLOR_FORMAT_L1_HTILED_MSB
#define lv_draw_i1_to_argb8888        lv_draw_l1_to_argb8888
#define lv_draw_i1_invert             lv_draw_l1_invert
#define lv_draw_i1_convert_to_vtiled  lv_draw_l1_convert_to_vtiled
#define LV_DRAW_TASK_TYPE_NONE           LV_DRAW_DSC_TYPE_NONE
#define LV_DRAW_TASK_TYPE_FILL           LV_DRAW_DSC_TYPE_FILL
#define LV_DRAW_TASK_TYPE_BORDER         LV_DRAW_DSC_TYPE_BORDER
#define LV_DRAW_TASK_TYPE_BOX_SHADOW     LV_DRAW_DSC_TYPE_BOX_SHADOW
#define LV_DRAW_TASK_TYPE_LETTER         LV_DRAW_DSC_TYPE_LETTER
#define LV_DRAW_TASK_TYPE_LABEL          LV_DRAW_DSC_TYPE_LABEL
#define LV_DRAW_TASK_TYPE_IMAGE          LV_DRAW_DSC_TYPE_IMAGE
#define LV_DRAW_TASK_TYPE_LAYER          LV_DRAW_DSC_TYPE_LAYER
#define LV_DRAW_TASK_TYPE_LINE           LV_DRAW_DSC_TYPE_LINE
#define LV_DRAW_TASK_TYPE_ARC            LV_DRAW_DSC_TYPE_ARC
#define LV_DRAW_TASK_TYPE_TRIANGLE       LV_DRAW_DSC_TYPE_TRIANGLE
#define LV_DRAW_TASK_TYPE_MASK_RECTANGLE LV_DRAW_DSC_TYPE_MASK_RECTANGLE
#define LV_DRAW_TASK_TYPE_BLUR           LV_DRAW_DSC_TYPE_BLUR
#define LV_DRAW_TASK_TYPE_VECTOR         LV_DRAW_DSC_TYPE_VECTOR
#define LV_DRAW_TASK_TYPE_3D             LV_DRAW_DSC_TYPE_3D

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* LV_API_MAP_V9_6_H */
