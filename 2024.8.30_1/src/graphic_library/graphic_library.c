/*
 * graphic_library.c
 *
 * created: 2024/3/4
 *  author: 
 */
#include "lvgl_ui.h"
#include "graphic_library.h"
#include "lvgl-7.0.1/lvgl.h"
/**
 * 在画布上画线
 * @param canvas 画布对象
 * @param x1 起点横坐标
 * @param y1 起点纵坐标
 * @param x2 结束点横坐标
 * @param y2 结束点纵坐标
 * @param coloridx 线的颜色，但是只能传索引（可以参考LVGL画点函数的参数）
 */
void drawline(lv_obj_t * canvas,int x1, int y1, int x2, int y2, lv_color_t coloridx)
{
	int tmp;
	int dx = x2 - x1;
	int dy = y2 - y1;

	if (abs(dx) < abs(dy))
	{
		if (y1 > y2)
		{
			tmp = x1; x1 = x2; x2 = tmp;
			tmp = y1; y1 = y2; y2 = tmp;
			dx = -dx; dy = -dy;
		}
		
		x1 <<= 16;

		dx = (dx << 16) / dy;
		while (y1 <= y2)
		{
			lv_canvas_set_px(canvas, x1 >> 16, y1, coloridx);
			x1 += dx;
			y1++;
		}
	}
	else
	{
		if (x1 > x2)
		{
			tmp = x1; x1 = x2; x2 = tmp;
			tmp = y1; y1 = y2; y2 = tmp;
			dx = -dx; dy = -dy;
		}
		y1 <<= 16;

		dy = dx ? (dy << 16) / dx : 0;
		while (x1 <= x2)
		{
		    lv_canvas_set_px(canvas, x1, y1 >> 16, coloridx);
			y1 += dy;
			x1++;
		}
	}
}
/**
 * 在画布上画实心矩形
 * @param canvas 画布对象
 * @param x1 起点横坐标
 * @param y1 起点纵坐标
 * @param Width 矩形宽度
 * @param Hight 矩形高度
 * @param coloridx 矩形的颜色，但是只能传索引（可以参考LVGL画点函数的参数）
 */
void fillrect(lv_obj_t * canvas,int x1, int y1, int Width, int Hight, lv_color_t coloridx)
{

    uint32_t x;
    uint32_t y;
    for( y = y1; y < y1+Hight; y++) {
        for( x = x1; x < x1+Width; x++) {
            lv_canvas_set_px(canvas, x, y, coloridx);
        }
    }
}
/**
 * 在画布上画实心三角形
 * @param canvas 画布对象
 * @param x1 起点横坐标
 * @param y1 起点纵坐标
 * @param Width 三角形的宽度
 * @param Hight 三角形的高度
 * @param coloridx 三角形的颜色，但是只能传索引（可以参考LVGL画点函数的参数）
 */
void DrawTriangle(lv_obj_t * canvas,int x1, int y1, int Width, int Hight, lv_color_t coloridx)
{
	unsigned short Point[3][2] = {0};

		Point[0][0] = x1;
		Point[0][1] = y1;
		Point[1][0] = x1-Width/2;
		Point[1][1] = y1+Hight;
		Point[2][0] = x1+Width/2;
		for (;Point[2][0] >= Point[1][0]; Point[1][0]++)
		{
			drawline(canvas,Point[0][0],Point[0][1],Point[1][0],Point[1][1],coloridx);
		}
}

/**
 * 在画布上画实心菱形
 * @param canvas 画布对象
 * @param usCenterX 起点横坐标
 * @param usStartY  起点纵坐标
 * @param length_of_side 菱形每条边的长度
 * @param coloridx 菱形的颜色，但是只能传索引（可以参考LVGL画点函数的参数）
 */
void lozenge(lv_obj_t * canvas,unsigned short usCenterX, unsigned short usStartY, unsigned short length_of_side,lv_color_t coloridx)
{
	unsigned short Point[3][2] = {0};
   // 计算等边三角形的高度
    unsigned short usHight  = (sqrt(3.0) / 2.0) * length_of_side;
    unsigned short usWidth  = length_of_side/2;
     // 画两个三角形
		Point[0][0] = usCenterX;
		Point[0][1] = usStartY;
		Point[1][0] = usCenterX-usWidth;
		Point[1][1] = usStartY+usHight;
		Point[2][0] = usCenterX+usWidth;
		Point[2][1] = usStartY+usHight;
		for (;Point[2][0] >= Point[1][0]; Point[1][0]++)
		{
			drawline(canvas,Point[0][0],Point[0][1],Point[1][0],Point[1][1],coloridx);
		}
		usStartY += (usHight*2);
		Point[0][0] = usCenterX;
		Point[0][1] = usStartY;
		Point[1][0] = usCenterX-usWidth;
		Point[1][1] = usStartY-usHight;
		Point[2][0] = usCenterX+usWidth;
		Point[2][1] = usStartY-usHight;
		for (;Point[2][0] >= Point[1][0]; Point[1][0]++)
		{
			drawline(canvas,Point[0][0],Point[0][1],Point[1][0],Point[1][1],coloridx);
		}
}

/**
 * 在画布上画实心五边形
 * @param canvas   画布对象
 * @param centerX  起点横坐标
 * @param centerY  起点纵坐标
 * @param radius   五边形的半径
 * @param coloridx 五边形的颜色，但是只能传索引（可以参考LVGL画点函数的参数）
 */
void drawPentagon(lv_obj_t * canvas,int centerX, int centerY, int radius,lv_color_t coloridx) {
    int i;
    int x[5], y[5];
    unsigned short Point[3][2] = {0};
    x[0] = centerX; y[0] = centerY - radius;
    x[1] = centerX- radius; y[1] = centerY ;
    x[2] = centerX - (radius/2); y[2] = centerY+ radius;
    x[3] = centerX + (radius / 2); y[3] = centerY + radius;
    x[4] = centerX+ radius; y[4] = centerY;
    // 画四个三角形
		for (i = x[1];x[4] >= i; i++)
		{
			drawline(canvas,x[0],y[0],i,y[1],coloridx);
		}
        for (i = x[1];centerX >= i; i++)
		{
			drawline(canvas,x[2],y[2],i,centerY,coloridx);
		}
		for (i = x[2];x[3] >= i; i++)
		{
			drawline(canvas,centerX,centerY,i,y[3],coloridx);
		}
		for (i = centerX;x[4] >= i; i++)
		{
			drawline(canvas,x[3],y[3],i,y[4],coloridx);
		}
}
#if 0
void fb_yuan(lv_obj_t * canvas,int x, int y, int z, lv_color_t coloridx)
{
    int i = 0,j = 0;
    double dstart = 0;
    dstart = sqrt((z*z)+(0*0));
    for(i = x-z;i < x+z;i++)
    {
        for(j = y-z;j < y+z;j++)
        {
            if((sqrt(((i-x)*(i-x))+((j-y)*(j-y)))*1.0)<dstart*1.0)
                lv_canvas_set_px(canvas,i,j,coloridx);
        }
    }
}

#else

void fb_yuan(lv_obj_t * canvas, int x0, int y0, int radius, lv_color_t color)
{
    int x = radius;
    int y = 0;
    int xChange = 1 - 2 * radius;
    int yChange = 1;
    int radiusError = 0;
    int i;
    while (x >= y)
    {
        for (i = x0 - x; i <= x0 + x; i++) {
            lv_canvas_set_px(canvas, i, y0 + y, color);
            lv_canvas_set_px(canvas, i, y0 - y, color);
        }
        for (i = x0 - y; i <= x0 + y; i++) {
            lv_canvas_set_px(canvas, i, y0 + x, color);
            lv_canvas_set_px(canvas, i, y0 - x, color);
        }

        y++;
        radiusError += yChange;
        yChange += 2;
        if (2 * radiusError + xChange > 0) {
            x--;
            radiusError += xChange;
            xChange += 2;
        }
    }
}
#endif
