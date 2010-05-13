#include "yu.h"

/* 本文件中的函数用来处理配置文件，配置文件会类似 yum,
 * - 只有一个 [main] 部分
 * - 【将来】读取多个配置文件
 */


/* 读取一个配置文件，并修改给定的 YuConfig 结构。
 * 之所以没有用返回值方式返回一个 YuConfig 机构，
 * 是为了将来有可能读取多个文件
 */
extern int yu_read_config (YuConfig *yuconfig)
{
  FILE *fp;
  char line[LINE_LENGTH_MAX];
  char *pstr=NULL;
  char *ret=NULL;

  if ((fp = fopen (yuconfig->file, "r")) == NULL)
    {
      printf (_("Can not open config file: %s\n"), yuconfig->file);
      return 1;
    }

  //ret = fgets (line, LINE_LENGTH_MAX, fp);
  while ((ret = fgets (line, LINE_LENGTH_MAX, fp)) != NULL)
    {
      printf ("%s", line);
    }

  return 0;
}
