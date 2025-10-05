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

// ������ʽ�ĺ���
double evaluateExpression(const char *expr);
void decToBinary(int n, char *binaryStr); // 10����ת������
static lv_obj_t *decimalism = NULL;       // 10���ƽ���ı�
static lv_obj_t *hexadecimal = NULL;      // 16���ƽ���ı�
static lv_obj_t *binary_system = NULL;    // 2���ƽ���ı�
static lv_obj_t *pwd_text_area = NULL;    // �����ı�

// ȷ�ϰ������¼��ص�
static void btn_event_callback(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_VALUE_CHANGED)
    {
        uint16_t ID = lv_btnmatrix_get_active_btn(obj); // ��ȡID
        char *buf[100] = {0};
        char binaryStr[32]; // �㹻�洢32λ�����Ķ������ַ���
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
                         "\xE8\xBF\x9B" /*��*/ ""
                         "\xE5\x88\xB6" /*��*/ ":%0.2f",
                    number);
            lv_label_set_text(decimalism, buf);
            if (number >= 0)
            { // ���������
                sprintf(buf, "16"
                             "\xE8\xBF\x9B" /*��*/ ""
                             "\xE5\x88\xB6" /*��*/ ":%X",
                        (int)number);
                lv_label_set_text(hexadecimal, buf);
                decToBinary((int)number, binaryStr);
                sprintf(buf, "2"
                             "\xE8\xBF\x9B" /*��*/ ""
                             "\xE5\x88\xB6" /*��*/ ":%s",
                        binaryStr);
                lv_label_set_text(binary_system, buf);
            }
            else
            { // ����Ǹ���
                sprintf(buf, "16"
                             "\xE8\xBF\x9B" /*��*/ ""
                             "\xE5\x88\xB6" /*��*/ ":-%X",
                        (int)(number * -1));
                lv_label_set_text(hexadecimal, buf);
                decToBinary((int)(number * -1), binaryStr);
                sprintf(buf, "2"
                             "\xE8\xBF\x9B" /*��*/ ""
                             "\xE5\x88\xB6" /*��*/ " :-%s",
                        binaryStr);
                lv_label_set_text(binary_system, buf);
            }
        }
        else
        {
            const char *txt = lv_btnmatrix_get_btn_text(obj, ID); // ��ȡ��ť�ı�
            const char *text = lv_label_get_text(pwd_text_area);  // ��ȡ���������ı�
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
    { /* ���󱻵�� */

        lv_obj_set_hidden(main_create, false); // ��ʾ�б�
        lv_obj_t *parent_obj = lv_obj_get_parent(obj);
        lv_obj_del_async(parent_obj);
    }
}
void computer(void)
{
    lv_obj_t *tile1 = lv_obj_create(lv_scr_act(), NULL);
    // LV_HOR_RES ��ǰ��Ļ��ˮƽ�ֱ���
    // LV_VER_RES ��ǰ��Ļ�Ĵ�ֱ�ֱ���
    lv_obj_set_size(tile1, LV_HOR_RES, LV_VER_RES);

    lv_obj_t *label; // ���ذ�ť
    lv_obj_t *btn1 = lv_btn_create(tile1, NULL);
    lv_obj_set_event_cb(btn1, event_computer);
    lv_obj_align(btn1, tile1, LV_ALIGN_IN_TOP_LEFT, 5, 5);
    label = lv_label_create(btn1, NULL);
    lv_label_set_text(label, "Esc");

    lv_obj_t *pwd_main_cont = lv_obj_create(tile1, NULL); // ��������������
                                                          // ����һ����ʽ
    static lv_style_t main_cont_style;
    // �����ʽ�е��������Ժ����з�����ڴ档
    lv_style_reset(&main_cont_style);
    // ��ʼ����ʽ
    lv_style_init(&main_cont_style);
    lv_style_set_radius(&main_cont_style, LV_STATE_DEFAULT, 20);               // ������ʽ��Բ�ǻ���
    lv_style_set_border_width(&main_cont_style, LV_STATE_DEFAULT, 0);          // ���ñ߿���
    lv_style_set_bg_opa(&main_cont_style, LV_STATE_DEFAULT, LV_OPA_100);       // ������ʽ������͸���ȣ���͸��
    lv_style_set_bg_color(&main_cont_style, LV_STATE_DEFAULT, LV_COLOR_WHITE); // ��ɫ����
    lv_style_set_pad_inner(&main_cont_style, LV_STATE_DEFAULT, 0);             // �����
    lv_obj_add_style(pwd_main_cont, NULL, &main_cont_style);                   // �����������ʽ
    lv_obj_set_size(pwd_main_cont, 400, 480);                                  // 500*480
    lv_obj_align(pwd_main_cont, NULL, LV_ALIGN_IN_TOP_MID, 90, 0);             // λ��

    lv_obj_t *btnm1 = lv_btnmatrix_create(pwd_main_cont, NULL);           // ���������

    // ����һ����ʽ
    static lv_style_t btnmatrix_style;
    // �����ʽ�е��������Ժ����з�����ڴ档
    lv_style_reset(&btnmatrix_style);
    // ��ʼ����ʽ
    lv_style_init(&btnmatrix_style);
    lv_style_set_radius(&btnmatrix_style, LV_STATE_DEFAULT, 20);                // ������ʽ��Բ�ǻ���
    lv_style_set_border_width(&btnmatrix_style, LV_STATE_DEFAULT, 0);          // ���ñ߿���
    lv_style_set_bg_opa(&btnmatrix_style, LV_STATE_DEFAULT, LV_OPA_100);       // ������ʽ������͸���ȣ���͸��
    lv_style_set_bg_color(&btnmatrix_style, LV_STATE_DEFAULT, LV_COLOR_WHITE); // ��ɫ����
    lv_style_set_pad_inner(&btnmatrix_style, LV_STATE_DEFAULT, 15);            // ���ö����ڲ��ļ��
    // ����һ����ʽ
    static lv_style_t btnmatrix_style1;
    // �����ʽ�е��������Ժ����з�����ڴ档
    lv_style_reset(&btnmatrix_style1);
    // ��ʼ����ʽ
    lv_style_init(&btnmatrix_style1);
    lv_style_set_radius(&btnmatrix_style1, LV_STATE_DEFAULT, 20);                 // ������ʽ��Բ�ǻ���
    lv_style_set_border_width(&btnmatrix_style1, LV_STATE_DEFAULT, 2);            // ���ñ߿���
    lv_style_set_bg_opa(&btnmatrix_style1, LV_STATE_DEFAULT, LV_OPA_100);         // ������ʽ������͸���ȣ���͸��
    lv_style_set_bg_color(&btnmatrix_style1, LV_STATE_DEFAULT, LV_COLOR_WHITE);   // ��ɫ����
    lv_style_set_text_color(&btnmatrix_style1, LV_STATE_DEFAULT, LV_COLOR_BLACK); // �����ı���ɫΪ��ɫ

    lv_btnmatrix_set_map(btnm1, keyboard_map);                                                                                           // ��ť����
    lv_obj_add_style(btnm1, LV_BTNMATRIX_PART_BG, &btnmatrix_style);                                                                     // �����������ʽ
    lv_obj_add_style(btnm1, LV_BTNMATRIX_PART_BTN, &btnmatrix_style1);                                                                   // �����������ʽ
    lv_obj_set_size(btnm1, lv_obj_get_width(pwd_main_cont), (int)(lv_obj_get_height(pwd_main_cont) * 0.8));                              // ���ô�С800x480
    lv_obj_align(btnm1, NULL, LV_ALIGN_IN_TOP_MID, 0, lv_obj_get_height(pwd_main_cont) - (int)(lv_obj_get_height(pwd_main_cont) * 0.8)); // λ��
    lv_btnmatrix_set_btn_width(btnm1, 4, 2);                                                                                             // ����'/'�����
    lv_btnmatrix_set_btn_width(btnm1, 8, 2);                                                                                             // ����'.'�����
    lv_btnmatrix_set_btn_width(btnm1, 12, 2);                                                                                            // ����'-'�����
    lv_btnmatrix_set_btn_width(btnm1, 16, 2);                                                                                            // ����'+'�����
    lv_obj_set_event_cb(btnm1, btn_event_callback);                                                                                      // ���þ���ť�ص�����

    pwd_text_area = lv_label_create(pwd_main_cont, NULL);                                                   // �����ı�
    lv_obj_set_style_local_text_color(pwd_text_area, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK); // �ı����óɺ�ɫ
    lv_obj_add_style(pwd_text_area, LV_LABEL_PART_MAIN, &style);                                            // ��������
    lv_label_set_text(pwd_text_area, "");                                                                   // �������
    lv_obj_align(pwd_text_area, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 10);
    lv_obj_set_size(pwd_text_area, lv_obj_get_width(pwd_main_cont), 30); // ���ô�С

    decimalism = lv_label_create(pwd_main_cont, NULL);                                                   // 10�����ı�
    lv_obj_set_style_local_text_color(decimalism, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK); // �ı����óɺ�ɫ
    lv_obj_add_style(decimalism, LV_LABEL_PART_MAIN, &style);                                            // ��������
    lv_label_set_text(decimalism, "");
    lv_obj_align(decimalism, pwd_text_area, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
    lv_obj_set_size(decimalism, lv_obj_get_width(pwd_main_cont) / 2, 30); // ���ô�С

    hexadecimal = lv_label_create(pwd_main_cont, NULL);                                                   // 16�����ı�
    lv_obj_set_style_local_text_color(hexadecimal, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK); // �ı����óɺ�ɫ
    lv_obj_add_style(hexadecimal, LV_LABEL_PART_MAIN, &style);                                            // ��������
    lv_label_set_text(hexadecimal, "");
    lv_obj_align(hexadecimal, pwd_text_area, LV_ALIGN_OUT_BOTTOM_LEFT, lv_obj_get_width(pwd_main_cont) / 2, 0);
    lv_obj_set_size(hexadecimal, lv_obj_get_width(pwd_main_cont) / 2, 30); // ���ô�С

    binary_system = lv_label_create(pwd_main_cont, NULL);                                                   // 2�����ı�
    lv_obj_set_style_local_text_color(binary_system, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK); // �ı����óɺ�ɫ
    lv_obj_add_style(binary_system, LV_LABEL_PART_MAIN, &style);                                            // ��������
    lv_label_set_text(binary_system, "");
    lv_obj_align(binary_system, decimalism, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0); // λ��
    lv_obj_set_size(binary_system, lv_obj_get_width(pwd_main_cont), 30);     // ���ô�С
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
        return 0; // �Ƿ������
    }
}

// ִ���������ֵ�����
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

// ������ʽ�ĺ���
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
            --opIndex; // ����������
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
// 10����ת������
void decToBinary(int n, char *binaryStr)
{
    // ��ʱ����洢����������ÿһλ
    char temp[32];
    int i = 0;

    // ��֤n�Ǹ�
    if (n < 0)
    {
        rt_kprintf("Only non-negative numbers are supported.\n");
        return;
    }

    // ������nΪ0
    if (n == 0)
    {
        strcpy(binaryStr, "0");
        return;
    }

    // ѭ��ֱ��n��Ϊ0
    while (n > 0)
    {
        temp[i++] = (n % 2) + '0'; // ������ת��Ϊ�ַ����洢
        n = n / 2;
    }

    // ��ת�ַ���
    int j = 0;
    for (i = i - 1; i >= 0; i--)
    {
        binaryStr[j++] = temp[i];
    }
    binaryStr[j] = '\0'; // �ַ���������
}
