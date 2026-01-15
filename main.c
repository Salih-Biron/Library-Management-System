#include "data.h"
#include "logic.h"
#include "store.h"
#include <stdio.h>
#include <string.h>

#define PERSISTENCE_FILE "library_data.json"

/**
 * @brief 打印帮助信息
 */
void print_help() {
    printf("Library Management System\n");
    printf("Commands:\n");
    printf("  add <isbn> <title> <author> <stock>  - Add a new book\n");
    printf("  delete <isbn>                        - Delete a book\n");
    printf("  search <keyword>                      - Search by keyword\n");
    printf("  isbn <isbn>                           - Search by ISBN\n");
    printf("  loan <isbn> <quantity>                - Record a loan\n");
    printf("  return <isbn> <quantity>              - Return a book\n");
    printf("  sort stock                            - Sort by stock\n");
    printf("  sort loan                             - Sort by loan count\n");
    printf("  report                                - Generate report\n");
    printf("  export csv <filename>                 - Export to CSV\n");
    printf("  export json <filename>                - Export to JSON\n");
    printf("  exit                                  - Exit program\n");
}

/**
 * @brief 主命令解析循环
 */
void command_loop(BookNode **head) {
    char command[100];

    while (1) {
        printf("> ");
        if (!fgets(command, sizeof(command), stdin)) {
            break;
        }

        // 移除换行符
        size_t len = strlen(command);
        // Trim trailing newline from fgets.
        size_t len = strlen(command);
        if (len > 0 && command[len - 1] == '\n') {
            command[len - 1] = '\0';
        }

        // 解析命令
        char cmd[20];
        sscanf_s(command, "%s", cmd, 20);

        if (strcmp(cmd, "exit") == 0) {
            break;
        } else if (strcmp(cmd, "help") == 0) {
            print_help();
        } else if (strcmp(cmd, "add") == 0) {
            // TODO: 解析add命令参数
            // 用sscanf提取参数，调用add_book
        } else if (strcmp(cmd, "delete") == 0) {
            char isbn[20];
            if (sscanf_s(command, "%19s %19s", cmd, 20, isbn, 20) == 2) {
                if (delete_book(head, isbn) == 0) {
                    printf("Book deleted successfully.\n");
                } else {
                    printf("Error: ISBN %s not found\n", isbn);
                }
            } else {
                printf("Usage: delete <isbn>\n");
            }
        } else if (strcmp(cmd, "search") == 0) {
            // TODO: 解析搜索关键词，调用search_by_keyword
        } else if (strcmp(cmd, "isbn") == 0) {
            // TODO: 解析ISBN，调用search_by_isbn
        } else if (strcmp(cmd, "loan") == 0) {
            char isbn[20];
            int quantity = 0;
            if (sscanf_s(command, "%19s %19s %d", cmd, 20, isbn, 20, &quantity) == 3) {
                if (loan_book(*head, isbn, quantity) == 0) {
                    log_loan(isbn, quantity);
                    printf("Book loaned successfully.
");
                } else {
                    printf("Error: loan failed for ISBN %s
", isbn);
                }
            } else {
                printf("Usage: loan <isbn> <quantity>
");
            }
        } else if (strcmp(cmd, "return") == 0) {
            char isbn[20];
            int quantity = 1;
            int parsed = sscanf_s(command, "%19s %19s %d", cmd, 20, isbn, 20, &quantity);
            if (parsed == 2 || parsed == 3) {
                if (return_book(*head, isbn, quantity) == 0) {
                    printf("Book returned successfully.\n");
                } else {
                    printf("Error: return failed for ISBN %s\n", isbn);
                }
            } else {
                printf("Usage: return <isbn> [quantity]\n");
            }
        } else if (strcmp(cmd, "sort") == 0) {
            // TODO: 解析排序类型
        } else if (strcmp(cmd, "report") == 0) {
            generate_report(*head);
        } else if (strncmp(cmd, "export", 6) == 0) {
            // TODO: 解析导出命令
        } else {
            printf("Unknown command. Type 'help' for usage.\n");
        }
    }
}

int main() {
    BookNode *head = NULL;

    // 尝试从持久化文件加载数据
    BookNode *loaded = load_books_from_json(PERSISTENCE_FILE);
    if (loaded) {
        head = loaded;
        printf("Loaded library data from %s\n", PERSISTENCE_FILE);
    } else {
        printf("No existing library data found. Starting with empty library.\n");
    }

    // 加载历史借阅记录
    load_loans(head);

    printf("Library Management System (Type 'help' for commands)\n");
    command_loop(&head);

    // 退出前保存数据
    printf("Saving library data to %s...\n", PERSISTENCE_FILE);
    if (persist_books_json(PERSISTENCE_FILE, head) == 0) {
        printf("Data saved successfully.\n");
    } else {
        printf("Warning: Failed to save library data.\n");
    }

    // 清理资源
    destroy_list(head);

    return 0;
}
