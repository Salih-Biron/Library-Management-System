#include "store.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

typedef struct LoanLog {
    char isbn[20];
    int quantity;
    time_t timestamp;
} LoanLog;

void log_loan(const char *isbn, int quantity) {
    // 1) 参数检查：ISBN 必须有效，借阅数量必须为正。
    if (!isbn || quantity <= 0) {
        return;
    }

    // 2) 以追加方式写入二进制借阅日志文件。
    FILE *fp = fopen("loan.bin", "ab");
    if (!fp) {
        return;
    }

    // 3) 组装日志记录并写入。
    LoanLog log_item;
    memset(&log_item, 0, sizeof(log_item));
    snprintf(log_item.isbn, sizeof(log_item.isbn), "%s", isbn);
    log_item.quantity = quantity;
    log_item.timestamp = time(NULL);
    fwrite(&log_item, sizeof(log_item), 1, fp);
    fclose(fp);
}

void load_loans(BookNode *head) {
    // 1) 空链表直接返回。
    if (!head) {
        return;
    }

    // 2) 读取历史借阅记录，逐条更新库存与借阅量。
    FILE *fp = fopen("loan.bin", "rb");
    if (!fp) {
        return;
    }

    LoanLog log_item;
    while (fread(&log_item, sizeof(log_item), 1, fp) == 1) {
        BookNode *target = search_by_isbn(head, log_item.isbn);
        if (!target) {
            continue;
        }

        // 3) 更新借阅量；库存不足时下限为 0，避免负数。
        if (target->stock >= log_item.quantity) {
            target->stock -= log_item.quantity;
        } else {
            target->stock = 0;
        }
        target->loaned += log_item.quantity;
    }

    fclose(fp);
}
