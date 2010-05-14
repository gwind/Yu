#include "yu.h"
#include "commands/download.h"

// 分配命令
int yu_cmd_dispatcher (int argc, char **argv)
{
  if (argc < 1)
    return -1;

  // update
  if (0 == strcmp(argv[0], "update")) {
    printf ("[Command] update\n");
    return 0;
  }

  // install
  if (0 == strcmp (argv[0], "install")) {
    printf ("[Command] install\n");
    return 0;
  }

  // download
  if (0 == strcmp (argv[0], "download") ||
      0 == strcmp (argv[0], "dl")) {
    printf ("[Command] download|dl\n");
    return yu_cmd_download (argc - 1, argv);
  }

  // search
  if (0 == strcmp (argv[0], "search")) {
    printf ("[Command] search\n");
    return 0;
  }

  // mirror
  if (0 == strcmp (argv[0], "mirror")) {
    printf ("[Command] mirro\n");
    return 0;
  }

  // 如果到这里，说明遇到一个我们还没有支持的命令
  printf (_("I don't know the command: %s\n"), argv[0]);
  return -2;
}
