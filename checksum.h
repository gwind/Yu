extern int 
yu_checksum_compare_file_sha (unsigned char *hex_str,
                              char *file,
                              char *sha_type);


extern int
yu_checksum_sha1 (unsigned char *hex_str, char *file);


extern int
yu_checksum_sha256 (unsigned char *hex_str, char *file);
