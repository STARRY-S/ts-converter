#ifndef __CONVERTER_FILELIST_H__
#define __CONVERTER_FILELIST_H__

#ifndef FILE_LIST_SIZE
#define FILE_LIST_SIZE 512
#endif

struct video {
        int id;
        int file_format;
        char* path;
        char* name;

        struct video *next;
        struct video *prev;
};

/* 双向链表 */
struct List {
        struct video *begin;
        struct video *end;
        unsigned size;
};

struct List *converter_filelist_get_list();
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
