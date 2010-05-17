#include "yu.h"

/*
 * C是个很小的语言，很多常用的函数都需要自己定义
 */

/* 用 newstr 替换 str 中的 oldstr */
extern void yu_str_replace (char *str, 
                           char *oldstr,
                           char *newstr)
{
  char swapstr[LINE_LENGTH_MAX]={'\0'};
  char *pstr=NULL;
  int str_length = strlen(oldstr);
  pstr = strstr(str, oldstr);
  if (pstr != NULL)
    {
      *pstr = '\0';
      strcpy (swapstr, pstr + str_length);
      strcat (str, newstr);
      strcat (str, swapstr);
    }
}
