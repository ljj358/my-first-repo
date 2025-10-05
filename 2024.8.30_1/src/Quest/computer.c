/*
 * computer.c
 *
 * created: 2024/2/20
 *  author:
 */
#include "computer.h"
#include "lvgl_ui.h"
#include <stdio.h>
#include <stdlib.h>

// 计算表达式的函数
double evaluateExpression(const char *expr);
void decToBinary(int n, char *binaryStr); // 10进制转二进制
static lv_obj_t *decimalism = NULL;       // 10进制结果文本
static lv_obj_t *hexadecimal = NULL;      // 16进制结果文本
static lv_obj_t *binary_system = NULL;    // 2进制结果文本
static lv_obj_t *pwd_text_area = NULL;    // 输入文本

// 确认按键的事件回调
static void btn_event_callback(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_VALUE_CHANGED)
    {
        uint16_t ID = lv_btnmatrix_get_active_btn(obj); // 获取ID
        char *buf[100] = {0};
        char binaryStr[32]; // 足够存储32位整数的二进制字符串
        if (ID == 0)
        {
            lv_label_set_text(pwd_text_area, "");
            lv_label_set_text(decimalism, "");
            lv_label_set_text(hexadecimal, "");
            lv_label_set_text(binary_system, "");
        }
        else if (ID == 15)
        {
            double number = evaluateExpression(lv_label_get_text(pwd_text_area));
            sprintf(buf, "10"
                         "\xE8\xBF\x9B" /*进*/ ""
                         "\xE5\x88\xB6" /*制*/ ":%0.2f",
                    number);
            lv_label_set_text(decimalism, buf);
            if (number >= 0)
            { // 结果是正数
                sprintf(buf, "16"
                             "\xE8\xBF\x9B" /*进*/ ""
                             "\xE5\x88\xB6" /*制*/ ":%X",
                        (int)number);
                lv_label_set_text(hexadecimal, buf);
                decToBinary((int)number, binaryStr);
                sprintf(buf, "2"
                             "\xE8\xBF\x9B" /*进*/ ""
                             "\xE5\x88\xB6" /*制*/ ":%s",
                        binaryStr);
                lv_label_set_text(binary_system, buf);
            }
            else
            { // 结果是负数
                sprintf(buf, "16"
                             "\xE8\xBF\x9B" /*进*/ ""
                             "\xE5\x88\xB6" /*制*/ ":-%X",
                        (int)(number * -1));
                lv_label_set_text(hexadecimal, buf);
                decToBinary((int)(number * -1), binaryStr);
                sprintf(buf, "2"
                             "\xE8\xBF\x9B" /*进*/ ""
                             "\xE5\x88\xB6" /*制*/ " :-%s",
                        binaryStr);
                lv_label_set_text(binary_system, buf);
            }
        }
        else
        {
            const char *txt = lv_btnmatrix_get_btn_text(obj, ID); // 获取按钮文本
            const char *text = lv_label_get_text(pwd_text_area);  // 获取输入内容文本
            strcpy(buf, text);
            strcat(buf, txt);
            lv_label_set_text(pwd_text_area, buf);
        }
    }
}

static const char *keyboard_map[] =
    {
        "clear", "\n",
        "7", "8", "9", "/", "\n",
        "4", "5", "6", "*", "\n",
        "1", "2", "3", "-", "\n",
        ".", "0", "=", "+", ""};
static void event_computer(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_SHORT_CLICKED)
    { /* 对象被点击 */

        lv_obj_set_hidden(main_create, false); // 显示列表
        lv_obj_t *parent_obj = lv_obj_get_parent(obj);
        lv_obj_del_async(parent_obj);
    }
}
void computer(void)
{
    lv_obj_t *tile1 = lv_obj_create(lv_scr_act(), NULL);
    // LV_HOR_RES 当前屏幕的水平分辨率
    // LV_VER_RES 当前屏幕的垂直分辨率
    lv_obj_set_size(tile1, LV_HOR_RES, LV_VER_RES);

    lv_obj_t *label; // 返回按钮
    lv_obj_t *btn1 = lv_btn_create(tile1, NULL);
    lv_obj_set_event_cb(btn1, event_computer);
    lv_obj_align(btn1, tile1, LV_ALIGN_IN_TOP_LEFT, 5, 5);
    label = lv_label_create(btn1, NULL);
    lv_label_set_text(label, "Esc");

    lv_obj_t *pwd_main_cont = lv_obj_create(tile1, NULL); // 计算机界面的容器
                                                          // 创建一个样式
    static lv_style_t main_cont_style;
    // 清除样式中的所有属性和所有分配的内存。
    lv_style_reset(&main_cont_style);
    // 初始化样式
    lv_style_init(&main_cont_style);
    lv_style_set_radius(&main_cont_style, LV_STATE_DEFAULT, 20);               // 设置样式的圆角弧度
    lv_style_set_border_width(&main_cont_style, LV_STATE_DEFAULT, 0);          // 设置边框宽度
    lv_style_set_bg_opa(&main_cont_style, LV_STATE_DEFAULT, LV_OPA_100);       // 设置样式背景的透明度，不透明
    lv_style_set_bg_color(&main_cont_style, LV_STATE_DEFAULT, LV_COLOR_WHITE); // 白色背景
    lv_style_set_pad_inner(&main_cont_style, LV_STATE_DEFAULT, 0);             // 不填充
    lv_obj_add_style(pwd_main_cont, NULL, &main_cont_style);                   // 给对象添加样式
    lv_obj_set_size(pwd_main_cont, 400, 480);                                  // 500*480
    lv_obj_align(pwd_main_cont, NULL, LV_ALIGN_IN_TOP_MID, 90, 0);             // 位置

    lv_obj_t *btnm1 = lv_btnmatrix_create(pwd_main_cont, NULL);           // 计算机键盘

    // 创建一个样式
    static lv_style_t btnmatrix_style;
    // 清除样式中的所有属性和所有分配的内存。
    lv_style_reset(&btnmatrix_style);
    // 初始化样式
    lv_style_init(&btnmatrix_style);
    lv_style_set_radius(&btnmatrix_style, LV_STATE_DEFAULT, 20);                // 设置样式的圆角弧度
    lv_style_set_border_width(&btnmatrix_style, LV_STATE_DEFAULT, 0);          // 设置边框宽度
    lv_style_set_bg_opa(&btnmatrix_style, LV_STATE_DEFAULT, LV_OPA_100);       // 设置样式背景的透明度，不透明
    lv_style_set_bg_color(&btnmatrix_style, LV_STATE_DEFAULT, LV_COLOR_WHITE); // 白色背景
    lv_style_set_pad_inner(&btnmatrix_style, LV_STATE_DEFAULT, 15);            // 设置对象内部的间距
    // 创建一个样式
    static lv_style_t btnmatrix_style1;
    // 清除样式中的所有属性和所有分配的内存。
    lv_style_reset(&btnmatrix_style1);
    // 初始化样式
    lv_style_init(&btnmatrix_style1);
    lv_style_set_radius(&btnmatrix_style1, LV_STATE_DEFAULT, 20);                 // 设置样式的圆角弧度
    lv_style_set_border_width(&btnmatrix_style1, LV_STATE_DEFAULT, 2);            // 设置边框宽度
    lv_style_set_bg_opa(&btnmatrix_style1, LV_STATE_DEFAULT, LV_OPA_100);         // 设置样式背景的透明度，不透明
    lv_style_set_bg_color(&btnmatrix_style1, LV_STATE_DEFAULT, LV_COLOR_WHITE);   // 白色背景
    lv_style_set_text_color(&btnmatrix_style1, LV_STATE_DEFAULT, LV_COLOR_BLACK); // 设置文本颜色为黑色

    lv_btnmatrix_set_map(btnm1, keyboard_map);                                                                                           // 按钮内容
    lv_obj_add_style(btnm1, LV_BTNMATRIX_PART_BG, &btnmatrix_style);                                                                     // 给对象添加样式
    lv_obj_add_style(btnm1, LV_BTNMATRIX_PART_BTN, &btnmatrix_style1);                                                                   // 给对象添加样式
    lv_obj_set_size(btnm1, lv_obj_get_width(pwd_main_cont), (int)(lv_obj_get_height(pwd_main_cont) * 0.8));                              // 设置大小800x480
    lv_obj_align(btnm1, NULL, LV_ALIGN_IN_TOP_MID, 0, lv_obj_get_height(pwd_main_cont) - (int)(lv_obj_get_height(pwd_main_cont) * 0.8)); // 位置
    lv_btnmatrix_set_btn_width(btnm1, 4, 2);                                                                                             // 设置'/'键宽度
    lv_btnmatrix_set_btn_width(btnm1, 8, 2);                                                                                             // 设置'.'键宽度
    lv_btnmatrix_set_btn_width(btnm1, 12, 2);                                                                                            // 设置'-'键宽度
    lv_btnmatrix_set_btn_width(btnm1, 16, 2);                                                                                            // 设置'+'键宽度
    lv_obj_set_event_cb(btnm1, btn_event_callback);                                                                                      // 设置矩阵按钮回掉函数

    pwd_text_area = lv_label_create(pwd_main_cont, NULL);                                                   // 输入文本
    lv_obj_set_style_local_text_color(pwd_text_area, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK); // 文本设置成黑色
    lv_obj_add_style(pwd_text_area, LV_LABEL_PART_MAIN, &style);                                            // 设置字体
    lv_label_set_text(pwd_text_area, "");                                                                   // 清空内容
    lv_obj_align(pwd_text_area, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 10);
    lv_obj_set_size(pwd_text_area, lv_obj_get_width(pwd_main_cont), 30); // 设置大小

    decimalism = lv_label_create(pwd_main_cont, NULL);                                                   // 10进制文本
    lv_obj_set_style_local_text_color(decimalism, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK); // 文本设置成黑色
    lv_obj_add_style(decimalism, LV_LABEL_PART_MAIN, &style);                                            // 设置字体
    lv_label_set_text(decimalism, "");
    lv_obj_align(decimalism, pwd_text_area, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
    lv_obj_set_size(decimalism, lv_obj_get_width(pwd_main_cont) / 2, 30); // 设置大小

    hexadecimal = lv_label_create(pwd_main_cont, NULL);                                                   // 16进制文本
    lv_obj_set_style_local_text_color(hexadecimal, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK); // 文本设置成黑色
    lv_obj_add_style(hexadecimal, LV_LABEL_PART_MAIN, &style);                                            // 设置字体
    lv_label_set_text(hexadecimal, "");
    lv_obj_align(hexadecimal, pwd_text_area, LV_ALIGN_OUT_BOTTOM_LEFT, lv_obj_get_width(pwd_main_cont) / 2, 0);
    lv_obj_set_size(hexadecimal, lv_obj_get_width(pwd_main_cont) / 2, 30); // 设置大小

    binary_system = lv_label_create(pwd_main_cont, NULL);                                                   // 2进制文本
    lv_obj_set_style_local_text_color(binary_system, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK); // 文本设置成黑色
    lv_obj_add_style(binary_system, LV_LABEL_PART_MAIN, &style);                                            // 设置字体
    lv_label_set_text(binary_system, "");
    lv_obj_align(binary_system, decimalism, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0); // 位置
    lv_obj_set_size(binary_system, lv_obj_get_width(pwd_main_cont), 30);     // 设置大小
}

int getPriority(char op)
{
    if (op == '+' || op == '-')
    {
        return 1;
    }
    else if (op == '*' || op == '/' || op == '%')
    {
        return 2;
    }
    else if (op == '^')
    {
        return 3;
    }
    else if (op == '&' || op == '|')
    {
        return 4;
    }
    else if (op == '!')
    {
        return 5;
    }
    else
    {
        return 0; // 非法运算符
    }
}

// 执行两个数字的运算
double applyOperator(double a, double b, char op)
{
    switch (op)
    {
    case '+':
        return a + b;
    case '-':
        return a - b;
    case '*':
        return a * b;
    case '/':
        if (b != 0)
        {
            return a / b;
        }
        else
        {
            printf("Error: Division by zero\n");
        }
    case '%':
        if (b != 0)
        {
            return fmod(a, b);
        }
        else
        {
            printf("Error: Division by zero\n");
        }
    case '^':
        return pow(a, b);
    case '&':
        return (int)a & (int)b;
    case '|':
        return (int)a | (int)b;
    case '!':
        return !a;
    default:
        printf("Error: Invalid operator\n");
    }
}

// 计算表达式的函数
double evaluateExpression(const char *expr)
{
    int len = strlen(expr);
    double numbers[len];
    char operators[len];
    int numIndex = 0, opIndex = 0;
    int i;
    for (i = 0; i < len; ++i)
    {
        if (isdigit(expr[i]))
        {
            double num = strtod(expr + i, NULL);
            numbers[numIndex++] = num;
            while (i < len && (isdigit(expr[i]) || expr[i] == '.'))
            {
                ++i;
            }
            --i;
        }
        else if (expr[i] == '(')
        {
            operators[opIndex++] = expr[i];
        }
        else if (expr[i] == ')')
        {
            while (opIndex > 0 && operators[opIndex - 1] != '(')
            {
                double b = numbers[--numIndex];
                double a = numbers[--numIndex];
                char op = operators[--opIndex];
                numbers[numIndex++] = applyOperator(a, b, op);
            }
            --opIndex; // 弹出左括号
        }
        else if (expr[i] == '+' || expr[i] == '-' || expr[i] == '*' || expr[i] == '/' || expr[i] == '%' ||
                 expr[i] == '^' || expr[i] == '&' || expr[i] == '|' || expr[i] == '!')
        {
            while (opIndex > 0 && getPriority(operators[opIndex - 1]) >= getPriority(expr[i]))
            {
                double b = numbers[--numIndex];
                double a = numbers[--numIndex];
                char op = operators[--opIndex];
                numbers[numIndex++] = applyOperator(a, b, op);
            }
            operators[opIndex++] = expr[i];
        }
    }

    while (opIndex > 0)
    {
        double b = numbers[--numIndex];
        double a = numbers[--numIndex];
        char op = operators[--opIndex];
        numbers[numIndex++] = applyOperator(a, b, op);
    }

    return numbers[0];
}
// 10进制转二进制
void decToBinary(int n, char *binaryStr)
{
    // 临时数组存储二进制数的每一位
    char temp[32];
    int i = 0;

    // 保证n非负
    if (n < 0)
    {
        rt_kprintf("Only non-negative numbers are supported.\n");
        return;
    }

    // 特例：n为0
    if (n == 0)
    {
        strcpy(binaryStr, "0");
        return;
    }

    // 循环直到n变为0
    while (n > 0)
    {
        temp[i++] = (n % 2) + '0'; // 将余数转换为字符并存储
        n = n / 2;
    }

    // 翻转字符串
    int j = 0;
    for (i = i - 1; i >= 0; i--)
    {
        binaryStr[j++] = temp[i];
    }
    binaryStr[j] = '\0'; // 字符串结束符
}
