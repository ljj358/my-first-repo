/*
 * bkrc_voice.h
 *
 * created: 2022/2/28
 *  author:
 */

#ifndef _BKRC_VOICE_H
#define _BKRC_VOICE_H

void UART3_Config_Init(void);
void voice_broadcast(unsigned char instruct);
unsigned char Voice_Drive(void);
void voice_figure( int instruct);
void voice_broadcast(unsigned char instruct);
#endif // _BKRC_VOICE_H


