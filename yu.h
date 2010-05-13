/* yu.h 包含
 * 全局需要包含的标准库，
 * 全局变量，宏，结构
 */

// 谁都不喜欢把这些常用的库函数重复写多次
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 全局可用的宏总要给个唯一地方定义
#define VERSION "0.1"
#define PROGRAME_NAME "yu"
#define PACKAGE PROGRAME_NAME
#define program_name PROGRAME_NAME


// i18n -- 国际化也是到处需要的
#include <locale.h>
#include <libintl.h>
#define _(S) gettext(S)
#define LOCALEDIR "/usr/share/locale"


// 定义配置文件中行的最大长度
#define LINE_LENGTH_MAX 256

// 配置选项，可从命令行或者配置文件读取
typedef struct
{
  char cachedir[LINE_LENGTH_MAX];
  char logfile[LINE_LENGTH_MAX];
  int keepcache;
  int debuglevel;
  char repomd[16];  // repomd 名字默认是 "repomd.xml"
} YuOpt;


// 定义一个简单链表，保存所有 repodata 数据库信息
typedef struct REPO
{
  char name[LINE_LENGTH_MAX/8];
  char summary[LINE_LENGTH_MAX];
  char baseurl[LINE_LENGTH_MAX];
  int enabled;
  struct REPO *next;
} YuRepo;


// yu 的总配置文件
typedef struct 
{
  char file[LINE_LENGTH_MAX/8]; /* 配置文件的名字 */
  YuOpt opt;
  YuRepo *repo;
} YuConfig;


