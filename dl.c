/*
 * 使用 curl 库的下载 api
 */

#include "yu.h"
#include "dl.h"

#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>


static size_t 
__yu_dl_write (char *ptr, size_t size,
               size_t nmemb, void *stream);

static int
__yu_dl_progress (//void *x,
                  //double *l_size,
                  YuDlFileData *fdata,
                  double t, double d,
                  double ultotal, double ulnow);


// 定义函数指针，是为了满足 curl 的格式检查，详见本文件结尾 2.
size_t (*yu_write) 
(char *ptr, size_t size,
 size_t nmemb, void *stream) = &__yu_dl_write;

int (*yu_progress) 
(YuDlFileData *fdata,double t, double d,
 double ultotal, double ulnow) = &__yu_dl_progress;


/* 可恢复断点的下载 */
extern int
yu_dl_resume_and_progress_bar (char *url, 
                         char *save_name)
{
  CURL *curl;
  FILE *fp;
  time_t start_sec;
  curl_off_t local_file_size = -1;
  CURLcode ret = CURLE_GOT_NOTHING;
  struct stat file_stat;
  int use_resume = 0;
  YuDlFileData *fdata;
  fdata = (YuDlFileData *) malloc (sizeof (YuDlFileData));
  strcpy(fdata->name, save_name);

  curl_global_init (CURL_GLOBAL_ALL);
  curl = curl_easy_init ();

  // 计算本地文件大小，并选择是否需要续传
  if (stat (save_name, &file_stat) == 0)
    {
      local_file_size = file_stat.st_size;
      fdata->size = (double) local_file_size;
      use_resume = 1;
    }

  if ((fp = fopen (save_name, "ab+")) == NULL)
    {
      printf (_("Open file %s have a error!\n"), save_name);
      curl_easy_cleanup (curl);
      return 1;
    }

  // 设置 CURLOPT_VERBOSE 为 1 ，可以看到详细的处理过程
  //curl_easy_setopt (curl, CURLOPT_VERBOSE, 1L);
  curl_easy_setopt (curl, CURLOPT_URL, url);
  curl_easy_setopt (curl, CURLOPT_RESUME_FROM_LARGE,
                    use_resume?local_file_size:0);
  curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, yu_write);
  curl_easy_setopt (curl, CURLOPT_WRITEDATA, fp);

  // 设置进度显示
  curl_easy_setopt (curl, CURLOPT_NOPROGRESS, 0L);
  curl_easy_setopt (curl, CURLOPT_PROGRESSFUNCTION, 
                    yu_progress);
  curl_easy_setopt (curl, CURLOPT_PROGRESSDATA, fdata);

  // 设置执行开始时间
  //printf ("%s                  ", save_name);
  /*
  char *pstr=NULL;
  pstr = strrchr (save_name, '/');
  printf ("%s                  ", pstr+1);
  */
  fflush (stdout); // 将现有的缓冲区立即输出更友好
  printf ("    "
          "       "
          "     ");
  fdata->start = time(&start_sec);
  ret = curl_easy_perform (curl);

  if (ret != CURLE_OK)
    {
      printf (_("%s\n"), curl_easy_strerror(ret));
      return 1;
    }

  //printf ("\n");

  fclose (fp);
  curl_easy_cleanup (curl);
  free(fdata);
  
  return 0;
}


// 输出 curl 信息
extern char *
yu_dl_with_curl_version (void)
{
  return curl_version();
}


static size_t
__yu_dl_write (char *ptr, size_t size,
               size_t nmemb, void *stream)
{
  return fwrite (ptr, size, nmemb, stream);
}



// 符合 <curl/curl.h> 中 curl_progress_callback 标准
static int
__yu_dl_progress (//void *x,
                  //double *l_size,
                  YuDlFileData *fdata,
                  double t, // 下载时总大小
                  double d, // 已经下载大小
                  double ultotal, // 上传是总大小
                  double ulnow)   // 已经上传大小
{
  time_t now_sec;
  time(&now_sec);
  if ((t - 0.0 > 0.001) &&
      now_sec - fdata->start > 0.001)
    {
      double cur_d = d + fdata->size;
      double ture_t = t + fdata->size;
      double speed;
      speed = (double)d/((now_sec - fdata->start)*1024);
      printf ("\b\b\b"
              "\b\b\b\b\b\b\b"
              "\b\b\b\b\b");
      if (speed > 1000.0)
        printf ("%6.1fMB/s", speed/1024);
      else
        printf ("%6.1fKB/s", speed);
      printf ("%4.0f%%", cur_d*100.0/ture_t);
      fflush (stdout);
    }
  return 0;
}



/* libcurl 编程 */

/* 1. -- 初始化
 * CURLcode curl_global_init(long flags);

 * 这个函数只能用一次。(其实在调用curl_global_cleanup 函数后
 * 仍然可再用) 如果这个函数在curl_easy_init函数调用时还没调用，
 * 它讲由libcurl库自动完成。

 * 参数：flags

 CURL_GLOBAL_ALL      //初始化所有的可能的调用。
 CURL_GLOBAL_SSL      //初始化支持 安全套接字层。
 CURL_GLOBAL_WIN32    //初始化win32套接字库。
 CURL_GLOBAL_NOTHING  //没有额外的初始化。

*/


/* 1. -- 数据传输进度相关的参数
 * CURLOPT_NOPROGRESS 
 * CURLOPT_PROGRESSFUNCTION
 * CURLOPT_PROGRESSDATA
 *
 * CURLOPT_PROGRESSFUNCTION 指定的函数默认情况下每秒被 libcurl
 * 调用一次，为了使 CURLOPT_PROGRESSFUNCTION 被调用，
 * CURLOPT_NOPROGRESS 必须被设置为 false，CURLOPT_PROGRESSDATA
 * 指定的参数将作为 CURLOPT_PROGRESSFUNCTION 指定函数的第一个参数
 */


/* 2. -- callback 类型函数格式
 * CURLOPT_WRITEFUNCTION 和 CURLOPT_PROGRESSFUNCTION 
 * 都传递一个特定格式的函数指针，格式在 <curl.h> 中有定义，在
 * <typecheck-gcc.h> 中有检查流程，并有几个变通的格式

 * curl_write_callback 格式:

typedef size_t (*curl_write_callback)(char *buffer,
                                      size_t size,
                                      size_t nitems,
                                      void *outstream);

 * curl_progress_callback 格式：

typedef int (*curl_progress_callback)(void *clientp,
                                      double dltotal,
                                      double dlnow,
                                      double ultotal,
                                      double ulnow);

 * 如果没有定义正确的格式就会出现类似下面错误：

dl.c:61: warning: call to ‘_curl_easy_setopt_err_write_callback’ declared with attribute warning: curl_easy_setopt expects a curl_write_callback argument for this option

dl.c:67: warning: call to ‘_curl_easy_setopt_err_progress_cb’ declared with attribute warning: curl_easy_setopt expects a curl_progress_callback argument for this option

*/
