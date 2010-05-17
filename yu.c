#include "yu.h"
#include "opt.h"
#include "load_config.h"
#include "dispatcher.h"
#include "yu_error.h"
#include "usage.h"
#include "yu_debug.h"


static void __yu_init_struc_YuConfig (YuConfig *yuconfig);


// 主程序
int main (int argc, char **argv)
{
  int ret=0, myoptind;
  // YuConfig 保存程序运行时的所有配置信息，主要包括 YuOpt 和 YuRepo
  YuConfig *yuconfig;

  // 初始化 yuconfig 默认值
  yuconfig = (YuConfig *)malloc (sizeof (YuConfig));
  __yu_init_struc_YuConfig(yuconfig);

  // 处理命令行参数，返回处理的参数个数
  myoptind = yu_main_opt(argc, argv, yuconfig);
  if (myoptind < 0)
    {
      if (myoptind == -1)
        {
          usage ();
          return 0;
        }
      else
        return yu_err (YU_OPT_ERR);
    }


  // 读配置文件
  ret = yu_read_config (yuconfig);
  if (ret != 0)
    return yu_err (YU_CONFIG_ERR);


  // 处理完命令行选项，并读完配置文件，在这里 DEBUG 配置信息合适
  //yu_print_struc_YuConfig (yuconfig);


  // 处理剩余的命令行参数，未处理参数个数为 argc - optind
  //printf ("argc=%d, myoptind=%d\n", argc, myoptind);
  ret = yu_cmd_dispatcher (argc - myoptind, argv + myoptind);
  if (ret < 0)
    usage ();
  

  return 0;
}


static void
__yu_init_struc_YuConfig (YuConfig *yuconfig)
{
  strcat (yuconfig->file, DF_YUCONFIG_FILE);

  // YuOpt
  strcat (yuconfig->opt.repomd, DF_YUCONFIG_OPT_REPOMD);
  strcat (yuconfig->opt.cachedir, DF_YUCONFIG_OPT_CACHEDIR);
  yuconfig->opt.keepcache = DF_YUCONFIG_OPT_KEEPCACHE;
  yuconfig->opt.debuglevel = DF_YUCONFIG_OPT_DEBUGLEVEL;
}
