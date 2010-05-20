#include "common.h"

/* yu_cmd_download 处理下载任务
 * 计划：先实现 yumdownload 功能
 */
int yu_cmd_download (int argc, char **argv)
{

  // 如果 download|dl 后面没有参数就退出
  printf ("argc = %d\n", argc);
  if (argc == 0)
    {
      printf (_("[download|dl] command must contant one arg\nTry --help to get usage.\n"));
      return 0;
    }

  char save_name[LINE_LENGTH_MAX]={'\0'};
  char url[LINE_LENGTH_MAX]={'\0'};

  printf ("[调试] libcurl 版本： %s\n", yu_dl_with_curl_version());



  strcpy (url, "http://download.turbolinux.com.cn/TMS%3a/2.0%3a/LessWatts/standard/src/powertop-1.11-5.src.rpm");
  strcpy (save_name, "powertop-1.11-5.src.rpm");
  //printf ("[get] %s\n", url);
  yu_dl_resume_and_progress_bar (url, save_name);

  strcpy (url, "http://download.turbolinux.com.cn/TMS%3a/2.0%3a/LessWatts/standard/src/kernel-2.6.30.1-6.1.3.src.rpm");
  strcpy (save_name, "kernel-2.6.30.1-6.1.3.src.rpm");
  //printf ("[get] %s\n", url);
  yu_dl_resume_and_progress_bar (url, save_name);


  strcpy (url, "http://download.turbolinux.com.cn/TMS%3a/2.0%3a/LessWatts/standard/src/xfce4-desktop-branding-moblin-0.6-3.2.src.rpm");
  strcpy (save_name, "xfce4-desktop-branding-moblin-0.6-3.2.src.rpm");
  //printf ("[get] %s\n", url);
  yu_dl_resume_and_progress_bar (url, save_name);


  return 0;
}
