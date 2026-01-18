#ifndef LIBRARY_USER_H
#define LIBRARY_USER_H

typedef enum {
    ROLE_NONE = -1,
    ROLE_STUDENT = 0,
    ROLE_ADMIN = 1
} UserRole;

 typedef struct User {
    UserRole role;
    char account[50];
    char password[50];
    char question[100];
    char answer[100];
    struct User *next;
} UserNode;

int register_user(UserNode **head, UserRole role, const char *account, const char *password, const char *question, const char *answer);
int verify_login(UserNode *head, const char *account, const char *password, UserRole *out_role);
int verify_secret(UserNode *head, const char *account, const char *answer);
char* get_secret_question(UserNode *head, const char *account);
int change_password(UserNode *head, const char *account, const char *new_password);
int account_exists(UserNode *head, const char *account);
void destroy_user_list(UserNode *head);
UserNode *load_users_from_file(const char *filename);
int save_users_to_file(const char *filename, UserNode *head);

#endif