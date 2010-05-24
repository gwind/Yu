#include <unistd.h>
#include <time.h>
#include <sqlite3.h>
#include <dirent.h>

#include "yu.h"
#include "util.h"
#include "dl.h"
#include "checksum.h"
#include "sql.h"


static int
__yu_callback (int *remove_flag, int argc, char **argv, char **value);

extern int
yu_sql_mirror_primary (char *the_baseurl, 
                       char *the_repodir,
                       char *primary_data_name)
{
  char *baseurl, *repodir=NULL;
  baseurl = yu_make_sure_last_slash_exist (the_baseurl);
  repodir = yu_make_sure_last_slash_exist (the_repodir);

  sqlite3 *db;
  char *zErrMsg = NULL;
  char **dbResult;
  int i,j,ret,index,nRow,nColumn;
  char *pstr;

  char *sql = "select pkgId, name, arch, location_href, location_base, checksum_type from packages;";
  char sha[LINE_LENGTH_MAX]={'\0'};
  char sha_type[16];

  time_t timep;
  struct tm *p;
  time (&timep);
  p = localtime (&timep);
  
  char dl_file[LINE_LENGTH_MAX]={'\0'};
  char save_file[LINE_LENGTH_MAX]={'\0'};
  char location_base[LINE_LENGTH_MAX]={'\0'};
  char location_href[LINE_LENGTH_MAX]={'\0'};
  char name[LINE_LENGTH_MAX/2]={'\0'};
  char arch[16]={'\0'};
  char regstr[LINE_LENGTH_MAX/2]={'\0'};
  strcpy (dl_file, repodir);
  strcat (dl_file, "/");
  strcat (dl_file, YU_MIRROR_REPODATA_NAME);
  strcat (dl_file, "/");
  strcat (dl_file, primary_data_name);

  if (0 != access (dl_file, R_OK))
    {
      printf (_("The file doesn't exist: %s\n"), dl_file);
      return 1;
    }

  ret = sqlite3_open (dl_file, &db);
  if (ret)
    {
      fprintf (stderr, "Can't open database %s: %s\n",
               dl_file, sqlite3_errmsg(db));
      sqlite3_close(db);
      return 2;
    }

  ret = sqlite3_get_table (db, sql, &dbResult, &nRow, &nColumn, &zErrMsg);

  if (SQLITE_OK != ret)
    {
      printf (_("Search sql error: %s\n"), zErrMsg);
      goto clean;
    }

  index = nColumn;
  printf (_("%d data have been searched.\n"), nRow);
  for (i=0; i<nRow; i++)
    {
      pstr=NULL;
      for (j=0; j<nColumn; j++)
        {
          if (! strcmp (dbResult[j], "pkgId"))
            strcpy (sha, dbResult[index]);

          if (! strcmp (dbResult[j], "checksum_type"))
            strcpy (sha_type, dbResult[index]);

          if (! strcmp (dbResult[j], "location_base") &&
              (dbResult[index] != NULL) &&
              (dbResult[index][0] != '=') &&  // 还真遇到有这个例子 "=/root/lab/createrepo/"
              (dbResult[index][0] != '/'))
            strcpy (location_base, dbResult[index]);

          if (! strcmp (dbResult[j], "location_href"))
            strcpy(location_href, dbResult[index]);

          if (! strcmp (dbResult[j], "name"))
            strcpy (name, dbResult[index]);

          if (! strcmp (dbResult[j], "arch"))
            strcpy (arch, dbResult[index]);

          ++ index;
        }

      // 确定 dl_file
      if (strlen(location_base) != 0)
        strcpy (dl_file, location_base);
      else
        strcpy (dl_file, baseurl);
      strcat (dl_file, "/");
      strcat (dl_file, location_href);

      // 确定 save_file
      pstr = strrchr(location_href, '/');
      if (NULL != pstr)
        pstr = pstr + 1;
      else
        pstr = location_href;
      yu_concatenation_str_together(save_file,repodir,pstr);

      // 显示信息样式
      printf ("%d/%d - %s ", i+1, nRow, pstr);

      // 如果文件不存在，先下载
      if (0 != access (save_file, R_OK))
        {
          // 还没有想到一个更好的同步方法，先用这
          if (strlen(name) == 0)
            printf(_("SQL error: no 'name' found!\n"));
          else
            {
              strcpy (regstr, name);
              strcat (regstr, "-[0-9].*");
              strcat (regstr, arch);
              strcat (regstr, ".*");
              yu_remove_file_in_regex (repodir, regstr);
            }
          yu_dl_resume_and_progress_bar (dl_file, save_file);
        }

      // 第一次校验
      ret = yu_checksum_compare_file_sha ((unsigned char *)sha, save_file, sha_type);
      if (0 != ret)
        {
          // 第一次校验不对，再下载一次，一般为断点恢复下载提供便利
          yu_dl_resume_and_progress_bar (dl_file, save_file);
          // 第二次校验
          ret = yu_checksum_compare_file_sha ((unsigned char *)sha, save_file, sha_type);
          if (0 != ret)
            {
              // 第二次校验不对就删除文件，重新下载
              remove(save_file);
              yu_dl_resume_and_progress_bar (dl_file, save_file);
              // 第三次校验
              ret = yu_checksum_compare_file_sha ((unsigned char *)sha, save_file, sha_type);
              if (0 != ret)
                // 第三次校验不对就直接输出 FAILED 信息
                printf ("FAILED\n");
            }
        }

      printf ("\n");
    }


  // 更新完毕后，开始删除本地旧文件
  printf ("Remove old file in the %s\n", repodir);
  strcpy (dl_file, repodir);
  strcat (dl_file, "/");
  strcat (dl_file, YU_MIRROR_REPODATA_NAME);
  strcat (dl_file, "/");
  strcat (dl_file, primary_data_name);
  yu_remove_file_doesnt_in_primarydb (repodir, dl_file);

 clean:
  sqlite3_free_table (dbResult);
  sqlite3_close (db);
  free (baseurl);
  free (repodir);
  return 0;
}



// 从 primary 的 sqlite 数据库读取信息，删除本地目录下旧文件
extern int
yu_remove_file_doesnt_in_primarydb (char *dir, char *dbfile)
{
  DIR *pdir=NULL;
  struct dirent *pdirent;
  sqlite3 *db;  
  char *errmsg = NULL;
  char sql[LINE_LENGTH_MAX] = {'\0'};
  int ret=0;

  if (0 != access (dbfile, R_OK))
    {
      printf (_("The file doesn't exist: %s\n"), dbfile);
      return 1;
    }

  if ((pdir = opendir (dir)) == NULL)
    {
      printf (_("Open Directory Error: %s\n"), dir);
      return 2;
    }

  ret = sqlite3_open (dbfile, &db);
  if (ret)
    {
      fprintf (stderr, "Can't open database %s: %s\n",
               dbfile, sqlite3_errmsg(db));
      sqlite3_close(db);
      return 2;
    }

  int remove_flag=0;
  char file[LINE_LENGTH_MAX]={'\0'};
  while ((pdirent = readdir (pdir)) != NULL)
    {
      if (! strcmp (pdirent->d_name, ".") ||
          ! strcmp (pdirent->d_name, "..") ||
          ! strcmp (pdirent->d_name, "repodata") ||
          ! strcmp (pdirent->d_name, YU_MIRROR_REPODATA_NAME))
        continue;
      //.separator "-"; select name,version,release,arch from packages;
      sprintf(sql, "select location_href from packages where location_href like \"%%%s\";", pdirent->d_name);
      //printf ("sql = %s\n", sql);
      // 这是一个 hack 的方法，只要 sqlite3_exec 查询到结果就调用回调函数，就可以在其内设置 remove_flag 
      remove_flag = 1;
      ret = sqlite3_exec(db, sql, __yu_callback, &remove_flag, &errmsg);
      if (SQLITE_OK != ret)
        printf (_("Search sql error: %s\n"), errmsg);

      if (remove_flag == 1)
        {
          strcpy (file, dir);
          strcat (file, "/");
          strcat (file, pdirent->d_name);
          printf ("Remove: %s\n", file);
          remove(file);
        }
    }

  sqlite3_close (db);
  return 0;
}


// sqlite3_exec 的回调函数
static int
__yu_callback (int *remove_flag, int argc, char **argv, char **value)
{
  *remove_flag=0;
  return 0;
}
