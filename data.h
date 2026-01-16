#ifndef LIBRARY_DATA_H
#define LIBRARY_DATA_H

/**
 * @brief 图书节点结构体（链表节点）
 *
 * 说明：该结构用于存储单本图书信息以及链表指针。
 */
typedef struct Book {
    char isbn[20];     // ISBN 编号
    char title[100];   // 书名
    char author[50];   // 作者
    int stock;         // 库存量
    int loaned;        // 借阅量
    struct Book *next; // 指向下一个节点
} BookNode;

/**
 * @brief 添加新书到链表末尾
 *
 * @param head 链表头指针的指针
 * @param isbn ISBN 编号
 * @param title 书名
 * @param author 作者
 * @param stock 库存量
 * @return int 0=成功, -1=失败（ISBN 重复/参数无效/内存分配失败）
 */
int add_book(BookNode **head, const char *isbn, const char *title, const char *author, int stock);

/**
 * @brief 按 ISBN 删除图书
 *
 * @param head 链表头指针的指针
 * @param isbn ISBN 编号
 * @return int 0=成功, -1=未找到或参数无效
 */
int delete_book(BookNode **head, const char *isbn);

/**
 * @brief 借阅图书（减少库存，增加借阅量）
 *
 * @param head 链表头指针
 * @param isbn ISBN 编号
 * @param quantity 借阅数量
 * @return int 0=成功, -1=失败（库存不足/未找到/参数无效）
 */
int loan_book(BookNode *head, const char *isbn, int quantity);

/**
 * @brief 归还图书（增加库存，减少借阅量）
 *
 * @param head 链表头指针
 * @param isbn ISBN 编号
 * @param quantity 归还数量
 * @return int 0=成功, -1=失败（借阅量不足/未找到/参数无效）
 */
int return_book(BookNode *head, const char *isbn, int quantity);

/**
 * @brief 按 ISBN 精确查找图书
 *
 * @param head 链表头指针
 * @param isbn ISBN 编号
 * @return BookNode* 找到返回节点指针，未找到返回 NULL
 */
BookNode *search_by_isbn(BookNode *head, const char *isbn);

/**
 * @brief 关键词模糊搜索（书名或作者包含关键字）
 *
 * @param head 链表头指针
 * @param keyword 关键字
 * @return BookNode* 匹配结果链表头指针，未匹配返回 NULL
 */
BookNode *search_by_keyword(BookNode *head, const char *keyword);

/**
 * @brief 按书名精确匹配搜索
 *
 * @param head 链表头指针
 * @param title 需要精确匹配的书名
 * @return BookNode* 匹配结果链表头指针，未匹配返回 NULL
 */
BookNode *search_by_title(BookNode *head, const char *title);

/**
 * @brief 按作者精确匹配搜索
 *
 * @param head 链表头指针
 * @param author 需要精确匹配的作者名
 * @return BookNode* 匹配结果链表头指针，未匹配返回 NULL
 */
BookNode *search_by_author(BookNode *head, const char *author);

/**
 * @brief 按 ISBN 修改图书信息
 *
 * @param head 链表头指针
 * @param isbn ISBN 编号
 * @param title 新书名
 * @param author 新作者
 * @param stock 新库存
 * @return int 0=成功, -1=未找到或参数无效
 */
int update_book(BookNode *head, const char *isbn, const char *title, const char *author, int stock);

/**
 * @brief 释放链表所有节点内存
 *
 * @param head 链表头指针
 */
void destroy_list(BookNode *head);

#endif // LIBRARY_DATA_H
