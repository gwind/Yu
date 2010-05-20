#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h> // read
#include <fcntl.h>
#include <openssl/sha.h>

#include "yu.h"
#include "util.h"
#include "checksum.h"


#define BUFFER_SIZE 2048


/* 先用指定 sha 类型选择加密算法计算指定文件的校验值，
 * 再与指定的字符串比较，返回比较结果，0为正确。
 */
extern int 
yu_checksum_compare_file_sha (unsigned char *hex_str,
                              char *file,
                              char *sha_type)
{
  if (0 == strlen(sha_type))
    return 11;

  int ret=0;
  unsigned char *local_sha;
  local_sha = (unsigned char *) malloc (LINE_LENGTH_MAX);

  if (0 == strcmp (sha_type, "sha"))
    yu_checksum_sha1 (local_sha, file);

  if (0 == strcmp (sha_type, "sha256"))
    yu_checksum_sha256 (local_sha, file);

  ret = strcmp ((char *)hex_str, (char *)local_sha);
  free (local_sha);
  return ret;
}


// 计算 sha1 (sha 即 sha1)
extern int
yu_checksum_sha1 (unsigned char *hex_str, char *file)
{
  SHA_CTX s;
  int bytes_read, from_fd, i, j;
  unsigned char md[SHA_DIGEST_LENGTH] = {'\0'};
  char buffer[BUFFER_SIZE] = {'\0'};

  SHA1_Init(&s);

  from_fd = open (file, O_RDONLY);
  if (from_fd <= 0)
    {
      printf (_("Open file error: %s\n"), file);
      return 1;
    }

  while (0 != (bytes_read = read(from_fd, buffer, BUFFER_SIZE)))
    {
      if ((bytes_read == -1))
        break;
      else
        SHA1_Update (&s, buffer, bytes_read);
    }

  SHA1_Final (md, &s);
  for (i=0,j=0; i<SHA_DIGEST_LENGTH; i++, j+=2)
    {
      hex_str[j] = yu_decimal_to_hex_char (md[i]/16);
      hex_str[j+1] = yu_decimal_to_hex_char (md[i]%16);
    }

  hex_str[j] = '\0';
  close (from_fd);

  return 0;
}


extern int
yu_checksum_sha256 (unsigned char *hex_str, char *file)
{
  SHA256_CTX s;
  int bytes_read, from_fd, i, j;
  unsigned char md[SHA256_DIGEST_LENGTH] = {'\0'};
  char buffer[BUFFER_SIZE] = {'\0'};

  SHA256_Init(&s);

  from_fd = open (file, O_RDONLY);
  if (from_fd <= 0)
    {
      printf (_("Open file error: %s\n"), file);
      return 1;
    }

  while (0 != (bytes_read = read(from_fd, buffer, BUFFER_SIZE)))
    {
      if ((bytes_read == -1))
        break;
      else
        SHA256_Update (&s, buffer, bytes_read);
    }

  SHA256_Final (md, &s);
  for (i=0,j=0; i<SHA256_DIGEST_LENGTH; i++, j+=2)
    {
      hex_str[j] = yu_decimal_to_hex_char (md[i]/16);
      hex_str[j+1] = yu_decimal_to_hex_char (md[i]%16);
    }

  hex_str[j] = '\0';
  close (from_fd);

  return 0;
}
