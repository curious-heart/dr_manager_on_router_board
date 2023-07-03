#ifndef _LCD_DISPLAY_H
#define _LCD_DISPLAY_H

#include <stdbool.h>
#include <stdint.h>

bool open_lcd_dev(const char* dev_name, uint8_t i2c_addr);
bool close_lcd_dev();
void clear_screen();
void write_img_to_px_rect(const uint8_t* img_buf, int img_px_w, int img_px_h, 
                      int scrn_px_x, int scrn_px_y, int scrn_px_w, int scrn_px_h);
void write_img_to_px_pos(const uint8_t* img_buf, int img_px_w, int img_px_h, int scrn_px_x, int scrn_px_y);
bool write_img_file_to_px_pos(const char* img_file_name, int rect_x, int rect_y);
bool write_img_file_to_px_rect(const char* img_file_name,
                            int rect_x, int rect_y, int rect_w, int rect_h);

void all_scrn_px_on();
void all_scrn_px_off();
void exit_all_scrn_mode();

#endif
