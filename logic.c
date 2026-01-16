#include "logic.h"
#include <stdlib.h>

/*
 * 功能：交换索引数组中的两个节点指针。
 */
static void swap_nodes(BookNode **a, BookNode **b) {
    BookNode *tmp = *a;
    *a = *b;
    *b = tmp;
}

/*
 * 功能：比较库存量，升序排序（库存越小越靠前）。
 */
static int compare_stock_asc(const BookNode *a, const BookNode *b) {
    if (a->stock < b->stock) {
        return -1;
    }
    if (a->stock > b->stock) {
        return 1;
    }
    return 0;
}

/*
 * 功能：比较借阅量，降序排序（借阅量越大越靠前）。
 */
static int compare_loan_desc(const BookNode *a, const BookNode *b) {
    if (a->loaned > b->loaned) {
        return -1;
    }
    if (a->loaned < b->loaned) {
        return 1;
    }
    return 0;
}

/*
 * 功能：Lomuto 分区，将小于等于基准的元素放到左侧。
 */
static int partition(BookNode **arr, int low, int high,
                     int (*cmp)(const BookNode *, const BookNode *)) {
    BookNode *pivot = arr[high];
    int i = low - 1;
    for (int j = low; j < high; ++j) {
        if (cmp(arr[j], pivot) <= 0) {
            ++i;
            swap_nodes(&arr[i], &arr[j]);
        }
    }
    swap_nodes(&arr[i + 1], &arr[high]);
    return i + 1;
}

/*
 * 功能：快速排序，递归划分左右区间。
 */
static void quick_sort(BookNode **arr, int low, int high,
                       int (*cmp)(const BookNode *, const BookNode *)) {
    if (low < high) {
        int pi = partition(arr, low, high, cmp);
        quick_sort(arr, low, pi - 1, cmp);
        quick_sort(arr, pi + 1, high, cmp);
    }
}

/*
 * 功能：将链表节点转为指针数组，便于排序操作。
 */
static BookNode **build_index(BookNode *head, int *out_count) {
    int count = 0;
    for (BookNode *cur = head; cur != NULL; cur = cur->next) {
        ++count;
    }
    if (count == 0) {
        *out_count = 0;
        return NULL;
    }

    BookNode **arr = (BookNode **)malloc(sizeof(BookNode *) * (size_t)count);
    if (!arr) {
        *out_count = 0;
        return NULL;
    }

    int idx = 0;
    for (BookNode *cur = head; cur != NULL; cur = cur->next) {
        arr[idx++] = cur;
    }
    *out_count = count;
    return arr;
}

/*
 * 功能：按排序后的索引数组重新链接链表 next 指针。
 */
static void relink_from_index(BookNode **head, BookNode **arr, int count) {
    *head = arr[0];
    for (int i = 0; i < count - 1; ++i) {
        arr[i]->next = arr[i + 1];
    }
    arr[count - 1]->next = NULL;
}

/*
 * 功能：按库存量升序排序图书链表。
 */
void sort_by_stock(BookNode **head) {
    if (!head || !*head) {
        return;
    }

    int count = 0;
    BookNode **arr = build_index(*head, &count);
    if (!arr || count < 2) {
        free(arr);
        return;
    }

    quick_sort(arr, 0, count - 1, compare_stock_asc);
    relink_from_index(head, arr, count);
    free(arr);
}

/*
 * 功能：按借阅量降序排序图书链表。
 */
void sort_by_loan(BookNode **head) {
    if (!head || !*head) {
        return;
    }

    int count = 0;
    BookNode **arr = build_index(*head, &count);
    if (!arr || count < 2) {
        free(arr);
        return;
    }

    quick_sort(arr, 0, count - 1, compare_loan_desc);
    relink_from_index(head, arr, count);
    free(arr);
}
