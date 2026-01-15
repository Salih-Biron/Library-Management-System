#include "data.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int add_book(BookNode **head, const char *isbn, const char *title, const char *author, int stock) {
    // 检查指针和必填字段是否有效，避免空指针访问。
    if (!head || !isbn || !title || !author) {
        return -1;
    }

    // 遍历链表，若 ISBN 已存在则返回失败并提示。
    for (BookNode *cur = *head; cur != NULL; cur = cur->next) {
        if (strcmp(cur->isbn, isbn) == 0) {
            printf("Error: ISBN %s already exists\n", isbn);
            return -1;
        }
    }

    // 分配新节点，分配失败直接返回。
    BookNode *node = (BookNode *)malloc(sizeof(BookNode));
    if (!node) {
        return -1;
    }

    // 初始化节点字段，借阅量从零开始。
    snprintf(node->isbn, sizeof(node->isbn), "%s", isbn);
    snprintf(node->title, sizeof(node->title), "%s", title);
    snprintf(node->author, sizeof(node->author), "%s", author);
    node->stock = stock;
    node->loaned = 0;
    node->next = NULL;

    // 追加到尾部，保持录入顺序。
    if (*head == NULL) {
        *head = node;
    } else {
        BookNode *tail = *head;
        while (tail->next != NULL) {
            tail = tail->next;
        }
        tail->next = node;
    }

    return 0;
}

int delete_book(BookNode **head, const char *isbn) {
    // 检查参数是否有效。
    if (!head || !isbn) {
        return -1;
    }

    // 使用前驱指针遍历链表，便于删除时断链。
    BookNode *cur = *head;
    BookNode *prev = NULL;

    while (cur != NULL) {
        if (strcmp(cur->isbn, isbn) == 0) {
            // 找到目标后断链并释放节点内存。
            if (prev) {
                prev->next = cur->next;
            } else {
                *head = cur->next;
            }
            free(cur);
            return 0;
        }
        prev = cur;
        cur = cur->next;
    }

    return -1;
}

int loan_book(BookNode *head, const char *isbn, int quantity) {
    // 检查 ISBN 和借阅数量是否有效。
    if (!isbn || quantity <= 0) {
        return -1;
    }

    // 定位目标图书节点。
    BookNode *target = search_by_isbn(head, isbn);
    if (!target) {
        return -1;
    }

    // 库存不足时拒绝借阅，避免库存为负。
    if (target->stock < quantity) {
        return -1;
    }

    // 更新库存与借阅量。
    target->stock -= quantity;
    target->loaned += quantity;
    return 0;
}

int return_book(BookNode *head, const char *isbn, int quantity) {
    // 检查 ISBN 和归还数量是否有效。
    if (!isbn || quantity <= 0) {
        return -1;
    }

    // 定位目标图书节点。
    BookNode *target = search_by_isbn(head, isbn);
    if (!target) {
        return -1;
    }

    // 借阅量不足时拒绝归还，避免出现负数。
    if (target->loaned < quantity) {
        return -1;
    }

    // 更新借阅量与库存量。
    target->loaned -= quantity;
    target->stock += quantity;
    return 0;
}

BookNode *search_by_isbn(BookNode *head, const char *isbn) {
    // ISBN 不能为空，否则无法匹配。
    if (!isbn) {
        return NULL;
    }

    // 线性遍历链表，精确匹配 ISBN。
    for (BookNode *cur = head; cur != NULL; cur = cur->next) {
        if (strcmp(cur->isbn, isbn) == 0) {
            return cur;
        }
    }

    return NULL;
}

BookNode *search_by_keyword(BookNode *head, const char *keyword) {
    // 关键字不能为空。
    if (!keyword) {
        return NULL;
    }

    // 构建独立的结果链表，避免改动原链表。
    BookNode *result_head = NULL;
    BookNode *result_tail = NULL;

    for (BookNode *cur = head; cur != NULL; cur = cur->next) {
        // 书名或作者包含关键字即可命中。
        if (strstr(cur->title, keyword) == NULL && strstr(cur->author, keyword) == NULL) {
            continue;
        }

        // 为命中项创建新节点。
        BookNode *node = (BookNode *)malloc(sizeof(BookNode));
        if (!node) {
            // 分配失败时释放已构建结果链表。
            destroy_list(result_head);
            return NULL;
        }

        // 字段为定长数组，浅拷贝即可安全复制。
        *node = *cur;
        node->next = NULL;

        // 按顺序追加到结果链表尾部。
        if (!result_head) {
            result_head = node;
            result_tail = node;
        } else {
            result_tail->next = node;
            result_tail = node;
        }
    }

    return result_head;
}

void destroy_list(BookNode *head) {
    // 逐节点释放内存，直到链表结束。
    BookNode *cur = head;
    while (cur != NULL) {
        BookNode *next = cur->next;
        free(cur);
        cur = next;
    }
}
