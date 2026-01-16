#ifndef LIBRARY_LOGIC_H
#define LIBRARY_LOGIC_H

#include "data.h"

/**
 * @brief 按库存量升序排序（快速排序）
 *
 * @param head 链表头指针的指针
 */
void sort_by_stock(BookNode **head);

/**
 * @brief 按借阅量降序排序
 *
 * @param head 链表头指针的指针
 */
void sort_by_loan(BookNode **head);

/**
 * @brief 生成统计报告
 *
 * @param head 链表头指针
 */
void generate_report(BookNode *head);

#endif // LIBRARY_LOGIC_H
