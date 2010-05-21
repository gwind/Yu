extern void yu_str_replace (char *str, 
                            char *oldstr, char *newstr);

extern int yu_decompress_file (char *file);

extern char yu_decimal_to_hex_char (int dec);
extern char yu_itoa (unsigned char i);

extern int yu_make_sure_dir_exist (char *dir);
extern char * yu_make_sure_last_slash_exist (char *str);
extern char * yu_concatenation_str (char *astr, char *bstr);
extern int yu_concatenation_str_together (char *str, char *astr, char *bstr);

extern char *yu_get_filename_in_dir (char *dir, char *regex);
