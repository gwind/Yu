#include <errno.h>
#include <unistd.h>

#include "common.h"

static int __yu_mirror (char *url, char *localdir);

static int 
__yu_mirror_update_localdir (char *url, char *localdir);


/* mirror 命令 */
extern int yu_cmd_mirror (int argc, char **argv)
{
  printf ("argc = %d\n", argc);
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
    }
  else
    strcpy (localdir, argv[2]);


  __yu_mirror(argv[1], localdir);
  return 0;
}



static int __yu_mirror (char *url, char *localdir)
{
  printf (_("Run: %s mirror %s %s\n"), 
          PROGRAME_NAME, url, localdir);
  
  __yu_mirror_update_localdir(url, localdir);
  printf ("last localdir = %s\n", localdir);
  return 0;
}


/* 使用 url 的最后一级目录作为本地目录 */
static int 
__yu_mirror_update_localdir (char *url, char *localdir)
{
  char *pstr=NULL;
  char tmpstr[LINE_LENGTH_MAX];
  int url_length = strlen(url);
  strcpy(tmpstr, url);
  if ( *(tmpstr + url_length - 1) == '/')
    *(tmpstr + url_length - 1) = '\0';
  pstr = strrchr (tmpstr, '/');
  strcat (localdir, "/");
  strcat (localdir, pstr+1);
  yu_str_replace (localdir, "//", "/");
  return 0;
}
