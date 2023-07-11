#ifndef _LCD_DISPLAY_RESOURCE_H_
#define _LCD_DISPLAY_RESOURCE_H_

/*DO NOT include this file in multi-files since it includes big var.*/

typedef struct
{
    int img_w, img_h;
    const unsigned char* img;
}lcd_display_resource_t;

#define LCD_DISPLAY_ORI_X 2
#define LCD_DISPLAY_ORI_Y 4
#define LCD_DISPLAY_H_GAP 0
#define LCD_DISPLAY_V_GAP 4

/*----------------------------------------*/
/*The following are static image info.*/

#define LCD_STATIC_VOLT_IMG_W (48 + 8) /* title + colon */
#define LCD_STATIC_VOLT_IMG_H 16
static const unsigned char gs_static_volt_display_data[/* LCD_STATIC_VOLT_IMG_W * ceil(LCD_STATIC_VOLT_IMG_H / 8) */] =
{
    0x88, 0x64, 0x23, 0xA2, 0xA6, 0xAA, 0xA2, 0xBA, 0xA4, 0xA3, 0xA2, 0x26, 0x2A, 0xA2, 0x62, 0x00, 0x00, 0x00, 0xF8, 0x88, 0x88, 0x88, 0x88, 0xFF, 0x88, 0x88, 0x88, 0x88, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE, 0x02, 0x82, 0x82, 0x82, 0x82, 0xFA, 0x82, 0x82, 0x82, 0x82, 0x82, 0x02, 0x00, /* colon */ 0x00, 0x00, 0x00, 0xC0, 0xC0, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0xFF, 0x4A, 0x4A, 0x4A, 0x4A, 0x4A, 0x4A, 0x4B, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x08, 0x08, 0x08, 0x08, 0x7F, 0x88, 0x88, 0x88, 0x88, 0x9F, 0x80, 0xF0, 0x00, 0x80, 0x60, 0x1F, 0x40, 0x40, 0x40, 0x40, 0x40, 0x7F, 0x40, 0x40, 0x44, 0x58, 0x40, 0x40, 0x00, /* colon */ 0x00, 0x00, 0x00, 0x30, 0x30, 0x00, 0x00, 0x00, 
};
static const lcd_display_resource_t gs_static_volt_res =
{
    LCD_STATIC_VOLT_IMG_W, LCD_STATIC_VOLT_IMG_H, 
    gs_static_volt_display_data,
};
#define LCD_STATIC_VOLT_POS_X LCD_DISPLAY_ORI_X
#define LCD_STATIC_VOLT_POS_Y LCD_DISPLAY_ORI_Y
#define LCD_STATIC_VOLT_POS_W LCD_STATIC_VOLT_IMG_W
#define LCD_STATIC_VOLT_POS_H LCD_STATIC_VOLT_IMG_H

#define LCD_STATIC_AMT_S_IMG_W (80 + 8) /* title + colon */
#define LCD_STATIC_AMT_S_IMG_H 16
static const unsigned char gs_static_amt_s_display_data[/* LCD_STATIC_AMT_S_IMG_W * ceil(LCD_STATIC_AMT_S_IMG_H / 8) */] = 
{
    0x00, 0x00, 0xF8, 0x88, 0x88, 0x88, 0x88, 0xFF, 0x88, 0x88, 0x88, 0x88, 0xF8, 0x00, 0x00, 0x00, 0x10, 0x60, 0x02, 0x8C, 0x00, 0x44, 0x64, 0x54, 0x4D, 0x46, 0x44, 0x54, 0x64, 0xC4, 0x04, 0x00, 0x00, 0xFC, 0x84, 0x84, 0x84, 0xFC, 0x00, 0x10, 0x10, 0x10, 0x10, 0x10, 0xFF, 0x10, 0x10, 0x00, 0x00, 0xF8, 0x01, 0x06, 0x00, 0xF0, 0x12, 0x12, 0x12, 0xF2, 0x02, 0x02, 0x02, 0xFE, 0x00, 0x00, 0x20, 0x24, 0x24, 0xA4, 0xFE, 0x23, 0x22, 0x20, 0xFC, 0x04, 0x04, 0x04, 0x04, 0xFC, 0x00, 0x00, /* colon */ 0x00, 0x00, 0x00, 0xC0, 0xC0, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x1F, 0x08, 0x08, 0x08, 0x08, 0x7F, 0x88, 0x88, 0x88, 0x88, 0x9F, 0x80, 0xF0, 0x00, 0x04, 0x04, 0x7E, 0x01, 0x80, 0x40, 0x3E, 0x00, 0x00, 0xFE, 0x00, 0x00, 0x7E, 0x80, 0xE0, 0x00, 0x00, 0x3F, 0x10, 0x10, 0x10, 0x3F, 0x00, 0x00, 0x01, 0x06, 0x40, 0x80, 0x7F, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x1F, 0x11, 0x11, 0x11, 0x1F, 0x00, 0x40, 0x80, 0x7F, 0x00, 0x00, 0x10, 0x08, 0x06, 0x01, 0xFF, 0x01, 0x06, 0x80, 0x63, 0x19, 0x01, 0x01, 0x09, 0x33, 0xC0, 0x00, /* colon */ 0x00, 0x00, 0x00, 0x30, 0x30, 0x00, 0x00, 0x00, 
};
static const lcd_display_resource_t gs_static_amt_s_res = 
{
    LCD_STATIC_AMT_S_IMG_W, LCD_STATIC_AMT_S_IMG_H,
    gs_static_amt_s_display_data,
};
#define LCD_STATIC_AMT_S_POS_X LCD_DISPLAY_ORI_X
#define LCD_STATIC_AMT_S_POS_Y (LCD_DISPLAY_ORI_Y + LCD_STATIC_VOLT_POS_H + LCD_DISPLAY_V_GAP)
#define LCD_STATIC_AMT_S_POS_W LCD_STATIC_AMT_S_IMG_W
#define LCD_STATIC_AMT_S_POS_H LCD_STATIC_AMT_S_IMG_H

#define LCD_STATIC_DEV_ST_IMG_W (32 + 8) /* title + colon */
#define LCD_STATIC_DEV_ST_IMG_H 16
static const unsigned char gs_static_dev_st_display_data[/* LCD_STATIC_DEV_ST_IMG_W * ceil(LCD_STATIC_DEV_ST_IMG_H / 8) */] = 
{
    0x00, 0x08, 0x30, 0x00, 0xFF, 0x20, 0x20, 0x20, 0x20, 0xFF, 0x20, 0x20, 0x22, 0x2C, 0x20, 0x00, 0x00, 0x04, 0x84, 0x84, 0x44, 0x24, 0x54, 0x8F, 0x14, 0x24, 0x44, 0x84, 0x84, 0x04, 0x00, 0x00, /*colon*/ 0x00, 0x00, 0x00, 0xC0, 0xC0, 0x00, 0x00, 0x00, 
    0x04, 0x04, 0x02, 0x01, 0xFF, 0x80, 0x40, 0x30, 0x0E, 0x01, 0x06, 0x18, 0x20, 0x40, 0x80, 0x00, 0x41, 0x39, 0x00, 0x00, 0x3C, 0x40, 0x40, 0x42, 0x4C, 0x40, 0x40, 0x70, 0x04, 0x09, 0x31, 0x00, /*colon*/ 0x00, 0x00, 0x00, 0x30, 0x30, 0x00, 0x00, 0x00, 
};
static const lcd_display_resource_t gs_static_dev_st_res = 
{
    LCD_STATIC_DEV_ST_IMG_W, LCD_STATIC_DEV_ST_IMG_H,
    gs_static_dev_st_display_data,
};
#define LCD_STATIC_DEV_ST_POS_X (LCD_DISPLAY_ORI_X)
#define LCD_STATIC_DEV_ST_POS_Y (LCD_DISPLAY_ORI_Y +  LCD_STATIC_VOLT_POS_H + LCD_STATIC_AMT_S_POS_H + \
                                    2 * LCD_DISPLAY_V_GAP)
#define LCD_STATIC_DEV_ST_POS_W LCD_STATIC_DEV_ST_IMG_W
#define LCD_STATIC_DEV_ST_POS_H LCD_STATIC_DEV_ST_IMG_H

#define LCD_STATIC_DIST_IMG_W (32 + 8) /* title + colon */
#define LCD_STATIC_DIST_IMG_H 16
static const unsigned char gs_static_dist_display_data[/* LCD_STATIC_DIST_IMG_W * ceil(LCD_STATIC_DIST_IMG_H / 8) */] =
{
    0x00, 0x3E, 0x22, 0xE2, 0x22, 0x3E, 0x00, 0xFE, 0x22, 0x22, 0x22, 0x22, 0x22, 0xE2, 0x02, 0x00, 0x04, 0x04, 0x04, 0xF4, 0x84, 0xD4, 0xA5, 0xA6, 0xA4, 0xD4, 0x84, 0xF4, 0x04, 0x04, 0x04, 0x00, /* colon */ 0x00, 0x00, 0x00, 0xC0, 0xC0, 0x00, 0x00, 0x00, 
    0x20, 0x3F, 0x20, 0x1F, 0x11, 0x11, 0x00, 0x7F, 0x44, 0x44, 0x44, 0x44, 0x44, 0x47, 0x40, 0x00, 0x00, 0xFE, 0x02, 0x02, 0x12, 0x3A, 0x16, 0x13, 0x12, 0x1A, 0x32, 0x42, 0x82, 0x7E, 0x00, 0x00, /* colon */ 0x00, 0x00, 0x00, 0x30, 0x30, 0x00, 0x00, 0x00, 
};
static const lcd_display_resource_t gs_static_dist_res =
{
    LCD_STATIC_DIST_IMG_W, LCD_STATIC_DIST_IMG_H,
    gs_static_dist_display_data,
};
#define LCD_STATIC_DIST_POS_X (LCD_DISPLAY_ORI_X + LCD_STATIC_DEV_ST_POS_W + 16*4 + 50)
#define LCD_STATIC_DIST_POS_Y LCD_STATIC_DEV_ST_POS_Y
#define LCD_STATIC_DIST_POS_W LCD_STATIC_DIST_IMG_W
#define LCD_STATIC_DIST_POS_H LCD_STATIC_DIST_IMG_H

#define LCD_STATIC_AMT_IMG_W (48 + 8)
#define LCD_STATIC_AMT_IMG_H 16
static const unsigned char gs_static_amt_display_data[/* LCD_STATIC_AMT_IMG_W * ceil(LCD_STATIC_AMT_IMG_H / 8) */] = {};
static const lcd_display_resource_t gs_static_amt_res = 
{
    LCD_STATIC_AMT_IMG_W, LCD_STATIC_AMT_IMG_H,
    gs_static_amt_display_data,
};
#define LCD_STATIC_AMT_POS_X 0
#define LCD_STATIC_AMT_POS_Y 0
#define LCD_STATIC_AMT_POS_W 0
#define LCD_STATIC_AMT_POS_H 0

#define LCD_STATIC_DURA_IMG_W (64 + 8)
#define LCD_STATIC_DURA_IMG_H 16
static const unsigned char gs_static_dura_display_data[/* LCD_STATIC_DURA_IMG_W * ceil(LCD_STATIC_DURA_IMG_H / 8) */] = {};
static const lcd_display_resource_t gs_static_dura_res = 
{
    LCD_STATIC_DURA_IMG_W, LCD_STATIC_DURA_IMG_H,
    gs_static_dura_display_data,
};
#define LCD_STATIC_DURA_POS_X 0
#define LCD_STATIC_DURA_POS_Y 0
#define LCD_STATIC_DURA_POS_W 0
#define LCD_STATIC_DURA_POS_H 0

#define LCD_STATIC_LOGO_IMG_W 36
#define LCD_STATIC_LOGO_IMG_H 20
static const unsigned char gs_static_logo_display_data[/* LCD_STATIC_LOGO_IMG_W * ceil(LCD_STATIC_LOGO_IMG_H / 8) */] = {};
static const lcd_display_resource_t gs_static_logo_res = 
{
    LCD_STATIC_LOGO_IMG_W, LCD_STATIC_LOGO_IMG_H,
    gs_static_logo_display_data,
};
#define LCD_STATIC_LOGO_POS_X 0
#define LCD_STATIC_LOGO_POS_Y 0
#define LCD_STATIC_LOGO_POS_W 0
#define LCD_STATIC_LOGO_POS_H 0

/*The following are digit , alpha table and punctuation resource.*/
#define LCD_DIGIT_FONT_W 8
#define LCD_DIGIT_FONT_H 16
static unsigned char gs_digits_font[][16 /* LCD_DIGIT_FONT_W * ceil(LCD_DIGIT_FONT_H / 8) */] = 
{
    {/* 0 */
        0x00, 0xE0, 0x10, 0x08, 0x08, 0x10, 0xE0, 0x00, 
        0x00, 0x0F, 0x10, 0x20, 0x20, 0x10, 0x0F, 0x00, 
    },
    {/* 1 */
        0x00, 0x00, 0x10, 0x10, 0xF8, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x20, 0x20, 0x3F, 0x20, 0x20, 0x00, 
    },
    {/* 2 */
        0x00, 0x70, 0x08, 0x08, 0x08, 0x08, 0xF0, 0x00, 
        0x00, 0x30, 0x28, 0x24, 0x22, 0x21, 0x30, 0x00, 
    },
    {/* 3 */
        0x00, 0x30, 0x08, 0x08, 0x08, 0x88, 0x70, 0x00, 
        0x00, 0x18, 0x20, 0x21, 0x21, 0x22, 0x1C, 0x00, 
    },
    {/* 4 */
        0x00, 0x00, 0x80, 0x40, 0x30, 0xF8, 0x00, 0x00, 
        0x00, 0x06, 0x05, 0x24, 0x24, 0x3F, 0x24, 0x24, 
    },
    {/* 5 */
        0x00, 0xF8, 0x88, 0x88, 0x88, 0x08, 0x08, 0x00, 
        0x00, 0x19, 0x20, 0x20, 0x20, 0x11, 0x0E, 0x00, 
    },
    {/* 6 */
        0x00, 0xE0, 0x10, 0x88, 0x88, 0x90, 0x00, 0x00, 
        0x00, 0x0F, 0x11, 0x20, 0x20, 0x20, 0x1F, 0x00, 
    },
    {/* 7 */
        0x00, 0x18, 0x08, 0x08, 0x88, 0x68, 0x18, 0x00, 
        0x00, 0x00, 0x00, 0x3E, 0x01, 0x00, 0x00, 0x00, 
    },
    {/* 8 */
        0x00, 0x70, 0x88, 0x08, 0x08, 0x88, 0x70, 0x00, 
        0x00, 0x1C, 0x22, 0x21, 0x21, 0x22, 0x1C, 0x00, 
    },
    {/* 9 */
        0x00, 0xF0, 0x08, 0x08, 0x08, 0x10, 0xE0, 0x00, 
        0x00, 0x01, 0x12, 0x22, 0x22, 0x11, 0x0F, 0x00, 
    },
};

#define LCD_ALPHA_LOW_FONT_W 8 
#define LCD_ALPHA_LOW_FONT_H 16
static unsigned char gs_alpha_low_chars_font[][16 /* LCD_ALPHA_LOW_FONT_W * ceil(LCD_ALPHA_LOW_FONT_H / 8) */] =
{
    {/* a */
        0x00, 0x00, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 
        0x00, 0x19, 0x24, 0x24, 0x12, 0x3F, 0x20, 0x00, 
    },
    {/* b */
        0x10, 0xF0, 0x00, 0x80, 0x80, 0x00, 0x00, 0x00, 
        0x00, 0x3F, 0x11, 0x20, 0x20, 0x11, 0x0E, 0x00, 
    },
    {/* c */
        0x00, 0x00, 0x00, 0x80, 0x80, 0x80, 0x00, 0x00, 
        0x00, 0x0E, 0x11, 0x20, 0x20, 0x20, 0x11, 0x00, 
    },
    {/* d */
        0x00, 0x00, 0x80, 0x80, 0x80, 0x90, 0xF0, 0x00, 
        0x00, 0x1F, 0x20, 0x20, 0x20, 0x10, 0x3F, 0x20, 
    },
    {/* e */
        0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 
        0x00, 0x1F, 0x24, 0x24, 0x24, 0x24, 0x17, 0x00, 
    },
    {/* f */
        0x00, 0x80, 0x80, 0xE0, 0x90, 0x90, 0x20, 0x00, 
        0x00, 0x20, 0x20, 0x3F, 0x20, 0x20, 0x00, 0x00, 
    },
    {/* g */
        0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 
        0x00, 0x6B, 0x94, 0x94, 0x94, 0x93, 0x60, 0x00, 
    },
    {/* h */
        0x10, 0xF0, 0x00, 0x80, 0x80, 0x80, 0x00, 0x00, 
        0x20, 0x3F, 0x21, 0x00, 0x00, 0x20, 0x3F, 0x20, 
    },
    {/* i */
        0x00, 0x80, 0x98, 0x98, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x20, 0x20, 0x3F, 0x20, 0x20, 0x00, 0x00, 
    },
    {/* j */
        0x00, 0x00, 0x00, 0x80, 0x98, 0x98, 0x00, 0x00, 
        0x00, 0xC0, 0x80, 0x80, 0x80, 0x7F, 0x00, 0x00, 
    },
    {/* k */
        0x10, 0xF0, 0x00, 0x00, 0x80, 0x80, 0x80, 0x00, 
        0x20, 0x3F, 0x24, 0x06, 0x29, 0x30, 0x20, 0x00, 
    },
    {/* l */
        0x00, 0x10, 0x10, 0xF8, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x20, 0x20, 0x3F, 0x20, 0x20, 0x00, 0x00, 
    },
    {/* m */
        0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 
        0x20, 0x3F, 0x20, 0x00, 0x3F, 0x20, 0x00, 0x3F, 
    },
    {/* n */
        0x80, 0x80, 0x00, 0x80, 0x80, 0x80, 0x00, 0x00, 
        0x20, 0x3F, 0x21, 0x00, 0x00, 0x20, 0x3F, 0x20, 
    },
    {/* o */
        0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 
        0x00, 0x1F, 0x20, 0x20, 0x20, 0x20, 0x1F, 0x00, 
    },
    {/* p */
        0x80, 0x80, 0x00, 0x80, 0x80, 0x00, 0x00, 0x00, 
        0x80, 0xFF, 0x91, 0x20, 0x20, 0x11, 0x0E, 0x00, 
    },
    {/* q */
        0x00, 0x00, 0x00, 0x80, 0x80, 0x00, 0x80, 0x00, 
        0x00, 0x0E, 0x11, 0x20, 0x20, 0x91, 0xFF, 0x80, 
    },
    {/* r */
        0x80, 0x80, 0x80, 0x00, 0x80, 0x80, 0x80, 0x00, 
        0x20, 0x20, 0x3F, 0x21, 0x20, 0x00, 0x01, 0x00, 
    },
    {/* s */
        0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 
        0x00, 0x33, 0x24, 0x24, 0x24, 0x24, 0x19, 0x00, 
    },
    {/* t */
        0x00, 0x80, 0x80, 0xE0, 0x80, 0x80, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x1F, 0x20, 0x20, 0x10, 0x00, 
    },
    {/* u */
        0x80, 0x80, 0x00, 0x00, 0x00, 0x80, 0x80, 0x00, 
        0x00, 0x1F, 0x20, 0x20, 0x20, 0x10, 0x3F, 0x20, 
    },
    {/* v */
        0x80, 0x80, 0x80, 0x00, 0x80, 0x80, 0x80, 0x00, 
        0x00, 0x03, 0x0C, 0x30, 0x0C, 0x03, 0x00, 0x00, 
    },
    {/* w */
        0x80, 0x80, 0x00, 0x80, 0x80, 0x00, 0x80, 0x80, 
        0x01, 0x0E, 0x30, 0x0C, 0x07, 0x38, 0x06, 0x01, 
    },
    {/* x */
        0x00, 0x80, 0x80, 0x80, 0x00, 0x80, 0x80, 0x00, 
        0x00, 0x20, 0x31, 0x0E, 0x2E, 0x31, 0x20, 0x00, 
    },
    {/* y */
        0x80, 0x80, 0x80, 0x00, 0x00, 0x80, 0x80, 0x80, 
        0x00, 0x81, 0x86, 0x78, 0x18, 0x06, 0x01, 0x00, 
    },
    {/* z */
        0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 
        0x00, 0x21, 0x30, 0x2C, 0x22, 0x21, 0x30, 0x00, 
    },
};

#define ALPHA_HIGH_FONT_W 8
#define ALPHA_HIGH_FONT_H  16
static unsigned char gs_alpha_high_chars_font[][16 /* ALPHA_HIGH_FONT_W * ceil(ALPHA_HIGH_FONT_H / 8) */] =
{
    {/* A */
        0x00, 0x00, 0xC0, 0x38, 0xE0, 0x00, 0x00, 0x00, 
        0x20, 0x3C, 0x23, 0x02, 0x02, 0x27, 0x38, 0x20, 
    },
    {/* B */
        0x08, 0xF8, 0x88, 0x88, 0x88, 0x70, 0x00, 0x00, 
        0x20, 0x3F, 0x20, 0x20, 0x20, 0x11, 0x0E, 0x00, 
    },
    {/* C */
        0xC0, 0x30, 0x08, 0x08, 0x08, 0x08, 0x38, 0x00, 
        0x07, 0x18, 0x20, 0x20, 0x20, 0x10, 0x08, 0x00, 
    },
    {/* D */
        0x08, 0xF8, 0x08, 0x08, 0x08, 0x10, 0xE0, 0x00, 
        0x20, 0x3F, 0x20, 0x20, 0x20, 0x10, 0x0F, 0x00, 
    },
    {/* E */
        0x08, 0xF8, 0x88, 0x88, 0xE8, 0x08, 0x10, 0x00, 
        0x20, 0x3F, 0x20, 0x20, 0x23, 0x20, 0x18, 0x00, 
    },
    {/* F */
        0x08, 0xF8, 0x88, 0x88, 0xE8, 0x08, 0x10, 0x00, 
        0x20, 0x3F, 0x20, 0x00, 0x03, 0x00, 0x00, 0x00, 
    },
    {/* G */
        0xC0, 0x30, 0x08, 0x08, 0x08, 0x38, 0x00, 0x00, 
        0x07, 0x18, 0x20, 0x20, 0x22, 0x1E, 0x02, 0x00, 
    },
    {/* H */
        0x08, 0xF8, 0x08, 0x00, 0x00, 0x08, 0xF8, 0x08, 
        0x20, 0x3F, 0x21, 0x01, 0x01, 0x21, 0x3F, 0x20, 
    },
    {/* I */
        0x00, 0x08, 0x08, 0xF8, 0x08, 0x08, 0x00, 0x00, 
        0x00, 0x20, 0x20, 0x3F, 0x20, 0x20, 0x00, 0x00, 
    },
    {/* J */
        0x00, 0x00, 0x08, 0x08, 0xF8, 0x08, 0x08, 0x00, 
        0xC0, 0x80, 0x80, 0x80, 0x7F, 0x00, 0x00, 0x00, 
    },
    {/* K */
        0x08, 0xF8, 0x88, 0xC0, 0x28, 0x18, 0x08, 0x00, 
        0x20, 0x3F, 0x20, 0x01, 0x26, 0x38, 0x20, 0x00, 
    },
    {/* L */
        0x08, 0xF8, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x20, 0x3F, 0x20, 0x20, 0x20, 0x20, 0x30, 0x00, 
    },
    {/* M */
        0x08, 0xF8, 0xF8, 0x00, 0xF8, 0xF8, 0x08, 0x00, 
        0x20, 0x3F, 0x01, 0x3E, 0x01, 0x3F, 0x20, 0x00, 
    },
    {/* N */
        0x08, 0xF8, 0x30, 0xC0, 0x00, 0x08, 0xF8, 0x08, 
        0x20, 0x3F, 0x20, 0x00, 0x07, 0x18, 0x3F, 0x00, 
    },
    {/* O */
        0xE0, 0x10, 0x08, 0x08, 0x08, 0x10, 0xE0, 0x00, 
        0x0F, 0x10, 0x20, 0x20, 0x20, 0x10, 0x0F, 0x00, 
    },
    {/* P */
        0x08, 0xF8, 0x08, 0x08, 0x08, 0x08, 0xF0, 0x00, 
        0x20, 0x3F, 0x21, 0x01, 0x01, 0x01, 0x00, 0x00, 
    },
    {/* Q */
        0xE0, 0x10, 0x08, 0x08, 0x08, 0x10, 0xE0, 0x00, 
        0x0F, 0x10, 0x28, 0x28, 0x30, 0x50, 0x4F, 0x00, 
    },
    {/* R */
        0x08, 0xF8, 0x88, 0x88, 0x88, 0x88, 0x70, 0x00, 
        0x20, 0x3F, 0x20, 0x00, 0x03, 0x0C, 0x30, 0x20, 
    },
    {/* S */
        0x00, 0x70, 0x88, 0x08, 0x08, 0x08, 0x38, 0x00, 
        0x00, 0x38, 0x20, 0x21, 0x21, 0x22, 0x1C, 0x00, 
    },
    {/* T */
        0x18, 0x08, 0x08, 0xF8, 0x08, 0x08, 0x18, 0x00, 
        0x00, 0x00, 0x20, 0x3F, 0x20, 0x00, 0x00, 0x00, 
    },
    {/* U */
        0x08, 0xF8, 0x08, 0x00, 0x00, 0x08, 0xF8, 0x08, 
        0x00, 0x1F, 0x20, 0x20, 0x20, 0x20, 0x1F, 0x00, 
    },
    {/* V */
        0x08, 0x78, 0x88, 0x00, 0x00, 0xC8, 0x38, 0x08, 
        0x00, 0x00, 0x07, 0x38, 0x0E, 0x01, 0x00, 0x00, 
    },
    {/* W */
        0x08, 0xF8, 0x00, 0xF8, 0x00, 0xF8, 0x08, 0x00, 
        0x00, 0x03, 0x3E, 0x01, 0x3E, 0x03, 0x00, 0x00, 
    },
    {/* X */
        0x08, 0x18, 0x68, 0x80, 0x80, 0x68, 0x18, 0x08, 
        0x20, 0x30, 0x2C, 0x03, 0x03, 0x2C, 0x30, 0x20, 
    },
    {/* Y */
        0x08, 0x38, 0xC8, 0x00, 0xC8, 0x38, 0x08, 0x00, 
        0x00, 0x00, 0x20, 0x3F, 0x20, 0x00, 0x00, 0x00, 
    },
    {/* Z */
        0x10, 0x08, 0x08, 0x08, 0xC8, 0x38, 0x08, 0x00, 
        0x20, 0x38, 0x26, 0x21, 0x20, 0x20, 0x18, 0x00, 
    },
};

#define PUNC_COLON_FONT_W 8
#define PUNC_COLON_FONT_H 16
static unsigned char gs_punc_colon_font[ /* PUNC_COLON_FONT_W * ceil(PUNC_COLON_FONT_H / 8) */] = 
{/* : */
    0x00, 0x00, 0x00, 0xC0, 0xC0, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x30, 0x30, 0x00, 0x00, 0x00, 
};

#define PUNC_DOT_FONT_W 8
#define PUNC_DOT_FONT_H 16
static unsigned char gs_punc_dot_font[ /* PUNC_DOT_FONT_W * ceil(PUNC_DOT_FONT_H / 8) */] = 
{/* . */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 
};

/*--------------------The following are "dynamaic" info resurce.*/
#define LCD_BAT_POS_X 0
#define LCD_BAT_POS_Y 0
#define LCD_BAT_POS_W 0
#define LCD_BAT_POS_H 0
#define LCD_BAT_IMG_W 0
#define LCD_BAT_IMG_H 0
static unsigned char gs_lcd_bat_res[/* LCD_BAT_IMG_W * ceil(LCD_BAT_IMG_H / 8) */] = {};

#define LCD_WAN_BEAR_POS_X 0
#define LCD_WAN_BEAR_POS_Y 0
#define LCD_WAN_BEAR_POS_W 0
#define LCD_WAN_BEAR_POS_H 0
#define LCD_WAN_BEAR_IMG_W 0
#define LCD_WAN_BEAR_IMG_H 0
static unsigned char gs_wan_bear_res[/* LCD_WAN_BEAR_IMG_W * ceil(LCD_WAN_BEAR_IMG_W / 8) */] = {};

#define LCD_CELL_SRV_ST_POS_X 0
#define LCD_CELL_SRV_ST_POS_Y 0
#define LCD_CELL_SRV_ST_POS_W 0
#define LCD_CELL_SRV_ST_POS_H 0
#define LCD_CELL_SRV_ST_IMG_W 0
#define LCD_CELL_SRV_ST_IMG_H 0
static unsigned char gs_cell_srv_st_res[/* LCD_CELL_SRV_ST_IMG_W * ceil(LCD_CELL_SRV_ST_IMG_W / 8) */] = {};

#define LCD_WIFI_WAN_ST_POS_X 0
#define LCD_WIFI_WAN_ST_POS_Y 0
#define LCD_WIFI_WAN_ST_POS_W 0
#define LCD_WIFI_WAN_ST_POS_H 0
#define LCD_WIFI_WAN_ST_IMG_W 0
#define LCD_WIFI_WAN_ST_IMG_H 0
static unsigned char gs_wifi_wan_st_res[/* LCD_WIFI_WAN_ST_IMG_W * ceil(LCD_WIFI_WAN_ST_IMG_W / 8) */] = {};

#define LCD_SIM_CARD_ST_POS_X 0
#define LCD_SIM_CARD_ST_POS_Y 0
#define LCD_SIM_CARD_ST_POS_W 0
#define LCD_SIM_CARD_ST_POS_H 0
#define LCD_SIM_CARD_ST_IMG_W 0
#define LCD_SIM_CARD_ST_IMG_H 0
static unsigned char gs_sim_card_st_res[/* LCD_SIM_CARD_ST_IMG_W * ceil(LCD_SIM_CARD_ST_IMG_W / 8) */] = {};

#define LCD_DSP_CONN_POS_X 0
#define LCD_DSP_CONN_POS_Y 0
#define LCD_DSP_CONN_POS_W 0
#define LCD_DSP_CONN_POS_H 0
#define LCD_DSP_CONN_IMG_W 0
#define LCD_DSP_CONN_IMG_H 0
static unsigned char gs_dsp_conn_res[/* LCD_DSP_CONN_IMG_W * ceil(LCD_DSP_CONN_IMG_W / 8) */] = {};

#define LCD_CUBE_VOLT_POS_X (LCD_STATIC_VOLT_POS_X + LCD_STATIC_VOLT_POS_W)
#define LCD_CUBE_VOLT_POS_Y LCD_STATIC_VOLT_POS_Y 
#define LCD_CUBE_VOLT_POS_W 40 /*maximum width*/
#define LCD_CUBE_VOLT_POS_H 16

#define LCD_CUBE_AMTS_POS_X (LCD_STATIC_AMT_S_POS_X + LCD_STATIC_AMT_S_POS_W)
#define LCD_CUBE_AMTS_POS_Y LCD_STATIC_AMT_S_POS_Y 
#define LCD_CUBE_AMTS_POS_W 56 /*maximum width*/
#define LCD_CUBE_AMTS_POS_H 16

#define LCD_EXPO_ST_IDLE_IMG_W 32
#define LCD_EXPO_ST_IDLE_IMG_H 16
static const unsigned char gs_expo_st_idle_display_data[/* LCD_EXPO_ST_IDLE_IMG_W * ceil(LCD_EXPO_ST_IDLE_IMG_H / 8) */] = 
{
    0x10, 0x0C, 0x44, 0x24, 0x14, 0x04, 0x05, 0x06, 0x04, 0x04, 0x14, 0x24, 0x44, 0x14, 0x0C, 0x00, 0x00, 0xF8, 0x01, 0x42, 0x40, 0x42, 0x42, 0xFA, 0x42, 0x42, 0x42, 0x42, 0x02, 0xFE, 0x00, 0x00, 
    0x00, 0x40, 0x40, 0x41, 0x41, 0x41, 0x41, 0x7F, 0x41, 0x41, 0x41, 0x41, 0x40, 0x40, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x08, 0x04, 0x02, 0x01, 0x3F, 0x01, 0x02, 0x04, 0x48, 0x80, 0x7F, 0x00, 0x00, 
};
static const lcd_display_resource_t gs_expo_st_idle_res =
{
    LCD_EXPO_ST_IDLE_IMG_W, LCD_EXPO_ST_IDLE_IMG_H,
    gs_expo_st_idle_display_data,
};
#define LCD_EXPO_ST_EXPOSING_IMG_W 64
#define LCD_EXPO_ST_EXPOSING_IMG_H 16
static const unsigned char gs_expo_st_exposing_display_data[/* LCD_EXPO_ST_EXPOSING_IMG_W * ceil(LCD_EXPO_ST_EXPOSING_IMG_H / 8) */] = 
{
    0x00, 0x02, 0x02, 0xC2, 0x02, 0x02, 0x02, 0xFE, 0x82, 0x82, 0x82, 0x82, 0x82, 0x02, 0x00, 0x00, 0x08, 0x08, 0x88, 0xC8, 0x38, 0x0C, 0x0B, 0x08, 0x08, 0xE8, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00, 0xFC, 0x84, 0x84, 0xFC, 0x00, 0x40, 0x5F, 0xF5, 0x55, 0x55, 0x55, 0xF5, 0x5F, 0x40, 0x00, 0x00, 0x40, 0x40, 0x42, 0x44, 0x58, 0xC0, 0x40, 0x7F, 0x40, 0xC0, 0x50, 0x48, 0x46, 0x40, 0x40, 0x00, 
    0x40, 0x40, 0x40, 0x7F, 0x40, 0x40, 0x40, 0x7F, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x00, 0x02, 0x01, 0x00, 0xFF, 0x40, 0x41, 0x41, 0x41, 0x41, 0x7F, 0x41, 0x41, 0x41, 0x41, 0x40, 0x00, 0x3F, 0x10, 0x10, 0x3F, 0x01, 0x09, 0x45, 0x2B, 0x91, 0xFD, 0x11, 0x2B, 0x45, 0x09, 0x01, 0x00, 0x80, 0x80, 0x40, 0x20, 0x18, 0x07, 0x00, 0x00, 0x00, 0x3F, 0x40, 0x40, 0x40, 0x40, 0x78, 0x00, 
};
static const lcd_display_resource_t gs_expo_st_exposing_res = 
{
    LCD_EXPO_ST_EXPOSING_IMG_W, LCD_EXPO_ST_EXPOSING_IMG_H,
    gs_expo_st_exposing_display_data,
};
#define LCD_EXPO_ST_HV_DISCONN_IMG_W 48
#define LCD_EXPO_ST_HV_DISCONN_IMG_H 16
static const unsigned char gs_expo_st_hv_disconn_display_data[/* LCD_EXPO_ST_HV_DISCONN_IMG_W * ceil(LCD_EXPO_ST_HV_DISCONN_IMG_H / 8) */] = 
{
    0x80, 0x80, 0x88, 0x88, 0x88, 0x88, 0x88, 0xFF, 0x88, 0x88, 0x88, 0x88, 0x88, 0x80, 0x80, 0x00, 0x40, 0x40, 0x42, 0xCC, 0x00, 0x04, 0x44, 0x64, 0x5C, 0x47, 0xF4, 0x44, 0x44, 0x44, 0x04, 0x00, 0x10, 0x10, 0x10, 0xFF, 0x10, 0x50, 0x44, 0x54, 0x65, 0xC6, 0x44, 0x64, 0x54, 0x44, 0x40, 0x00, 
    0x20, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01, 0xFF, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x20, 0x00, 0x00, 0x40, 0x20, 0x1F, 0x20, 0x44, 0x44, 0x44, 0x44, 0x44, 0x7F, 0x44, 0x44, 0x44, 0x44, 0x00, 0x04, 0x44, 0x82, 0x7F, 0x01, 0x82, 0x82, 0x4A, 0x56, 0x23, 0x22, 0x52, 0x4E, 0x82, 0x02, 0x00, 
};
static const lcd_display_resource_t gs_expo_st_hv_disconn_res = 
{
    LCD_EXPO_ST_HV_DISCONN_IMG_W, LCD_EXPO_ST_HV_DISCONN_IMG_H,
    gs_expo_st_hv_disconn_display_data,
};
#define LCD_EXPO_ST_POS_X (LCD_STATIC_DEV_ST_POS_X + LCD_STATIC_DEV_ST_POS_W)
#define LCD_EXPO_ST_POS_Y (LCD_STATIC_DEV_ST_POS_Y)
#define LCD_EXPO_ST_POS_W 64 /*maximum width*/
#define LCD_EXPO_ST_POS_H 16

#define LCD_DISTANCE_POS_X (LCD_STATIC_DIST_POS_X + LCD_STATIC_DIST_POS_W)
#define LCD_DISTANCE_POS_Y (LCD_STATIC_DIST_POS_Y)
#define LCD_DISTANCE_POS_W 56 /*maximum width*/
#define LCD_DISTANCE_POS_H 16
#define LCD_DISTANCE_MAX_INT_CHAR_NUM 3 /*e.g. "100*/
#define LCD_DISTANCE_MAX_FRAC_CHAR_NUM 1 /*e.g. "100.1"*/
#define LCD_DISTANCE_UNIT_CHAR_NUM 2 /*"100.1cm"*/

#endif
