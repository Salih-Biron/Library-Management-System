#include "store.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

enum { BORROW_ACTION_LOAN = 1, BORROW_ACTION_RETURN = 2 };

static const char *kBorrowLogFile = "borrow_log.bin";
static const char *kLegacyLoanLogFile = "loan.bin";
static const char *kOperationLogFile = "operation.log";

typedef struct BorrowLogRecord {
    int action;
    char isbn[20];
    char title[100];
    int quantity;
    time_t timestamp;
} BorrowLogRecord;

typedef struct LegacyLoanLog {
    char isbn[20];
    int quantity;
    time_t timestamp;
} LegacyLoanLog;

/*
 * 功能：将时间戳格式化为可读字符串。
 */
static void format_time(time_t ts, char *buf, size_t len) {
    struct tm *tm_info = localtime(&ts);
    if (!tm_info) {
        snprintf(buf, len, "未知");
        return;
    }
    strftime(buf, len, "%Y-%m-%d %H:%M:%S", tm_info);
}

/*
 * 功能：追加借阅/归还日志记录到二进制日志文件。
 * 说明：当参数非法或文件打开失败时直接返回。
 */
static void append_borrow_log(int action, const char *isbn, const char *title, int quantity) {
    if (!isbn || quantity <= 0) {
        return;
    }

    FILE *fp = fopen(kBorrowLogFile, "ab");
    if (!fp) {
        return;
    }

    BorrowLogRecord record;
    memset(&record, 0, sizeof(record));
    record.action = action;
    snprintf(record.isbn, sizeof(record.isbn), "%s", isbn);
    if (title) {
        snprintf(record.title, sizeof(record.title), "%s", title);
    }
    record.quantity = quantity;
    record.timestamp = time(NULL);
    fwrite(&record, sizeof(record), 1, fp);
    fclose(fp);
}

/*
 * 功能：记录一次借阅操作到借阅日志。
 */
void log_loan(const char *isbn, const char *title, int quantity) {
    append_borrow_log(BORROW_ACTION_LOAN, isbn, title, quantity);
}

/*
 * 功能：记录一次归还操作到借阅日志。
 */
void log_return(const char *isbn, const char *title, int quantity) {
    append_borrow_log(BORROW_ACTION_RETURN, isbn, title, quantity);
}

/*
 * 功能：记录一条操作日志（文本形式，便于导出查看）。
 */
void log_operation(const char *action, const char *isbn, const char *title) {
    if (!action) {
        return;
    }

    FILE *fp = fopen(kOperationLogFile, "a");
    if (!fp) {
        return;
    }

    char time_buf[32];
    format_time(time(NULL), time_buf, sizeof(time_buf));
    fprintf(fp, "%s | %s", time_buf, action);
    if (isbn && *isbn) {
        fprintf(fp, " | ISBN:%s", isbn);
    }
    if (title && *title) {
        fprintf(fp, " | 书名:%s", title);
    }
    fprintf(fp, "\n");
    fclose(fp);
}

/*
 * 功能：加载借阅日志并同步库存/借阅量。
 */
void load_loans(BookNode *head) {
    if (!head) {
        return;
    }

    FILE *fp = fopen(kBorrowLogFile, "rb");
    if (!fp) {
        fp = fopen(kLegacyLoanLogFile, "rb");
        if (!fp) {
            return;
        }

        LegacyLoanLog legacy;
        while (fread(&legacy, sizeof(legacy), 1, fp) == 1) {
            BookNode *target = search_by_isbn(head, legacy.isbn);
            if (!target) {
                continue;
            }

            if (target->stock >= legacy.quantity) {
                target->stock -= legacy.quantity;
            } else {
                target->stock = 0;
            }
            target->loaned += legacy.quantity;
        }
        fclose(fp);
        return;
    }

    BorrowLogRecord record;
    while (fread(&record, sizeof(record), 1, fp) == 1) {
        BookNode *target = search_by_isbn(head, record.isbn);
        if (!target) {
            continue;
        }

        if (record.action == BORROW_ACTION_LOAN) {
            if (target->stock >= record.quantity) {
                target->stock -= record.quantity;
            } else {
                target->stock = 0;
            }
            target->loaned += record.quantity;
        } else if (record.action == BORROW_ACTION_RETURN) {
            if (target->loaned >= record.quantity) {
                target->loaned -= record.quantity;
                target->stock += record.quantity;
            } else {
                target->stock += target->loaned;
                target->loaned = 0;
            }
        }
    }

    fclose(fp);
}

/*
 * 功能：将操作日志导出到指定文本文件。
 * 返回：0=成功，-1=失败。
 */
int export_operation_log(const char *filename) {
    if (!filename) {
        return -1;
    }

    FILE *src = fopen(kOperationLogFile, "r");
    if (!src) {
        return -1;
    }

    FILE *dst = fopen(filename, "w");
    if (!dst) {
        fclose(src);
        return -1;
    }

    char buffer[4096];
    size_t bytes = 0;
    while ((bytes = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        fwrite(buffer, 1, bytes, dst);
    }

    fclose(src);
    fclose(dst);
    return 0;
}

/*
 * 功能：导出学生借阅数据（仅借阅时间与书名）。
 * 返回：0=成功，-1=失败。
 */
int export_borrow_data(const char *filename) {
    if (!filename) {
        return -1;
    }

    FILE *src = fopen(kBorrowLogFile, "rb");
    if (!src) {
        return -1;
    }

    FILE *dst = fopen(filename, "w");
    if (!dst) {
        fclose(src);
        return -1;
    }

    fprintf(dst, "借阅时间,书名\n");

    BorrowLogRecord record;
    char time_buf[32];
    while (fread(&record, sizeof(record), 1, src) == 1) {
        if (record.action != BORROW_ACTION_LOAN) {
            continue;
        }
        format_time(record.timestamp, time_buf, sizeof(time_buf));
        fprintf(dst, "%s,%s\n", time_buf, record.title);
    }

    fclose(src);
    fclose(dst);
    return 0;
}

/*
 * 功能：在控制台输出借阅历史（已还/未还、时间、书名）。
 */
void print_borrow_history(void) {
    FILE *fp = fopen(kBorrowLogFile, "rb");
    if (!fp) {
        printf("暂无借阅历史。\n");
        return;
    }

    typedef struct LoanEntry {
        BorrowLogRecord record;
        int remaining;
    } LoanEntry;

    LoanEntry *loans = NULL;
    size_t count = 0;
    size_t capacity = 0;

    BorrowLogRecord record;
    while (fread(&record, sizeof(record), 1, fp) == 1) {
        if (record.action == BORROW_ACTION_LOAN) {
            if (count == capacity) {
                size_t new_capacity = capacity == 0 ? 8 : capacity * 2;
                LoanEntry *tmp = (LoanEntry *)realloc(loans, new_capacity * sizeof(*loans));
                if (!tmp) {
                    free(loans);
                    fclose(fp);
                    printf("无法读取借阅历史。\n");
                    return;
                }
                loans = tmp;
                capacity = new_capacity;
            }
            loans[count].record = record;
            loans[count].remaining = record.quantity;
            count++;
        } else if (record.action == BORROW_ACTION_RETURN) {
            int remaining = record.quantity;
            for (size_t i = 0; i < count && remaining > 0; ++i) {
                if (strcmp(loans[i].record.isbn, record.isbn) != 0) {
                    continue;
                }
                if (loans[i].remaining <= 0) {
                    continue;
                }
                int used = loans[i].remaining < remaining ? loans[i].remaining : remaining;
                loans[i].remaining -= used;
                remaining -= used;
            }
        }
    }

    fclose(fp);

    if (count == 0) {
        printf("暂无借阅历史。\n");
        free(loans);
        return;
    }

    printf("借阅历史：\n");
    for (size_t i = 0; i < count; ++i) {
        char time_buf[32];
        format_time(loans[i].record.timestamp, time_buf, sizeof(time_buf));
        printf("%s | %s | %s\n", loans[i].remaining == 0 ? "已归还" : "未归还",
               time_buf, loans[i].record.title);
    }

    free(loans);
}

/*
 * 功能：跳过 JSON 文本中的空白字符（空格、换行、制表符等）。
 */
static void skip_json_ws(const char **p) {
    while (**p && isspace((unsigned char)**p)) {
        ++(*p);
    }
}

/*
 * 功能：解析 JSON 字符串，返回动态分配的 C 字符串。
 * 返回：成功返回非空指针，失败返回 NULL（需释放返回值）。
 */
static char *parse_json_string(const char **p) {
    if (!p || **p != '"') {
        return NULL;
    }

    ++(*p);
    size_t max_len = strlen(*p) + 1;
    char *out = (char *)malloc(max_len);
    if (!out) {
        return NULL;
    }

    size_t idx = 0;
    while (**p && **p != '"') {
        char ch = **p;
        if (ch == '\\') {
            ++(*p);
            ch = **p;
            if (!ch) {
                free(out);
                return NULL;
            }
            switch (ch) {
                case '"': out[idx++] = '"'; break;
                case '\\': out[idx++] = '\\'; break;
                case '/': out[idx++] = '/'; break;
                case 'b': out[idx++] = '\b'; break;
                case 'f': out[idx++] = '\f'; break;
                case 'n': out[idx++] = '\n'; break;
                case 'r': out[idx++] = '\r'; break;
                case 't': out[idx++] = '\t'; break;
                case 'u':
                    /* 简化处理：跳过 4 位十六进制，写入占位符。 */
                    for (int i = 0; i < 4 && (*p)[1]; ++i) {
                        ++(*p);
                    }
                    out[idx++] = '?';
                    break;
                default:
                    out[idx++] = ch;
                    break;
            }
            ++(*p);
            continue;
        }
        out[idx++] = ch;
        ++(*p);
    }

    if (**p != '"') {
        free(out);
        return NULL;
    }
    ++(*p);
    out[idx] = '\0';
    return out;
}

/*
 * 功能：解析 JSON 整数（仅支持十进制）。
 * 返回：成功返回 0 并输出数值，失败返回 -1。
 */
static int parse_json_int(const char **p, int *out_value) {
    if (!p || !out_value) {
        return -1;
    }
    skip_json_ws(p);
    char *end = NULL;
    long value = strtol(*p, &end, 10);
    if (end == *p) {
        return -1;
    }
    *out_value = (int)value;
    *p = end;
    return 0;
}

/*
 * 功能：跳过一个 JSON 值。
 * 说明：用于忽略没啥用的字段，保证解析流程可继续。
 */
static void skip_json_value(const char **p) {
    skip_json_ws(p);
    if (**p == '"') {
        char *tmp = parse_json_string(p);
        free(tmp);
        return;
    }
    if (**p == '{') {
        ++(*p);
        while (**p && **p != '}') {
            skip_json_ws(p);
            char *key = parse_json_string(p);
            free(key);
            skip_json_ws(p);
            if (**p == ':') {
                ++(*p);
            }
            skip_json_value(p);
            skip_json_ws(p);
            if (**p == ',') {
                ++(*p);
            }
        }
        if (**p == '}') {
            ++(*p);
        }
        return;
    }
    if (**p == '[') {
        ++(*p);
        while (**p && **p != ']') {
            skip_json_value(p);
            skip_json_ws(p);
            if (**p == ',') {
                ++(*p);
            }
        }
        if (**p == ']') {
            ++(*p);
        }
        return;
    }
    if (isdigit((unsigned char)**p) || **p == '-') {
        int dummy = 0;
        parse_json_int(p, &dummy);
        return;
    }
    if (strncmp(*p, "true", 4) == 0) {
        *p += 4;
        return;
    }
    if (strncmp(*p, "false", 5) == 0) {
        *p += 5;
        return;
    }
    if (strncmp(*p, "null", 4) == 0) {
        *p += 4;
        return;
    }
}

/*
 * 功能：追加一个加载的图书节点到链表尾部。
 */
static int append_loaded_book(BookNode **head, BookNode **tail, const char *isbn,
                              const char *title, const char *author, int stock, int loaned) {
    if (!isbn || !title) {
        return -1;
    }

    BookNode *node = (BookNode *)malloc(sizeof(BookNode));
    if (!node) {
        return -1;
    }

    snprintf(node->isbn, sizeof(node->isbn), "%s", isbn);
    snprintf(node->title, sizeof(node->title), "%s", title);
    snprintf(node->author, sizeof(node->author), "%s", author ? author : "");
    node->stock = stock;
    node->loaned = loaned;
    node->next = NULL;

    if (!*head) {
        *head = node;
        *tail = node;
    } else {
        (*tail)->next = node;
        *tail = node;
    }

    return 0;
}

/*
 * 功能：写入 JSON 字符串并进行必要的转义。
 * 说明：确保输出内容可被标准 JSON 解析器正确读取。
 */
static void write_json_string(FILE *fp, const char *text) {
    fputc('"', fp);
    if (text) {
        for (const unsigned char *p = (const unsigned char *)text; *p; ++p) {
            switch (*p) {
                case '\\': fputs("\\\\", fp); break;
                case '"': fputs("\\\"", fp); break;
                case '\n': fputs("\\n", fp); break;
                case '\r': fputs("\\r", fp); break;
                case '\t': fputs("\\t", fp); break;
                default: fputc(*p, fp); break;
            }
        }
    }
    fputc('"', fp);
}

/*
 * 功能：将图书信息持久化为带元数据的 JSON 文件。
 * 返回：0=成功，-1=失败。
 */
int persist_books_json(const char *filename, BookNode *head) {
    if (!filename) {
        return -1;
    }

    FILE *fp = fopen(filename, "w");
    if (!fp) {
        return -1;
    }

    time_t now = time(NULL);
    fprintf(fp, "{\n");
    fprintf(fp, "  \"metadata\": {\n");
    fprintf(fp, "    \"version\": \"1.0\",\n");
    fprintf(fp, "    \"created\": \"%ld\"\n", (long)now);
    fprintf(fp, "  },\n");
    fprintf(fp, "  \"books\": [\n");

    BookNode *cur = head;
    while (cur) {
        fprintf(fp, "    {\n");
        fprintf(fp, "      \"isbn\": ");
        write_json_string(fp, cur->isbn);
        fprintf(fp, ",\n      \"title\": ");
        write_json_string(fp, cur->title);
        fprintf(fp, ",\n      \"author\": ");
        write_json_string(fp, cur->author);
        fprintf(fp, ",\n      \"stock\": %d,\n", cur->stock);
        fprintf(fp, "      \"loaned\": %d\n", cur->loaned);
        fprintf(fp, "    }%s\n", cur->next ? "," : "");
        cur = cur->next;
    }

    fprintf(fp, "  ]\n");
    fprintf(fp, "}\n");
    fclose(fp);
    return 0;
}

/*
 * 功能：从 JSON 文件加载图书信息并构建链表。
 * 说明：只解析 books 数组中的字段，忽略其他未知字段。
 * 返回：加载后的链表头指针，失败返回 NULL。
 */
BookNode *load_books_from_json(const char *filename) {
    if (!filename) {
        return NULL;
    }

    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    if (size <= 0) {
        fclose(fp);
        return NULL;
    }
    rewind(fp);

    char *buffer = (char *)malloc((size_t)size + 1);
    if (!buffer) {
        fclose(fp);
        return NULL;
    }

    size_t read_bytes = fread(buffer, 1, (size_t)size, fp);
    buffer[read_bytes] = '\0';
    fclose(fp);

    const char *p = buffer;
    BookNode *head = NULL;
    BookNode *tail = NULL;

    while (*p) {
        skip_json_ws(&p);
        char *key = parse_json_string(&p);
        if (!key) {
            ++p;
            continue;
        }
        skip_json_ws(&p);
        if (*p != ':') {
            free(key);
            continue;
        }
        ++p;

        if (strcmp(key, "books") != 0) {
            skip_json_value(&p);
            free(key);
            continue;
        }

        skip_json_ws(&p);
        if (*p != '[') {
            free(key);
            break;
        }
        ++p;

        while (*p) {
            skip_json_ws(&p);
            if (*p == ']') {
                ++p;
                break;
            }
            if (*p == ',') {
                ++p;
                continue;
            }
            if (*p != '{') {
                ++p;
                continue;
            }
            ++p;

            char *isbn = NULL;
            char *title = NULL;
            char *author = NULL;
            int stock = 0;
            int loaned = 0;

            while (*p) {
                skip_json_ws(&p);
                if (*p == '}') {
                    ++p;
                    break;
                }

                char *field = parse_json_string(&p);
                if (!field) {
                    ++p;
                    continue;
                }
                skip_json_ws(&p);
                if (*p == ':') {
                    ++p;
                }

                if (strcmp(field, "isbn") == 0) {
                    free(isbn);
                    isbn = parse_json_string(&p);
                } else if (strcmp(field, "title") == 0) {
                    free(title);
                    title = parse_json_string(&p);
                } else if (strcmp(field, "author") == 0) {
                    free(author);
                    author = parse_json_string(&p);
                } else if (strcmp(field, "stock") == 0) {
                    parse_json_int(&p, &stock);
                } else if (strcmp(field, "loaned") == 0) {
                    parse_json_int(&p, &loaned);
                } else {
                    skip_json_value(&p);
                }

                free(field);
                skip_json_ws(&p);
                if (*p == ',') {
                    ++p;
                }
            }

            if (isbn && title) {
                if (append_loaded_book(&head, &tail, isbn, title, author, stock, loaned) != 0) {
                    free(isbn);
                    free(title);
                    free(author);
                    destroy_list(head);
                    head = NULL;
                    tail = NULL;
                    break;
                }
            }

            free(isbn);
            free(title);
            free(author);
        }

        free(key);
        break;
    }

    free(buffer);
    return head;
}
