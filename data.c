#include "data.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * 功能：将已有图书节点复制一份并追加到结果链表末尾。
 * 返回：成功返回新节点指针，失败返回 NULL（需由调用方释放已建结果链表）。
 */
static BookNode *append_copy_node(BookNode **head, BookNode **tail, const BookNode *src) {
    BookNode *node = (BookNode *)malloc(sizeof(BookNode));
    if (!node) {
        return NULL;
    }
    *node = *src;
    node->next = NULL;
    if (!*head) {
        *head = node;
        *tail = node;
    } else {
        (*tail)->next = node;
        *tail = node;
    }
    return node;
}

/*
 * 功能：向链表尾部添加一本新书。
 * 说明：会检查 ISBN 是否重复，成功后 loaned 置 0。
 * 返回：0=成功，-1=失败（参数无效或 ISBN 重复或内存分配失败）。
 */
int add_book(BookNode **head, const char *isbn, const char *title, const char *author, const char *category, int stock) {
    // 检查指针和必填字段是否有效，避免空指针访问。
    if (!head || !isbn || !title || !author) {
        return -1;
    }

    // 遍历链表，若 ISBN 已存在则返回失败并提示。
    for (BookNode *cur = *head; cur != NULL; cur = cur->next) {
        if (strcmp(cur->isbn, isbn) == 0) {
            printf("错误：ISBN %s 已存在\n", isbn);
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
    snprintf(node->category, sizeof(node->category), "%s", category ? category : "未分类");
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

/*
 * 功能：按 ISBN 删除指定图书节点。
 * 返回：0=成功，-1=未找到或参数无效。
 */
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

/*
 * 功能：借阅指定 ISBN 的图书，减少库存并增加借阅量。
 * 说明：库存不足或参数非法时返回失败。
 * 返回：0=成功，-1=失败。
 */
int loan_book(BookNode *head, const char *isbn, int quantity) {
    // 检查 ISBN 和借阅数量是否有效。
    if (!isbn || quantity <= 0) {
        return -1;
    }

    // 定位目标图书。
    BookNode *target = search_by_isbn(head, isbn);
    if (!target) {
        return -1;
    }

    // 库存不足时拒绝借阅
    if (target->stock < quantity) {
        return -1;
    }

    // 更新库存与借阅量。
    target->stock -= quantity;
    target->loaned += quantity;
    return 0;
}

/*
 * 功能：归还指定 ISBN 的图书，增加库存并减少借阅量。
 * 说明：归还数量超过借阅量或参数非法时返回失败。
 * 返回：0=成功，-1=失败。
 */
int return_book(BookNode *head, const char *isbn, int quantity) {
    // 检查 ISBN 和归还数量是否有效。
    if (!isbn || quantity <= 0) {
        return -1;
    }

    // 定位目标图书。
    BookNode *target = search_by_isbn(head, isbn);
    if (!target) {
        return -1;
    }

    // 借阅量不足时拒绝归还
    if (target->loaned < quantity) {
        return -1;
    }

    // 更新借阅量与库存量。
    target->loaned -= quantity;
    target->stock += quantity;
    return 0;
}

/*
 * 功能：按 ISBN 精确查找单本图书。
 * 返回：找到则返回节点指针，未找到返回 NULL。
 */
BookNode *search_by_isbn(BookNode *head, const char *isbn) {
    // ISBN 不能为空
    if (!isbn) {
        return NULL;
    }

    //遍历链表，精确匹配 ISBN。
    for (BookNode *cur = head; cur != NULL; cur = cur->next) {
        if (strcmp(cur->isbn, isbn) == 0) {
            return cur;
        }
    }

    return NULL;
}

/*
 * 功能：按关键词模糊搜索（书名或作者包含关键字即可匹配）。
 * 说明：返回的是新建结果链表，使用后需释放。
 * 返回：结果链表头指针，未命中返回 NULL。
 */
BookNode *search_by_keyword(BookNode *head, const char *keyword) {
    // 关键字不能为空。
    if (!keyword) {
        return NULL;
    }

    BookNode *result_head = NULL;
    BookNode *result_tail = NULL;

    for (BookNode *cur = head; cur != NULL; cur = cur->next) {
        // 书名、作者或分类包含关键字即可命中。
        if (strstr(cur->title, keyword) == NULL && strstr(cur->author, keyword) == NULL && strstr(cur->category, keyword) == NULL) {
            continue;
        }

        // 为命中项创建新节点。
        BookNode *node = (BookNode *)malloc(sizeof(BookNode));
        if (!node) {
            // 分配失败时释放已构建结果链表。
            destroy_list(result_head);
            return NULL;
        }
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

/*
 * 功能：按书名精确匹配搜索。
 * 说明：返回的新链表需要释放。
 * 返回：结果链表头指针，未命中返回 NULL。
 */
BookNode *search_by_title(BookNode *head, const char *title) {
    if (!title) {
        return NULL;
    }

    BookNode *result_head = NULL;
    BookNode *result_tail = NULL;

    for (BookNode *cur = head; cur != NULL; cur = cur->next) {
        if (strcmp(cur->title, title) != 0) {
            continue;
        }

        if (!append_copy_node(&result_head, &result_tail, cur)) {
            destroy_list(result_head);
            return NULL;
        }
    }

    return result_head;
}

/*
 * 功能：按作者精确匹配搜索。
 * 说明：返回的新链表需要释放。
 * 返回：结果链表头指针，未命中返回 NULL。
 */
BookNode *search_by_author(BookNode *head, const char *author) {
    if (!author) {
        return NULL;
    }

    BookNode *result_head = NULL;
    BookNode *result_tail = NULL;

    for (BookNode *cur = head; cur != NULL; cur = cur->next) {
        if (strcmp(cur->author, author) != 0) {
            continue;
        }

        if (!append_copy_node(&result_head, &result_tail, cur)) {
            destroy_list(result_head);
            return NULL;
        }
    }

    return result_head;
}

/*
 * 功能：按 ISBN 修改图书信息（书名/作者/库存）。
 * 返回：0=成功，-1=未找到或参数无效。
 */
int update_book(BookNode *head, const char *isbn, const char *title, const char *author, const char *category, int stock) {
    if (!isbn || !title || !author || stock < 0) {
        return -1;
    }

    BookNode *target = search_by_isbn(head, isbn);
    if (!target) {
        return -1;
    }

    snprintf(target->title, sizeof(target->title), "%s", title);
    snprintf(target->author, sizeof(target->author), "%s", author);
    snprintf(target->category, sizeof(target->category), "%s", category ? category : "未分类");
    target->stock = stock;
    return 0;
}

/*
 * 功能：释放链表所有节点内存。
 */
void destroy_list(BookNode *head) {
    // 逐节点释放内存，直到链表结束。
    BookNode *cur = head;
    while (cur != NULL) {
        BookNode *next = cur->next;
        free(cur);
        cur = next;
    }
}
