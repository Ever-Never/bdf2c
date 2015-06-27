/**
 * @file    fontdata.h
 * @brief   base font data
 * @author  Yunhui Fu (yhfudev@gmail.com)
 * @version 1.0
 * @date    2015-06-19
 * @copyright GPL/BSD
 */
#ifndef FONT_DRAW_H
#define FONT_DRAW_H

#include "ppmhdr.h"
#include "fontdata.h"

int ppm_cavas_fontdraw (ppm_cavas_t * pppm, size_t x, size_t y, uint8_t color_front[4], uint8_t color_background[4], char c);
int ppm_cavas_drawstring (ppm_cavas_t * pppm, size_t x, size_t y, uint8_t color_front[4], uint8_t color_background[4], char * msg);

#endif