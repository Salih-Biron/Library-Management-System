## 项目概述

- 开发一个基于控制台（或可选图形界面）的图书管理系统，实现对图书信息和借阅记录的基本管理功能，帮助用户高效地完成图书的录入、查询、借阅与归还等操作。

## 需求分析

### 1.1用户使用权限

管理员

- 增加图书
- 删除图书
- 图书批量导入
- 按库存/借阅量排序
- 导出操作日志
- 导出借阅数据
- 导出图书数据到CSV
- 导出图书数据到JSON
- 退出登录

学生

- 查询图书
- 查看所有图书
- 借阅图书
- 归还图书
- 查看借阅历史
- 导出借阅数据
- 退出登录

### 1.2图书管理

- 添加、编辑、删除图书信息（书名、作者、ISBN）
- 支持按书名、作者、分类进行模糊搜索

### 1.3借阅与归还

- 借阅图书
  输入ISBN
  确认借阅
  库存不足返回主菜单
- 归还图书
  输入ISBN
  确认归还

### 1.4查询和排序

- 查询图书
  支持输入精准书名、ISBN、作者名字和模糊搜索书名、作者名字
- 按库存/借阅量排序
  按库存量从少到多排序

### 1.5借阅记录管理

- 读者可查看自己的借阅历史（含未还与已还以及借阅时间和书本）

### 1.6用户管理

- 注册新用户（账号、密码）
- 设置密保
- 选择身份

## 架构设计

采用分层架构模式，分为以下几个层次：

- **表示层（Presentation Layer）**：负责用户界面展示和用户交互，由[main.c](file:///d:/vscode/Library-Management-System/main.c)和[terminal.c](file:///d:/vscode/Library-Management-System/terminal.c)组成

- **业务逻辑层（Business Logic Layer）**：实现核心业务逻辑，由[logic.c](file:///d:/vscode/Library-Management-System/logic.c)和[user.c](file:///d:/vscode/Library-Management-System/user.c)组成

- **数据访问层（Data Access Layer）**：负责数据的存储和持久化，由[data.c](file:///d:/vscode/Library-Management-System/data.c)和[store.c](file:///d:/vscode/Library-Management-System/store.c)组成

- **数据模型层（Data Model Layer）**：定义数据结构，由[data.h](file:///d:/vscode/Library-Management-System/data.h)定义

```

+------------------------+

|     表示层              |

|  (main.c, terminal.c)  |

+------------------------+

|    业务逻辑层            |

|  (logic.c, user.c)     |

+------------------------+

|    数据访问层            |

|  (data.c, store.c)     |

+------------------------+

|    数据模型层            |

|      (data.h)          |

+------------------------+

```

# 数据结构设计

图书馆管理系统主要包含以下数据结构：

- 图书节点结构 (BookNode)

- 用户节点结构 (UserNode)

- 用户角色枚举 (UserRole)

## 3. 图书数据结构

### 3.1 图书节点结构定义

```c

typedef struct BookNode {

    char isbn[20];              // ISBN号，最长20个字符

    char title[100];            // 书名，最长100个字符

    char author[50];            // 作者，最长50个字符

    char category[50];          // 分类，最长50个字符

    int stock;                  // 库存数量

    int loaned;                 // 已借出数量

    struct BookNode *next;      // 指向下一个节点的指针

} BookNode;

```

### 3.2 字段说明

- **isbn**: 图书的国际标准书号，用于唯一标识一本图书，长度固定为20字节

- **title**: 图书标题，最大支持100个字符

- **author**: 图书作者姓名，最大支持50个字符

- **category**: 图书所属类别，最大支持50个字符

- **stock**: 当前库存数量，整型变量

- **loaned**: 已借出的数量，整型变量

- **next**: 指向链表中下一个图书节点的指针，用于构建图书链表

### 3.3 内存布局

```

+----------+----------+----------+----------+----------+----------+----------+

|   isbn   |  title   |  author  | category |  stock   | loaned   |  next    |

| [20字节] |[100字节] | [50字节] | [50字节] | [4字节]  | [4字节]  | [8字节]  |

+----------+----------+----------+----------+----------+----------+----------+

```

### 3.4 链表结构

图书信息以单向链表的形式组织，便于动态插入和删除图书记录：

```

[BookNode1] -> [BookNode2] -> [BookNode3] -> ... -> [NULL]

```

## 4. 用户数据结构

### 4.1 用户角色枚举

```c

typedef enum {

    ROLE_NONE = 0,              // 无角色

    ROLE_STUDENT = 1,           // 学生角色

    ROLE_ADMIN = 2              // 管理员角色

} UserRole;

```

### 4.2 用户节点结构定义

```c

typedef struct UserNode {

    UserRole role;              // 用户角色

    char account[50];           // 账号，最长50个字符

    char password[50];          // 密码，最长50个字符

    char secret_question[100];  // 密保问题，最长100个字符

    char secret_answer[100];    // 密保答案，最长100个字符

    struct UserNode *next;      // 指向下一个用户节点的指针

} UserNode;

```

### 4.3 字段说明

- **role**: 用户角色，区分学生和管理员

- **account**: 用户账号，最大支持50个字符

- **password**: 用户密码，最大支持50个字符

- **secret_question**: 密保问题，用于密码找回功能，最大支持100个字符

- **secret_answer**: 密保问题的答案，最大支持100个字符

- **next**: 指向链表中下一个用户节点的指针

### 4.4 内存布局

```

+----------+----------+----------+----------+------------------+------------------+----------+

|   role   | account  | password | sec_ques |   sec_answer     |   sec_answer     |  next    |

|  [4字节] | [50字节] | [50字节] |[100字节]|     (续上)       |    [8字节]       | [8字节]  |

+----------+----------+----------+----------+------------------+------------------+----------+

```

### 4.5 用户链表结构

用户信息同样以单向链表的形式组织：

```

[UserNode1] -> [UserNode2] -> [UserNode3] -> ... -> [NULL]

```

## 5. 数据关系图

```

+-------------------+                    +-------------------+

|                   | 1              N   |                   |

|   User System     |--------------------|   Book System     |

|                   |  Manage            |                   |

+-------------------+                    +-------------------+

| - UserRole        |                    | - isbn            |

| - account[50]     |                    | - title[100]      |

| - password[50]    |                    | - author[50]      |

| - secret_q[100]   |                    | - category[50]    |

| - secret_a[100]   |                    | - stock           |

| - *next           |                    | - loaned          |

+-------------------+                    | - *next           |

                                        +-------------------+

```

## 6. 数据操作接口

### 6.1 图书操作接口

- add_book() - 添加图书

- delete_book() - 删除图书

- search_by_keyword() - 关键词搜索图书

- search_by_isbn() - 按ISBN搜索图书

- loan_book() - 借阅图书

- return_book() - 归还图书

- sort_by_stock() - 按库存排序

- sort_by_loan() - 按借阅量排序

### 6.2 用户操作接口

- verify_login() - 验证登录

- register_user() - 注册用户

- change_password() - 修改密码

- account_exists() - 检查账号是否存在

- get_secret_question() - 获取密保问题

- verify_secret() - 验证密保答案

## 模块设计

| 模块             |                                                                                                                                                                                                                                       模块功能 |         模块之间的关系         |
| ---------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------: | :----------------------------: |
| 1.进入主界面     |                                                                                                                                                                                                                     1.进入主程序<br>2.退出系统 |        是所有板块的起始        |
| 2.注册账号界面   |                                                                                                                                                                                       1.输入账号<br>2.设置账号<br>3.再次输入密码<br>4.设置密保 |          板块1->板块2          |
| 3.身份选择       |                                                                                                                                                                                                                             1.学生<br>2.管理员 |  板块2->板块3<br>板块3->板块4  |
| 4.账号登陆界面   |                                                                                                                                                                                                         1.学生账号<br>2.密码输入<br>3.忘记密码 | 板块3->板块4（除功能3）->板块8 |
| 5.权限管理系统   |                                                                                                                                                                                                                                     1.输入账号 | 板块4的忘记密码->板块5->板块6  |
| 6.密保系统       |                                                                                                                                                                                                                         输入自己设置的密保答案 |          板块6->板块7          |
| 7.重设置密码界面 |                                                                                                                                                                                                 1.输入新的密码<br>2.再次输入新的密码<br>3.提交 | 板块7->板块4（除功能3）->板块8 |
| 8.主菜单         | 1.添加图书<br>2.查询图书（支持模糊搜索'书名'和'作者'、精准匹配'ISBN')<br>3.删除图书<br>4.借阅图书<br>5.归还图书<br>6.按库存/借阅量排序<br>7.导出数据（CSV/JSON）<br>8.退出系统<br>9.其他问题：Telephone:131-271-3397 or email:IDWK@outlook.com |         整个系统的核心         |

# 详细设计

## 1. 核心数据结构

### 1.1 图书节点结构

```c

typedef struct BookNode {

    char isbn[20];              // ISBN号，最长20个字符

    char title[100];            // 书名，最长100个字符

    char author[50];            // 作者，最长50个字符

    char category[50];          // 分类，最长50个字符

    int stock;                  // 库存数量

    int loaned;                 // 已借出数量

    struct BookNode *next;      // 指向下一个节点的指针

} BookNode;

```

### 1.2 用户节点结构

```c

typedef enum {

    ROLE_NONE = 0,              // 无角色

    ROLE_STUDENT = 1,           // 学生角色

    ROLE_ADMIN = 2              // 管理员角色

} UserRole;



typedef struct UserNode {

    UserRole role;              // 用户角色

    char account[50];           // 账号，最长50个字符

    char password[50];          // 密码，最长50个字符

    char secret_question[100];  // 密保问题，最长100个字符

    char secret_answer[100];    // 密保答案，最长100个字符

    struct UserNode *next;      // 指向下一个用户节点的指针

} UserNode;

```

## 2. 核心功能函数

### 2.1 图书管理函数

#### 添加图书

```c

int add_book(BookNode **head, const char *isbn, const char *title,

             const char *author, const char *category, int stock);

```

- 功能：在图书链表中添加新图书
- 参数：head指向头节点指针的指针，其余为图书属性
- 返回值：成功返回0，失败返回非0

#### 删除图书

```c

int delete_book(BookNode **head, const char *isbn);

```

- 功能：根据ISBN删除图书
- 参数：head指向头节点指针的指针，isbn为待删除图书的ISBN
- 返回值：成功返回0，失败返回非0

#### 搜索图书

```c

BookNode* search_by_keyword(BookNode *head, const char *keyword);

```

- 功能：根据关键词搜索图书（匹配书名、作者或ISBN）
- 参数：head为链表头节点，keyword为搜索关键词
- 返回值：匹配的图书节点链表

#### 搜索特定ISBN图书

```c

BookNode* search_by_isbn(BookNode *head, const char *isbn);

```

- 功能：根据ISBN精确查找图书
- 参数：head为链表头节点，isbn为搜索的ISBN号
- 返回值：匹配的图书节点或NULL

#### 借阅图书

```c

int loan_book(BookNode *head, const char *isbn, int quantity);

```

- 功能：借阅指定数量的图书
- 参数：head为链表头节点，isbn为图书ISBN，quantity为借阅数量
- 返回值：成功返回0，失败返回非0

#### 归还图书

```c

int return_book(BookNode *head, const char *isbn, int quantity);

```

- 功能：归还指定数量的图书
- 参数：head为链表头节点，isbn为图书ISBN，quantity为归还数量
- 返回值：成功返回0，失败返回非0

### 2.2 用户管理函数

#### 用户验证

```c

int verify_login(UserNode *users, const char *account,

                 const char *password, UserRole *role);

```

- 功能：验证用户账号和密码
- 参数：users为用户链表，account为账号，password为密码，role为输出参数
- 返回值：成功返回0，失败返回非0

#### 注册用户

```c

int register_user(UserNode **users, UserRole role, const char *account,

                  const char *password, const char *question, const char *answer);

```

- 功能：注册新用户
- 参数：users指向用户链表头节点指针的指针，role为用户角色，其余为用户信息
- 返回值：成功返回0，失败返回非0

#### 修改密码

```c

int change_password(UserNode *users, const char *account, const char *new_password);

```

- 功能：修改用户密码

- 参数：users为用户链表，account为账号，new_password为新密码

- 返回值：成功返回0，失败返回非0

## 3. 文件存储格式

### 3.1 数据持久化格式

系统支持两种数据存储格式：

#### DAT二进制格式

- 使用`fread`和`fwrite`进行二进制读写

- 包含完整的图书链表信息

- 每本书籍信息包括：ISBN、标题、作者、分类、库存、借阅量

#### JSON文本格式

- 作为向后兼容的存储方式

- 使用标准JSON格式存储图书数据

### 3.2 用户数据存储格式

- 使用自定义二进制格式保存用户信息

- 包括角色、账号、加密密码、密保问题和答案

## 4. 算法实现

### 4.1 排序算法

#### 按库存排序

```c

void sort_by_stock(BookNode **head) {

    if (!head || !(*head) || !(*head)->next) return;

    int swapped;

    BookNode *ptr;

    BookNode *last = NULL;

    do {

        swapped = 0;

        ptr = *head;

        while (ptr->next != last) {

            if (ptr->stock > ptr->next->stock) {

                // 交换数据

                swap_book_nodes(ptr, ptr->next);

                swapped = 1;

            }

            ptr = ptr->next;

        }

        last = ptr;

    } while (swapped);

}

```

#### 按借阅量排序

```c

void sort_by_loan(BookNode **head) {

    if (!head || !(*head) || !(*head)->next) return;

    int swapped;

    BookNode *ptr;

    BookNode *last = NULL;

    do {

        swapped = 0;

        ptr = *head;

        while (ptr->next != last) {

            if (ptr->loaned < ptr->next->loaned) {  // 按降序排列

                swap_book_nodes(ptr, ptr->next);

                swapped = 1;

            }

            ptr = ptr->next;

        }

        last = ptr;

    } while (swapped);

}

```

### 4.2 搜索算法

#### 关键词匹配算法

```c

BookNode* search_by_keyword(BookNode *head, const char *keyword) {

    BookNode *result_head = NULL;

    BookNode *result_tail = NULL;

    for (BookNode *current = head; current != NULL; current = current->next) {

        if (strstr(current->title, keyword) ||

            strstr(current->author, keyword) ||

            strstr(current->isbn, keyword) ||

            strstr(current->category, keyword)) {

            BookNode *new_node = malloc(sizeof(BookNode));

            memcpy(new_node, current, sizeof(BookNode));

            new_node->next = NULL;

            if (!result_head) {

                result_head = result_tail = new_node;

            } else {

                result_tail->next = new_node;

                result_tail = new_node;

            }

        }

    }

    return result_head;

}

```

## 5. 错误处理机制

### 5.1 输入验证

- 对所有用户输入进行长度检查，防止缓冲区溢出
- 验证ISBN格式、数字输入的有效性
- 检查文件路径的安全性

### 5.2 内存管理

- 在创建新节点时检查malloc返回值，防止内存分配失败
- 在删除节点时释放相关内存，避免内存泄漏
- 使用安全字符串函数如strncpy替代strcpy

### 5.3 文件操作错误处理

```c

FILE *fp = fopen(filename, "rb");

if (!fp) {

    fprintf(stderr, "无法打开文件: %s\n", filename);

    return -1;  // 错误代码

}



// 读取文件内容

size_t result = fread(buffer, 1, size, fp);

if (result != size) {

    if (feof(fp)) {

        fprintf(stderr, "提前到达文件末尾\n");

    } else if (ferror(fp)) {

        fprintf(stderr, "读取文件时发生错误\n");

    }

    fclose(fp);

    return -1;  // 错误代码

}



fclose(fp);

```

### 5.4 业务逻辑错误处理

- 借阅时检查库存是否充足
- 归还时检查借阅记录是否足够
- 添加图书时检查ISBN是否重复
- 删除图书时检查图书是否存在

## 6. 日志与审计功能

### 6.1 操作日志

- 记录所有重要操作（添加、删除、借阅、归还等）
- 包含时间戳、操作类型、ISBN、书名等信息

  ### 6.2 借阅历史

- 记录每次借阅和归还操作
- 可以导出为CSV或JSON格式

## 7. 安全机制

### 7.1 用户认证

- 实现多级用户权限（学生、管理员）
- 提供密码找回功能（通过密保问题）

### 7.2 输入过滤

- 防止SQL注入类似的攻击（虽然没有使用数据库）
- 限制输入长度，防止缓冲区溢出

## 测试用例设计

### 1.图书管理模块

| 功能         |                                       输入数据/操作 |         预期结果         |
| ------------ | --------------------------------------------------: | :----------------------: |
| 添加图书     |                                      输入ISBN和书名 |       图书成功入库       |
| 删除图书     |                                            输入ISBN |       图书成功出库       |
| 查询图书     | 精准输入书名/ISBN/作者名字或者模糊输入书名/作者名字 |   返回所有匹配图书列表   |
| 按库存量排序 |                 管理员选择"按库存量-->从少到多排序" | 图书列表首相为最小值图书 |
| 按借阅量排序 |                 管理员选择按“借阅量-->从少到多排序” | 图书列表首项为最小值图书 |

### 2.用户管理模块

| 功能     |                输入数据/操作 |          预期结果          |
| -------- | ---------------------------: | :------------------------: |
| 注册账号 | 输入账号、设置密码、设置密保 | 用户成功创建，开始选择身份 |
| 登陆账号 |               输入账号和密码 |      进入各身份主菜单      |
| 身份选择 |           选择管理员或者学生 |      进入账号登陆界面      |
| 设置密保 |               设置问题和答案 |        进入身份选择        |

### 3.借还功能模块

| 功能     |          输入数据/操作 |              预期结果              |
| -------- | ---------------------: | :--------------------------------: |
| 借阅图书 |         输入书名或ISBN | 成功借阅并导出借阅时间和书本的数据 |
| 归还图书 | 输入ISBN，点击确认归还 |              归还成功              |

## 项目分工

| 成员     | 负责模块                                                    | 具体任务                                                                                                            |
| -------- | ----------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------- |
| 裴芯南   | `design.md`                                                 | `设计文档`                                                                                                          |
| 覃文杰   | `logic.c`/`logic.h`和`data.c`/`data.h`和`store.c`/`store.h` | `数据容器：链表操作（`add_book`, `destroy_list`）业务逻辑：查询、排序、统计 文件接口：`log_loan`, `export_to_csv` ` |
| 黄浚铭   | `所有模块`                                                    | 撰写`测试报告`                                                                                            |
