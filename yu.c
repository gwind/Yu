#include "yu.h"
#include "opt.h"
#include "load_config.h"
#include "dispatcher.h"
#include "error.h"


// 主程序
int main (int argc, char **argv)
{
  int ret,myoptind;
  YuConfig *yuconfig;

  // 初始化 yuconfig
  yuconfig = (YuConfig *)malloc (sizeof (YuConfig));
  // 默认配置文件： /etc/yu/yu.conf
  strcat (yuconfig->file, "/etc/yu/yu.conf");

  // 处理命令行参数，返回处理的参数个数
  myoptind = yu_main_opt(argc, argv, yuconfig);
  if (myoptind < 0)
    return yu_err (YU_OPT_ERR);

  // 读配置文件
  ret = yu_read_config (yuconfig);
  if (ret != 0)
    return yu_err (YU_CONFIG_ERR);


  // 处理完命令行选项，并读完配置文件，在这里 DEBUG 配置信息合适
  printf ("[调试] 配置文件： %s\n", yuconfig->file);

  // 处理剩余的命令行参数，未处理参数个数为 argc - optind
  printf ("argc=%d, myoptind=%d\n", argc, myoptind);
  yu_cmd_dispatcher (argc - myoptind, argv + myoptind);

  return 0;
}


