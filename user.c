#include "user.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define USERS_FILE "users.dat"

static int append_user(UserNode **head, UserNode **tail, UserRole role, const char *account, const char *password, const char *question, const char *answer) {
    if (!head || !account || !password || !question || !answer) {
        return -1;
    }

    UserNode *node = (UserNode *)malloc(sizeof(UserNode));
    if (!node) {
        return -1;
    }

    node->role = role;
    snprintf(node->account, sizeof(node->account), "%s", account);
    snprintf(node->password, sizeof(node->password), "%s", password);
    snprintf(node->question, sizeof(node->question), "%s", question);
    snprintf(node->answer, sizeof(node->answer), "%s", answer);
    node->next = NULL;

    if (!*head) {
        *head = node;
        *tail = node;
    } else {
        (*tail)->next = node;
        *tail = node;
    }

    return 0;
}

int register_user(UserNode **head, UserRole role, const char *account, const char *password, const char *question, const char *answer) {
    if (!head || !account || !password || !question || !answer) {
        return -1;
    }

    if (account_exists(*head, account)) {
        return -1;
    }

    UserNode *tail = *head;
    if (tail) {
        while (tail->next) {
            tail = tail->next;
        }
    }

    return append_user(head, &tail, role, account, password, question, answer);
}

int verify_login(UserNode *head, const char *account, const char *password, UserRole *out_role) {
    if (!head || !account || !password) {
        return -1;
    }

    for (UserNode *cur = head; cur != NULL; cur = cur->next) {
        if (strcmp(cur->account, account) == 0 && strcmp(cur->password, password) == 0) {
            if (out_role) {
                *out_role = cur->role;
            }
            return 0;
        }
    }

    return -1;
}

int verify_secret(UserNode *head, const char *account, const char *answer) {
    if (!head || !account || !answer) {
        return -1;
    }

    for (UserNode *cur = head; cur != NULL; cur = cur->next) {
        if (strcmp(cur->account, account) == 0 && strcmp(cur->answer, answer) == 0) {
            return 0;
        }
    }

    return -1;
}

char* get_secret_question(UserNode *head, const char *account) {
    if (!head || !account) {
        return NULL;
    }

    for (UserNode *cur = head; cur != NULL; cur = cur->next) {
        if (strcmp(cur->account, account) == 0) {
            return cur->question;
        }
    }

    return NULL;
}

int change_password(UserNode *head, const char *account, const char *new_password) {
    if (!head || !account || !new_password) {
        return -1;
    }

    for (UserNode *cur = head; cur != NULL; cur = cur->next) {
        if (strcmp(cur->account, account) == 0) {
            snprintf(cur->password, sizeof(cur->password), "%s", new_password);
            return 0;
        }
    }

    return -1;
}

int account_exists(UserNode *head, const char *account) {
    if (!head || !account) {
        return 0;
    }

    for (UserNode *cur = head; cur != NULL; cur = cur->next) {
        if (strcmp(cur->account, account) == 0) {
            return 1;
        }
    }

    return 0;
}

void destroy_user_list(UserNode *head) {
    UserNode *cur = head;
    while (cur != NULL) {
        UserNode *next = cur->next;
        free(cur);
        cur = next;
    }
}

UserNode *load_users_from_file(const char *filename) {
    if (!filename) {
        filename = USERS_FILE;
    }

    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        return NULL;
    }

    UserNode *head = NULL;
    UserNode *tail = NULL;

    UserRole role;
    char account[50];
    char password[50];
    char question[100];
    char answer[100];

    while (fread(&role, sizeof(UserRole), 1, fp) == 1) {
        if (fread(account, sizeof(account), 1, fp) != 1) {
            break;
        }
        if (fread(password, sizeof(password), 1, fp) != 1) {
            break;
        }
        if (fread(question, sizeof(question), 1, fp) != 1) {
            break;
        }
        if (fread(answer, sizeof(answer), 1, fp) != 1) {
            break;
        }

        if (append_user(&head, &tail, role, account, password, question, answer) != 0) {
            destroy_user_list(head);
            fclose(fp);
            return NULL;
        }
    }

    fclose(fp);
    return head;
}

int save_users_to_file(const char *filename, UserNode *head) {
    if (!filename) {
        filename = USERS_FILE;
    }

    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        return -1;
    }

    for (UserNode *cur = head; cur != NULL; cur = cur->next) {
        fwrite(&cur->role, sizeof(UserRole), 1, fp);
        fwrite(cur->account, sizeof(cur->account), 1, fp);
        fwrite(cur->password, sizeof(cur->password), 1, fp);
        fwrite(cur->question, sizeof(cur->question), 1, fp);
        fwrite(cur->answer, sizeof(cur->answer), 1, fp);
    }

    fclose(fp);
    return 0;
}