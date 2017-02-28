
#ifndef _STRING_H_
#define _STRINGL_H_

#include "stm32f4xx.h"

void  hex2str(char *str, uint8_t data);
void  hex2str_32b(char *str, int data);
int   str2hex(const char *str, uint8_t *data, int strMaxLength);
int   embed_strncmp(const char *str1, const char *str2, int length);
char* embed_strtok(char *str, const char *delim);
int   embed_strnlen(const char *str, int max_length);

#endif
