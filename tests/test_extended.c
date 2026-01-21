#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../data.h"
#include "../user.h"

static int failures = 0;
#define ASSERT(cond, msg) \
    do { if (!(cond)) { printf("FAIL: %s\n", msg); ++failures; } else { printf("OK: %s\n", msg); } } while(0)

void test_data_edge_cases() {
    BookNode *head = NULL;
    ASSERT(add_book(&head, "DUP", "Book1", "A", "Cat", 1) == 0, "add first book succeeds");
    ASSERT(add_book(&head, "DUP", "BookDup", "B", "Cat", 2) == -1, "add duplicate ISBN fails");

    ASSERT(loan_book(head, "DUP", 2) == -1, "loan beyond stock fails");
    ASSERT(loan_book(head, "DUP", 1) == 0, "loan available succeeds");
    ASSERT(loan_book(head, "DUP", 1) == -1, "loan when out of stock fails");

    ASSERT(return_book(head, "DUP", 2) == -1, "return more than loaned fails");
    ASSERT(return_book(head, "DUP", 1) == 0, "return valid amount succeeds");

    // keyword search
    ASSERT(add_book(&head, "K1", "C Programming", "Author1", "CS", 3) == 0, "add book K1");
    ASSERT(add_book(&head, "K2", "Learn C", "Author2", "CS", 2) == 0, "add book K2");
    BookNode *res = search_by_keyword(head, "C");
    ASSERT(res != NULL, "search_by_keyword finds matches");
    if (res) {
        int found = 0;
        for (BookNode *p = res; p != NULL; p = p->next) {
            if ((p->title && strstr(p->title, "C")) || (p->author && strstr(p->author, "C"))) { found = 1; break; }
        }
        ASSERT(found, "match contains keyword in some node");
    }

    destroy_list(head);
}

void test_user_persistence() {
    UserNode *uh = NULL;
    const char *fname = "tests/users_test.json";
    ASSERT(register_user(&uh, ROLE_ADMIN, "bob", "pw", "q", "a") == 0, "register bob succeeds");
    ASSERT(save_users_to_file(fname, uh) == 0, "save_users_to_file succeeds");

    UserNode *loaded = load_users_from_file(fname);
    ASSERT(loaded != NULL, "load_users_from_file returned list");
    if (loaded) {
        ASSERT(account_exists(loaded, "bob") == 1, "loaded users contain bob");
    }

    destroy_user_list(uh);
    destroy_user_list(loaded);
    // try remove file, ignore failure
    remove(fname);
}

int main(void) {
    printf("Running extended unit tests...\n");
    test_data_edge_cases();
    test_user_persistence();
    if (failures == 0) {
        printf("ALL EXTENDED TESTS PASSED\n");
        return 0;
    } else {
        printf("%d EXTENDED TEST(S) FAILED\n", failures);
        return 1;
    }
}
