#ifndef LIBRARY_DATA_H
#define LIBRARY_DATA_H

/**
 * @brief 图书节点结构体
 *
 * 注意：必须使用此结构定义，不可修改
 */
typedef struct Book {
    char isbn[20];     // ISBN编号
    char title[100];   // 书名
    char author[50];   // 作者
    int stock;         // 库存量
    int loaned;        // 借阅量
    struct Book *next; // 指向下一节点
} BookNode;

/**
 * @brief 添加新书到链表
 *
 * @param head 链表头指针的指针
 * @param isbn ISBN编号
 * @param title 书名
 * @param author 作者
 * @param stock 库存量
 * @return int 0=成功, -1=ISBN已存在
 */
int add_book(BookNode **head, const char *isbn, const char *title, const char *author, int stock);

/**
 * @brief 将书本从链表中删除
 *
 * @param head 链表头指针的指针
 * @param isbn 根据ISBN删除书本
 * @return int 0=成功, -1=未找到
 */
int delete_book(BookNode **head, const char *isbn);

/**
 * @brief é€šè¿‡ISBNå€Ÿé˜…å›¾ä¹¦ (å‡å°‘åº“å­˜ï¼Œå¢žåŠ å€Ÿé˜…é‡?)
 *
 * @param head é“¾è¡¨å¤´æŒ‡é’ˆ
 * @param isbn æ ¹æ®ISBNå€Ÿé˜…å›¾ä¹¦
 * @param quantity å€Ÿé˜…æ•°é‡
 * @return int 0=æˆåŠŸ, -1=å¤±è´¥
 */
int loan_book(BookNode *head, const char *isbn, int quantity);

/**
 * @brief 通过ISBN归还图书 (增加库存，减少借阅量).
 *
 * @param head 链表头指针的指针
 * @param isbn 根据ISBN归还图书
 * @param quantity 归还数量
 * @return int 0=成功, -1=未找到, -2=归还数量无效
 */
int return_book(BookNode *head, const char *isbn, int quantity);

/**
 * @brief 通过ISBN精确查找图书
 *
 * @param head 链表头指针
 * @param isbn ISBN编号
 * @return BookNode* 找到的节点指针，NULL=未找到
 */
BookNode *search_by_isbn(BookNode *head, const char *isbn);

/**
 * @brief 按关键词（书名/作者）模糊搜索
 *
 * @param head 链表头指针
 * @param keyword 搜索关键词
 * @return BookNode* 匹配结果链表的头指针
 */
BookNode *search_by_keyword(BookNode *head, const char *keyword);

/**
 * @brief 销毁整个链表，释放内存
 *
 * @param head 链表头指针
 */
void destroy_list(BookNode *head);

#endif // LIBRARY_DATA_H
