#ifndef _LCD_DISPLAY_RESOURCE_H_
#define _LCD_DISPLAY_RESOURCE_H_

/*DO NOT include this file in multi-files since it includes big var.*/

#define LCD_BAT_POS_X 220
#define LCD_BAT_POS_Y 0
#define LCD_BAT_POS_W 36
#define LCD_BAT_POS_H 20
#define LCD_BAT_IMG_W 36
#define LCD_BAT_IMG_H 20
static unsigned char gs_lcd_bat_res[/* LCD_BAT_IMG_W * ceil(LCD_BAT_IMG_H / 8) */] = {};

#define LCD_WAN_BEAR_POS_X 220
#define LCD_WAN_BEAR_POS_Y 0
#define LCD_WAN_BEAR_POS_W 36
#define LCD_WAN_BEAR_POS_H 20
#define LCD_WAN_BEAR_IMG_W 36
#define LCD_WAN_BEAR_IMG_H 20
static unsigned char gs_wan_bear_res[/* LCD_WAN_BEAR_IMG_W * ceil(LCD_WAN_BEAR_IMG_W / 8) */] = {};

#define LCD_CELL_SRV_ST_POS_X 220
#define LCD_CELL_SRV_ST_POS_Y 0
#define LCD_CELL_SRV_ST_POS_W 36
#define LCD_CELL_SRV_ST_POS_H 20
#define LCD_CELL_SRV_ST_IMG_W 36
#define LCD_CELL_SRV_ST_IMG_H 20
static unsigned char gs_cell_srv_st_res[/* LCD_CELL_SRV_ST_IMG_W * ceil(LCD_CELL_SRV_ST_IMG_W / 8) */] = {};

#define LCD_WIFI_WAN_ST_POS_X 220
#define LCD_WIFI_WAN_ST_POS_Y 0
#define LCD_WIFI_WAN_ST_POS_W 36
#define LCD_WIFI_WAN_ST_POS_H 20
#define LCD_WIFI_WAN_ST_IMG_W 36
#define LCD_WIFI_WAN_ST_IMG_H 20
static unsigned char gs_wifi_wan_st_res[/* LCD_WIFI_WAN_ST_IMG_W * ceil(LCD_WIFI_WAN_ST_IMG_W / 8) */] = {};

#define LCD_SIM_CARD_ST_POS_X 220
#define LCD_SIM_CARD_ST_POS_Y 0
#define LCD_SIM_CARD_ST_POS_W 36
#define LCD_SIM_CARD_ST_POS_H 20
#define LCD_SIM_CARD_ST_IMG_W 36
#define LCD_SIM_CARD_ST_IMG_H 20
static unsigned char gs_sim_card_st_res[/* LCD_SIM_CARD_ST_IMG_W * ceil(LCD_SIM_CARD_ST_IMG_W / 8) */] = {};

#define LCD_DSP_CONN_POS_X 220
#define LCD_DSP_CONN_POS_Y 0
#define LCD_DSP_CONN_POS_W 36
#define LCD_DSP_CONN_POS_H 20
#define LCD_DSP_CONN_IMG_W 36
#define LCD_DSP_CONN_IMG_H 20
static unsigned char gs_dsp_conn_res[/* LCD_DSP_CONN_IMG_W * ceil(LCD_DSP_CONN_IMG_W / 8) */] = {};

#define LCD_CUBE_VOLT_POS_X 220
#define LCD_CUBE_VOLT_POS_Y 0
#define LCD_CUBE_VOLT_POS_W 36
#define LCD_CUBE_VOLT_POS_H 20
#define LCD_CUBE_VOLT_IMG_W 36
#define LCD_CUBE_VOLT_IMG_H 20
static unsigned char gs_cube_volt_res[/* LCD_CUBE_VOLT_IMG_W * ceil(LCD_CUBE_VOLT_IMG_W / 8) */] = {};

#define LCD_CUBE_AMTS_POS_X 220
#define LCD_CUBE_AMTS_POS_Y 0
#define LCD_CUBE_AMTS_POS_W 36
#define LCD_CUBE_AMTS_POS_H 20
#define LCD_CUBE_AMTS_IMG_W 36
#define LCD_CUBE_AMTS_IMG_H 20
static unsigned char gs_cube_amts_res[/* LCD_CUBE_AMTS_IMG_W * ceil(LCD_CUBE_AMTS_IMG_W / 8) */] = {};

#define LCD_EXPO_ST_POS_X 220
#define LCD_EXPO_ST_POS_Y 0
#define LCD_EXPO_ST_POS_W 36
#define LCD_EXPO_ST_POS_H 20
#define LCD_EXPO_ST_IMG_W 36
#define LCD_EXPO_ST_IMG_H 20
static unsigned char gs_expo_st_res[/* LCD_EXPO_ST_IMG_W * ceil(LCD_EXPO_ST_IMG_W / 8) */] = {};

#define LCD_DISTANCE_POS_X 220
#define LCD_DISTANCE_POS_Y 0
#define LCD_DISTANCE_POS_W 36
#define LCD_DISTANCE_POS_H 20
#define LCD_DISTANCE_IMG_W 36
#define LCD_DISTANCE_IMG_H 20
static unsigned char gs_distance_res[/* LCD_DISTANCE_IMG_W * ceil(LCD_DISTANCE_IMG_W / 8) */] = {};

/*----------------------------------------*/
/*The following are static image info.*/
#define LCD_STATIC_DEV_ST_POS_X 220
#define LCD_STATIC_DEV_ST_POS_Y 0
#define LCD_STATIC_DEV_ST_POS_W 36
#define LCD_STATIC_DEV_ST_POS_H 20
#define LCD_STATIC_DEV_ST_IMG_W 36
#define LCD_STATIC_DEV_ST_IMG_H 20
static unsigned char gs_static_dev_st_res[/* LCD_STATIC_DEV_ST_IMG_W * ceil(LCD_STATIC_DEV_ST_IMG_W / 8) */] = {};

#define LCD_STATIC_VOLT_POS_X 220
#define LCD_STATIC_VOLT_POS_Y 0
#define LCD_STATIC_VOLT_POS_W 36
#define LCD_STATIC_VOLT_POS_H 20
#define LCD_STATIC_VOLT_IMG_W 36
#define LCD_STATIC_VOLT_IMG_H 20
static unsigned char gs_static_volt_res[/* LCD_STATIC_VOLT_IMG_W * ceil(LCD_STATIC_VOLT_IMG_W / 8) */] = {};

#define LCD_STATIC_AMT_S_POS_X 220
#define LCD_STATIC_AMT_S_POS_Y 0
#define LCD_STATIC_AMT_S_POS_W 36
#define LCD_STATIC_AMT_S_POS_H 20
#define LCD_STATIC_AMT_S_IMG_W 36
#define LCD_STATIC_AMT_S_IMG_H 20
static unsigned char gs_static_amt_s_res[/* LCD_STATIC_AMT_S_IMG_W * ceil(LCD_STATIC_AMT_S_IMG_W / 8) */] = {};

#define LCD_STATIC_AMT_POS_X 220
#define LCD_STATIC_AMT_POS_Y 0
#define LCD_STATIC_AMT_POS_W 36
#define LCD_STATIC_AMT_POS_H 20
#define LCD_STATIC_AMT_IMG_W 36
#define LCD_STATIC_AMT_IMG_H 20
static unsigned char gs_static_amt_res[/* LCD_STATIC_AMT_IMG_W * ceil(LCD_STATIC_AMT_IMG_W / 8) */] = {};

#define LCD_STATIC_DURA_POS_X 220
#define LCD_STATIC_DURA_POS_Y 0
#define LCD_STATIC_DURA_POS_W 36
#define LCD_STATIC_DURA_POS_H 20
#define LCD_STATIC_DURA_IMG_W 36
#define LCD_STATIC_DURA_IMG_H 20
static unsigned char gs_static_dura_res[/* LCD_STATIC_DURA_IMG_W * ceil(LCD_STATIC_DURA_IMG_W / 8) */] = {};

#define LCD_STATIC_DIST_POS_X 220
#define LCD_STATIC_DIST_POS_Y 0
#define LCD_STATIC_DIST_POS_W 36
#define LCD_STATIC_DIST_POS_H 20
#define LCD_STATIC_DIST_IMG_W 36
#define LCD_STATIC_DIST_IMG_H 20
static unsigned char gs_static_dist_res[/* LCD_STATIC_DIST_IMG_W * ceil(LCD_STATIC_DIST_IMG_W / 8) */] = {};

#define LCD_STATIC_LOGO_POS_X 220
#define LCD_STATIC_LOGO_POS_Y 0
#define LCD_STATIC_LOGO_POS_W 36
#define LCD_STATIC_LOGO_POS_H 20
#define LCD_STATIC_LOGO_IMG_W 36
#define LCD_STATIC_LOGO_IMG_H 20
static unsigned char gs_static_logo_res[/* LCD_STATIC_LOGO_IMG_W * ceil(LCD_STATIC_LOGO_IMG_W / 8) */] = {};

#endif
