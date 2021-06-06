#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <glib.h>

#include "converter-filelist.h"

typedef struct video SV;

/*
 * 双向链表
 * 与C++的迭代器不同：
 * begin: 指向第一个文件的前一个结点 [0]
 * end:   指向最后一个文件的结点，便于从尾端插入
 */
static struct List List = {
        .begin = NULL,
        .end = NULL,
        .size = 0
};

static const char *unknow_format = "UNKNOW";

int insert_at(const char *path,const char *name, SV* here);
bool isNumber(char cc);
bool isAlphabet(char cc);

/* public methods: */
struct List *converter_filelist_get_list()
{
        return &List;
}

int init_file_list()
{
        List.begin = List.end = (SV*) malloc(sizeof(SV));
        memset(List.begin, 0, sizeof(SV));
        if (List.begin == NULL) {
                fprintf(stderr, "[MALLOC ERROR] Failed to initialize list.\n");
                return -1;
        }
        List.begin->next = List.begin->prev = NULL;
        List.size = 0;
        return 0;
}

int release_file_list()
{
        if (List.begin == NULL) {
                return 0;
        }

        for (SV *p = List.begin; p != NULL; p = p->next)
        {
                SV* pp = p;
                free(pp->format);
                free(pp->path);
                free(pp->name);
                free(pp);
        }

        return 0;
}

int insert_back(const char *const path, const char *const name)
{
        if (path == NULL || name == NULL) {
                fprintf(stderr, "[ASSERT FAILED] path or name is NULL\n");
                return -1;
        }

        SV* p = (SV*) malloc(sizeof(SV));
        const char *format = strrchr(name, '.');
        if (format == NULL) {
                format = unknow_format;
        }
        int formatlen = strlen(format);
        int pathlen = strlen(path);
        int namelen = strlen(name);
        p->id = List.size + 1;
        p->path = (char*) malloc(pathlen * sizeof(char) + 1);
        p->name = (char*) malloc(namelen * sizeof(char) + 1);
        p->format = (char*) malloc(formatlen * sizeof(char) + 1);
        if (p->path == NULL || p->name == NULL || p->format == NULL) {
                fprintf(stderr, "[MALLOC ERROR] Failed to insert back.\n");
                free(p->path);
                free(p->name);
                free(p->format);
                return -1;
        }
        strcpy(p->path, path);
        strcpy(p->name, name);
        strcpy(p->format, format);
        p->next = NULL;
        p->prev = List.end;

        List.end->next = p;
        List.end = p;
        p = NULL;
        ++List.size;

        return 0;
}

int insert_head(const char *const path, const char *const name)
{
        if (path == NULL || name == NULL) {
                fprintf(stderr, "[ASSERT FAILED] path or name is NULL\n");
                return -1;
        }

        SV* p = (SV*) malloc(sizeof(SV));

        const char *format = strrchr(name, '.');
        if (format == NULL) {
                format = unknow_format;
        }
        int formatlen = strlen(format);
        int pathlen = strlen(path);
        int namelen = strlen(name);
        p->id = List.size + 1;
        p->path = (char*) malloc(pathlen * sizeof(char) + 1);
        p->name = (char*) malloc(namelen * sizeof(char) + 1);
        p->format = (char*) malloc(formatlen * sizeof(char) + 1);
        if (p->path == NULL || p->name == NULL || p->format == NULL) {
                fprintf(stderr, "[MALLOC ERROR] Failed to insert back.\n");
                free(p->path);
                free(p->name);
                free(p->format);
                return -1;
        }
        strcpy(p->path, path);
        strcpy(p->name, name);
        strcpy(p->format, format);

        // int pathlen = strlen(path);
        // int namelen = strlen(name);
        // p->id = List.size;
        // p->path = (char*) malloc(pathlen * sizeof(char) + 1);
        // p->name = (char*) malloc(namelen * sizeof(char) + 1);
        // if (p->path == NULL || p->name == NULL) {
        //         fprintf(stderr, "[MALLOC ERROR] Failed to insert head.\n");
        //         free(p->path);
        //         free(p->name);
        //         return -1;
        // }
        // strcpy(p->path, path);
        // strcpy(p->name, name);
        if (List.begin->next == NULL) {
                List.end = p;
        }
        p->next = List.begin->next;
        p->prev = List.begin;
        List.begin->next = p;
        p = NULL;
        ++List.size;

        return 0;
}

/* Customize sort function */
int insert_sort(
        const char *const path,
        const char *const name,
        int (*isbigger) (const char *const str1, const char *const str2))
{
        bool inserted = false;
        SV *p = NULL;

        if (List.begin == NULL) {
                return -1;
        }

        for (p = List.begin->next; p != NULL && !inserted; p = p->next) {
                if (isbigger(p->name, name)) {
                        insert_at(path, name, p);
                        inserted = true;
                }
        }

        if (!inserted && List.size == 0) {
                insert_head(path, name);
        } else if (!inserted) {
                insert_back(path, name);
        }

        return 0;
}

/*
 * 默认的字符串排序函数
 * 遇到数字时将数字转换文件名数字部分转换为整数比较大小
 * 比较整数之后再比较没有数字的部分
 */
int default_str_is_larger(const char *const str1, const char *const str2)
{
        if (str1 == NULL || str2 == NULL) {
                return -1;
        }

        int len1 = strlen(str1);
        int len2 = strlen(str2);
        int str1_num = 0;
        int str2_num = 0;

        int i = 0, j = 0;

        // rule:
        // 1234 is larger than 123
        // 1-10.txt is larger than 1-1.txt
        // 1-10-3.txt is larger than 1-10-2.txt
        // 2021-05-21.txt is larger than 2021-05-03.txt
        // 0003.txt is larger than 2.txt
        // 2.txt is same as 2.mp4
        for ( ; i < len1 && j < len2; )
        {
                if (str1[i] == str2[j]) {
                        ++i, ++j;
                        continue;
                }

                while (isNumber(str1[i])) {
                        str1_num *= 10;
                        str1_num += str1[i] - '0';
                        ++i;
                }

                while (isNumber(str2[j])) {
                        str2_num *= 10;
                        str2_num += str2[j] - '0';
                        ++j;
                }

                if (str1[i] != str2[i]) {
                        break;
                } else if (str1_num != str2_num) {
                        break;
                }
        }

        if (str1_num != str2_num) {
                return str1_num > str2_num;
        }

        return str1[i] > str2[i];
}

/* recalcuate file id after sort insert
 * return file list size, -1 on error.
 */
int calculate_file_id()
{
        if (List.begin == NULL) {
                return -1;
        }

        int i = 0;
        for (SV* p = List.begin; p != NULL; p = p->next)
        {
                p->id = i++;
        }

        return i-1;
}

int get_file_list_size()
{
        return List.size;
}

/* For debug */
void print_file_list()
{
        for (SV* p = List.begin->next; p != List.end; p = p->next)
        {
                printf("id: %d, path: %s\n", p->id, p->path);
        }
}

/* private methods: */
/* insert a node before "here" */
int insert_at(const char *path,const char *name, SV* here)
{
        if (path == NULL || name == NULL || here == NULL) {
                fprintf(stderr, "[ASSERT FAILED] path, name or here is NULL\n");
                return -1;
        }

        SV* p = (SV*) malloc(sizeof(SV));

        const char *format = strrchr(name, '.');
        if (format == NULL) {
                format = unknow_format;
        }
        int formatlen = strlen(format);
        int pathlen = strlen(path);
        int namelen = strlen(name);
        p->id = List.size + 1;
        p->path = (char*) malloc(pathlen * sizeof(char) + 1);
        p->name = (char*) malloc(namelen * sizeof(char) + 1);
        p->format = (char*) malloc(formatlen * sizeof(char) + 1);
        if (p->path == NULL || p->name == NULL || p->format == NULL) {
                fprintf(stderr, "[MALLOC ERROR] Failed to insert back.\n");
                free(p->path);
                free(p->name);
                free(p->format);
                return -1;
        }
        strcpy(p->path, path);
        strcpy(p->name, name);
        strcpy(p->format, format);

        // int pathlen = strlen(path);
        // int namelen = strlen(name);
        // p->id = 0;              /* calculate after insert*/
        // p->path = (char*) malloc(pathlen * sizeof(char) + 1);
        // p->name = (char*) malloc(namelen * sizeof(char) + 1);
        // if (p->path == NULL || p->name == NULL) {
        //         fprintf(stderr, "[MALLOC ERROR] Failed to insert head.\n");
        //         free(p->path);
        //         free(p->name);
        //         return -1;
        // }
        // strcpy(p->path, path);
        // strcpy(p->name, name);

        here->prev->next = p;
        p->prev = here->prev;
        p->next = here;
        here->prev = p;
        ++List.size;
        p = NULL;

        return 0;
}

bool isNumber(char cc)
{
        return (cc >= '0' && cc <= '9');
}

bool isAlphabet(char cc)
{
        if (cc >= 'a' && cc <= 'z') {
                return true;
        }

        if (cc >= 'A' && cc <= 'Z') {
                return true;
        }

        return false;
}
