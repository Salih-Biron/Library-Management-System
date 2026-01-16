#include "store.h"
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
