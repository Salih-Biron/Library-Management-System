#include "logic.h"
#include <stdlib.h>

// 交换索引数组中的两个指针，用于排序过程的元素交换。
static void swap_nodes(BookNode **a, BookNode **b) {
    BookNode *tmp = *a;
    *a = *b;
    *b = tmp;
}

// 库存量升序比较：库存量越小越靠前。
static int compare_stock_asc(const BookNode *a, const BookNode *b) {
    if (a->stock < b->stock) {
        return -1;
    }
    if (a->stock > b->stock) {
        return 1;
    }
    return 0;
}

// 借阅量降序比较：借阅量越大越靠前。
static int compare_loan_desc(const BookNode *a, const BookNode *b) {
    if (a->loaned > b->loaned) {
        return -1;
    }
    if (a->loaned < b->loaned) {
        return 1;
    }
    return 0;
}

// Lomuto 分区：以末尾元素为基准，将小于等于基准的放左侧。
static int partition(BookNode **arr, int low, int high,
                     int (*cmp)(const BookNode *, const BookNode *)) {
    BookNode *pivot = arr[high];
    int i = low - 1;
    for (int j = low; j < high; ++j) {
        // 符合比较规则的元素移动到左侧分区。
        if (cmp(arr[j], pivot) <= 0) {
            ++i;
            swap_nodes(&arr[i], &arr[j]);
        }
    }
    // 将基准元素放到分区边界位置。
    swap_nodes(&arr[i + 1], &arr[high]);
    return i + 1;
}

// 快速排序：递归划分左右区间。
static void quick_sort(BookNode **arr, int low, int high,
                       int (*cmp)(const BookNode *, const BookNode *)) {
    if (low < high) {
        int pi = partition(arr, low, high, cmp);
        quick_sort(arr, low, pi - 1, cmp);
        quick_sort(arr, pi + 1, high, cmp);
    }
}

// 将链表节点转为指针数组，便于排序；返回数组长度。
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

// 按排序后的索引数组顺序重建链表的 next 指针。
static void relink_from_index(BookNode **head, BookNode **arr, int count) {
    *head = arr[0];
    for (int i = 0; i < count - 1; ++i) {
        arr[i]->next = arr[i + 1];
    }
    arr[count - 1]->next = NULL;
}

void sort_by_stock(BookNode **head) {
    // 空链表或空指针直接返回。
    if (!head || !*head) {
        return;
    }

    //构建指针索引数组。
    int count = 0;
    BookNode **arr = build_index(*head, &count);
    if (!arr || count < 2) {
        free(arr);
        return;
    }

    // 按库存量升序排序，并重连链表顺序。
    quick_sort(arr, 0, count - 1, compare_stock_asc);
    relink_from_index(head, arr, count);
    free(arr);
}

void sort_by_loan(BookNode **head) {
    // 空链表或空指针直接返回。
    if (!head || !*head) {
        return;
    }

    //构建指针索引数组。
    int count = 0;
    BookNode **arr = build_index(*head, &count);
    if (!arr || count < 2) {
        free(arr);
        return;
    }

    // 按借阅量降序排序，并重连链表顺序。
    quick_sort(arr, 0, count - 1, compare_loan_desc);
    relink_from_index(head, arr, count);
    free(arr);
}
