#include "yu.h"


void usage (void) {

  fputs (_("\
Yu is faster and smaller than Yum.\n\n\
"), stdout);

  printf (_("\
Usage : %s [OPTION] COMMAND [PARAMENTS]\n\n\
"), program_name);

  fputs (_("\
  -c, --config        Specify the config file for yu\n\
                      default is /etc/yu.conf\n\
  -b, --background    run background, the log is write to log file,\n\
                      log file specified by -l\n\
  -l, --log           the log file, default is /var/log/cum.log\n\
  -v, --verbose       printf more info\n\
"), stdout);

  // Print the avilable command.
  fputs (_("\
install           install pkgs\n\
download|dl       download pkgs, don't install it.\n\
update            update the pkgs installed in system.\n\
search            search pkg name.\n\
file              search the owner pkg of file.\n\
mirror            mirror a repodata from url to localdir.\n\
"), stdout);

}
