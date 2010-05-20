#include <unistd.h>
#include <time.h>
#include <sqlite3.h>

#include "yu.h"
#include "util.h"
#include "dl.h"
#include "checksum.h"


extern int
yu_sql_mirror_primary (char *the_baseurl, char *the_repodir)
{
  char *baseurl, *repodir=NULL;
  baseurl = yu_make_sure_last_slash_exist (the_baseurl);
  repodir = yu_make_sure_last_slash_exist (the_repodir);

  sqlite3 *db;
  char *zErrMsg = NULL;
  char **dbResult;
  int i,j,ret,index,nRow,nColumn;
  char *pstr;

  
  char *sql = "select pkgId, location_href, location_base, checksum_type from packages;";
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
  yu_concatenation_str_together (dl_file, repodir, "repodata/primary.sqlite");

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
              (dbResult[index] != NULL))
            strcpy (location_base, dbResult[index]);

          if (! strcmp (dbResult[j], "location_href"))
            strcpy(location_href, dbResult[index]);

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
      if (NULL == pstr)
        pstr = location_href;
      yu_concatenation_str_together(save_file,repodir,pstr+1);

      // 显示信息样式
      printf ("%d/%d - %s ", i+1, nRow, pstr+1);

      if (0 != access (save_file, R_OK))
        {
          yu_dl_resume_and_progress_bar (dl_file, save_file);
          continue;
        }
      
      ret = yu_checksum_compare_file_sha ((unsigned char *)sha, save_file, sha_type);
      if (0 == ret)
        printf ("OK\n");
      else
        printf ("FAILED\n");
    }

 clean:
  sqlite3_free_table (dbResult);
  sqlite3_close (db);
  free (baseurl);
  free (repodir);
  return 0;
}
