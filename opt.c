#include "yu.h"
#include "usage.h"


// 处理命令行参数可参考 : http://www.gnu.org/s/libc/manual/html_node/Getopt-Long-Option-Example.html
#include <getopt.h>

#define GETOPT_HELP_OPTION_DECL                 \
  "help", no_argument, NULL, 'h'
#define GETOPT_VERSION_OPTION_DECL              \
  "version", no_argument, NULL, 'V'

// Flag set by '--verbose'
static int verbose_flag;

static struct option const long_opts[] = {
  // gnu 建议使用 --brief 代替 --verbose
  {"verbose", no_argument,       &verbose_flag, 1},
  {"brief",   no_argument,       &verbose_flag, 0},

  {"baseurl", required_argument, NULL, 'u'},
  {"background", no_argument, NULL, 'b'},
  {"config", required_argument, NULL, 'c'},
  {"log", required_argument, NULL, 'l'},
  {GETOPT_HELP_OPTION_DECL},
  {GETOPT_VERSION_OPTION_DECL},
  {NULL, 0, NULL, 0}
};


// 处理命令行参数
int yu_main_opt (int argc, char **argv, YuConfig *yuconfig) 
{
  int ret, c;
  YuOpt *yuopt;
  const char * const short_options ="c:u:hs:bl:V";

  // 初始化 yuopt
  yuopt = (YuOpt *)malloc (sizeof (YuOpt));
  strcat (yuopt->repomd, "repomd.xml");

  setlocale (LC_ALL, "");
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);

  while (1)
    {
      // getopt_long stores the option index here.
      int option_index = 0;

      c = getopt_long(argc, argv, short_options, 
                      long_opts, &option_index);

      // 如果 c == -1 ，处理完毕
      if (c == -1)
        break;

      switch (c)
        {
        case 0:
          /* If this option set a flag, do nothing else now. */
          printf ("get the '--verbose'\n");
          break;
        case 'c' :
          printf ("[调试] 指定配置文件: %s\n", optarg);
          strcpy (yuconfig->file, optarg);
          break;
        case 'h' :
          usage (0);
          ret = 0;
          break;
        case 'V' :
          printf (_("%s : Version %s\n"), PROGRAME_NAME, VERSION);
          ret = 0;
          break;
        case 'u':
          printf ("get the '--baseurl' : %s\n", optarg);
          break;
        default :
          usage (1);
          ret = 1;
          break;
        }
    }

  // 玩
  if (verbose_flag)
    puts ("verbose flag is set");


  if (ret != 0)
    return optind;
  else
    return -1;
}


