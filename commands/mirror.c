#include <errno.h>
#include <unistd.h>

#include "common.h"

static int __yu_mirror (char *url, char *localdir);

static int 
__yu_mirror_update_localdir (char *url, char *localdir);


/* mirror 命令 */
extern int yu_cmd_mirror (int argc, char **argv)
{
  if (argc < 1 || argc > 2)
    {
      printf (_("%s mirror repodata_url [localdir]\n"
                "Try --help to get more info.\n"),
              PROGRAME_NAME);
      return 1;
    }

  char localdir[LINE_LENGTH_MAX];
  char *ret = NULL;

  if (argc == 1)
    {
      // 如果到达此处，说明 argv[1] 是 url
      ret = getcwd (localdir, (size_t)LINE_LENGTH_MAX);
      if (ret == NULL)
        {
          printf (_("Get current directory make a mistake!\n"));
          return 1;
        }
      if (errno == ERANGE) 
        {
          printf (_("The current directory is longer than %d bytes!\n"),LINE_LENGTH_MAX);
          return 1;
        }
      // 将 url 的最后一个目录追加到 localdir 上
      __yu_mirror_update_localdir(argv[1], localdir);
    }
  else
    strcpy (localdir, argv[2]);

  __yu_mirror(argv[1], localdir);
  return 0;
}



/* 执行 mirror 功能，
 * 输入： 
 *   url       下有 repodata 子目录的 url 地址字符串指针
 *   localdir  本地存放目录字符串指针
 */
static int __yu_mirror (char *url, char *localdir)
{
  char repodata_dir[LINE_LENGTH_MAX]={'\0'};
  strcpy (repodata_dir, localdir);
  strcat (repodata_dir, "/");
  strcat (repodata_dir, YU_MIRROR_REPODATA_NAME);
  strcat (repodata_dir, "/");
  yu_str_replace (repodata_dir, "//", "/");
  printf (_("Run: %s mirror %s %s\n"), 
          PROGRAME_NAME, url, localdir);

  printf (_("Updating repodata : %s ...\n"), repodata_dir);
  if (0 != yu_xml_update_repodata (url, repodata_dir))
    printf ("Update have a error!\n");


  // 判断 primary data name
  char *primary_data_name=NULL;
  primary_data_name = yu_get_filename_in_dir (repodata_dir,"primary.sqlite");
  if (primary_data_name != NULL)
    yu_sql_mirror_primary (url, localdir, primary_data_name);
  else
    {
      primary_data_name = yu_get_filename_in_dir (repodata_dir,"primary.xml");

      if (primary_data_name == NULL)
        {
          printf (_("Have not found primary db name!\n"));
          return 1;
        }

      printf ("Have not support download from xml data!\n");
    }

  free (primary_data_name);
  return 0;
}


/* 添加 url 的最后一级目录到 localdir 的名称后面 */
static int 
__yu_mirror_update_localdir (char *url, char *localdir)
{
  char *pstr=NULL;
  char tmpstr[LINE_LENGTH_MAX];
  int url_length = strlen(url);
  
  // 复制 url 到 tmpstr , 如字符串以 '/' 字符结尾就置为0
  // （可以保证 strrchr 搜索的是最后一级目录）
  strcpy(tmpstr, url);
  if ( *(tmpstr + url_length - 1) == '/')
    *(tmpstr + url_length - 1) = '\0';

  // 搜索最后一级目录
  pstr = strrchr (tmpstr, '/');

  // 追加目录分隔符
  strcat (localdir, "/");

  // 追加 url 中的最后一级目录
  strcat (localdir, pstr+1);

  // 替换可能出现的 "//" 分割符，好看点 ;-)
  yu_str_replace (localdir, "//", "/");

  return 0;
}
