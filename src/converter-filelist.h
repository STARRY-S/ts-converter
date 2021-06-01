#ifndef _FILELIST_H_
#define _FILELIST_H_

#ifndef FILE_LIST_SIZE
#define FILE_LIST_SIZE 512
#endif

int init_file_list();
int release_file_list();
int insert_back(const char *const path, const char *const name);
int insert_head(const char *const path, const char *const name);
int insert_sort(
        const char *const path,
        const char *const name,
        int (*ssort) (const char *const str1, const char *const str2)
);
int default_str_is_larger(const char *const str1, const char *const str2);
int calculate_file_id();
int get_file_list_size();
void print_file_list();

#endif
