#include "data.h"
#include "logic.h"
#include "store.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define PERSISTENCE_FILE "library_data.json"

/* 获取终端窗口宽度 */
static int get_terminal_width(void){
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    if (w.ws_col == 0) {
        char *cols = getenv("COLUMNS");
        return cols ? atoi(cols) : 80;
    }
    return w.ws_col;
}

/* 获取终端窗口高度 */
static int get_terminal_height(void){
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    if (w.ws_row == 0) {
        char *lines = getenv("LINES");
        return lines ? atoi(lines) : 24;
    }
    return w.ws_row;
}

/* ---------- 工具 ---------- */
static void trim_newline(char *s){
    size_t l = strlen(s);
    if (l && s[l-1]=='\n') s[l-1]=0;
}

/* 从 0~1 返回 RGB 值：start→end 线性插值 */
static void gradient_rgb(double t,
                         int r0,int g0,int b0,
                         int r1,int g1,int b1,
                         int *rd,int *gd,int *bd){
    *rd = r0 + (int)((r1-r0)*t);
    *gd = g0 + (int)((g1-g0)*t);
    *bd = b0 + (int)((b1-b0)*t);
}

/* 打印一行真彩背景 + 前景文字（自动换行）*/
static void print_color_line(const char *text,
                             int bg_r,int bg_g,int bg_b,
                             int fg_r,int fg_g,int fg_b){
    printf("\033[48;2;%d;%d;%dm\033[38;2;%d;%d;%dm%s\033[0m\n",
           bg_r,bg_g,bg_b, fg_r,fg_g,fg_b, text);
}

/* 读取密码，回显 '*' */
static void read_pwd(char *buf,size_t sz){
    size_t i=0;
    int ch;
    while(i<sz-1 && (ch=getchar())!=EOF && ch!='\n'){
        if((ch==127||ch==8) && i>0){
            --i; fputs("\b \b",stdout);
        }else{
            buf[i++]=(char)ch;
            putchar('*');
        }
    }
    buf[i]=0; putchar('\n');
}

/* ---------- 原有函数 ---------- */
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
    printf("  exit                                  - 退出程序\n\033[0m");
}

/* ---------- 彩色主界面 ---------- */
static void color_main_menu(void){
    printf("\033[2J\033[H");

    int term_width = get_terminal_width();
    int term_height = get_terminal_height();

    if (term_width < 60) term_width = 60;
    if (term_height < 20) term_height = 20;

    printf("\033[38;2;0;206;209m");

    char separator[2048];
    int sep_len = term_width;
    if (sep_len > 2047) sep_len = 2047;
    memset(separator, '-', sep_len);
    separator[sep_len] = '\0';

    int center_offset = (term_width - 48) / 2;
    if (center_offset < 2) center_offset = 2;

    printf("%s\n", separator);

    for (int i = 0; i < center_offset; i++) {
        printf(" ");
    }
    printf("####      ####      #       #     #    \n");
    for (int i = 0; i < center_offset; i++) {
        printf(" ");
    }
    printf("#    #    #    #     #     #       #   \n");
    for (int i = 0; i < center_offset; i++) {
        printf(" ");
    }
    printf("#    #    #    #     #    #        #   \n");
    for (int i = 0; i < center_offset; i++) {
        printf(" ");
    }
    printf("#    #    #    #     #  #          #   \n");
    for (int i = 0; i < center_offset; i++) {
        printf(" ");
    }
    printf("#    #    #    #     #    #        #   \n");
    for (int i = 0; i < center_offset; i++) {
        printf(" ");
    }
    printf("#    #    #    #     #      #      #   \n");
    for (int i = 0; i < center_offset; i++) {
        printf(" ");
    }
    printf("#    #    #    #     #        #    #   \n");
    for (int i = 0; i < center_offset; i++) {
        printf(" ");
    }
    printf(" ####      ####      #          #  #########\n");

    for (int i = 0; i < center_offset; i++) {
        printf(" ");
    }
    printf("\033[38;2;0;230;200mOcean of Knowledege Library  (OOKL)\033[38;2;0;206;209m\n");

    for (int i = 0; i < center_offset + 2; i++) {
        printf(" ");
    }
    printf("\033[38;2;0;230;200mVersion: 1.0.0\033[38;2;0;206;209m\n");

    printf("%s\n", separator);
    printf("%s\n", separator);

    int menu_center_offset = (term_width - 40) / 2;
    if (menu_center_offset < 2) menu_center_offset = 2;

    for (int i = 0; i < menu_center_offset; i++) {
        printf(" ");
    }
    printf("[1] 进入主程序\n");

    for (int i = 0; i < menu_center_offset; i++) {
        printf(" ");
    }
    printf("[2] 退出系统\n");

    printf("%s\n", separator);

    int logo_lines = 10;
    int used_lines = logo_lines + 5;
    int remaining_height = term_height - used_lines;
    for (int i = 0; i < remaining_height; i++) {
        printf("\n");
    }

    printf("\033[0m");
    fflush(stdout);
}

/* ---------- 命令循环 ---------- */
void command_loop(BookNode **head) {
    char input[256];
    char command[32];
    char arg1[128], arg2[128], arg3[128];
    int arg4;

    printf("\033[38;2;255;255;255m\n请输入命令（输入 help 查看帮助）：\033[0m");
    fflush(stdout);

    while (1) {
        if (fgets(input, sizeof(input), stdin)) {
            trim_newline(input);
        } else {
            break;
        }

        if (strcmp(input, "exit") == 0) {
            break;
        }

        if (strcmp(input, "help") == 0) {
            print_help();
            printf("\033[38;2;255;255;255m\n请输入命令：\033[0m");
            fflush(stdout);
            continue;
        }

        int parsed = sscanf(input, "%31s %127s %127s %127s %d", command, arg1, arg2, arg3, &arg4);

        if (parsed < 1) {
            printf("\033[38;2;255;255;255m无效命令，输入 help 查看帮助\n\n请输入命令：\033[0m");
            fflush(stdout);
            continue;
        }

        if (strcmp(command, "add") == 0 && parsed >= 5) {
            if (add_book(head, arg1, arg2, arg3, arg4) == 0) {
                printf("\033[38;2;0;255;0m添加图书成功\n\033[0m");
            } else {
                printf("\033[38;2;255;0;0m添加图书失败\n\033[0m");
            }
        } else if (strcmp(command, "delete") == 0 && parsed >= 2) {
            if (delete_book(head, arg1) == 0) {
                printf("\033[38;2;0;255;0m删除图书成功\n\033[0m");
            } else {
                printf("\033[38;2;255;0;0m删除图书失败\n\033[0m");
            }
        } else if (strcmp(command, "search") == 0 && parsed >= 2) {
            BookNode *result = search_by_keyword(*head, arg1);
            print_book_list(result);
        } else if (strcmp(command, "title") == 0 && parsed >= 2) {
            BookNode *result = search_by_title(*head, arg1);
            print_book_list(result);
        } else if (strcmp(command, "author") == 0 && parsed >= 2) {
            BookNode *result = search_by_author(*head, arg1);
            print_book_list(result);
        } else if (strcmp(command, "isbn") == 0 && parsed >= 2) {
            BookNode *result = search_by_isbn(*head, arg1);
            if (result) {
                print_book(result);
            } else {
                printf("未找到该图书。\n");
            }
        } else if (strcmp(command, "loan") == 0 && parsed >= 3) {
            if (confirm_action("借阅")) {
                int qty = (parsed >= 3) ? atoi(arg2) : 1;
                if (loan_book(*head, arg1, qty) == 0) {
                    printf("\033[38;2;0;255;0m借阅成功\n\033[0m");
                } else {
                    printf("\033[38;2;255;0;0m借阅失败\n\033[0m");
                }
            }
        } else if (strcmp(command, "return") == 0 && parsed >= 3) {
            if (confirm_action("归还")) {
                int qty = (parsed >= 3) ? atoi(arg2) : 1;
                if (return_book(*head, arg1, qty) == 0) {
                    printf("\033[38;2;0;255;0m归还成功\n\033[0m");
                } else {
                    printf("\033[38;2;255;0;0m归还失败\n\033[0m");
                }
            }
        } else if (strcmp(command, "sort") == 0 && parsed >= 2) {
            if (strcmp(arg1, "stock") == 0) {
                printf("按库存排序功能待实现\n");
            } else if (strcmp(arg1, "loan") == 0) {
                printf("按借阅量排序功能待实现\n");
            }
        } else if (strcmp(command, "history") == 0) {
            print_borrow_history();
        } else if (strcmp(command, "export") == 0 && parsed >= 3) {
            if (strcmp(arg1, "log") == 0) {
                export_operation_log(arg2);
            } else if (strcmp(arg1, "borrow") == 0) {
                export_borrow_data(arg2);
            }
        } else {
            printf("\033[38;2;255;0;0m未知命令：%s，输入 help 查看帮助\n\033[0m", command);
        }

        printf("\033[38;2;255;255;255m\n请输入命令：\033[0m");
        fflush(stdout);
    }
}

/* ---------- main ---------- */
int main(void){
    BookNode *book_list = load_books_from_json(PERSISTENCE_FILE);

    char choice[10];
    while(1){
        color_main_menu();
        if(!fgets(choice,sizeof choice,stdin)) break;
        trim_newline(choice);

        if(strcmp(choice,"1")==0){
            printf("\033[38;2;255;255;255m\n进入图书管理系统...\n\033[0m");
            print_help();
            command_loop(&book_list);
        }else if(strcmp(choice,"2")==0){
            printf("\033[38;2;255;255;255m\n感谢使用图书管理系统，再见！\n\033[0m");
            break;
        }else{
            printf("\033[38;2;255;0;0m无效选择，请输入 1 或 2。\n\033[0m");
        }
    }

    persist_books_json(PERSISTENCE_FILE, book_list);
    destroy_list(book_list);
    return 0;
}
