#ifndef LIBRARY_STORE_H
#define LIBRARY_STORE_H

#include "data.h"

/**
 * @brief 记录借阅操作到二进制日志
 *
 * @param isbn ISBN 编号
 * @param title 书名
 * @param quantity 借阅数量
 */
void log_loan(const char *isbn, const char *title, int quantity);

/**
 * @brief 记录归还操作到二进制日志
 *
 * @param isbn ISBN 编号
 * @param title 书名
 * @param quantity 归还数量
 */
void log_return(const char *isbn, const char *title, int quantity);

/**
 * @brief 记录操作日志（文本）
 *
 * @param action 操作名称
 * @param isbn ISBN（可为空）
 * @param title 书名（可为空）
 */
void log_operation(const char *action, const char *isbn, const char *title);

/**
 * @brief 导出操作日志到指定文件
 *
 * @param filename 输出文件名
 * @return int 0=成功, -1=失败
 */
int export_operation_log(const char *filename);

/**
 * @brief 导出学生借阅数据（仅借阅时间与书名）
 *
 * @param filename 输出文件名
 * @return int 0=成功, -1=失败
 */
int export_borrow_data(const char *filename);

/**
 * @brief 打印借阅历史（已还/未还、借阅时间、书名）
 */
void print_borrow_history(void);

/**
 * @brief 从借阅日志加载历史记录并同步库存/借阅量
 *
 * @param head 链表头指针
 */
void load_loans(BookNode *head);

/**
 * @brief 持久化图书信息到 JSON 文件（系统内部使用）
 *
 * @param filename 输出文件名
 * @param head 链表头指针
 * @return int 0=成功, -1=失败
 */
int persist_books_json(const char *filename, BookNode *head);

/**
 * @brief 从 JSON 文件恢复图书信息
 *
 * @param filename 输入文件名
 * @return BookNode* 恢复后的链表头指针（NULL 表示失败）
 */
BookNode *load_books_from_json(const char *filename);

/**
 * @brief 导出图书数据到 CSV 文件（外部使用）
 *
 * @param filename 输出文件名
 * @param head 链表头指针
 */
void export_to_csv(const char *filename, BookNode *head);

/**
 * @brief 导出图书数据到 JSON 文件（外部使用）
 *
 * @param filename 输出文件名
 * @param head 链表头指针
 */
void export_to_json(const char *filename, BookNode *head);

#endif // LIBRARY_STORE_H
