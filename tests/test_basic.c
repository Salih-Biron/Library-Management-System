#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../data.h"
#include "../user.h"

static int failures = 0;

#define ASSERT(cond, msg) \
    do { if (!(cond)) { printf("FAIL: %s\n", msg); ++failures; } else { printf("OK: %s\n", msg); } } while(0)

void test_data_basic() {
    BookNode *head = NULL;
    ASSERT(add_book(&head, "ISBN1", "Test Book", "Alice", "Sci", 5) == 0, "add_book returns 0");
    BookNode *b = search_by_isbn(head, "ISBN1");
    ASSERT(b != NULL, "search_by_isbn finds inserted book");
    ASSERT(strcmp(b->title, "Test Book") == 0, "book title matches");

    ASSERT(loan_book(head, "ISBN1", 2) == 0, "loan_book succeeds");
    b = search_by_isbn(head, "ISBN1");
    ASSERT(b->stock == 3 && b->loaned == 2, "stock and loaned updated");

    ASSERT(return_book(head, "ISBN1", 2) == 0, "return_book succeeds");
    b = search_by_isbn(head, "ISBN1");
    ASSERT(b->stock == 5 && b->loaned == 0, "stock and loaned restored");

    ASSERT(update_book(head, "ISBN1", "New Title", "Bob", "Sci", 7) == 0, "update_book succeeds");
    b = search_by_isbn(head, "ISBN1");
    ASSERT(strcmp(b->title, "New Title") == 0 && b->stock == 7, "update applied");

    ASSERT(delete_book(&head, "ISBN1") == 0, "delete_book succeeds");
    b = search_by_isbn(head, "ISBN1");
    ASSERT(b == NULL, "book removed");

    destroy_list(head);
}

void test_user_basic() {
    UserNode *uh = NULL;
    ASSERT(register_user(&uh, ROLE_STUDENT, "alice", "pwd123", "q", "a") == 0, "register_user succeeds");
    UserRole role = ROLE_NONE;
    ASSERT(verify_login(uh, "alice", "pwd123", &role) == 0 && role == ROLE_STUDENT, "verify_login succeeds");
    ASSERT(account_exists(uh, "alice") == 1, "account_exists true");
    ASSERT(change_password(uh, "alice", "newpwd") == 0, "change_password succeeds");
    ASSERT(verify_login(uh, "alice", "newpwd", &role) == 0, "login with new password succeeds");

    destroy_user_list(uh);
}

int main(void) {
    printf("Running basic unit tests...\n");
    test_data_basic();
    test_user_basic();
    if (failures == 0) {
        printf("ALL TESTS PASSED\n");
        return 0;
    } else {
        printf("%d TEST(S) FAILED\n", failures);
        return 1;
    }
}
