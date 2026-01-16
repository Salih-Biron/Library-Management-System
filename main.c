#include "data.h"
#include "logic.h"
#include "store.h"
#include <stdio.h>
#include <string.h>

#define PERSISTENCE_FILE "library_data.json"

/*
 * 功能：去除输入字符串末尾的换行符。
 */
static void trim_newline(char *text) {
    size_t len = strlen(text);
    if (len > 0 && text[len - 1] == '\n') {
        text[len - 1] = '\0';
    }
}

/*
 * 功能：打印单本图书信息到控制台。
 */
static void print_book(const BookNode *book) {
    if (!book) {
        return;
    }
    printf("ISBN:%s | 标题:%s | 作者:%s | 库存量:%d | 借阅量:%d\n",
           book->isbn, book->title, book->author, book->stock, book->loaned);
}

/*
 * 功能：打印图书链表（通常为搜索结果）。
 */
static void print_book_list(BookNode *head) {
    if (!head) {
        printf("未找到相关图书。\n");
        return;
    }

    for (BookNode *cur = head; cur != NULL; cur = cur->next) {
        print_book(cur);
    }
}

/*
 * 功能：借阅/归还等操作前的确认提示。
 */
static int confirm_action(const char *action) {
    char input[8];
    while (1) {
        printf("确认%s吗？1=确认，2=取消：", action);
        if (!fgets(input, sizeof(input), stdin)) {
            return 0;
        }
        if (input[0] == '1') {
            return 1;
        }
        if (input[0] == '2') {
            return 0;
        }
        printf("请输入 1 或 2。\n");
    }
}

/*
 * 功能：输出命令帮助信息。
 */
static void print_help(void) {
    printf("图书管理系统\n");
    printf("命令列表：\n");
    printf("  add <isbn> <title> <author> <stock>   - 添加图书\n");
    printf("  edit <isbn> <title> <author> <stock>  - 编辑图书信息\n");
    printf("  delete <isbn>                         - 删除图书\n");
    printf("  search <keyword>                      - 关键词搜索\n");
    printf("  title <title>                         - 书名精确查询\n");
    printf("  author <author>                       - 作者精确查询\n");
    printf("  isbn <isbn>                           - ISBN 查询\n");
    printf("  loan <isbn> <quantity>                - 借阅图书\n");
    printf("  return <isbn> <quantity>              - 归还图书\n");
    printf("  sort stock                            - 按库存排序\n");
    printf("  sort loan                             - 按借阅量排序\n");
    printf("  history                               - 查看借阅历史\n");
    printf("  report                                - 生成统计报告\n");
    printf("  export log <filename>                 - 导出操作日志\n");
    printf("  export borrow <filename>              - 导出借阅数据\n");
    printf("  exit                                  - 退出程序\n");
}

/*
 * 功能：主命令循环，解析用户输入并执行对应操作。
 */
void command_loop(BookNode **head) {

}

/*
 * 功能：程序入口，负责加载数据、进入命令循环并保存退出数据。
 */
int main(void) {

    return 0;
}
