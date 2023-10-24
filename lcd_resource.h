#ifndef _LCD_DISPLAY_RESOURCE_H_
#define _LCD_DISPLAY_RESOURCE_H_

/*DO NOT include this file in multi-files since it includes big var.*/

#define LCD_SCRN_WIDTH 256
#define LCD_SCRN_HEIGHT 64

typedef struct
{
    int img_w, img_h;
    const unsigned char* img;
}lcd_display_resource_t;

#define LCD_DISPLAY_ORI_X 2
#define LCD_DISPLAY_ORI_Y 2
#define LCD_DISPLAY_H_GAP 2
#define LCD_DISPLAY_V_GAP 3

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

#define LCD_STATIC_AMT_IMG_W (48 + 8) /* title + colon */
#define LCD_STATIC_AMT_IMG_H 16
static const unsigned char gs_static_amt_display_data[/* LCD_STATIC_AMT_IMG_W * ceil(LCD_STATIC_AMT_IMG_H / 8) */] = 
{
    0x88, 0x64, 0x23, 0xA2, 0xA6, 0xAA, 0xA2, 0xBA, 0xA4, 0xA3, 0xA2, 0x26, 0x2A, 0xA2, 0x62, 0x00, 0x00, 0x00, 0xF8, 0x88, 0x88, 0x88, 0x88, 0xFF, 0x88, 0x88, 0x88, 0x88, 0xF8, 0x00, 0x00, 0x00, 0x10, 0x60, 0x02, 0x8C, 0x00, 0x44, 0x64, 0x54, 0x4D, 0x46, 0x44, 0x54, 0x64, 0xC4, 0x04, 0x00, /* colon */ 0x00, 0x00, 0x00, 0xC0, 0xC0, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0xFF, 0x4A, 0x4A, 0x4A, 0x4A, 0x4A, 0x4A, 0x4B, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x08, 0x08, 0x08, 0x08, 0x7F, 0x88, 0x88, 0x88, 0x88, 0x9F, 0x80, 0xF0, 0x00, 0x04, 0x04, 0x7E, 0x01, 0x80, 0x40, 0x3E, 0x00, 0x00, 0xFE, 0x00, 0x00, 0x7E, 0x80, 0xE0, 0x00, /* colon */ 0x00, 0x00, 0x00, 0x30, 0x30, 0x00, 0x00, 0x00, 
};
static const lcd_display_resource_t gs_static_amt_res = 
{
    LCD_STATIC_AMT_IMG_W, LCD_STATIC_AMT_IMG_H,
    gs_static_amt_display_data,
};
#define LCD_STATIC_AMT_POS_X LCD_DISPLAY_ORI_X
#define LCD_STATIC_AMT_POS_Y (LCD_DISPLAY_ORI_Y + LCD_STATIC_VOLT_POS_H + LCD_DISPLAY_V_GAP)
#define LCD_STATIC_AMT_POS_W LCD_STATIC_AMT_IMG_W
#define LCD_STATIC_AMT_POS_H LCD_STATIC_AMT_IMG_H

#define LCD_STATIC_DURA_IMG_W (32 + 8) /* title + colon */
#define LCD_STATIC_DURA_IMG_H 16
static const unsigned char gs_static_dura_display_data[/* LCD_STATIC_DURA_IMG_W * ceil(LCD_STATIC_DURA_IMG_H / 8) */] = 
{
    0x00, 0xFC, 0x84, 0x84, 0x84, 0xFC, 0x00, 0x10, 0x10, 0x10, 0x10, 0x10, 0xFF, 0x10, 0x10, 0x00, 0x00, 0xF8, 0x01, 0x06, 0x00, 0xF0, 0x12, 0x12, 0x12, 0xF2, 0x02, 0x02, 0x02, 0xFE, 0x00, 0x00,  /* colon */ 0x00, 0x00, 0x00, 0xC0, 0xC0, 0x00, 0x00, 0x00, 
    0x00, 0x3F, 0x10, 0x10, 0x10, 0x3F, 0x00, 0x00, 0x01, 0x06, 0x40, 0x80, 0x7F, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x1F, 0x11, 0x11, 0x11, 0x1F, 0x00, 0x40, 0x80, 0x7F, 0x00, 0x00,  /* colon */ 0x00, 0x00, 0x00, 0x30, 0x30, 0x00, 0x00, 0x00, 
};
static const lcd_display_resource_t gs_static_dura_res = 
{
    LCD_STATIC_DURA_IMG_W, LCD_STATIC_DURA_IMG_H,
    gs_static_dura_display_data,
};
/* 8*8 is the width of maximum amt len, e.gl 10.123mA; 20 is margin.*/
#define LCD_STATIC_DURA_POS_X (LCD_STATIC_AMT_POS_X + LCD_STATIC_AMT_POS_W + 8*8 + 20)
#define LCD_STATIC_DURA_POS_Y LCD_STATIC_AMT_POS_Y
#define LCD_STATIC_DURA_POS_W LCD_STATIC_DURA_IMG_W
#define LCD_STATIC_DURA_POS_H LCD_STATIC_DURA_IMG_H

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
#define LCD_STATIC_DEV_ST_POS_Y (LCD_DISPLAY_ORI_Y +  LCD_STATIC_VOLT_POS_H + LCD_STATIC_AMT_POS_H + \
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
static const unsigned char gs_digits_font[][16 /* LCD_DIGIT_FONT_W * ceil(LCD_DIGIT_FONT_H / 8) */] = 
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

#define LCD_SMALL_DIGIT_NUM 10 //0~9
#define LCD_SMALL_DIGIT_IMG_W 3
#define LCD_SMALL_DIGIT_IMG_H 5
#define LCD_SMALL_DIGIT_IMG_SIZE 3 /*LCD_SMALL_DIGIT_IMG_W * ceil(LCD_SMALL_DIGIT_IMG_H / 8)*/
static const unsigned char gs_lcd_small_digit_res[LCD_SMALL_DIGIT_NUM][LCD_SMALL_DIGIT_IMG_SIZE] =
{
    {//0
        0x1F, 0x11, 0x1F, 
    },
    {//1
        0x02, 0x1F, 0x00, 
    },
    {//2
        0x1D, 0x15, 0x17, 
    },
    {//3
        0x15, 0x15, 0x1F, 
    },
    {//4
        0x07, 0x04, 0x1F, 
    },
    {//5
        0x17, 0x15, 0x1D, 
    },
    {//6
        0x1F, 0x15, 0x1D, 
    },
    {//7
        0x01, 0x01, 0x1F, 
    },
    {//8
        0x1F, 0x15, 0x1F, 
    },
    {//9
        0x17, 0x15, 0x1F, 
    },
};

#define LCD_SMALL_PLUS_3X3_IMG_W 3
#define LCD_SMALL_PLUS_3X3_IMG_H 3
static unsigned char gs_lcd_small_plus_3x3_res[] = 
{
    0x02, 0x07, 0x02, 
};

#define LCD_SMALL_DOT_3X5_IMG_W 3
#define LCD_SMALL_DOT_3X5_IMG_H 5
static unsigned char gs_lcd_small_dot_3x5_res[] = 
{
    0x00, 0x10, 0x00, 
};
#define LCD_SMALL_MINUS_3X5_IMG_W 3
#define LCD_SMALL_MINUS_3X5_IMG_H 5
static unsigned char gs_lcd_small_minus_3x5_res[] = 
{
    0x04, 0x04, 0x04, 
};

#define LCD_SMALL_V_3X5_IMG_W 3
#define LCD_SMALL_V_3X5_IMG_H 5
static unsigned char gs_lcd_small_v_3x5_res[] = 
{
    0x06, 0x18, 0x06, 
};

#define LCD_ALPHA_LOW_FONT_W 8 
#define LCD_ALPHA_LOW_FONT_H 16
static const unsigned char gs_alpha_low_chars_font[][16 /* LCD_ALPHA_LOW_FONT_W * ceil(LCD_ALPHA_LOW_FONT_H / 8) */] =
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

#define LCD_ALPHA_HIGH_FONT_W 8
#define LCD_ALPHA_HIGH_FONT_H  16
static const unsigned char gs_alpha_high_chars_font[][16 /* LCD_ALPHA_HIGH_FONT_W * ceil(LCD_ALPHA_HIGH_FONT_H / 8) */] =
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

#define PUNC_SPACE_FONT_W 8
#define PUNC_SPACE_FONT_H 16
static const unsigned char gs_punc_space_font[ /* PUNC_SPACE_FONT_W * ceil(PUNC_SPACE_FONT_H / 8) */] = 
{/* . */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
};

#define PUNC_COLON_FONT_W 8
#define PUNC_COLON_FONT_H 16
static const unsigned char gs_punc_colon_font[ /* PUNC_COLON_FONT_W * ceil(PUNC_COLON_FONT_H / 8) */] = 
{/* : */
    0x00, 0x00, 0x00, 0xC0, 0xC0, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x30, 0x30, 0x00, 0x00, 0x00, 
};

#define PUNC_DOT_FONT_W 8
#define PUNC_DOT_FONT_H 16
static const unsigned char gs_punc_dot_font[ /* PUNC_DOT_FONT_W * ceil(PUNC_DOT_FONT_H / 8) */] = 
{/* . */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 
};

#define LCD_DISPLAY_DEFAULT_CHAR_W 8
#define LCD_DISPLAY_DEFAULT_CHAR_H 16
static const unsigned char gs_lcd_display_def_char[ /* LCD_DISPLAY_DEFAULT_CHAR_W * ceil(LCD_DISPLAY_DEFAULT_CHAR_H / 8) */] = 
{/* hollow square */
    0xFF, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xFF, 
    0xFF, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xFF, 
};

#define LCD_SMALL_ALPHA_3X5_FONT_W 3
#define LCD_SMALL_ALPHA_3X5_FONT_H 5
#define LCD_SMALL_ALPHA_3X5_FONT_SIZE 3 /* LCD_SMALL_ALPHA_3X5_FONT_W * ceil(LCD_SMALL_ALPHA_3X5_FONT_H / 8) */
static const unsigned char gs_lcd_small_alpha_3x5_font_res[][LCD_SMALL_ALPHA_3X5_FONT_SIZE] =
{
    {//'A'
        0x1F, 0x05, 0x1F, 
    },
    {//'b'
        0x1F, 0x14, 0x1C, 
    },
    {//'C'
        0x1F, 0x11, 0x11, 
    },
    {//'d'
        0x1C, 0x14, 0x1F, 
    },
    {//'E'
        0x1F, 0x15, 0x15, 
    },
    {//'F'
        0x1F, 0x05, 0x05, 
    },
    {//'G'
        0x1F, 0x11, 0x1D, 
    },
    {//'H'
        0x1F, 0x04, 0x1F, 
    },
    {//'I'
        0x11, 0x1F, 0x11, 
    },
    {//'J'
        0x10, 0x1F, 0x00, 
    },
    {//'k'
        0x1E, 0x08, 0x14, 
    },
    {//'L'
        0x1F, 0x10, 0x10, 
    },
    {//'M'
        0x1F, 0x0E, 0x1F, 
    },
    {//'n'
        0x1C, 0x04, 0x1C, 
    },
    {//'o'
        0x1C, 0x14, 0x1C, 
    },
    {//'p'
        0x1F, 0x05, 0x07, 
    },
    {//'q'
        0x07, 0x05, 0x1F, 
    },
    {//'r'
        0x1F, 0x04, 0x02, 
    },
    {//'S'
        0x15, 0x15, 0x15, 
    },
    {//'T'
        0x01, 0x1F, 0x01, 
    },
    {//'U'
        0x1F, 0x10, 0x1F, 
    },
    {//'v'
        0x06, 0x18, 0x06, 
    },
    {//'w'
        0x1F, 0x1C, 0x1F, 
    },
    {//'x'
        0x1A, 0x04, 0x1A, 
    },
    {//'Y'
        0x03, 0x1C, 0x03, 
    },
    {//'z'
        0x19, 0x15, 0x13, 
    },
};

/*--------------------The following are "dynamaic" info resurce.*/

/* The following "box" is the area contains all "main" icons, including sim, wifi, cell signal, hotspot, battery.
 * It is right aligned to the srn, and the main icons are bottom aligned to it.
 * */
#define LCD_ICONS_DISP_MAIN_BOX_POS_W 85
#define LCD_ICONS_DISP_MAIN_BOX_POS_H 12
#define LCD_ICONS_DISP_MAIN_BOX_POS_X (LCD_SCRN_WIDTH - LCD_ICONS_DISP_MAIN_BOX_POS_W)
#define LCD_ICONS_DISP_MAIN_BOX_POS_Y 1

#define LCD_SIM_CARD_ST_IMG_W 10
#define LCD_SIM_CARD_ST_IMG_H 11
static const unsigned char gs_sim_card_err_res[/* LCD_SIM_CARD_ST_IMG_W * ceil(LCD_SIM_CARD_ST_IMG_W / 8) */] =
{

    0xF0, 0x0C, 0x02, 0x01, 0x01, 0xBD, 0x01, 0x01, 0x01, 0xFF, 
    0x07, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x07, 
};
#define LCD_SIM_CARD_ST_POS_X LCD_ICONS_DISP_MAIN_BOX_POS_X
#define LCD_SIM_CARD_ST_POS_Y (LCD_ICONS_DISP_MAIN_BOX_POS_Y + (LCD_ICONS_DISP_MAIN_BOX_POS_H - LCD_SIM_CARD_ST_IMG_H))
#define LCD_SIM_CARD_ST_POS_W LCD_SIM_CARD_ST_IMG_W
#define LCD_SIM_CARD_ST_POS_H LCD_SIM_CARD_ST_IMG_H

#define MAX_WIFI_WAN_LEVELS 5 //0~4
#define LCD_WIFI_WAN_ST_IMG_W 13 
#define LCD_WIFI_WAN_ST_IMG_H 9
#define LCD_WIFI_WAN_ST_IMG_SIZE 26 /* LCD_WIFI_WAN_ST_IMG_W * ceil(LCD_WIFI_WAN_ST_IMG_W / 8) */
static const unsigned char gs_wifi_wan_st_res[MAX_WIFI_WAN_LEVELS][LCD_WIFI_WAN_ST_IMG_SIZE] = 
{
    {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    },

    {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    },

    {
        0x00, 0x00, 0x00, 0x00, 0x80, 0x40, 0x40, 0x40, 0x80, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    },

    {
        0x00, 0x00, 0x40, 0x20, 0x90, 0x48, 0x48, 0x48, 0x90, 0x20, 0x40, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    },

    {
        0x30, 0x18, 0x4C, 0x26, 0x92, 0x49, 0x49, 0x49, 0x92, 0x26, 0x4C, 0x18, 0x30, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    },
};
#define LCD_WIFI_WAN_ST_POS_X (LCD_SIM_CARD_ST_POS_X + LCD_SIM_CARD_ST_POS_W + LCD_DISPLAY_H_GAP)
#define LCD_WIFI_WAN_ST_POS_Y (LCD_ICONS_DISP_MAIN_BOX_POS_Y + (LCD_ICONS_DISP_MAIN_BOX_POS_H - LCD_WIFI_WAN_ST_IMG_H))
#define LCD_WIFI_WAN_ST_POS_W LCD_WIFI_WAN_ST_IMG_W
#define LCD_WIFI_WAN_ST_POS_H LCD_WIFI_WAN_ST_IMG_H

#define MAX_CELL_WAN_LEVELS 6 //0~5
#define LCD_CELL_SRV_ST_IMG_W 17
#define LCD_CELL_SRV_ST_IMG_H 11
#define LCD_CELL_SRV_ST_IMG_SIZE 34 /* LCD_CELL_SRV_ST_IMG_W * ceil(LCD_CELL_SRV_ST_IMG_W / 8) */
static const unsigned char gs_cell_srv_st_res[MAX_CELL_WAN_LEVELS][LCD_CELL_SRV_ST_IMG_SIZE] = 
{
    {
        0x01, 0x03, 0x05, 0x09, 0x11, 0x21, 0xFF, 0x21, 0x11, 0x09, 0x05, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    },
    {
        0x01, 0x03, 0x05, 0x09, 0x11, 0x21, 0xFF, 0x21, 0x11, 0x09, 0x05, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    },
    {
        0x01, 0x03, 0x05, 0x09, 0x11, 0x21, 0xFF, 0x21, 0x11, 0x09, 0x05, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x04, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    },
    {
        0x01, 0x03, 0x05, 0x09, 0x11, 0x21, 0xFF, 0x21, 0x11, 0x09, 0x05, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x04, 0x00, 0x06, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 
    },
    {
        0x01, 0x03, 0x05, 0x09, 0x11, 0x21, 0xFF, 0x21, 0x11, 0x09, 0x05, 0x03, 0x01, 0x00, 0x80, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x04, 0x00, 0x06, 0x00, 0x07, 0x00, 0x07, 0x00, 0x00, 
    },
    {
        0x01, 0x03, 0x05, 0x09, 0x11, 0x21, 0xFF, 0x21, 0x11, 0x09, 0x05, 0x03, 0x01, 0x00, 0x80, 0x00, 0xE0, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x04, 0x00, 0x06, 0x00, 0x07, 0x00, 0x07, 0x00, 0x07, 
    },
};
#define LCD_CELL_SRV_ST_POS_X (LCD_WIFI_WAN_ST_POS_X + LCD_WIFI_WAN_ST_POS_W + LCD_DISPLAY_H_GAP)
#define LCD_CELL_SRV_ST_POS_Y (LCD_ICONS_DISP_MAIN_BOX_POS_Y + (LCD_ICONS_DISP_MAIN_BOX_POS_H - LCD_CELL_SRV_ST_IMG_H))
#define LCD_CELL_SRV_ST_POS_W LCD_CELL_SRV_ST_IMG_W
#define LCD_CELL_SRV_ST_POS_H LCD_CELL_SRV_ST_IMG_H

#define LCD_CELL_MODE_NUM 4 //noserv, 3G, 4G, 5G
#define LCD_CELL_MODE_IMG_W 8
#define LCD_CELL_MODE_IMG_H 5
#define LCD_CELL_MODE_IMG_SIZE 8 /*LCD_CELL_MODE_IMG_W * ceil(LCD_CELL_MODE_IMG_H / 8) */
static const unsigned char gs_lcd_mode_res[LCD_CELL_MODE_NUM][LCD_CELL_MODE_IMG_SIZE] =
{
    {/* CELLULAR_MODE_NOSRV = 0*/
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    },
    {/* CELLULAR_MODE_3G */
        0x15, 0x15, 0x1F, 0x00, 0x1E, 0x13, 0x11, 0x1B, 
    },
    {/* CELLULAR_MODE_4G */
        0x07, 0x04, 0x1F, 0x00, 0x1E, 0x13, 0x11, 0x1B, 
    },
    {/* CELLULAR_MODE_5G */
        0x17, 0x15, 0x1D, 0x00, 0x1E, 0x13, 0x11, 0x1B, 
    },
};
#define LCD_CELL_MODE_POS_X (LCD_CELL_SRV_ST_POS_X + 14)
#define LCD_CELL_MODE_POS_Y (LCD_CELL_SRV_ST_POS_Y - 1)
#define LCD_CELL_MODE_POS_W LCD_CELL_MODE_IMG_W
#define LCD_CELL_MODE_POS_H LCD_CELL_MODE_IMG_H

#define LCD_WAN_CONN_IMG_W 5
#define LCD_WAN_CONN_IMG_H 4
static const unsigned char gs_lcd_wan_conn_res[ /*LCD_WAN_CONN_IMG_W * ceil(LCD_WAN_CONN_IMG_H / 8) */ ] =
{
    0x04, 0x0F, 0x00, 0x0F, 0x02, 
};
#define LCD_CELL_WAN_CONN_POS_X LCD_CELL_SRV_ST_POS_X
#define LCD_CELL_WAN_CONN_POS_Y (LCD_CELL_SRV_ST_POS_Y + (LCD_CELL_SRV_ST_IMG_H - LCD_WAN_CONN_IMG_H))
#define LCD_CELL_WAN_CONN_POS_W LCD_WAN_CONN_IMG_W
#define LCD_CELL_WAN_CONN_POS_H LCD_WAN_CONN_IMG_H

#define LCD_HOTSPOT_IMG_W 13
#define LCD_HOTSPOT_IMG_H 11 
static const unsigned char gs_lcd_hotspot_res[/* LCD_HOTSPOT_IMG_W * ceil(LCD_HOTSPOT_IMG_H / 8) */] = 
{
    0xF0, 0x08, 0x04, 0xF2, 0x09, 0x75, 0x55, 0x75, 0x09, 0xF2, 0x04, 0x08, 0xF0, 
    0x00, 0x01, 0x02, 0x04, 0x01, 0x00, 0x00, 0x00, 0x01, 0x04, 0x02, 0x01, 0x00, 
};
#define LCD_HOTSPOT_POS_X (LCD_CELL_MODE_POS_X + LCD_CELL_MODE_POS_W + LCD_DISPLAY_H_GAP)
#define LCD_HOTSPOT_POS_Y (LCD_ICONS_DISP_MAIN_BOX_POS_Y + (LCD_ICONS_DISP_MAIN_BOX_POS_H - LCD_HOTSPOT_IMG_H))
/*be careful, the area includes: hotspot icon, small digit, small plus 3x3*/
#define LCD_HOTSPOT_POS_W LCD_HOTSPOT_IMG_W
#define LCD_HOTSPOT_POS_H (LCD_HOTSPOT_IMG_H + 3)

#define LCD_HOT_SPOT_NUM_POS_X (LCD_HOTSPOT_POS_X + 5)
#define LCD_HOT_SPOT_NUM_POS_Y (LCD_HOTSPOT_POS_Y + 9)
#define LCD_HOT_SPOT_NUM_POS_W LCD_SMALL_DIGIT_IMG_W
#define LCD_HOT_SPOT_NUM_POS_H LCD_SMALL_DIGIT_IMG_H

#define LCD_HOT_SPOT_NUM_PLUS_POS_X (LCD_HOT_SPOT_NUM_POS_X + LCD_HOT_SPOT_NUM_POS_W + 1)
#define LCD_HOT_SPOT_NUM_PLUS_POS_Y (LCD_HOT_SPOT_NUM_POS_Y + (LCD_SMALL_DIGIT_IMG_H - LCD_SMALL_PLUS_3X3_IMG_H))
#define LCD_HOT_SPOT_NUM_PLUS_POS_W LCD_SMALL_PLUS_3X3_IMG_W
#define LCD_HOT_SPOT_NUM_PLUS_POS_H LCD_SMALL_PLUS_3X3_IMG_H

#define BATTERY_LEVELS 5 //0~4
static const int gs_lcd_bat_lvls[BATTERY_LEVELS] =
{0, 5, 20, 50, 80};
#define LCD_BAT_IMG_W 18
#define LCD_BAT_IMG_H 10
#define LCD_BAT_IMG_SIZE 36 /* LCD_BAT_IMG_W * ceil(LCD_BAT_IMG_H / 8) */
typedef const unsigned char (*lcd_battery_img_type)[LCD_BAT_IMG_SIZE] ;
static const unsigned char gs_lcd_bat_res[BATTERY_LEVELS][LCD_BAT_IMG_SIZE] = 
{
    {
        0x78, 0x48, 0xCF, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xFF, 
        0x00, 0x00, 0x03, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x03, 
    },
    {
        0x78, 0x48, 0xCF, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xFF, 0x01, 0x01, 0xFF, 
        0x00, 0x00, 0x03, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x03, 0x02, 0x02, 0x03, 
    },
    {
        0x78, 0x48, 0xCF, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xFF, 0x01, 0x01, 0xFF, 0x01, 0x01, 0xFF, 
        0x00, 0x00, 0x03, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x03, 0x02, 0x02, 0x03, 0x02, 0x02, 0x03, 
    },
    {
        0x78, 0x48, 0xCF, 0x01, 0x01, 0x01, 0x01, 0x01, 0xFF, 0x01, 0x01, 0xFF, 0x01, 0x01, 0xFF, 0x01, 0x01, 0xFF, 
        0x00, 0x00, 0x03, 0x02, 0x02, 0x02, 0x02, 0x02, 0x03, 0x02, 0x02, 0x03, 0x02, 0x02, 0x03, 0x02, 0x02, 0x03, 
    },
    {
        0x78, 0x48, 0xCF, 0x01, 0x01, 0xFF, 0x01, 0x01, 0xFF, 0x01, 0x01, 0xFF, 0x01, 0x01, 0xFF, 0x01, 0x01, 0xFF, 
        0x00, 0x00, 0x03, 0x02, 0x02, 0x03, 0x02, 0x02, 0x03, 0x02, 0x02, 0x03, 0x02, 0x02, 0x03, 0x02, 0x02, 0x03, 
    },
};
#define LCD_BAT_POS_X (LCD_HOTSPOT_POS_X + LCD_HOTSPOT_POS_W + LCD_DISPLAY_H_GAP)
#define LCD_BAT_POS_Y (LCD_ICONS_DISP_MAIN_BOX_POS_Y + (LCD_ICONS_DISP_MAIN_BOX_POS_H - LCD_BAT_IMG_H))
#define LCD_BAT_POS_W LCD_BAT_IMG_W
#define LCD_BAT_POS_H LCD_BAT_IMG_H
static const unsigned char gs_lcd_bat_lightning_res[BATTERY_LEVELS][LCD_BAT_IMG_SIZE] = 
{
    {
        0x78, 0x48, 0xCF, 0x01, 0x01, 0x01, 0x09, 0x11, 0x21, 0x61, 0x3D, 0x09, 0x11, 0x21, 0x41, 0x01, 0x01, 0xFF, 
        0x00, 0x00, 0x03, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x03, 
    },
    {
        0x78, 0x48, 0xCF, 0x01, 0x01, 0x09, 0x11, 0x21, 0x61, 0x3D, 0x09, 0x11, 0x21, 0x41, 0xFF, 0x01, 0x01, 0xFF, 
        0x00, 0x00, 0x03, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x03, 0x02, 0x02, 0x03, 
    },
    {
        0x78, 0x48, 0xCF, 0x01, 0x01, 0x09, 0x11, 0x21, 0x61, 0x3D, 0x09, 0xEF, 0x21, 0x41, 0xFF, 0x01, 0x01, 0xFF, 
        0x00, 0x00, 0x03, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x03, 0x02, 0x02, 0x03, 0x02, 0x02, 0x03, 
    },
    {
        0x78, 0x48, 0xCF, 0x01, 0x01, 0x09, 0x11, 0x21, 0x9F, 0x3D, 0x09, 0xEF, 0x21, 0x41, 0xFF, 0x01, 0x01, 0xFF, 
        0x00, 0x00, 0x03, 0x02, 0x02, 0x02, 0x02, 0x02, 0x03, 0x02, 0x02, 0x03, 0x02, 0x02, 0x03, 0x02, 0x02, 0x03, 
    },
    {
        0x78, 0x48, 0xCF, 0x01, 0x01, 0xF7, 0x11, 0x21, 0x9F, 0x3D, 0x09, 0xEF, 0x21, 0x41, 0xFF, 0x01, 0x01, 0xFF, 
        0x00, 0x00, 0x03, 0x02, 0x02, 0x03, 0x02, 0x02, 0x03, 0x02, 0x02, 0x03, 0x02, 0x02, 0x03, 0x02, 0x02, 0x03, 
    },
};

#define LCD_CHARGER_IMG_W 9 
#define LCD_CHARGER_IMG_H 10
static const unsigned char gs_lcd_charger_res[/* LCD_CHARGER_IMG_W * ceil(LCD_CHARGER_IMG_H / 8) */] = 
{
    0x3C, 0x24, 0xE7, 0xA4, 0xA4, 0xA4, 0xE7, 0x24, 0x3C, 
    0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 
};
#define LCD_CHARGER_POS_X (LCD_ICONS_DISP_MAIN_BOX_POS_X + LCD_ICONS_DISP_MAIN_BOX_POS_W - LCD_CHARGER_IMG_W)
#define LCD_CHARGER_POS_Y (LCD_ICONS_DISP_MAIN_BOX_POS_Y + LCD_ICONS_DISP_MAIN_BOX_POS_H + 2)
#define LCD_CHARGER_POS_W LCD_CHARGER_IMG_W
#define LCD_CHARGER_POS_H LCD_CHARGER_IMG_H

#define LCD_CUBE_VOLT_POS_X (LCD_STATIC_VOLT_POS_X + LCD_STATIC_VOLT_POS_W)
#define LCD_CUBE_VOLT_POS_Y LCD_STATIC_VOLT_POS_Y 
#define LCD_CUBE_VOLT_POS_W (5*8) /*maximum width, e.g. 100kV */
#define LCD_CUBE_VOLT_POS_H 16
#define LCD_CUBE_VOLT_MAX_INT_CHAR_NUM 5//3 /*e.g. 100.  we give 5 char to elimate compile warning. */

#define LCD_CUBE_AMT_POS_X (LCD_STATIC_AMT_POS_X + LCD_STATIC_AMT_POS_W)
#define LCD_CUBE_AMT_POS_Y LCD_STATIC_AMT_POS_Y 
#define LCD_CUBE_AMT_POS_W (8*8) /*maximum width, e.g. 10.123mA */
#define LCD_CUBE_AMT_POS_H 16
#define LCD_CUBE_AMT_MAX_INT_CHAR_NUM 2 /*e.g. 10.123*/
#define LCD_CUBE_AMT_MAX_FRAC_CHAR_NUM 3 /*e.g. 10.123*/

#define LCD_DURA_POS_X (LCD_STATIC_DURA_POS_X + LCD_STATIC_DURA_POS_W)
#define LCD_DURA_POS_Y LCD_STATIC_DURA_POS_Y 
#define LCD_DURA_POS_W (7*8) /*maximum width, e.g. 10.123s */
#define LCD_DURA_POS_H 16
#define LCD_DURA_MAX_INT_CHAR_NUM 2 /*e.g. 10.123*/
#define LCD_DURA_MAX_FRAC_CHAR_NUM 3 /*e.g. 10.123*/

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

static const char* gs_LCD_DISPLAY_UNIT_STR_CM = "cm";
static const char* gs_LCD_DISPLAY_UNIT_STR_KV = "kV";
static const char* gs_LCD_DISPLAY_UNIT_STR_AMT = "mA";
static const char* gs_LCD_DISPLAY_UNIT_STR_SEC = "s";

/*
"v" dsp-sw_ver "-" dr_manger_v "." gpio_processor_v "." openwrt_ver
v123.456-00.11.22.0.333.00.00.00 
                  ^
                  0 means release, other value means dbg.
 */
#define LCD_VER_STR_MAX_CHAR_CNT 32
/*"v" and "." and "-" are of the same width with small digit*/
#define LCD_VER_STR_POS_W (LCD_VER_STR_MAX_CHAR_CNT * (LCD_SMALL_DIGIT_IMG_W + 1)) 
#define LCD_VER_STR_POS_H (LCD_SMALL_DIGIT_IMG_H)
#define LCD_VER_STR_POS_X (LCD_SCRN_WIDTH - LCD_VER_STR_POS_W - 1)
#define LCD_VER_STR_POS_Y (LCD_DISTANCE_POS_Y + LCD_DISTANCE_POS_H + LCD_DISPLAY_V_GAP -1)

#define LCD_VER_STR_RIGHT_ALIGN_POS_X (LCD_SCRN_WIDTH - 1 - 1)
#define LCD_VER_STR_RIGHT_ALIGN_POS_Y LCD_VER_STR_POS_Y

#define LCD_WIFI_MAC_TAIL6_POS_X LCD_DISPLAY_ORI_X
#define LCD_WIFI_MAC_TAIL6_POS_Y (LCD_DISTANCE_POS_Y + LCD_DISTANCE_POS_H + LCD_DISPLAY_V_GAP -1)


/*
 * If "number" is integer,  "precision" must be set to 1.
 * The "format_str" must contain ".*" to use the "precision". 
 *
 */
#define PRINT_NUMBER_WITH_UNIT_TO_SCRN(number, max_len, format_str, precision, unit_str, start_x, start_y, pos_w, pos_h) \
{\
    char number_str[max_len + 1];\
    int block_w = 0;\
\
    snprintf(number_str, sizeof(number_str), format_str, precision, number); \
    \
    block_w += print_one_line_to_scrn(number_str, sizeof(number_str), start_x, start_y);\
    block_w += print_one_line_to_scrn(unit_str, strlen(unit_str), start_x + block_w, start_y);\
    if(block_w < pos_w)\
    {\
        clear_screen_area(start_x + block_w, start_y, pos_w - block_w, pos_h);\
    }\
}

#endif
