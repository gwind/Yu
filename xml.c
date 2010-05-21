#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h> // access
#include <fcntl.h>
#include <libxml/parser.h>

#include "yu.h"
#include "dl.h"
#include "util.h"
#include "xml.h"
#include "checksum.h"


static int
__yu_xml_dl_repodata (xmlDocPtr doc, 
                      char *baseurl,
                      char *local_repodata_dir);

static int
__yu_xml_do_children_node_of_repomd 
(xmlNodePtr children, char *baseurl, char *local_repodata_dir);

static int
__yu_xml_dl_and_decompress(char *baseurl,
                           char *href,
                           char *local_file);


// 更新 repodata 数据库
extern int
yu_xml_update_repodata (char *address, char *dir)
{
  int ret=0;
  xmlDocPtr doc=NULL;

  char *baseurl, *local_repodata_dir;
  baseurl = yu_make_sure_last_slash_exist (address);
  local_repodata_dir = yu_make_sure_last_slash_exist (dir);
  yu_make_sure_dir_exist (local_repodata_dir);

  char url[LINE_LENGTH_MAX]={'\0'};
  yu_concatenation_str_together(url, baseurl, "repodata/repomd.xml");
  char local_file[LINE_LENGTH_MAX]={'\0'};
  yu_concatenation_str_together(local_file, local_repodata_dir, "repomd.xml");

  // 检查本地 repomd.xml 文件是否存在
  if (0 != access (local_file, R_OK))
    {
      //printf ("Get %s ", strrchr(url, '/') + 1);
      printf ("Get %s ", url);
      yu_dl_resume_and_progress_bar (url, local_file);
    }
  else
    {
      // 如果 repomd.xml 太旧，早于 3600 秒之前，就更新它
      time_t curtime;
      struct stat buf;

      stat (local_file, &buf);
      time (&curtime);
      int diff_time = (unsigned long) curtime - buf.st_mtime;

      if (diff_time > 3600)
        {
          remove (local_file);
          printf ("Get %s ", strrchr(url, '/') + 1);
          yu_dl_resume_and_progress_bar (url, local_file);
        }
    }

  // 打开 repomd.xml 文件
  xmlKeepBlanksDefault (0);
  doc = xmlReadFile (local_file, "UTF-8", XML_PARSE_RECOVER);
  if (doc == NULL)
    {
      fprintf (stderr, _("Read xml file have a error: %s\n"), local_file);
      return 1;
    }

  __yu_xml_dl_repodata(doc, baseurl, local_repodata_dir);

  xmlFreeDoc (doc);
  xmlCleanupParser ();
  free (baseurl);
  free (local_repodata_dir);
  return ret;
}


static int
__yu_xml_dl_repodata (xmlDocPtr doc, 
                      char *baseurl,
                      char *local_repodata_dir)
{
  xmlNodePtr root=NULL, curnode=NULL;
  char *type=NULL;

  // 获取 doc 指向的 xmlDocPtr 结构对象的 root 节点
  if ((root = xmlDocGetRootElement (doc)) == NULL)
    {
      fprintf (stderr, _("No root node have found, wrong XML file.\n"));
      return 2;
    }

  curnode = root->children;

  do {
    if (0 != xmlStrcmp (curnode->name, BAD_CAST "data"))
      continue;

    type = (char *) xmlGetProp (curnode, BAD_CAST "type");

    /*
    // 过滤不想处理的子节点
    if ((0 != strcmp (type, "other_db")) &&
        (0 != strcmp (type, "filelists_db")) &&
        (0 != strcmp (type, "primary_db")))
      continue;
    */

    // 处理子节点
    if (0 != __yu_xml_do_children_node_of_repomd (curnode->children, baseurl, local_repodata_dir))
      return 1;

  } while ((curnode  = curnode->next) != NULL);

  return 0;
}


static int
__yu_xml_do_children_node_of_repomd 
(xmlNodePtr children,
 char *baseurl,
 char *local_repodata_dir)
{
  char local_file[LINE_LENGTH_MAX] = {'\0'};
  char *href=NULL;
  unsigned char *sha=NULL;
  char sha_type[16]={'\0'};

  // 循环判断字节点的名字，取得 "location" 和 
  // "open-checksum" 节点的信息，处理之
  do {

    if (0==xmlStrcmp(children->name,BAD_CAST"location"))
      {
        href = (char *)xmlGetProp(children,BAD_CAST"href");
        strcpy (local_file, local_repodata_dir);
        strcat (local_file, strrchr (href, '/') + 1);
      }

    if (0==xmlStrcmp(children->name,BAD_CAST"open-checksum"))
      {
        strcpy(sha_type, (char *)xmlGetProp(children,BAD_CAST"type"));
        sha = (unsigned char *)xmlNodeGetContent(children);
      }

  } while ((children = children->next) != NULL);

  char decompressed_file[LINE_LENGTH_MAX]={'\0'};
  char *pstr=NULL;
  strcpy (decompressed_file, local_file);
  if (0 != (pstr = strstr (decompressed_file, ".bz2")) ||
      0 != (pstr = strstr (decompressed_file, ".gz")))
    *pstr = '\0';

  // 如果解压缩后的文件不存在，下载之，并解压缩
  if (0 != access (decompressed_file, R_OK) &&
      (0 != __yu_xml_dl_and_decompress(baseurl,href,
                                       local_file)))
        return 1;

  // 计算指定sha类型的文件校验，并与指定sha比较，返回比较结果
  // 此处计算的是解压后文件的sha校验，对应 repomd.xml 中的
  // <open-checksum> 节点的信息
  int ret = yu_checksum_compare_file_sha (sha, decompressed_file, sha_type);
  if (0 == ret)
    return 0;
  if (11 != ret)
    {
      printf ("Remove %s\n", decompressed_file);
      remove (decompressed_file);
    }

  return  __yu_xml_dl_and_decompress(baseurl, href, local_file);
}


static int
__yu_xml_dl_and_decompress(char *baseurl,
                           char *href,
                           char *local_file)
{
  char url[LINE_LENGTH_MAX]={'\0'};
  int ret=0;
  strcpy (url, baseurl);
  strcat (url, href);
  printf ("Get %s ", strrchr(url, '/') + 1);
  ret = yu_dl_resume_and_progress_bar (url, local_file);
  printf ("\n");
  if (ret != 0)
    return ret;
  ret = yu_decompress_file (local_file);
  if (ret != 0)
    return ret;

  return 0;
}
