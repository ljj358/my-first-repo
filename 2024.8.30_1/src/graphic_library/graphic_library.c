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
 * �ڻ����ϻ���
 * @param canvas ��������
 * @param x1 ��������
 * @param y1 ���������
 * @param x2 �����������
 * @param y2 ������������
 * @param coloridx �ߵ���ɫ������ֻ�ܴ����������Բο�LVGL���㺯���Ĳ�����
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
 * �ڻ����ϻ�ʵ�ľ���
 * @param canvas ��������
 * @param x1 ��������
 * @param y1 ���������
 * @param Width ���ο��
 * @param Hight ���θ߶�
 * @param coloridx ���ε���ɫ������ֻ�ܴ����������Բο�LVGL���㺯���Ĳ�����
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
 * �ڻ����ϻ�ʵ��������
 * @param canvas ��������
 * @param x1 ��������
 * @param y1 ���������
 * @param Width �����εĿ��
 * @param Hight �����εĸ߶�
 * @param coloridx �����ε���ɫ������ֻ�ܴ����������Բο�LVGL���㺯���Ĳ�����
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
 * �ڻ����ϻ�ʵ������
 * @param canvas ��������
 * @param usCenterX ��������
 * @param usStartY  ���������
 * @param length_of_side ����ÿ���ߵĳ���
 * @param coloridx ���ε���ɫ������ֻ�ܴ����������Բο�LVGL���㺯���Ĳ�����
 */
void lozenge(lv_obj_t * canvas,unsigned short usCenterX, unsigned short usStartY, unsigned short length_of_side,lv_color_t coloridx)
{
	unsigned short Point[3][2] = {0};
   // ����ȱ������εĸ߶�
    unsigned short usHight  = (sqrt(3.0) / 2.0) * length_of_side;
    unsigned short usWidth  = length_of_side/2;
     // ������������
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
 * �ڻ����ϻ�ʵ�������
 * @param canvas   ��������
 * @param centerX  ��������
 * @param centerY  ���������
 * @param radius   ����εİ뾶
 * @param coloridx ����ε���ɫ������ֻ�ܴ����������Բο�LVGL���㺯���Ĳ�����
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
    // ���ĸ�������
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
