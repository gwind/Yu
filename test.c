#include "yu.h"
#include "checksum.h"

int main (int argc, char **argv)
{

  unsigned char local_sha[41]={'\0'};
  unsigned char sha256[81]={'\0'};
  yu_checksum_sha1 (local_sha, argv[1]);
  yu_checksum_sha256 (sha256, argv[1]);


  printf ("%s\n", local_sha);
  printf ("%s\n", sha256);

  int ret=0;
  ret = yu_checksum_compare_file_sha(argv[2],argv[1],argv[3]);

  printf ("ret = %d\n", ret);

  return 0;
}
