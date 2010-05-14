#include "yu.h"
#include "yu_debug.h"


extern void yu_print_struc_YuConfig (YuConfig *yuconfig)
{
  YuRepo *yurepo;
  printf("\n\
Defind:\n\n\
typedef struct\n\
{\n\
  char file[LINE_LENGTH_MAX/8];\n\
  YuOpt opt;\n\
  YuRepo *repo;\n\
} YuConfig;\n\n\
Value:\n\n\
{\n");

  yu_print_struc_YuOpt (yuconfig->opt);

  if (yuconfig->repo != NULL)
    {
      yurepo = yuconfig->repo;
      do { yu_print_struc_YuRepo (*yurepo); }
      while((yurepo = yurepo->next) != NULL);
    }

  printf ("}\n");
}


// 输出 YuOpt 结构信息
extern void yu_print_struc_YuOpt (YuOpt opt)
{
  // sub struc YuOpt
  printf ("  [YuOpt] opt {\n");

  printf ("    cachedir => \"%s\"\n", opt.cachedir);
  printf ("    keepcache = %d\n", opt.keepcache);
  printf ("    debuglevel = %d\n", opt.debuglevel);
  printf ("    repomd[16] => \"%s\"\n", opt.repomd);

  printf ("  }\n");
}


// 输出 YuRepo 结构信息
extern void yu_print_struc_YuRepo (YuRepo repo)
{
  // sub struc YuRepo
  printf ("  [YuRepo] repo {\n");

  printf ("    name => \"%s\"\n", repo.name);
  printf ("    summary => \"%s\"\n", repo.summary);
  printf ("    baseurl => \"%s\"\n", repo.baseurl);
  printf ("    enabled = %d\n", repo.enabled);

  printf ("  }\n");
}
