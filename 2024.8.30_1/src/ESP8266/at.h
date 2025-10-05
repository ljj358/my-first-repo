/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-10-05     lzh28       the first version
 */
#ifndef APPLICATIONS_AT_H_
#define APPLICATIONS_AT_H_

#define AT_RESP_OK        0
#define AT_RESP_ERROR     -1
#define AT_RESP_TIMEOUT   -2
#define AT_RESP_BUFF_FULL -3
#define AT_STATUS_OK        -4

typedef struct _at_resp_t {
    #define AT_MAX_RESP_LEN 256
    char buf[AT_MAX_RESP_LEN];
    unsigned int buf_len;       /* ��ǰ��Ӧ�Ļ�������С */
    unsigned int line_counts;   /* ��¼��Ӧ�������� */
} at_resp_t;

typedef struct _at_urc_t
{
    const char *cmd_prefix;     /* ǰ׺ */
    const char *cmd_suffix;     /* ��׺ */
    void (*func)(const char *data, unsigned int size);  /* ��Ӧִ�к��� */
} at_urc_t;


void at_client_init(void);
int at_exce_cmd(const char *cmd, at_resp_t *resp, unsigned int timeout);
int at_client_wait_connect(unsigned int timeout);
int at_client_senddata(const char *buf, unsigned int len, unsigned int timeout);
int at_client_recvchar(char *ch, unsigned int timeout);
void at_recv_parser(void *parameter);
void mqtt_client_test(void *parameter);
#endif /* APPLICATIONS_AT_H_ */
