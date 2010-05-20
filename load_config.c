#include "yu.h"

/* 本文件中的函数用来处理配置文件，配置文件会类似 yum,
 * - 只有一个 [main] 部分
 * - 【将来】读取多个配置文件
 */

static int __yu_line_delete_white_space (char *str);
static int __yu_config_do_main (YuConfig *yuconfig, FILE *fp, char *line, int *index);
static int __yu_config_do_main_parse (YuConfig *yuconfig, char *line);
static int __yu_config_do_repo (YuConfig *yuconfig, FILE *fp, char *line, int *index);
static int __yu_config_do_each_repo (YuRepo *repo, FILE *fp, char *line, int *index);
static int __yu_config_do_each_repo_parse (YuRepo *repo, char *line);

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
  int index=0, exit_status=0;

  if ((fp = fopen (yuconfig->file, "r")) == NULL)
    {
      printf (_("Can not open config file: %s\n"), yuconfig->file);
      return 1;
    }

  // 此处一个故事就是未初始化的值都是不稳定的，我在 Ubuntu 10.04 上
  // 不做初始化就可以，在 GTES11.3(RHEL5.4) 上就出错了。
  yuconfig->repo = NULL;

  //ret = fgets (line, LINE_LENGTH_MAX, fp);
  while ((ret = fgets (line, LINE_LENGTH_MAX, fp)) != NULL)
    {
      index ++;
      //printf ("[%d] %s", index, line);

      // 将行中的空白字符过滤掉，返回值非0就继续下一行
      // 遇到注释也跳过
      if (__yu_line_delete_white_space (line) != 0)
        continue;

      // 处理 [main] 部分
      pstr = strstr (line, "[main]");
      if (pstr != NULL)
        {
          // 思来想去还是把 line 通过指针传递进去方便，
          // 这样能保存未修改的但是已经读取的行
          __yu_config_do_main (yuconfig, fp, line, &index);
        }

      // 我把读取文件行操作放到while循环测试中，所以在这里用 goto 技巧解决多读一样问题
    dorepo:
      if (line[0] == '[')
        {
          if (__yu_config_do_repo (yuconfig, fp, line, &index) == 0)
            goto dorepo; 
        }
    }

  return exit_status;
}


// 过滤空白行，注释行
// 输入： 字符串指针
// 返回： 0 正确, 并修改输入的字符串值
static int __yu_line_delete_white_space (char *str)
{
  char strtmp[LINE_LENGTH_MAX]={'\0'};
  int len=strlen(str);
  unsigned int i=0, j=0;

  // 逐个字符复制 str 中的字符到 strtmp，跳过空白（当前仅处理空格）
  for (;i < len; i++)
    if (str[i] != ' ' && str[i] != '\n')
      strtmp[j++] = str[i];

  // '\0' 空行; '\n' 空行;
  // 注释字符可以再这里定义： '#', ';'
  // 下面有两种指针用法，它们实质是一样的 ;-)
  if ((*strtmp == '\0') || \
      (strtmp[0] == '#') || \
      (strtmp[0] == ';'))
    return 1;

  strtmp[j] = '\0';

  // 使用指针的好处就是就地修改
  strcpy (str, strtmp);
  return 0;
}


static int 
__yu_config_do_main (YuConfig *yuconfig, FILE *fp, char *line, int *index)
{
  while (fgets (line, LINE_LENGTH_MAX, fp) != NULL)
    {
      (*index) ++;
      //printf ("[%d] %s", *index, line);
      if (__yu_line_delete_white_space (line) != 0)
        continue;
      
      // 如果读到下一个配置部分 ([] 定义一个部分)，就跳出循环
      if (line[0] == '[')
        break;
      
      __yu_config_do_main_parse (yuconfig, line);
    }
  
  return 0;
}

static int 
__yu_config_do_main_parse (YuConfig *yuconfig, char *line)
{
  char *pstr=NULL;
  
  if ((pstr = strstr (line, "cachedir")) != NULL)
    {
      pstr = strstr (line, "=");
      strcpy (yuconfig->opt.cachedir, pstr+1);
      return 0;
    }

  if ((pstr = strstr (line, "keepcache")) != NULL)
    {
      pstr = strstr (line, "=");
      // 假定此值用一个阿拉伯“字符“就能表示
      // <stdlib.h> 中包含 atoi
      yuconfig->opt.keepcache = atoi(pstr+1);
      return 0;
    }

  if ((pstr = strstr (line, "debuglevel")) != NULL)
    {
      pstr = strstr (line, "=");
      yuconfig->opt.debuglevel = atoi(pstr+1);
      return 0;
    }

  if ((pstr = strstr (line, "logfile")) != NULL)
    {
      pstr = strstr (line, "=");
      strcpy (yuconfig->opt.logfile, pstr+1);
      return 0;
    }

  // 如果到这里，说明配置文件中的语法目前还不支持
  printf (_("Unknown config: \"%s\"\n"), line);
  return 1;
}


// 处理 repo 的总函数，遇到非 "[main]" 已 "[" 开头的行都开始
// 用此函数处理，直到遇到 [main]，或者文件处理完毕。
static int
__yu_config_do_repo (YuConfig *yuconfig, FILE *fp, char *line, int *index)
{
  YuRepo *tmprepo;
  tmprepo = (YuRepo *) malloc (sizeof(YuRepo));

  if (__yu_config_do_each_repo (tmprepo, fp, line, index) != 0)
    return 1;

  // 如果 yuconfig (总结构) 的 repo 还未指向，就链接到这里
  if (yuconfig->repo == NULL)
    {
      yuconfig->repo = tmprepo;
      return 0;
    }

  // 否则寻到 repo 链的最后一个，追加到此处
  YuRepo *swaprepo;
  swaprepo = yuconfig->repo;
  while (swaprepo->next != NULL)
    swaprepo = swaprepo->next;
  swaprepo->next = tmprepo;

  free (swaprepo);
  return 0;
}


// 对每个 repo 部分处理
static int
__yu_config_do_each_repo (YuRepo *repo, FILE *fp, char *line, int *index)
{
  char *pstr=NULL;

  pstr = strstr (line, "]");
  if (pstr == NULL)
    {
      printf (_("Wrong grammar: %s\n"), line);
      return 1;
    }
  
  // 替换 ']' 为 '\0'
  *pstr = '\0';
  strcpy (repo->name, line+1);

  while (fgets (line, LINE_LENGTH_MAX, fp) != NULL)
    {
      (*index) ++;
      //printf ("[%d] %s", *index, line);
      if (__yu_line_delete_white_space (line) != 0)
        continue;
      
      // 如果读到下一个配置部分 ([] 定义一个部分)，就跳出循环
      if (line[0] == '[')
        break;
      
      if (__yu_config_do_each_repo_parse (repo, line) != 0)
        return 1;
    }

  // 将 repo 结构的 next 指针指向空
  repo->next = NULL;
  return 0;
}


// 处理 repo 部分各行
static int 
__yu_config_do_each_repo_parse (YuRepo *repo, char *line)
{
  char *pstr=NULL;
  
  if ((pstr = strstr (line, "name")) != NULL)
    {
      pstr = strstr (line, "=");
      strcpy (repo->summary, pstr+1);
      return 0;
    }

  if ((pstr = strstr (line, "baseurl")) != NULL)
    {
      pstr = strstr (line, "=");
      strcpy (repo->baseurl, pstr+1);
      return 0;
    }

  if ((pstr = strstr (line, "enabled")) != NULL)
    {
      pstr = strstr (line, "=");
      repo->enabled = atoi(pstr+1);
      return 0;
    }

  printf (_("Have not support grammar: %s\n"), line);
  return 2;
}
