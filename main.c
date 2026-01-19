#include "data.h"
#include "logic.h"
#include "store.h"
#include "user.h"
#include "terminal.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

typedef struct {
    int r, g, b;
} Color;

/* 渐变色带（绿 → 青 → 蓝 → 紫） */
static Color gradient_colors[] = {
    {154, 255, 154},
    {0,   230, 200},
    {0,   206, 209},
    {65,  105, 225},
    {138, 43,  226}
};

#define GRADIENT_COUNT (sizeof(gradient_colors)/sizeof(gradient_colors[0]))

/* 全局颜色时间 */
static double g_color_time = 0.0;

/* 根据时间获取颜色（线性插值） */
static Color get_dynamic_color(double t) {
    int i0 = (int)t % GRADIENT_COUNT;
    int i1 = (i0 + 1) % GRADIENT_COUNT;

    double local_t = t - (int)t;

    Color c0 = gradient_colors[i0];
    Color c1 = gradient_colors[i1];

    Color out;
    out.r = c0.r + (int)((c1.r - c0.r) * local_t);
    out.g = c0.g + (int)((c1.g - c0.g) * local_t);
    out.b = c0.b + (int)((c1.b - c0.b) * local_t);
    return out;
}

#define PERSISTENCE_FILE "library_data.dat"
#define LEGACY_JSON_FILE "library_data.json"


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

/* ---------- 原有函数 ---------- */
static void print_book(const BookNode *book) {
    if (!book) {
        return;
    }
    printf("ISBN:%s | 标题:%s | 作者:%s | 分类:%s | 库存量:%d | 借阅量:%d\n",
           book->isbn, book->title, book->author, book->category, book->stock, book->loaned);
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
    printf("  add <isbn> <title> <author> <category> <stock>   - 添加图书\n");
    printf("  edit <isbn> <title> <author> <category> <stock>  - 编辑图书信息\n");
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
    printf("  export log <filename>                 - 导出操作日志\n");
    printf("  export borrow <filename>              - 导出借阅数据\n");
    printf("  export csv <filename>                  - 导出图书数据到CSV\n");
    printf("  export json <filename>                 - 导出图书数据到JSON\n");
    printf("  exit                                  - 退出程序\n");
    printf("其他问题：Telephone:131-271-3397 or email:IDWK@outlook.com\n\033[0m");
}

/* ---------- 注册界面 ---------- */
static void register_screen(UserNode **users){
    printf("\033[2J\033[H");
    
    int term_width = get_terminal_width();
    if (term_width < 80) term_width = 80;
    
    int title_pad = (term_width - 6) / 2;
    
    printf("\n");
    printf("%*s\033[38;2;154;205;50m", 0, "");
    for (int i = 0; i < term_width; i++) printf("-");
    printf("\033[0m\n");
    
    printf("%*s\033[38;2;154;205;50m注册新账号\033[0m\n", title_pad, "");
    
    printf("%*s\033[38;2;154;205;50m", 0, "");
    for (int i = 0; i < term_width; i++) printf("-");
    printf("\033[0m\n\n");
 
    char account[50];
    char password[50];
    char password_confirm[50];
    char question[100];
    char answer[100];
 
    printf("\033[38;2;255;255;255m请输入账号: \033[0m");
    if (!fgets(account, sizeof(account), stdin)) return;
    trim_newline(account);
 
    if (strlen(account) == 0) {
        printf("\033[38;2;255;0;0m账号不能为空！\n\033[0m");
        msleep(2000);
        return;
    }
 
    if (account_exists(*users, account)) {
        printf("\033[38;2;255;0;0m账号已存在！\n\033[0m");
        msleep(2000);
        return;
    }
 
    printf("\033[38;2;255;255;255m请设置密码: \033[0m");
    read_pwd(password, sizeof(password));
 
    printf("\033[38;2;255;255;255m请再次输入密码: \033[0m");
    read_pwd(password_confirm, sizeof(password_confirm));
 
    if (strcmp(password, password_confirm) != 0) {
        printf("\033[38;2;255;0;0m两次密码不一致！\n\033[0m");
        msleep(2000);
        return;
    }
  
    printf("\033[38;2;255;255;255m请设置密保问题: \033[0m");
    if (!fgets(question, sizeof(question), stdin)) return;
    trim_newline(question);
  
    if (strlen(question) == 0) {
        printf("\033[38;2;255;0;0m密保问题不能为空！\n\033[0m");
        msleep(2000);
        return;
    }
  
    printf("\033[38;2;255;255;255m请设置密保问题答案: \033[0m");
    if (!fgets(answer, sizeof(answer), stdin)) return;
    trim_newline(answer);
  
    if (strlen(answer) == 0) {
        printf("\033[38;2;255;0;0m密保答案不能为空！\n\033[0m");
        msleep(2000);
        return;
    }
 
    printf("\033[38;2;255;255;255m请选择身份:\n\033[0m");
    printf("[1] 学生\n");
    printf("[2] 管理员\n");
 
    char role_choice[10];
    UserRole role;
 
    if (!fgets(role_choice, sizeof(role_choice), stdin)) return;
    trim_newline(role_choice);
 
    if (strcmp(role_choice, "1") == 0) {
        role = ROLE_STUDENT;
    } else if (strcmp(role_choice, "2") == 0) {
        role = ROLE_ADMIN;
    } else {
        printf("\033[38;2;255;0;0m无效选择！\n\033[0m");
        msleep(2000);
        return;
    }
  
    if (register_user(users, role, account, password, question, answer) == 0) {
        printf("\033[38;2;0;255;0m注册成功！\n\033[0m");
        save_users_to_file(NULL, *users);
    } else {
        printf("\033[38;2;255;0;0m注册失败！\n\033[0m");
    }
  
    msleep(2000);
}

 /* ---------- 找回密码界面 ---------- */
 static void forgot_password_screen(UserNode *users){
     printf("\033[2J\033[H");
     
     int term_width = get_terminal_width();
     if (term_width < 80) term_width = 80;
     
     int title_pad = (term_width - 6) / 2;
     
     printf("\n");
     printf("%*s\033[38;2;154;205;50m", 0, "");
     for (int i = 0; i < term_width; i++) printf("-");
     printf("\033[0m\n");
     
     printf("%*s\033[38;2;154;205;50m找回密码\033[0m\n", title_pad, "");
     
     printf("%*s\033[38;2;154;205;50m", 0, "");
     for (int i = 0; i < term_width; i++) printf("-");
     printf("\033[0m\n\n");
   
     char account[50];
     char answer[100];
     printf("\033[38;2;255;255;255m请输入账号: \033[0m");
     if (!fgets(account, sizeof(account), stdin)) return;
     trim_newline(account);
  
      if (!account_exists(users, account)) {
          printf("\033[38;2;255;0;0m账号不存在！\n\033[0m");
          msleep(2000);
          return;
      }
   
      char *question = get_secret_question(users, account);
      if (!question) {
          printf("\033[38;2;255;0;0m获取密保问题失败！\n\033[0m");
          msleep(2000);
          return;
      }
   
      printf("\033[38;2;255;255;255m您的密保问题是: %s\n\033[0m", question);
      printf("\033[38;2;255;255;255m请输入密保问题答案: \033[0m");
      if (!fgets(answer, sizeof(answer), stdin)) return;
      trim_newline(answer);
   
      if (verify_secret(users, account, answer) != 0) {
          printf("\033[38;2;255;0;0m密保答案错误！\n\033[0m");
          msleep(2000);
          return;
      }
   
      char new_password[50];
      char new_password_confirm[50];
   
      printf("\033[38;2;255;255;255m请输入新密码: \033[0m");
      read_pwd(new_password, sizeof(new_password));
   
      printf("\033[38;2;255;255;255m请再次输入新密码: \033[0m");
      read_pwd(new_password_confirm, sizeof(new_password_confirm));
   
      if (strcmp(new_password, new_password_confirm) != 0) {
          printf("\033[38;2;255;0;0m两次密码不一致！\n\033[0m");
          msleep(2000);
          return;
      }
  
      if (change_password(users, account, new_password) == 0) {
          printf("\033[38;2;0;255;0m密码修改成功！\n\033[0m");
          save_users_to_file(NULL, users);
      } else {
          printf("\033[38;2;255;0;0m密码修改失败！\n\033[0m");
      }
  
      msleep(2000);
  }

 /* ---------- 登录界面 ---------- */
 static UserRole login_screen(UserNode *users){
     printf("\033[2J\033[H");
     
     int term_width = get_terminal_width();
     if (term_width < 80) term_width = 80;
     
     int title_pad = (term_width - 6) / 2;
     
     printf("\n");
     printf("%*s\033[38;2;154;205;50m", 0, "");
     for (int i = 0; i < term_width; i++) printf("-");
     printf("\033[0m\n");
     
     printf("%*s\033[38;2;154;205;50m账号登录\033[0m\n", title_pad, "");
     
     printf("%*s\033[38;2;154;205;50m", 0, "");
     for (int i = 0; i < term_width; i++) printf("-");
     printf("\033[0m\n");
     
     printf("%*s\033[38;2;255;165;0m[1] 登录账号\033[0m\n", (term_width - 10) / 2, "");
     printf("\n");
     
     printf("%*s\033[38;2;255;165;0m[2] 忘记密码\033[0m\n", (term_width - 10) / 2, "");
     printf("\n");
     
     printf("%*s\033[38;2;255;165;0m[3] 返回\033[0m\n", (term_width - 6) / 2, "");
     printf("\n");
     
     printf("%*s\033[38;2;154;205;50m", 0, "");
     for (int i = 0; i < term_width; i++) printf("-");
     printf("\033[0m\n\n");
     
     printf("\033[38;2;255;255;255m请输入选择: \033[0m");
     
    char choice[10];
    if (!fgets(choice, sizeof(choice), stdin)) return ROLE_NONE;
    trim_newline(choice);
    
    if (strcmp(choice, "3") == 0) {
        return ROLE_NONE;
    }
    
    if (strcmp(choice, "2") == 0) {
        forgot_password_screen(users);
        return ROLE_NONE;
    }
    
    if (strcmp(choice, "1") == 0) {
        printf("\033[2J\033[H");
        
        printf("\n");
        printf("%*s\033[38;2;154;205;50m", 0, "");
        for (int i = 0; i < term_width; i++) printf("-");
        printf("\033[0m\n");
        
        printf("%*s\033[38;2;154;205;50m账号登录\033[0m\n", title_pad, "");
        
        printf("%*s\033[38;2;154;205;50m", 0, "");
        for (int i = 0; i < term_width; i++) printf("-");
        printf("\033[0m\n\n");
        
        char account[50];
        char password[50];
        
        printf("\033[38;2;255;255;255m请输入账号: \033[0m");
        if (!fgets(account, sizeof(account), stdin)) return ROLE_NONE;
        trim_newline(account);
        
        if (strlen(account) == 0) {
            printf("\033[38;2;255;0;0m账号不能为空！\n\033[0m");
            msleep(2000);
            return ROLE_NONE;
        }
        
        printf("\033[38;2;255;255;255m请输入密码: \033[0m");
        read_pwd(password, sizeof(password));
        
        UserRole role;
        if (verify_login(users, account, password, &role) == 0) {
            printf("\033[38;2;0;255;0m登录成功！\n\033[0m");
            msleep(1000);
            return role;
        } else {
            printf("\033[38;2;255;0;0m账号或密码错误！\n\033[0m");
            msleep(2000);
            return ROLE_NONE;
        }
    }
    
    printf("\033[38;2;255;0;0m无效选择！\n\033[0m");
    msleep(2000);
    return ROLE_NONE;
 }

/* ---------- 彩色主界面 ---------- */
static void color_main_menu(void){
    printf("\033[2J\033[H");

    int term_width = get_terminal_width();
    int term_height = get_terminal_height();

    if (term_width < 60) term_width = 60;
    if (term_height < 20) term_height = 20;

    /* 推进颜色时间（控制速度） */
    g_color_time += 0.08;

    char separator[2048];
    int sep_len = term_width;
    if (sep_len > 2047) sep_len = 2047;
    memset(separator, '-', sep_len);
    separator[sep_len] = '\0';

    int center_offset = (term_width - 48) / 2;
    if (center_offset < 2) center_offset = 2;

    /* 分隔线颜色 */
    Color sep_color = get_dynamic_color(g_color_time);
    printf("\033[38;2;%d;%d;%dm%s\n",
           sep_color.r, sep_color.g, sep_color.b, separator);

    const char *logo[] = {
        "####      ####      #       #     #    ",
        "#    #    #    #     #     #       #   ",
        "#    #    #    #     #    #        #   ",
        "#    #    #    #     #  #          #   ",
        "#    #    #    #     #    #        #   ",
        "#    #    #    #     #      #      #   ",
        "#    #    #    #     #        #    #   ",
        " ####      ####      #          #  #########"
    };

    int logo_lines = sizeof(logo) / sizeof(logo[0]);

    /* LOGO：每一行不同相位 */
    for (int i = 0; i < logo_lines; i++) {
        Color c = get_dynamic_color(g_color_time + i * 0.35);
        for (int s = 0; s < center_offset; s++) printf(" ");
        printf("\033[38;2;%d;%d;%dm%s\n", c.r, c.g, c.b, logo[i]);
    }

    Color title_color = get_dynamic_color(g_color_time + logo_lines * 0.4);
    for (int i = 0; i < center_offset; i++) printf(" ");
    printf("\033[38;2;%d;%d;%dmOcean of Knowledege Library  (OOKL)\n",
           title_color.r, title_color.g, title_color.b);

    for (int i = 0; i < center_offset + 2; i++) printf(" ");
    printf("\033[38;2;%d;%d;%dmVersion: 1.0.0\n",
           title_color.r, title_color.g, title_color.b);

    printf("\033[38;2;%d;%d;%dm%s\n", sep_color.r, sep_color.g, sep_color.b, separator);
    printf("%s\n", separator);

    int menu_center_offset = (term_width - 40) / 2;
    if (menu_center_offset < 2) menu_center_offset = 2;

    for (int i = 0; i < menu_center_offset; i++) printf(" ");
    printf("[1] 进入主程序\n");

    for (int i = 0; i < menu_center_offset; i++) printf(" ");
    printf("[2] 退出系统\n");

    printf("%s\n", separator);

    int used_lines = logo_lines + 8;
    for (int i = 0; i < term_height - used_lines; i++) printf("\n");

    printf("\033[0m");
    fflush(stdout);
}

 /* ---------- 学生命令循环 ---------- */
static void student_menu(BookNode **head) {
    printf("\033[2J\033[H");
    
    int term_width = get_terminal_width();
    if (term_width < 80) term_width = 80;
    
    int title_pad = (term_width - 6) / 2;
    
    printf("\n");
    printf("%*s\033[38;2;154;205;50m", 0, "");
    for (int i = 0; i < term_width; i++) printf("-");
    printf("\033[0m\n");
    
    printf("%*s\033[38;2;154;205;50m学生菜单\033[0m\n", title_pad, "");
    
    printf("%*s\033[38;2;154;205;50m", 0, "");
    for (int i = 0; i < term_width; i++) printf("-");
    printf("\033[0m\n");
    
    printf("%*s\033[38;2;255;165;0m[1]查询图书\033[0m\n", (term_width - 10) / 2, "");
    printf("%*s\033[38;2;255;165;0m[2]查看所有图书\033[0m\n", (term_width - 10) / 2, "");
    printf("%*s\033[38;2;255;165;0m[3]借阅图书\033[0m\n", (term_width - 10) / 2, "");
    printf("%*s\033[38;2;255;165;0m[4]归还图书\033[0m\n", (term_width - 10) / 2, "");
    printf("%*s\033[38;2;255;165;0m[5]查看借阅历史\033[0m\n", (term_width - 10) / 2, "");
    printf("%*s\033[38;2;255;165;0m[6]导出借阅数据\033[0m\n", (term_width - 10) / 2, "");
    printf("%*s\033[38;2;255;165;0m[7]退出登录\033[0m\n", (term_width - 10) / 2, "");
    
    printf("%*s\033[38;2;154;205;50m", 0, "");
    for (int i = 0; i < term_width; i++) printf("-");
    printf("\033[0m\n\n");
}

static void student_command_loop(BookNode **head) {
    while (1) {
        student_menu(head);
        
        char choice[10];
        if (!fgets(choice, sizeof(choice), stdin)) break;
        trim_newline(choice);
        
        if (strcmp(choice, "1") == 0) {
            printf("\033[38;2;255;255;255m请输入搜索关键词（书名、作者或ISBN）：\033[0m");
            char keyword[128];
            if (!fgets(keyword, sizeof(keyword), stdin)) break;
            trim_newline(keyword);
            
            BookNode *result = search_by_keyword(*head, keyword);
            print_book_list(result);
        } else if (strcmp(choice, "2") == 0) {
            print_book_list(*head);
        } else if (strcmp(choice, "3") == 0) {
            printf("\033[38;2;255;255;255m请输入图书ISBN：\033[0m");
            char isbn[20];
            if (!fgets(isbn, sizeof(isbn), stdin)) break;
            trim_newline(isbn);
            
            printf("\033[38;2;255;255;255m请输入借阅数量：\033[0m");
            char qty_str[10];
            if (!fgets(qty_str, sizeof(qty_str), stdin)) break;
            int qty = atoi(qty_str);
            
            BookNode *book = search_by_isbn(*head, isbn);
            if (book && book->stock >= qty) {
                if (confirm_action("借阅")) {
                    if (loan_book(*head, isbn, qty) == 0) {
                        log_loan(isbn, book->title, qty);
                        printf("\033[38;2;0;255;0m借阅成功\n\033[0m");
                    } else {
                        printf("\033[38;2;255;0;0m借阅失败\n\033[0m");
                    }
                }
            } else {
                printf("\033[38;2;255;0;0m库存不足或图书不存在\n\033[0m");
            }
        } else if (strcmp(choice, "4") == 0) {
            printf("\033[38;2;255;255;255m请输入图书ISBN：\033[0m");
            char isbn[20];
            if (!fgets(isbn, sizeof(isbn), stdin)) break;
            trim_newline(isbn);
            
            printf("\033[38;2;255;255;255m请输入归还数量：\033[0m");
            char qty_str[10];
            if (!fgets(qty_str, sizeof(qty_str), stdin)) break;
            int qty = atoi(qty_str);
            
            BookNode *book = search_by_isbn(*head, isbn);
            if (book && book->loaned >= qty) {
                if (confirm_action("归还")) {
                    if (return_book(*head, isbn, qty) == 0) {
                        log_return(isbn, book->title, qty);
                        printf("\033[38;2;0;255;0m归还成功\n\033[0m");
                    } else {
                        printf("\033[38;2;255;0;0m归还失败\n\033[0m");
                    }
                }
            } else {
                printf("\033[38;2;255;0;0m借阅记录不足或图书不存在\n\033[0m");
            }
         } else if (strcmp(choice, "5") == 0) {
            print_borrow_history();
        } else if (strcmp(choice, "6") == 0) {
            printf("\033[38;2;255;255;255m请输入导出文件名：\033[0m");
            char filename[128];
            if (!fgets(filename, sizeof(filename), stdin)) break;
            trim_newline(filename);
            
            if (export_borrow_data(filename) == 0) {
                printf("\033[38;2;0;255;0m导出借阅数据成功\n\033[0m");
            } else {
                printf("\033[38;2;255;0;0m导出借阅数据失败\n\033[0m");
            }
        } else if (strcmp(choice, "7") == 0) {
            break;
        } else {
            printf("\033[38;2;255;0;0m无效选择，请重新输入\n\033[0m");
        }
        
        printf("\033[38;2;255;255;255m按回车键继续...\033[0m");
        getchar();
    }
}

 /* ---------- 管理员命令循环 ---------- */
static void admin_menu(BookNode **head) {
    printf("\033[2J\033[H");
    
    int term_width = get_terminal_width();
    if (term_width < 80) term_width = 80;
    
    int title_pad = (term_width - 6) / 2;
    
    printf("\n");
    printf("%*s\033[38;2;154;205;50m", 0, "");
    for (int i = 0; i < term_width; i++) printf("-");
    printf("\033[0m\n");
    
    printf("%*s\033[38;2;154;205;50m管理员菜单\033[0m\n", title_pad, "");
    
    printf("%*s\033[38;2;154;205;50m", 0, "");
    for (int i = 0; i < term_width; i++) printf("-");
    printf("\033[0m\n");
    
    printf("%*s\033[38;2;255;165;0m[1]添加图书\033[0m\n", (term_width - 10) / 2, "");
    printf("%*s\033[38;2;255;165;0m[2]删除图书\033[0m\n", (term_width - 10) / 2, "");
    printf("%*s\033[38;2;255;165;0m[3]查询图书\033[0m\n", (term_width - 10) / 2, "");
    printf("%*s\033[38;2;255;165;0m[4]查看所有图书\033[0m\n", (term_width - 10) / 2, "");
    printf("%*s\033[38;2;255;165;0m[5]借阅图书\033[0m\n", (term_width - 10) / 2, "");
    printf("%*s\033[38;2;255;165;0m[6]归还图书\033[0m\n", (term_width - 10) / 2, "");
    printf("%*s\033[38;2;255;165;0m[7]按库存排序\033[0m\n", (term_width - 10) / 2, "");
    printf("%*s\033[38;2;255;165;0m[8]按借阅量排序\033[0m\n", (term_width - 10) / 2, "");
    printf("%*s\033[38;2;255;165;0m[9]查看借阅历史\033[0m\n", (term_width - 10) / 2, "");
    printf("%*s\033[38;2;255;165;0m[10]导出操作日志\033[0m\n", (term_width - 10) / 2, "");
    printf("%*s\033[38;2;255;165;0m[11]导出借阅数据\033[0m\n", (term_width - 10) / 2, "");
    printf("%*s\033[38;2;255;165;0m[12]导出图书数据到CSV\033[0m\n", (term_width - 10) / 2, "");
    printf("%*s\033[38;2;255;165;0m[13]导出图书数据到JSON\033[0m\n", (term_width - 10) / 2, "");
    printf("%*s\033[38;2;255;165;0m[14]退出登录\033[0m\n", (term_width - 10) / 2, "");
    
    printf("%*s\033[38;2;154;205;50m", 0, "");
    for (int i = 0; i < term_width; i++) printf("-");
    printf("\033[0m\n\n");
}

void admin_command_loop(BookNode **head) {
    while (1) {
        admin_menu(head);
        
        char choice[10];
        if (!fgets(choice, sizeof(choice), stdin)) break;
        trim_newline(choice);
        
        if (strcmp(choice, "1") == 0) {
            printf("\033[38;2;255;255;255m请输入ISBN：\033[0m");
            char isbn[20];
            if (!fgets(isbn, sizeof(isbn), stdin)) break;
            trim_newline(isbn);
            
            printf("\033[38;2;255;255;255m请输入书名：\033[0m");
            char title[100];
            if (!fgets(title, sizeof(title), stdin)) break;
            trim_newline(title);
            
            printf("\033[38;2;255;255;255m请输入作者：\033[0m");
            char author[50];
            if (!fgets(author, sizeof(author), stdin)) break;
            trim_newline(author);
            
            printf("\033[38;2;255;255;255m请输入分类：\033[0m");
            char category[50];
            if (!fgets(category, sizeof(category), stdin)) break;
            trim_newline(category);
            
            printf("\033[38;2;255;255;255m请输入库存数量：\033[0m");
            char stock_str[10];
            if (!fgets(stock_str, sizeof(stock_str), stdin)) break;
            int stock = atoi(stock_str);
            
            if (add_book(head, isbn, title, author, category, stock) == 0) {
                log_operation("添加图书", isbn, title);
                if (persist_books_dat(PERSISTENCE_FILE, *head) != 0) {
                    printf("\033[38;2;255;0;0mFailed to persist book data.\n\033[0m");
                }
                printf("\033[38;2;0;255;0m添加图书成功\n\033[0m");
            } else {
                printf("\033[38;2;255;0;0m添加图书失败\n\033[0m");
            }
         } else if (strcmp(choice, "2") == 0) {
            printf("\033[38;2;255;255;255m请输入要删除的图书ISBN：\033[0m");
            char isbn[20];
            if (!fgets(isbn, sizeof(isbn), stdin)) break;
            trim_newline(isbn);
            
            if (confirm_action("删除")) {
                BookNode *book = search_by_isbn(*head, isbn);
                if (delete_book(head, isbn) == 0) {
                    log_operation("删除图书", isbn, book ? book->title : NULL);
                    printf("\033[38;2;0;255;0m删除图书成功\n\033[0m");
                } else {
                    printf("\033[38;2;255;0;0m删除图书失败\n\033[0m");
                }
            }
        } else if (strcmp(choice, "3") == 0) {
            printf("\033[38;2;255;255;255m请输入搜索关键词（书名、作者或ISBN）：\033[0m");
            char keyword[128];
            if (!fgets(keyword, sizeof(keyword), stdin)) break;
            trim_newline(keyword);
            
            BookNode *result = search_by_keyword(*head, keyword);
            print_book_list(result);
        } else if (strcmp(choice, "4") == 0) {
            print_book_list(*head);
        } else if (strcmp(choice, "5") == 0) {
            printf("\033[38;2;255;255;255m请输入图书ISBN：\033[0m");
            char isbn[20];
            if (!fgets(isbn, sizeof(isbn), stdin)) break;
            trim_newline(isbn);
            
            printf("\033[38;2;255;255;255m请输入借阅数量：\033[0m");
            char qty_str[10];
            if (!fgets(qty_str, sizeof(qty_str), stdin)) break;
            int qty = atoi(qty_str);
            
            BookNode *book = search_by_isbn(*head, isbn);
            if (book && book->stock >= qty) {
                if (confirm_action("借阅")) {
                    if (loan_book(*head, isbn, qty) == 0) {
                        log_loan(isbn, book->title, qty);
                        printf("\033[38;2;0;255;0m借阅成功\n\033[0m");
                    } else {
                        printf("\033[38;2;255;0;0m借阅失败\n\033[0m");
                    }
                }
            } else {
                printf("\033[38;2;255;0;0m库存不足或图书不存在\n\033[0m");
            }
        } else if (strcmp(choice, "5") == 0) {
            printf("\033[38;2;255;255;255m请输入图书ISBN：\033[0m");
            char isbn[20];
            if (!fgets(isbn, sizeof(isbn), stdin)) break;
            trim_newline(isbn);
            
            printf("\033[38;2;255;255;255m请输入借阅数量：\033[0m");
            char qty_str[10];
            if (!fgets(qty_str, sizeof(qty_str), stdin)) break;
            int qty = atoi(qty_str);
            
            BookNode *book = search_by_isbn(*head, isbn);
            if (book && book->stock >= qty) {
                if (confirm_action("借阅")) {
                    if (loan_book(*head, isbn, qty) == 0) {
                        log_loan(isbn, book->title, qty);
                        printf("\033[38;2;0;255;0m借阅成功\n\033[0m");
                    } else {
                        printf("\033[38;2;255;0;0m借阅失败\n\033[0m");
                    }
                }
            } else {
                printf("\033[38;2;255;0;0m库存不足或图书不存在\n\033[0m");
            }
        } else if (strcmp(choice, "6") == 0) {
            printf("\033[38;2;255;255;255m请输入图书ISBN：\033[0m");
            char isbn[20];
            if (!fgets(isbn, sizeof(isbn), stdin)) break;
            trim_newline(isbn);
            
            printf("\033[38;2;255;255;255m请输入归还数量：\033[0m");
            char qty_str[10];
            if (!fgets(qty_str, sizeof(qty_str), stdin)) break;
            int qty = atoi(qty_str);
            
            BookNode *book = search_by_isbn(*head, isbn);
            if (book && book->loaned >= qty) {
                if (confirm_action("归还")) {
                    if (return_book(*head, isbn, qty) == 0) {
                        log_return(isbn, book->title, qty);
                        printf("\033[38;2;0;255;0m归还成功\n\033[0m");
                    } else {
                        printf("\033[38;2;255;0;0m归还失败\n\033[0m");
                    }
                }
            } else {
                printf("\033[38;2;255;0;0m借阅记录不足或图书不存在\n\033[0m");
            }
        } else if (strcmp(choice, "7") == 0) {
            sort_by_stock(head);
            printf("\033[38;2;0;255;0m按库存排序完成\n\033[0m");
        } else if (strcmp(choice, "8") == 0) {
            sort_by_loan(head);
            printf("\033[38;2;0;255;0m按借阅量排序完成\n\033[0m");
        } else if (strcmp(choice, "9") == 0) {
            print_borrow_history();
        } else if (strcmp(choice, "10") == 0) {
            printf("\033[38;2;255;255;255m请输入导出文件名：\033[0m");
            char filename[128];
            if (!fgets(filename, sizeof(filename), stdin)) break;
            trim_newline(filename);
            
            if (export_operation_log(filename) == 0) {
                printf("\033[38;2;0;255;0m导出操作日志成功\n\033[0m");
            } else {
                printf("\033[38;2;255;0;0m导出操作日志失败\n\033[0m");
            }
        } else if (strcmp(choice, "11") == 0) {
            printf("\033[38;2;255;255;255m请输入导出文件名：\033[0m");
            char filename[128];
            if (!fgets(filename, sizeof(filename), stdin)) break;
            trim_newline(filename);
            
            if (export_borrow_data(filename) == 0) {
                printf("\033[38;2;0;255;0m导出借阅数据成功\n\033[0m");
            } else {
                printf("\033[38;2;255;0;0m导出借阅数据失败\n\033[0m");
            }
        } else if (strcmp(choice, "12") == 0) {
            printf("\033[38;2;255;255;255m请输入导出文件名：\033[0m");
            char filename[128];
            if (!fgets(filename, sizeof(filename), stdin)) break;
            trim_newline(filename);
            
            export_to_csv(filename, *head);
            printf("\033[38;2;0;255;0m导出图书数据到CSV成功\n\033[0m");
        } else if (strcmp(choice, "13") == 0) {
            printf("\033[38;2;255;255;255m请输入导出文件名：\033[0m");
            char filename[128];
            if (!fgets(filename, sizeof(filename), stdin)) break;
            trim_newline(filename);
            
            export_to_json(filename, *head);
            printf("\033[38;2;0;255;0m导出图书数据到JSON成功\n\033[0m");
        } else if (strcmp(choice, "14") == 0) {
            break;
        } else {
            printf("\033[38;2;255;0;0m无效选择，请重新输入\n\033[0m");
        }
        
        printf("\033[38;2;255;255;255m按回车键继续...\033[0m");
        getchar();
    }
}

/* ---------- main ---------- */
int main(void) {
    init_terminal();

    BookNode *book_list = load_books_from_dat(PERSISTENCE_FILE);
    if (!book_list) {
        book_list = load_books_from_json(LEGACY_JSON_FILE);
        if (book_list) {
            persist_books_dat(PERSISTENCE_FILE, book_list);
        }
    }

    UserNode *user_list = load_users_from_file(NULL);
    if (!user_list) {
        user_list = NULL;
    }

    char choice[10];

    while (1) {

        /* ===== 动画循环 ===== */
#if ENABLE_ANIMATION
        while (!platform_kbhit()) {
            color_main_menu();
            platform_sleep(50);
        }
#else
        color_main_menu();
#endif

        if (!fgets(choice, sizeof choice, stdin)) break;
        trim_newline(choice);

        /* ===== 原有逻辑，一行不动 ===== */
        if(strcmp(choice,"1")==0){
        printf("\033[38;2;255;255;255m\n进入图书管理系统...\n\033[0m");
           
        char menu_choice[10];
        while(1){
            printf("\033[2J\033[H");
            
            int term_width = get_terminal_width();
            if (term_width < 80) term_width = 80;
            
            int title_pad = (term_width - 4) / 2;
            int opt1_pad = (term_width - 10) / 2;
            int opt2_pad = (term_width - 10) / 2;
            int opt3_pad = (term_width - 6) / 2;

            printf("\n");
            printf("%*s\033[38;2;0;230;200mOcean of Knowledege Library  (OOKL)\033[0m\n", (term_width - 32) / 2, "");
            printf("%*s\033[38;2;0;230;200mVersion: 1.0.0\033[0m\n\n", (term_width - 13) / 2, "");

            printf("%*s\033[38;2;154;205;50m", 0, "");
            for (int i = 0; i < term_width; i++) printf("-");
            printf("\033[0m\n");

            printf("%*s\033[38;2;154;205;50m主菜单\033[0m\n", title_pad, "");

            printf("%*s\033[38;2;154;205;50m", 0, "");
            for (int i = 0; i < term_width; i++) printf("-");
            printf("\033[0m\n");

            printf("%*s\033[38;2;255;165;0m[1]注册账号\033[0m\n", opt1_pad, "");
            printf("\n");

            printf("%*s\033[38;2;255;165;0m[2]登陆账号\033[0m\n", opt2_pad, "");
            printf("\n");

            printf("%*s\033[38;2;255;165;0m[3]返回\033[0m\n", opt3_pad, "");
            printf("\n");

            printf("%*s\033[38;2;154;205;50m", 0, "");
            for (int i = 0; i < term_width; i++) printf("-");
            printf("\033[0m\n\n");

            if(!fgets(menu_choice,sizeof menu_choice,stdin)) break;
            trim_newline(menu_choice);

            if(strcmp(menu_choice,"1")==0){
                register_screen(&user_list);
            }else if(strcmp(menu_choice,"2")==0){
                UserRole role = login_screen(user_list);
                if(role == ROLE_ADMIN){
                    printf("\033[38;2;0;255;0m欢迎管理员！\n\033[0m");
                    msleep(1500);
                    admin_command_loop(&book_list);
                }else if(role == ROLE_STUDENT){
                    printf("\033[38;2;0;255;0m欢迎学生！\n\033[0m");
                    msleep(1500);
                    student_command_loop(&book_list);
                }
            }else if(strcmp(menu_choice,"3")==0){
                break;
            }else{
                printf("\033[38;2;255;0;0m无效选择，请输入 1、2 或 3。\n\033[0m");
                msleep(1500);
            }
        }
    }else if(strcmp(choice,"2")==0){
        printf("\033[38;2;255;255;255m\n感谢使用图书管理系统，再见！\n\033[0m");
        break;
    }else{
        printf("\033[38;2;255;0;0m无效选择，请输入 1 或 2。\n\033[0m");
    }
    }
    return 0;
}
