#if LV_BUILD_TEST
#include "../lvgl.h"
#include "../demos/lv_demos.h"

#include "unity/unity.h"

void setUp(void)
{
    /* Function run before every test */
}

void tearDown(void)
{
    /* Function run after every test */
    lv_display_set_color_format(lv_display_get_default(), LV_COLOR_FORMAT_XRGB8888);
}

/*Render all demo scenes into the given L1 layout and compare them with the
  reference images. All four L1 layouts hold the same logical pixels, so they
  all check out against the same reference files.*/
static void render_and_compare_l1(lv_color_format_t cf)
{
    lv_display_set_color_format(lv_display_get_default(), cf);

    lv_opa_t opa_values[2] = {0xff, 0xc0};
    uint32_t opa;
    for(opa = 0; opa < 2; opa++) {
        uint32_t i;
        for(i = 0; i < LV_DEMO_RENDER_SCENE_NUM; i++) {
            lv_demo_render(i, opa_values[opa]);

            char buf[128];
            lv_snprintf(buf, sizeof(buf), "draw/render/l1/demo_render_%s_opa_%d.png",
                        lv_demo_render_get_scene_name(i), opa_values[opa]);
            TEST_ASSERT_EQUAL_SCREENSHOT(buf);
        }
    }
}

void test_render_to_l1(void)
{
    /*NanoVG reads back the FBO as 32bpp BGRA, so non-XRGB8888 targets don't apply*/
#if LV_BIN_DECODER_RAM_LOAD && LV_USE_DRAW_VG_LITE == 0 && LV_USE_DRAW_NANOVG == 0
#if LV_DRAW_SW_SUPPORT_L1_HTILED_MSB
    render_and_compare_l1(LV_COLOR_FORMAT_L1_HTILED_MSB);
#endif
#if LV_DRAW_SW_SUPPORT_L1_HTILED_LSB
    render_and_compare_l1(LV_COLOR_FORMAT_L1_HTILED_LSB);
#endif
#if LV_DRAW_SW_SUPPORT_L1_VTILED_MSB
    render_and_compare_l1(LV_COLOR_FORMAT_L1_VTILED_MSB);
#endif
#if LV_DRAW_SW_SUPPORT_L1_VTILED_LSB
    render_and_compare_l1(LV_COLOR_FORMAT_L1_VTILED_LSB);
#endif
#else
    /*Without LV_BIN_DECODER_RAM_LOAD can't test rotated images*/
    TEST_PASS();
#endif
}

#endif