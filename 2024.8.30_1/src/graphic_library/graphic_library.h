/*
 * graphic_library.h
 *
 * created: 2024/3/4
 *  author: 
 */

#ifndef _GRAPHIC_LIBRARY_H
#define _GRAPHIC_LIBRARY_H

#ifdef __cplusplus
extern "C" {
#endif

void drawline(lv_obj_t * canvas,int x1, int y1, int x2, int y2, lv_color_t coloridx);
void fillrect(lv_obj_t * canvas,int x1, int y1, int x2, int y2, lv_color_t coloridx);
void DrawTriangle(lv_obj_t * canvas,int x1, int y1, int Width, int Hight, lv_color_t coloridx);
void lozenge(lv_obj_t * canvas,unsigned short usCenterX, unsigned short usStartY, unsigned short length_of_side,lv_color_t coloridx);
void fb_yuan(lv_obj_t * canvas,int x, int y, int z, lv_color_t coloridx);
void drawPentagon(lv_obj_t * canvas,int centerX, int centerY, int radius,lv_color_t coloridx);
#ifdef __cplusplus
}
#endif

#endif // _GRAPHIC_LIBRARY_H

