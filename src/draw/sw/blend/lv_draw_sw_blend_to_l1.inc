/**
 * @file lv_draw_sw_blend_to_l1.inc
 *
 * Template for the L1 (1 bit per pixel luminance) blend implementations. One
 * translation unit is generated per destination layout, see the
 * `lv_draw_sw_blend_to_l1_*.c` files.
 *
 * Template parameters, defined by the including translation unit:
 * - `LV_BLEND_COLOR_FN`:        the name of the public color fill function
 * - `LV_BLEND_IMAGE_FN`:        the name of the public image blend function
 * - `LV_BLEND_HOOK(name)`:      maps an accelerator hook, e.g. `L8_BLEND_NORMAL_TO_L1`,
 *                               to a function-like macro returning an `lv_result_t`.
 *                               Return `LV_RESULT_INVALID` to use the C implementation.
 * - `DEST_IS_HTILED`:           1 if the destination tiles 8 pixels horizontally in a
 *                               row (byte-level fill possible), 0 for vertical tiling
 * - `DEST_ROW(buf, stride, y)`: pointer to the buffer start of the local pixel row `y`,
 *                               `buf` points to the top-left pixel of the area
 * - `DEST_PX_ADDR(row, x, y)`:  address of the byte holding the destination pixel
 *                               (x, y) of the area, `row` comes from DEST_ROW
 * - `DEST_PX_SHIFT(x, y)`:      right shift to extract the bit of the destination
 *                               pixel (x, y) from the byte
 *
 * The destination pixel operations are derived from the primitives above:
 * - `L1_DEST_SET_PX(row, x, y)`:  set the destination bit
 * - `L1_DEST_CLR_PX(row, x, y)`:  clear the destination bit
 * - `L1_DEST_GET_PX(row, x, y)`:  read the destination bit
 *
 * Note: the *source* L1 pixels (l1_image_blend) are always read in the legacy
 * htiling MSB layout, which is the only L1 layout supported as a source.
 *
 */

/*********************
 *      DEFINES
 *********************/

#define L1_LUM_THRESHOLD LV_DRAW_SW_L1_LUM_THRESHOLD

#define L1_DEST_SET_PX(row, x, y)   (*(DEST_PX_ADDR((row), (x), (y))) |= (1 << DEST_PX_SHIFT((x), (y))))
#define L1_DEST_CLR_PX(row, x, y)   (*(DEST_PX_ADDR((row), (x), (y))) &= (uint8_t)~(1 << DEST_PX_SHIFT((x), (y))))
#define L1_DEST_GET_PX(row, x, y)   (((*(const uint8_t *)DEST_PX_ADDR((row), (x), (y))) >> DEST_PX_SHIFT((x), (y))) & 1U)

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

static void /* LV_ATTRIBUTE_FAST_MEM */ l1_image_blend(lv_draw_sw_blend_image_dsc_t * dsc);

#if LV_DRAW_SW_SUPPORT_L8
    static void /* LV_ATTRIBUTE_FAST_MEM */ l8_image_blend(lv_draw_sw_blend_image_dsc_t * dsc);
#endif

#if LV_DRAW_SW_SUPPORT_AL88
    static void /* LV_ATTRIBUTE_FAST_MEM */ al88_image_blend(lv_draw_sw_blend_image_dsc_t * dsc);
#endif

#if LV_DRAW_SW_SUPPORT_RGB565
    static void /* LV_ATTRIBUTE_FAST_MEM */ rgb565_image_blend(lv_draw_sw_blend_image_dsc_t * dsc);
#endif

#if LV_DRAW_SW_SUPPORT_RGB565_SWAPPED
    static void /* LV_ATTRIBUTE_FAST_MEM */ rgb565_swapped_image_blend(lv_draw_sw_blend_image_dsc_t * dsc);
#endif

#if LV_DRAW_SW_SUPPORT_RGB888 || LV_DRAW_SW_SUPPORT_XRGB8888
static void /* LV_ATTRIBUTE_FAST_MEM */ rgb888_image_blend(lv_draw_sw_blend_image_dsc_t * dsc,
                                                           const uint8_t src_px_size);
#endif

#if LV_DRAW_SW_SUPPORT_ARGB8888 || LV_DRAW_SW_SUPPORT_ARGB8888_PREMULTIPLIED
static void /* LV_ATTRIBUTE_FAST_MEM */ argb8888_image_blend(lv_draw_sw_blend_image_dsc_t * dsc,
                                                             bool premultiplied);
#endif

static inline void /* LV_ATTRIBUTE_FAST_MEM */ lv_color_8_8_mix(const uint8_t src, uint8_t * dest, uint8_t mix);

static inline void /* LV_ATTRIBUTE_FAST_MEM */ blend_non_normal_pixel(uint8_t * dest_byte, int32_t shift,
                                                                      lv_color32_t src, lv_blend_mode_t mode);

static inline uint8_t /* LV_ATTRIBUTE_FAST_MEM */ get_bit(const uint8_t * buf, int32_t bit_idx);

static inline void * /* LV_ATTRIBUTE_FAST_MEM */ drawbuf_next_row(const void * buf, uint32_t stride);

#if LV_DRAW_SW_SUPPORT_RGB565_SWAPPED
    static inline lv_color16_t /* LV_ATTRIBUTE_FAST_MEM */ lv_color16_from_u16(uint16_t raw);
#endif

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void LV_ATTRIBUTE_FAST_MEM LV_BLEND_COLOR_FN(lv_draw_sw_blend_fill_dsc_t * dsc)
{
    int32_t w = dsc->dest_w;
    int32_t h = dsc->dest_h;
    lv_opa_t opa = dsc->opa;
    const lv_opa_t * mask = dsc->mask_buf;
    int32_t mask_stride = dsc->mask_stride;
    int32_t dest_stride = dsc->dest_stride;

    uint8_t src_color = lv_color_luminance(dsc->color) / (L1_LUM_THRESHOLD + 1);
    uint8_t * dest_buf = dsc->dest_buf;

    const int32_t bit_ofs = dsc->relative_area.x1 % 8;
    const int32_t y1_abs = dsc->relative_area.y1;
    LV_UNUSED(bit_ofs);
    LV_UNUSED(y1_abs);

    /* Simple fill */
    if(mask == NULL && opa >= LV_OPA_MAX) {
        if(LV_RESULT_INVALID == LV_BLEND_HOOK(COLOR_BLEND_TO_L1)(dsc)) {
            const uint8_t fill_byte = src_color ? 0xFF : 0x00;
            if(DEST_IS_HTILED) {
                for(int32_t y = 0; y < h; y++) {
                    uint8_t * row = DEST_ROW(dest_buf, dest_stride, y);
                    int32_t x = 0;
                    while(x < w && ((x + bit_ofs) % 8) != 0) {
                        if(src_color) L1_DEST_SET_PX(row, x, y);
                        else L1_DEST_CLR_PX(row, x, y);
                        x++;
                    }
                    const int32_t full_bytes = (w - x) / 8;
                    if(full_bytes > 0) {
                        lv_memset(DEST_PX_ADDR(row, x, y), fill_byte, full_bytes);
                        x += full_bytes * 8;
                    }
                    while(x < w) {
                        if(src_color) L1_DEST_SET_PX(row, x, y);
                        else L1_DEST_CLR_PX(row, x, y);
                        x++;
                    }
                }
            }
            else {
                for(int32_t y = 0; y < h; y++) {
                    uint8_t * row = DEST_ROW(dest_buf, dest_stride, y);
                    for(int32_t x = 0; x < w; x++) {
                        if(src_color) L1_DEST_SET_PX(row, x, y);
                        else L1_DEST_CLR_PX(row, x, y);
                    }
                }
            }
        }
    }
    /* Opacity only */
    else if(mask == NULL && opa < LV_OPA_MAX) {
        if(LV_RESULT_INVALID == LV_BLEND_HOOK(COLOR_BLEND_TO_L1_WITH_OPA)(dsc)) {
            for(int32_t y = 0; y < h; y++) {
                uint8_t * row = DEST_ROW(dest_buf, dest_stride, y);
                for(int32_t x = 0; x < w; x++) {
                    uint8_t current_bit = L1_DEST_GET_PX(row, x, y);
                    uint8_t new_bit = (opa * src_color + (255 - opa) * current_bit) / 255;
                    if(new_bit) {
                        L1_DEST_SET_PX(row, x, y);
                    }
                    else {
                        L1_DEST_CLR_PX(row, x, y);
                    }
                }
            }
        }
    }
    /* Masked with full opacity */
    else if(mask && opa >= LV_OPA_MAX) {
        if(LV_RESULT_INVALID == LV_BLEND_HOOK(COLOR_BLEND_TO_L1_WITH_MASK)(dsc)) {
            for(int32_t y = 0; y < h; y++) {
                uint8_t * row = DEST_ROW(dest_buf, dest_stride, y);
                for(int32_t x = 0; x < w; x++) {
                    uint8_t mask_val = mask[x];
                    if(mask_val == LV_OPA_TRANSP) continue;
                    if(mask_val == LV_OPA_COVER) {
                        if(src_color) {
                            L1_DEST_SET_PX(row, x, y);
                        }
                        else {
                            L1_DEST_CLR_PX(row, x, y);
                        }
                    }
                    else {
                        uint8_t current_bit = L1_DEST_GET_PX(row, x, y);
                        uint8_t new_bit = (mask_val * src_color + (255 - mask_val) * current_bit) / 255;
                        if(new_bit) {
                            L1_DEST_SET_PX(row, x, y);
                        }
                        else {
                            L1_DEST_CLR_PX(row, x, y);
                        }
                    }
                }
                mask += mask_stride;
            }
        }
    }
    /* Masked with opacity */
    else {
        if(LV_RESULT_INVALID == LV_BLEND_HOOK(COLOR_BLEND_TO_L1_MIX_MASK_OPA)(dsc)) {
            for(int32_t y = 0; y < h; y++) {
                uint8_t * row = DEST_ROW(dest_buf, dest_stride, y);
                for(int32_t x = 0; x < w; x++) {
                    uint8_t mask_val = mask[x];
                    if(mask_val == LV_OPA_TRANSP) continue;
                    uint8_t current_bit = L1_DEST_GET_PX(row, x, y);
                    uint8_t blended_opa = (mask_val * opa) / 255;
                    uint8_t new_bit = (blended_opa * src_color + (255 - blended_opa) * current_bit) / 255;
                    if(new_bit) {
                        L1_DEST_SET_PX(row, x, y);
                    }
                    else {
                        L1_DEST_CLR_PX(row, x, y);
                    }
                }
                mask += mask_stride;
            }
        }
    }
}

void LV_ATTRIBUTE_FAST_MEM LV_BLEND_IMAGE_FN(lv_draw_sw_blend_image_dsc_t * dsc)
{
    switch(dsc->src_color_format) {
#if LV_DRAW_SW_SUPPORT_RGB565
        case LV_COLOR_FORMAT_RGB565:
            rgb565_image_blend(dsc);
            break;
#endif
#if LV_DRAW_SW_SUPPORT_RGB565_SWAPPED
        case LV_COLOR_FORMAT_RGB565_SWAPPED:
            rgb565_swapped_image_blend(dsc);
            break;
#endif
#if LV_DRAW_SW_SUPPORT_RGB888
        case LV_COLOR_FORMAT_RGB888:
            rgb888_image_blend(dsc, 3);
            break;
#endif
#if LV_DRAW_SW_SUPPORT_XRGB8888
        case LV_COLOR_FORMAT_XRGB8888:
            rgb888_image_blend(dsc, 4);
            break;
#endif
#if LV_DRAW_SW_SUPPORT_ARGB8888
        case LV_COLOR_FORMAT_ARGB8888:
            argb8888_image_blend(dsc, false);
            break;
#endif
#if LV_DRAW_SW_SUPPORT_ARGB8888_PREMULTIPLIED
        case LV_COLOR_FORMAT_ARGB8888_PREMULTIPLIED:
            argb8888_image_blend(dsc, true);
            break;
#endif
#if LV_DRAW_SW_SUPPORT_L8
        case LV_COLOR_FORMAT_L8:
            l8_image_blend(dsc);
            break;
#endif
#if LV_DRAW_SW_SUPPORT_AL88
        case LV_COLOR_FORMAT_AL88:
            al88_image_blend(dsc);
            break;
#endif
        case LV_COLOR_FORMAT_L1_HTILED_MSB:
            l1_image_blend(dsc);
            break;
        default:
            LV_LOG_WARN("Not supported source color format 0x%02X", dsc->src_color_format);
            break;
    }
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void LV_ATTRIBUTE_FAST_MEM l1_image_blend(lv_draw_sw_blend_image_dsc_t * dsc)
{
    int32_t w = dsc->dest_w;
    int32_t h = dsc->dest_h;
    lv_opa_t opa = dsc->opa;
    uint8_t * dest_buf_l1 = dsc->dest_buf;
    int32_t dest_stride = dsc->dest_stride;
    const uint8_t * src_buf_l1 = dsc->src_buf;
    int32_t src_stride = dsc->src_stride;
    const lv_opa_t * mask_buf = dsc->mask_buf;
    int32_t mask_stride = dsc->mask_stride;

    int32_t dest_x;
    int32_t src_x;
    int32_t y;

    const int32_t bit_ofs = dsc->relative_area.x1 % 8;
    const int32_t y1_abs = dsc->relative_area.y1;
    LV_UNUSED(bit_ofs);
    LV_UNUSED(y1_abs);

    if(dsc->blend_mode == LV_BLEND_MODE_NORMAL) {
        if(mask_buf == NULL && opa >= LV_OPA_MAX) {
            if(LV_RESULT_INVALID == LV_BLEND_HOOK(L1_BLEND_NORMAL_TO_L1)(dsc)) {
                for(y = 0; y < h; y++) {
                    uint8_t * row = DEST_ROW(dest_buf_l1, dest_stride, y);
                    for(dest_x = 0, src_x = 0; src_x < w; dest_x++, src_x++) {
                        if(get_bit(src_buf_l1, src_x)) {
                            L1_DEST_SET_PX(row, dest_x, y);
                        }
                        else {
                            L1_DEST_CLR_PX(row, dest_x, y);
                        }
                    }
                    src_buf_l1 = drawbuf_next_row(src_buf_l1, src_stride);
                }
            }
        }
        else if(mask_buf == NULL && opa < LV_OPA_MAX) {
            if(LV_RESULT_INVALID == LV_BLEND_HOOK(L1_BLEND_NORMAL_TO_L1_WITH_OPA)(dsc)) {
                for(y = 0; y < h; y++) {
                    uint8_t * row = DEST_ROW(dest_buf_l1, dest_stride, y);
                    for(dest_x = 0, src_x = 0; src_x < w; dest_x++, src_x++) {
                        uint8_t src = get_bit(src_buf_l1, src_x);
                        uint8_t dest = L1_DEST_GET_PX(row, dest_x, y);
                        uint8_t blended = (src * opa + dest * (255 - opa));
                        if(blended > L1_LUM_THRESHOLD) {
                            L1_DEST_SET_PX(row, dest_x, y);
                        }
                        else  {
                            L1_DEST_CLR_PX(row, dest_x, y);
                        }
                    }
                    src_buf_l1 = drawbuf_next_row(src_buf_l1, src_stride);
                }
            }
        }
        else if(mask_buf && opa >= LV_OPA_MAX) {
            if(LV_RESULT_INVALID == LV_BLEND_HOOK(L1_BLEND_NORMAL_TO_L1_WITH_MASK)(dsc)) {
                for(y = 0; y < h; y++) {
                    uint8_t * row = DEST_ROW(dest_buf_l1, dest_stride, y);
                    for(dest_x = 0, src_x = 0; src_x < w; dest_x++, src_x++) {
                        uint8_t mask_val = mask_buf[src_x];
                        uint8_t src = get_bit(src_buf_l1, src_x);
                        uint8_t dest = L1_DEST_GET_PX(row, dest_x, y);
                        uint8_t blended = (src * mask_val + dest * (255 - mask_val));
                        if(blended > L1_LUM_THRESHOLD) {
                            L1_DEST_SET_PX(row, dest_x, y);
                        }
                        else  {
                            L1_DEST_CLR_PX(row, dest_x, y);
                        }
                    }
                    src_buf_l1 = drawbuf_next_row(src_buf_l1, src_stride);
                    mask_buf += mask_stride;
                }
            }
        }
        else if(mask_buf && opa < LV_OPA_MAX) {
            if(LV_RESULT_INVALID == LV_BLEND_HOOK(L1_BLEND_NORMAL_TO_L1_MIX_MASK_OPA)(dsc)) {
                for(y = 0; y < h; y++) {
                    uint8_t * row = DEST_ROW(dest_buf_l1, dest_stride, y);
                    for(dest_x = 0, src_x = 0; src_x < w; dest_x++, src_x++) {
                        uint8_t mask_val = mask_buf[src_x];
                        if(mask_val == LV_OPA_TRANSP) continue;
                        uint8_t src = get_bit(src_buf_l1, src_x);
                        uint8_t dest = L1_DEST_GET_PX(row, dest_x, y);
                        uint8_t blend_opa = LV_OPA_MIX2(mask_val, opa);
                        uint8_t blended = (src * blend_opa + dest * (255 - blend_opa));
                        if(blended > L1_LUM_THRESHOLD) {
                            L1_DEST_SET_PX(row, dest_x, y);
                        }
                        else  {
                            L1_DEST_CLR_PX(row, dest_x, y);
                        }
                    }
                    src_buf_l1 = drawbuf_next_row(src_buf_l1, src_stride);
                    mask_buf += mask_stride;
                }
            }
        }
    }
    else {
        lv_color32_t src_argb;
        for(y = 0; y < h; y++) {
            uint8_t * row = DEST_ROW(dest_buf_l1, dest_stride, y);
            for(dest_x = 0, src_x = 0; src_x < w; dest_x++, src_x++) {
                src_argb.red = get_bit(src_buf_l1, src_x) * 255;
                src_argb.green = src_argb.red;
                src_argb.blue = src_argb.red;
                if(mask_buf == NULL) src_argb.alpha = opa;
                else src_argb.alpha = LV_OPA_MIX2(mask_buf[dest_x], opa);
                blend_non_normal_pixel(DEST_PX_ADDR(row, dest_x, y), DEST_PX_SHIFT(dest_x, y), src_argb, dsc->blend_mode);
            }
            if(mask_buf) mask_buf += mask_stride;
            src_buf_l1 = drawbuf_next_row(src_buf_l1, src_stride);
        }
    }
}

#if LV_DRAW_SW_SUPPORT_L8
static void LV_ATTRIBUTE_FAST_MEM l8_image_blend(lv_draw_sw_blend_image_dsc_t * dsc)
{
    int32_t w = dsc->dest_w;
    int32_t h = dsc->dest_h;
    lv_opa_t opa = dsc->opa;
    uint8_t * dest_buf_l1 = dsc->dest_buf;
    int32_t dest_stride = dsc->dest_stride;
    const uint8_t * src_buf_l8 = dsc->src_buf;
    int32_t src_stride = dsc->src_stride;
    const lv_opa_t * mask_buf = dsc->mask_buf;
    int32_t mask_stride = dsc->mask_stride;

    int32_t src_x, dest_x;
    int32_t y;

    const int32_t bit_ofs = dsc->relative_area.x1 % 8;
    const int32_t y1_abs = dsc->relative_area.y1;
    LV_UNUSED(bit_ofs);
    LV_UNUSED(y1_abs);

    if(dsc->blend_mode == LV_BLEND_MODE_NORMAL) {
        if(mask_buf == NULL && opa >= LV_OPA_MAX) {
            if(LV_RESULT_INVALID == LV_BLEND_HOOK(L8_BLEND_NORMAL_TO_L1)(dsc)) {
                for(y = 0; y < h; y++) {
                    uint8_t * row = DEST_ROW(dest_buf_l1, dest_stride, y);
                    for(dest_x = 0, src_x = 0; src_x < w; dest_x++, src_x++) {
                        if(src_buf_l8[src_x] > L1_LUM_THRESHOLD) {
                            L1_DEST_SET_PX(row, dest_x, y);
                        }
                        else {
                            L1_DEST_CLR_PX(row, dest_x, y);
                        }
                    }
                    src_buf_l8 = drawbuf_next_row(src_buf_l8, src_stride);
                }
            }
        }
        else if(mask_buf == NULL && opa < LV_OPA_MAX) {
            if(LV_RESULT_INVALID == LV_BLEND_HOOK(L8_BLEND_NORMAL_TO_L1_WITH_OPA)(dsc)) {
                for(y = 0; y < h; y++) {
                    uint8_t * row = DEST_ROW(dest_buf_l1, dest_stride, y);
                    for(dest_x = 0, src_x = 0; src_x < w; dest_x++, src_x++) {
                        uint8_t dest_val = L1_DEST_GET_PX(row, dest_x, y) * 255;
                        lv_color_8_8_mix(src_buf_l8[src_x], &dest_val, opa);
                        if(dest_val > L1_LUM_THRESHOLD) {
                            L1_DEST_SET_PX(row, dest_x, y);
                        }
                        else {
                            L1_DEST_CLR_PX(row, dest_x, y);
                        }
                    }
                    src_buf_l8 = drawbuf_next_row(src_buf_l8, src_stride);
                }
            }
        }
        else if(mask_buf && opa >= LV_OPA_MAX) {
            if(LV_RESULT_INVALID == LV_BLEND_HOOK(L8_BLEND_NORMAL_TO_L1_WITH_MASK)(dsc)) {
                for(y = 0; y < h; y++) {
                    uint8_t * row = DEST_ROW(dest_buf_l1, dest_stride, y);
                    for(dest_x = 0, src_x = 0; src_x < w; dest_x++, src_x++) {
                        uint8_t src_luminance = src_buf_l8[src_x];
                        uint8_t dest_val = L1_DEST_GET_PX(row, dest_x, y) * 255;
                        lv_color_8_8_mix(src_luminance, &dest_val, mask_buf[src_x]);
                        if(dest_val > L1_LUM_THRESHOLD) {
                            L1_DEST_SET_PX(row, dest_x, y);
                        }
                        else {
                            L1_DEST_CLR_PX(row, dest_x, y);
                        }
                    }
                    src_buf_l8 = drawbuf_next_row(src_buf_l8, src_stride);
                    mask_buf += mask_stride;
                }
            }
        }
        else if(mask_buf && opa < LV_OPA_MAX) {
            if(LV_RESULT_INVALID == LV_BLEND_HOOK(L8_BLEND_NORMAL_TO_L1_MIX_MASK_OPA)(dsc)) {
                for(y = 0; y < h; y++) {
                    uint8_t * row = DEST_ROW(dest_buf_l1, dest_stride, y);
                    for(dest_x = 0, src_x = 0; src_x < w; dest_x++, src_x++) {
                        uint8_t src_luminance = src_buf_l8[src_x];
                        uint8_t dest_val = L1_DEST_GET_PX(row, dest_x, y) * 255;
                        lv_color_8_8_mix(src_luminance, &dest_val, LV_OPA_MIX2(mask_buf[src_x], opa));
                        if(dest_val > L1_LUM_THRESHOLD) {
                            L1_DEST_SET_PX(row, dest_x, y);
                        }
                        else {
                            L1_DEST_CLR_PX(row, dest_x, y);
                        }
                    }
                    src_buf_l8 = drawbuf_next_row(src_buf_l8, src_stride);
                    mask_buf += mask_stride;
                }
            }
        }
    }
    else {
        lv_color32_t src_argb;
        for(y = 0; y < h; y++) {
            uint8_t * row = DEST_ROW(dest_buf_l1, dest_stride, y);
            for(src_x = 0, dest_x = 0; src_x < w; src_x++, dest_x++) {
                src_argb.red = src_buf_l8[src_x];
                src_argb.green = src_buf_l8[src_x];
                src_argb.blue = src_buf_l8[src_x];
                if(mask_buf == NULL) src_argb.alpha = opa;
                else src_argb.alpha = LV_OPA_MIX2(mask_buf[src_x], opa);
                blend_non_normal_pixel(DEST_PX_ADDR(row, dest_x, y), DEST_PX_SHIFT(dest_x, y), src_argb, dsc->blend_mode);
            }
            if(mask_buf) mask_buf += mask_stride;
            src_buf_l8 = drawbuf_next_row(src_buf_l8, src_stride);
        }
    }
}
#endif

#if LV_DRAW_SW_SUPPORT_AL88
static void LV_ATTRIBUTE_FAST_MEM al88_image_blend(lv_draw_sw_blend_image_dsc_t * dsc)
{
    int32_t w = dsc->dest_w;
    int32_t h = dsc->dest_h;
    lv_opa_t opa = dsc->opa;
    uint8_t * dest_buf_l1 = dsc->dest_buf;
    int32_t dest_stride = dsc->dest_stride;
    const lv_color16a_t * src_buf_al88 = dsc->src_buf;
    int32_t src_stride = dsc->src_stride;
    const lv_opa_t * mask_buf = dsc->mask_buf;
    int32_t mask_stride = dsc->mask_stride;

    int32_t dest_x;
    int32_t src_x;
    int32_t y;

    const int32_t bit_ofs = dsc->relative_area.x1 % 8;
    const int32_t y1_abs = dsc->relative_area.y1;
    LV_UNUSED(bit_ofs);
    LV_UNUSED(y1_abs);

    if(dsc->blend_mode == LV_BLEND_MODE_NORMAL) {
        if(mask_buf == NULL && opa >= LV_OPA_MAX) {
            if(LV_RESULT_INVALID == LV_BLEND_HOOK(AL88_BLEND_NORMAL_TO_L1)(dsc)) {
                for(y = 0; y < h; y++) {
                    uint8_t * row = DEST_ROW(dest_buf_l1, dest_stride, y);
                    for(dest_x = 0, src_x = 0; src_x < w; dest_x++, src_x++) {
                        uint8_t dest_val = L1_DEST_GET_PX(row, dest_x, y) * 255;
                        lv_color_8_8_mix(src_buf_al88[src_x].lumi, &dest_val, src_buf_al88[src_x].alpha);
                        if(dest_val > L1_LUM_THRESHOLD) {
                            L1_DEST_SET_PX(row, dest_x, y);
                        }
                        else {
                            L1_DEST_CLR_PX(row, dest_x, y);
                        }
                    }
                    src_buf_al88 = drawbuf_next_row(src_buf_al88, src_stride);
                }
            }
        }
        else if(mask_buf == NULL && opa < LV_OPA_MAX) {
            if(LV_RESULT_INVALID == LV_BLEND_HOOK(AL88_BLEND_NORMAL_TO_L1_WITH_OPA)(dsc)) {
                for(y = 0; y < h; y++) {
                    uint8_t * row = DEST_ROW(dest_buf_l1, dest_stride, y);
                    for(dest_x = 0, src_x = 0; src_x < w; dest_x++, src_x++) {
                        uint8_t dest_val = L1_DEST_GET_PX(row, dest_x, y) * 255;
                        lv_color_8_8_mix(src_buf_al88[src_x].lumi, &dest_val, LV_OPA_MIX2(src_buf_al88[src_x].alpha, opa));
                        if(dest_val > L1_LUM_THRESHOLD) {
                            L1_DEST_SET_PX(row, dest_x, y);
                        }
                        else {
                            L1_DEST_CLR_PX(row, dest_x, y);
                        }
                    }
                    src_buf_al88 = drawbuf_next_row(src_buf_al88, src_stride);
                }
            }
        }
        else if(mask_buf && opa >= LV_OPA_MAX) {
            if(LV_RESULT_INVALID == LV_BLEND_HOOK(AL88_BLEND_NORMAL_TO_L1_WITH_MASK)(dsc)) {
                for(y = 0; y < h; y++) {
                    uint8_t * row = DEST_ROW(dest_buf_l1, dest_stride, y);
                    for(dest_x = 0, src_x = 0; src_x < w; dest_x++, src_x++) {
                        uint8_t dest_val = L1_DEST_GET_PX(row, dest_x, y) * 255;
                        lv_color_8_8_mix(src_buf_al88[src_x].lumi, &dest_val,  LV_OPA_MIX2(src_buf_al88[src_x].alpha, mask_buf[src_x]));
                        if(dest_val > L1_LUM_THRESHOLD) {
                            L1_DEST_SET_PX(row, dest_x, y);
                        }
                        else {
                            L1_DEST_CLR_PX(row, dest_x, y);
                        }
                    }
                    src_buf_al88 = drawbuf_next_row(src_buf_al88, src_stride);
                    mask_buf += mask_stride;
                }
            }
        }
        else if(mask_buf && opa < LV_OPA_MAX) {
            if(LV_RESULT_INVALID == LV_BLEND_HOOK(AL88_BLEND_NORMAL_TO_L1_MIX_MASK_OPA)(dsc)) {
                for(y = 0; y < h; y++) {
                    uint8_t * row = DEST_ROW(dest_buf_l1, dest_stride, y);
                    for(dest_x = 0, src_x = 0; src_x < w; dest_x++, src_x++) {
                        uint8_t dest_val = L1_DEST_GET_PX(row, dest_x, y) * 255;
                        lv_color_8_8_mix(src_buf_al88[src_x].lumi, &dest_val, LV_OPA_MIX3(src_buf_al88[src_x].alpha, mask_buf[src_x], opa));
                        if(dest_val > L1_LUM_THRESHOLD) {
                            L1_DEST_SET_PX(row, dest_x, y);
                        }
                        else {
                            L1_DEST_CLR_PX(row, dest_x, y);
                        }
                    }
                    src_buf_al88 = drawbuf_next_row(src_buf_al88, src_stride);
                    mask_buf += mask_stride;
                }
            }
        }
    }
    else {
        lv_color32_t src_argb;
        for(y = 0; y < h; y++) {
            uint8_t * row = DEST_ROW(dest_buf_l1, dest_stride, y);
            for(dest_x = 0, src_x = 0; src_x < w; dest_x++, src_x++) {
                src_argb.red = src_buf_al88[src_x].lumi;
                src_argb.green = src_buf_al88[src_x].lumi;
                src_argb.blue = src_buf_al88[src_x].lumi;
                if(mask_buf == NULL) src_argb.alpha = LV_OPA_MIX2(src_buf_al88[src_x].alpha, opa);
                else src_argb.alpha = LV_OPA_MIX3(src_buf_al88[src_x].alpha, mask_buf[src_x], opa);
                blend_non_normal_pixel(DEST_PX_ADDR(row, dest_x, y), DEST_PX_SHIFT(dest_x, y), src_argb, dsc->blend_mode);
            }
            if(mask_buf) mask_buf += mask_stride;
            src_buf_al88 = drawbuf_next_row(src_buf_al88, src_stride);
        }
    }
}
#endif

#if LV_DRAW_SW_SUPPORT_ARGB8888 || LV_DRAW_SW_SUPPORT_ARGB8888_PREMULTIPLIED
static void LV_ATTRIBUTE_FAST_MEM argb8888_image_blend(lv_draw_sw_blend_image_dsc_t * dsc,
                                                       bool premultiplied)
{
    int32_t w = dsc->dest_w;
    int32_t h = dsc->dest_h;
    lv_opa_t opa = dsc->opa;
    uint8_t * dest_buf_l1 = dsc->dest_buf;
    int32_t dest_stride = dsc->dest_stride;
    const lv_color32_t * src_buf_c32 = dsc->src_buf;
    int32_t src_stride = dsc->src_stride;
    const lv_opa_t * mask_buf = dsc->mask_buf;
    int32_t mask_stride = dsc->mask_stride;

    int32_t x;
    int32_t y;

    const int32_t bit_ofs = dsc->relative_area.x1 % 8;
    const int32_t y1_abs = dsc->relative_area.y1;
    LV_UNUSED(bit_ofs);
    LV_UNUSED(y1_abs);

    if(dsc->blend_mode == LV_BLEND_MODE_NORMAL) {
        if(mask_buf == NULL && opa >= LV_OPA_MAX) {
            if(LV_RESULT_INVALID == LV_BLEND_HOOK(ARGB8888_BLEND_NORMAL_TO_L1)(dsc)) {
                for(y = 0; y < h; y++) {
                    uint8_t * row = DEST_ROW(dest_buf_l1, dest_stride, y);
                    for(x = 0; x < w; x++) {
                        uint8_t src = lv_color32_lumi_of(src_buf_c32[x], premultiplied);
                        uint8_t dest = L1_DEST_GET_PX(row, x, y) * 255;
                        lv_color_8_8_mix(src, &dest, src_buf_c32[x].alpha);
                        if(dest > L1_LUM_THRESHOLD) {
                            L1_DEST_SET_PX(row, x, y);
                        }
                        else {
                            L1_DEST_CLR_PX(row, x, y);
                        }
                    }
                    src_buf_c32 = drawbuf_next_row(src_buf_c32, src_stride);
                }
            }
        }
        else if(mask_buf == NULL && opa < LV_OPA_MAX) {
            if(LV_RESULT_INVALID == LV_BLEND_HOOK(ARGB8888_BLEND_NORMAL_TO_L1_WITH_OPA)(dsc)) {
                for(y = 0; y < h; y++) {
                    uint8_t * row = DEST_ROW(dest_buf_l1, dest_stride, y);
                    for(x = 0; x < w; x++) {
                        uint8_t src = lv_color32_lumi_of(src_buf_c32[x], premultiplied);
                        uint8_t dest = L1_DEST_GET_PX(row, x, y) * 255;
                        lv_color_8_8_mix(src, &dest, LV_OPA_MIX2(opa, src_buf_c32[x].alpha));
                        if(dest > L1_LUM_THRESHOLD) {
                            L1_DEST_SET_PX(row, x, y);
                        }
                        else {
                            L1_DEST_CLR_PX(row, x, y);
                        }
                    }
                    src_buf_c32 = drawbuf_next_row(src_buf_c32, src_stride);
                }
            }
        }
        else if(mask_buf && opa >= LV_OPA_MAX) {
            if(LV_RESULT_INVALID == LV_BLEND_HOOK(ARGB8888_BLEND_NORMAL_TO_L1_WITH_MASK)(dsc)) {
                for(y = 0; y < h; y++) {
                    uint8_t * row = DEST_ROW(dest_buf_l1, dest_stride, y);
                    for(x = 0; x < w; x++) {
                        uint8_t src = lv_color32_lumi_of(src_buf_c32[x], premultiplied);
                        uint8_t dest = L1_DEST_GET_PX(row, x, y) * 255;
                        lv_color_8_8_mix(src, &dest, LV_OPA_MIX2(mask_buf[x], src_buf_c32[x].alpha));
                        if(dest > L1_LUM_THRESHOLD) {
                            L1_DEST_SET_PX(row, x, y);
                        }
                        else {
                            L1_DEST_CLR_PX(row, x, y);
                        }
                    }
                    src_buf_c32 = drawbuf_next_row(src_buf_c32, src_stride);
                    mask_buf += mask_stride;
                }
            }
        }
        else if(mask_buf && opa < LV_OPA_MAX) {
            if(LV_RESULT_INVALID == LV_BLEND_HOOK(ARGB8888_BLEND_NORMAL_TO_L1_MIX_MASK_OPA)(dsc)) {
                for(y = 0; y < h; y++) {
                    uint8_t * row = DEST_ROW(dest_buf_l1, dest_stride, y);
                    for(x = 0; x < w; x++) {
                        uint8_t src = lv_color32_lumi_of(src_buf_c32[x], premultiplied);
                        uint8_t dest = L1_DEST_GET_PX(row, x, y) * 255;
                        lv_color_8_8_mix(src, &dest, LV_OPA_MIX3(opa, mask_buf[x], src_buf_c32[x].alpha));
                        if(dest > L1_LUM_THRESHOLD) {
                            L1_DEST_SET_PX(row, x, y);
                        }
                        else {
                            L1_DEST_CLR_PX(row, x, y);
                        }
                    }
                    src_buf_c32 = drawbuf_next_row(src_buf_c32, src_stride);
                    mask_buf += mask_stride;
                }
            }
        }
    }
    else {
        for(y = 0; y < h; y++) {
            uint8_t * row = DEST_ROW(dest_buf_l1, dest_stride, y);
            for(x = 0; x < w; x++) {
                /*The blend modes work on straight colors*/
                lv_color32_t color_argb = premultiplied ? lv_color32_unpremultiply(src_buf_c32[x])
                                          : src_buf_c32[x];
                if(mask_buf == NULL) color_argb.alpha = LV_OPA_MIX2(color_argb.alpha, opa);
                else color_argb.alpha = LV_OPA_MIX3(color_argb.alpha, mask_buf[x], opa);
                blend_non_normal_pixel(DEST_PX_ADDR(row, x, y), DEST_PX_SHIFT(x, y), color_argb, dsc->blend_mode);
            }
            if(mask_buf) mask_buf += mask_stride;
            src_buf_c32 = drawbuf_next_row(src_buf_c32, src_stride);
        }
    }
}
#endif

#if LV_DRAW_SW_SUPPORT_RGB888 || LV_DRAW_SW_SUPPORT_XRGB8888
static void LV_ATTRIBUTE_FAST_MEM rgb888_image_blend(lv_draw_sw_blend_image_dsc_t * dsc,
                                                     const uint8_t src_px_size)
{
    int32_t w = dsc->dest_w;
    int32_t h = dsc->dest_h;
    lv_opa_t opa = dsc->opa;
    uint8_t * dest_buf_l1 = dsc->dest_buf;
    int32_t dest_stride = dsc->dest_stride;
    const uint8_t * src_buf_rgb888 = dsc->src_buf;
    int32_t src_stride = dsc->src_stride;
    const lv_opa_t * mask_buf = dsc->mask_buf;
    int32_t mask_stride = dsc->mask_stride;

    int32_t dest_x;
    int32_t src_x;
    int32_t y;

    const int32_t bit_ofs = dsc->relative_area.x1 % 8;
    const int32_t y1_abs = dsc->relative_area.y1;
    LV_UNUSED(bit_ofs);
    LV_UNUSED(y1_abs);


    if(dsc->blend_mode == LV_BLEND_MODE_NORMAL) {
        /*Special case*/
        if(mask_buf == NULL && opa >= LV_OPA_MAX) {
            if(LV_RESULT_INVALID == LV_BLEND_HOOK(RGB888_BLEND_NORMAL_TO_L1)(dsc)) {
                for(y = 0; y < h; y++) {
                    uint8_t * row = DEST_ROW(dest_buf_l1, dest_stride, y);
                    for(dest_x = 0, src_x = 0; dest_x < w; dest_x++, src_x += src_px_size) {
                        uint8_t src = lv_color24_luminance(&src_buf_rgb888[src_x]);
                        if(src > L1_LUM_THRESHOLD) {
                            L1_DEST_SET_PX(row, dest_x, y);
                        }
                        else {
                            L1_DEST_CLR_PX(row, dest_x, y);
                        }
                    }
                    src_buf_rgb888 = drawbuf_next_row(src_buf_rgb888, src_stride);
                }
            }
        }
        else if(mask_buf == NULL && opa < LV_OPA_MAX) {
            if(LV_RESULT_INVALID == LV_BLEND_HOOK(RGB888_BLEND_NORMAL_TO_L1_WITH_OPA)(dsc)) {
                for(y = 0; y < h; y++) {
                    uint8_t * row = DEST_ROW(dest_buf_l1, dest_stride, y);
                    for(dest_x = 0, src_x = 0; dest_x < w; dest_x++, src_x += src_px_size) {
                        uint8_t src = lv_color24_luminance(&src_buf_rgb888[src_x]);
                        uint8_t dest = L1_DEST_GET_PX(row, dest_x, y) * 255;
                        lv_color_8_8_mix(src, &dest, opa);
                        if(dest > L1_LUM_THRESHOLD) {
                            L1_DEST_SET_PX(row, dest_x, y);
                        }
                        else {
                            L1_DEST_CLR_PX(row, dest_x, y);
                        }
                    }
                    src_buf_rgb888 = drawbuf_next_row(src_buf_rgb888, src_stride);
                }
            }
        }
        else if(mask_buf && opa >= LV_OPA_MAX) {
            if(LV_RESULT_INVALID == LV_BLEND_HOOK(RGB888_BLEND_NORMAL_TO_L1_WITH_MASK)(dsc)) {
                uint32_t mask_x;
                for(y = 0; y < h; y++) {
                    uint8_t * row = DEST_ROW(dest_buf_l1, dest_stride, y);
                    for(mask_x = 0, dest_x = 0, src_x = 0; dest_x < w; mask_x++, dest_x++, src_x += src_px_size) {
                        uint8_t src = lv_color24_luminance(&src_buf_rgb888[src_x]);
                        uint8_t dest = L1_DEST_GET_PX(row, dest_x, y) * 255;
                        lv_color_8_8_mix(src, &dest, mask_buf[mask_x]);
                        if(dest > L1_LUM_THRESHOLD) {
                            L1_DEST_SET_PX(row, dest_x, y);
                        }
                        else {
                            L1_DEST_CLR_PX(row, dest_x, y);
                        }
                    }
                    src_buf_rgb888 = drawbuf_next_row(src_buf_rgb888, src_stride);
                    mask_buf += mask_stride;
                }
            }
        }
        else if(mask_buf && opa < LV_OPA_MAX) {
            if(LV_RESULT_INVALID == LV_BLEND_HOOK(RGB888_BLEND_NORMAL_TO_L1_MIX_MASK_OPA)(dsc)) {
                uint32_t mask_x;
                for(y = 0; y < h; y++) {
                    uint8_t * row = DEST_ROW(dest_buf_l1, dest_stride, y);
                    for(mask_x = 0, dest_x = 0, src_x = 0; dest_x < w; mask_x++, dest_x++, src_x += src_px_size) {
                        uint8_t src = lv_color24_luminance(&src_buf_rgb888[src_x]);
                        uint8_t dest = L1_DEST_GET_PX(row, dest_x, y) * 255;
                        lv_color_8_8_mix(src, &dest, LV_OPA_MIX2(mask_buf[mask_x], opa));
                        if(dest > L1_LUM_THRESHOLD) {
                            L1_DEST_SET_PX(row, dest_x, y);
                        }
                        else {
                            L1_DEST_CLR_PX(row, dest_x, y);
                        }
                    }
                    src_buf_rgb888 = drawbuf_next_row(src_buf_rgb888, src_stride);
                    mask_buf += mask_stride;
                }
            }
        }
        else {
            lv_color32_t src_argb;
            for(y = 0; y < h; y++) {
                uint8_t * row = DEST_ROW(dest_buf_l1, dest_stride, y);
                for(dest_x = 0, src_x = 0; dest_x < w; dest_x++, src_x += src_px_size) {
                    src_argb.red = src_buf_rgb888[src_x + 2];
                    src_argb.green = src_buf_rgb888[src_x + 1];
                    src_argb.blue = src_buf_rgb888[src_x + 0];
                    if(mask_buf == NULL) src_argb.alpha = opa;
                    else src_argb.alpha = LV_OPA_MIX2(mask_buf[dest_x], opa);

                    blend_non_normal_pixel(DEST_PX_ADDR(row, dest_x, y), DEST_PX_SHIFT(dest_x, y), src_argb, dsc->blend_mode);
                }
                if(mask_buf) mask_buf += mask_stride;
                src_buf_rgb888 = drawbuf_next_row(src_buf_rgb888, src_stride);
            }
        }
    }
}
#endif

#if LV_DRAW_SW_SUPPORT_RGB565
static void LV_ATTRIBUTE_FAST_MEM rgb565_image_blend(lv_draw_sw_blend_image_dsc_t * dsc)
{
    int32_t w = dsc->dest_w;
    int32_t h = dsc->dest_h;
    lv_opa_t opa = dsc->opa;
    uint8_t * dest_buf_l1 = dsc->dest_buf;
    int32_t dest_stride = dsc->dest_stride;
    const lv_color16_t * src_buf_c16 = (const lv_color16_t *)dsc->src_buf;
    int32_t src_stride = dsc->src_stride;
    const lv_opa_t * mask_buf = dsc->mask_buf;
    int32_t mask_stride = dsc->mask_stride;

    int32_t src_x;
    int32_t dest_x;
    int32_t y;

    const int32_t bit_ofs = dsc->relative_area.x1 % 8;
    const int32_t y1_abs = dsc->relative_area.y1;
    LV_UNUSED(bit_ofs);
    LV_UNUSED(y1_abs);

    if(dsc->blend_mode == LV_BLEND_MODE_NORMAL) {
        if(mask_buf == NULL && opa >= LV_OPA_MAX) {
            if(LV_RESULT_INVALID == LV_BLEND_HOOK(RGB565_BLEND_NORMAL_TO_L1)(dsc)) {
                for(y = 0; y < h; y++) {
                    uint8_t * row = DEST_ROW(dest_buf_l1, dest_stride, y);

                    for(src_x = 0, dest_x = 0; src_x < w; dest_x++, src_x++) {
                        uint8_t src = lv_color16_luminance(src_buf_c16[src_x]);
                        if(src > L1_LUM_THRESHOLD) {
                            L1_DEST_SET_PX(row, dest_x, y);
                        }
                        else {
                            L1_DEST_CLR_PX(row, dest_x, y);
                        }
                    }
                    src_buf_c16 = drawbuf_next_row(src_buf_c16, src_stride);
                }
            }
        }
        else if(mask_buf == NULL && opa < LV_OPA_MAX) {
            if(LV_RESULT_INVALID == LV_BLEND_HOOK(RGB565_BLEND_NORMAL_TO_L1_WITH_OPA)(dsc)) {
                for(y = 0; y < h; y++) {
                    uint8_t * row = DEST_ROW(dest_buf_l1, dest_stride, y);
                    for(src_x = 0, dest_x = 0; src_x < w; dest_x++, src_x++) {
                        uint8_t src = lv_color16_luminance(src_buf_c16[src_x]);
                        uint8_t dest = L1_DEST_GET_PX(row, dest_x, y) * 255;
                        lv_color_8_8_mix(src, &dest, opa);
                        if(dest > L1_LUM_THRESHOLD) {
                            L1_DEST_SET_PX(row, dest_x, y);
                        }
                        else {
                            L1_DEST_CLR_PX(row, dest_x, y);
                        }
                    }
                    src_buf_c16 = drawbuf_next_row(src_buf_c16, src_stride);
                }
            }
        }
        else if(mask_buf && opa >= LV_OPA_MAX) {
            if(LV_RESULT_INVALID == LV_BLEND_HOOK(RGB565_BLEND_NORMAL_TO_L1_WITH_MASK)(dsc)) {
                uint32_t mask_x;
                for(y = 0; y < h; y++) {
                    uint8_t * row = DEST_ROW(dest_buf_l1, dest_stride, y);
                    for(mask_x = 0, dest_x = 0, src_x = 0; dest_x < w; mask_x++, dest_x++, src_x++) {
                        uint8_t src = lv_color16_luminance(src_buf_c16[src_x]);
                        uint8_t dest = L1_DEST_GET_PX(row, dest_x, y) * 255;
                        lv_color_8_8_mix(src, &dest, mask_buf[mask_x]);
                        if(dest > L1_LUM_THRESHOLD) {
                            L1_DEST_SET_PX(row, dest_x, y);
                        }
                        else {
                            L1_DEST_CLR_PX(row, dest_x, y);
                        }
                    }
                    src_buf_c16 = drawbuf_next_row(src_buf_c16, src_stride);
                    mask_buf += mask_stride;
                }
            }
        }
        else if(mask_buf && opa < LV_OPA_MAX)  {
            if(LV_RESULT_INVALID == LV_BLEND_HOOK(RGB565_BLEND_NORMAL_TO_L1_MIX_MASK_OPA)(dsc)) {
                uint32_t mask_x;
                for(y = 0; y < h; y++) {
                    uint8_t * row = DEST_ROW(dest_buf_l1, dest_stride, y);
                    for(mask_x = 0, dest_x = 0, src_x = 0; dest_x < w; mask_x++, dest_x++, src_x++) {
                        uint8_t src = lv_color16_luminance(src_buf_c16[src_x]);
                        uint8_t dest = L1_DEST_GET_PX(row, dest_x, y) * 255;
                        lv_color_8_8_mix(src, &dest, LV_OPA_MIX2(mask_buf[mask_x], opa));
                        if(dest > L1_LUM_THRESHOLD) {
                            L1_DEST_SET_PX(row, dest_x, y);
                        }
                        else {
                            L1_DEST_CLR_PX(row, dest_x, y);
                        }
                    }
                    src_buf_c16 = drawbuf_next_row(src_buf_c16, src_stride);
                    mask_buf += mask_stride;
                }
            }
        }
    }
    else {
        lv_color32_t src_argb;
        for(y = 0; y < h; y++) {
            uint8_t * row = DEST_ROW(dest_buf_l1, dest_stride, y);
            for(src_x = 0, dest_x = 0; src_x < w; src_x++, dest_x++) {
                src_argb.red = (src_buf_c16[src_x].red * 2106) >> 8;
                src_argb.green = (src_buf_c16[src_x].green * 1037) >> 8;
                src_argb.blue = (src_buf_c16[src_x].blue * 2106) >> 8;
                if(mask_buf == NULL) src_argb.alpha = opa;
                else src_argb.alpha = LV_OPA_MIX2(mask_buf[src_x], opa);

                blend_non_normal_pixel(DEST_PX_ADDR(row, dest_x, y), DEST_PX_SHIFT(dest_x, y), src_argb, dsc->blend_mode);
            }
            if(mask_buf) mask_buf += mask_stride;
            src_buf_c16 = drawbuf_next_row(src_buf_c16, src_stride);
        }
    }
}
#endif

#if LV_DRAW_SW_SUPPORT_RGB565_SWAPPED
static void LV_ATTRIBUTE_FAST_MEM rgb565_swapped_image_blend(lv_draw_sw_blend_image_dsc_t * dsc)
{
    int32_t w = dsc->dest_w;
    int32_t h = dsc->dest_h;
    lv_opa_t opa = dsc->opa;
    uint8_t * dest_buf_l1 = dsc->dest_buf;
    int32_t dest_stride = dsc->dest_stride;
    const uint16_t * src_buf_u16 = dsc->src_buf;
    int32_t src_stride = dsc->src_stride;
    const lv_opa_t * mask_buf = dsc->mask_buf;
    int32_t mask_stride = dsc->mask_stride;

    int32_t src_x;
    int32_t dest_x;
    int32_t y;
    lv_color16_t px;

    const int32_t bit_ofs = dsc->relative_area.x1 % 8;
    const int32_t y1_abs = dsc->relative_area.y1;
    LV_UNUSED(bit_ofs);
    LV_UNUSED(y1_abs);


    if(dsc->blend_mode == LV_BLEND_MODE_NORMAL) {
        if(mask_buf == NULL && opa >= LV_OPA_MAX) {
            if(LV_RESULT_INVALID == LV_BLEND_HOOK(RGB565_SWAPPED_BLEND_NORMAL_TO_L1)(dsc)) {
                for(y = 0; y < h; y++) {
                    uint8_t * row = DEST_ROW(dest_buf_l1, dest_stride, y);

                    for(src_x = 0, dest_x = 0; src_x < w; dest_x++, src_x++) {
                        px = lv_color16_from_u16(lv_color_swap_16(src_buf_u16[src_x]));
                        uint8_t src = lv_color16_luminance(px);
                        if(src > L1_LUM_THRESHOLD) {
                            L1_DEST_SET_PX(row, dest_x, y);
                        }
                        else {
                            L1_DEST_CLR_PX(row, dest_x, y);
                        }
                    }
                    src_buf_u16 = drawbuf_next_row(src_buf_u16, src_stride);
                }
            }
        }
        else if(mask_buf == NULL && opa < LV_OPA_MAX) {
            if(LV_RESULT_INVALID == LV_BLEND_HOOK(RGB565_SWAPPED_BLEND_NORMAL_TO_L1_WITH_OPA)(dsc)) {
                for(y = 0; y < h; y++) {
                    uint8_t * row = DEST_ROW(dest_buf_l1, dest_stride, y);
                    for(src_x = 0, dest_x = 0; src_x < w; dest_x++, src_x++) {
                        px = lv_color16_from_u16(lv_color_swap_16(src_buf_u16[src_x]));
                        uint8_t src = lv_color16_luminance(px);
                        uint8_t dest = L1_DEST_GET_PX(row, dest_x, y) * 255;
                        lv_color_8_8_mix(src, &dest, opa);
                        if(dest > L1_LUM_THRESHOLD) {
                            L1_DEST_SET_PX(row, dest_x, y);
                        }
                        else {
                            L1_DEST_CLR_PX(row, dest_x, y);
                        }
                    }
                    src_buf_u16 = drawbuf_next_row(src_buf_u16, src_stride);
                }
            }
        }
        else if(mask_buf && opa >= LV_OPA_MAX) {
            if(LV_RESULT_INVALID == LV_BLEND_HOOK(RGB565_SWAPPED_BLEND_NORMAL_TO_L1_WITH_MASK)(dsc)) {
                uint32_t mask_x;
                for(y = 0; y < h; y++) {
                    uint8_t * row = DEST_ROW(dest_buf_l1, dest_stride, y);
                    for(mask_x = 0, dest_x = 0, src_x = 0; dest_x < w; mask_x++, dest_x++, src_x++) {
                        px = lv_color16_from_u16(lv_color_swap_16(src_buf_u16[src_x]));
                        uint8_t src = lv_color16_luminance(px);
                        uint8_t dest = L1_DEST_GET_PX(row, dest_x, y) * 255;
                        lv_color_8_8_mix(src, &dest, mask_buf[mask_x]);
                        if(dest > L1_LUM_THRESHOLD) {
                            L1_DEST_SET_PX(row, dest_x, y);
                        }
                        else {
                            L1_DEST_CLR_PX(row, dest_x, y);
                        }
                    }
                    src_buf_u16 = drawbuf_next_row(src_buf_u16, src_stride);
                    mask_buf += mask_stride;
                }
            }
        }
        else if(mask_buf && opa < LV_OPA_MAX)  {
            if(LV_RESULT_INVALID == LV_BLEND_HOOK(RGB565_SWAPPED_BLEND_NORMAL_TO_L1_MIX_MASK_OPA)(dsc)) {
                uint32_t mask_x;
                for(y = 0; y < h; y++) {
                    uint8_t * row = DEST_ROW(dest_buf_l1, dest_stride, y);
                    for(mask_x = 0, dest_x = 0, src_x = 0; dest_x < w; mask_x++, dest_x++, src_x++) {
                        px = lv_color16_from_u16(lv_color_swap_16(src_buf_u16[src_x]));
                        uint8_t src = lv_color16_luminance(px);
                        uint8_t dest = L1_DEST_GET_PX(row, dest_x, y) * 255;
                        lv_color_8_8_mix(src, &dest, LV_OPA_MIX2(mask_buf[mask_x], opa));
                        if(dest > L1_LUM_THRESHOLD) {
                            L1_DEST_SET_PX(row, dest_x, y);
                        }
                        else {
                            L1_DEST_CLR_PX(row, dest_x, y);
                        }
                    }
                    src_buf_u16 = drawbuf_next_row(src_buf_u16, src_stride);
                    mask_buf += mask_stride;
                }
            }
        }
    }
    else {
        lv_color32_t src_argb;
        for(y = 0; y < h; y++) {
            uint8_t * row = DEST_ROW(dest_buf_l1, dest_stride, y);
            for(src_x = 0, dest_x = 0; src_x < w; src_x++, dest_x++) {
                px = lv_color16_from_u16(lv_color_swap_16(src_buf_u16[src_x]));
                src_argb.red = (px.red * 2106) >> 8;
                src_argb.green = (px.green * 1037) >> 8;
                src_argb.blue = (px.blue * 2106) >> 8;
                if(mask_buf == NULL) src_argb.alpha = opa;
                else src_argb.alpha = LV_OPA_MIX2(mask_buf[src_x], opa);

                blend_non_normal_pixel(DEST_PX_ADDR(row, dest_x, y), DEST_PX_SHIFT(dest_x, y), src_argb, dsc->blend_mode);
            }
            if(mask_buf) mask_buf += mask_stride;
            src_buf_u16 = drawbuf_next_row(src_buf_u16, src_stride);
        }
    }
}
#endif

static inline void LV_ATTRIBUTE_FAST_MEM blend_non_normal_pixel(uint8_t * dest_byte, int32_t shift,
                                                                lv_color32_t src, lv_blend_mode_t mode)
{
    uint8_t res;
    int32_t src_lumi = lv_color32_luminance(src);
    uint8_t dest_lumi = ((*dest_byte >> shift) & 1) * 255;
    switch(mode) {
        case LV_BLEND_MODE_ADDITIVE:
            res = LV_MIN(dest_lumi + src_lumi, 255);
            break;
        case LV_BLEND_MODE_SUBTRACTIVE:
            res = LV_MAX(dest_lumi - src_lumi, 0);
            break;
        case LV_BLEND_MODE_MULTIPLY:
            res = (dest_lumi * src_lumi) >> 8;
            break;
        case LV_BLEND_MODE_DIFFERENCE:
            res = LV_ABS(dest_lumi - src_lumi);
            break;
        default:
            LV_LOG_WARN("Not supported blend mode: %d", mode);
            return;
    }

    lv_color_8_8_mix(res, &dest_lumi, src.alpha);
    if(dest_lumi > L1_LUM_THRESHOLD) {
        *dest_byte |= (uint8_t)(1 << shift);
    }
    else {
        *dest_byte &= (uint8_t)~(1 << shift);
    }
}

static inline void LV_ATTRIBUTE_FAST_MEM lv_color_8_8_mix(const uint8_t src, uint8_t * dest, uint8_t mix)
{

    if(mix == 0) return;

    if(mix >= LV_OPA_MAX) {
        *dest = src;
    }
    else {
        lv_opa_t mix_inv = 255 - mix;
        *dest = (uint32_t)((uint32_t)src * mix + dest[0] * mix_inv) >> 8;
    }
}

static inline void * LV_ATTRIBUTE_FAST_MEM drawbuf_next_row(const void * buf, uint32_t stride)
{
    return (void *)((uint8_t *)buf + stride);
}

static inline uint8_t LV_ATTRIBUTE_FAST_MEM get_bit(const uint8_t * buf, int32_t bit_idx)
{
    return (buf[bit_idx / 8] >> (7 - (bit_idx % 8))) & 1;
}

#if LV_DRAW_SW_SUPPORT_RGB565_SWAPPED
static inline lv_color16_t LV_ATTRIBUTE_FAST_MEM lv_color16_from_u16(uint16_t raw)
{
    lv_color16_t c;
    c.red = (raw >> 11) & 0x1F;
    c.green = (raw >> 5) & 0x3F;
    c.blue = raw & 0x1F;
    return c;
}
#endif