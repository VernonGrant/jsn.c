#include "./jsn.h"
#include <check.h>
#include <stdio.h>
#include <stdlib.h>

/* CONSTANTS:
 * --------------------------------------------------------------------------*/

#define JSN_TESTING_DATA_FILE_COUNT 5
#define JSN_TESTING_DATA_FILES_PATHS                                      \
    (char[5][50]) {                                                       \
        "./data/data_1.json", "./data/data_2.json", "./data/data_3.json", \
        "./data/data_4.json", "./data/data_5.json"}

#define JSN_TESTING_BAD_DATA_FILE_COUNT 1
#define JSN_TESTING_BAD_DATA_FILES_PATHS                                  \
    (char[1][50]) { "./data/data_bad_1.json", }

/* PARSER FROM TESTS:
 * --------------------------------------------------------------------------*/

/**
 * Checks that all sample files can be parsed.
 */
START_TEST(jsn_from_file_test) {
    for (unsigned int i = 0; i < JSN_TESTING_DATA_FILE_COUNT; i++) {
        jsn_handle root_node = jsn_from_file(JSN_TESTING_DATA_FILES_PATHS[i]);
        ck_assert_ptr_nonnull(root_node);
        jsn_free(root_node);
    }
}
END_TEST

/**
 * Checks that all sample files can be parsed and written to a file.
 */
START_TEST(jsn_to_file_test) {
    for (unsigned int i = 0; i < JSN_TESTING_DATA_FILE_COUNT; i++) {
        jsn_handle root_node = jsn_from_file(JSN_TESTING_DATA_FILES_PATHS[i]);
        ck_assert_ptr_nonnull(root_node);
        jsn_to_file(root_node, "./data/data_written.json");
        jsn_free(root_node);
    }
}
END_TEST

/**
 * Checks that bad file paths returns NULL.
 */
START_TEST(jsn_from_file_unknown_file_test) {
    jsn_handle root = jsn_from_file("./data/data_100.json");
    ck_assert_ptr_null(root);
}
END_TEST

/**
 * Checks that file's with syntax errors returns NULL.
 */
START_TEST(jsn_from_file_bad_file_test) {
    for (unsigned int i = 0; i < JSN_TESTING_BAD_DATA_FILE_COUNT; i++) {
        jsn_handle root = jsn_from_file(JSN_TESTING_BAD_DATA_FILES_PATHS[i]);
        ck_assert_ptr_null(root);
    }
}
END_TEST

/* PUBLIC API TESTS:
 * -------------------------------------------------------------------------*/

START_TEST(jsn_get_test) {
    jsn_handle root;

    // Root level nodes.
    root = jsn_from_file(JSN_TESTING_DATA_FILES_PATHS[0]);
    ck_assert_ptr_nonnull(root);

    jsn_handle root_child;
    root_child = jsn_get(root, 1, "height");
    ck_assert_ptr_nonnull(root_child);
    root_child = jsn_get(root, 1, "time");
    ck_assert_ptr_nonnull(root_child);
    root_child = jsn_get(root, 1, "block_index");
    ck_assert_ptr_nonnull(root_child);
    root_child = jsn_get(root, 1, "hash");
    ck_assert_ptr_nonnull(root_child);

    // Nested nodes.
    root = jsn_from_file(JSN_TESTING_DATA_FILES_PATHS[1]);
    ck_assert_ptr_nonnull(root);

    root_child = jsn_get(root, 2, "rates", "USD");
    ck_assert_ptr_nonnull(root);
    root_child = jsn_get(root, 2, "rates", "AFN");
    ck_assert_ptr_nonnull(root);

}

START_TEST(jsn_get_unknown_key_test) {
    jsn_handle root = jsn_from_file(JSN_TESTING_DATA_FILES_PATHS[0]);
    ck_assert_ptr_nonnull(root);

    jsn_handle root_child;

    // Unknown nodes.
    root_child = jsn_get(root, 0);
    ck_assert_ptr_null(root_child);
    root_child = jsn_get(root, 1, "unknown key");
    ck_assert_ptr_null(root_child);
    root_child = jsn_get(root, 2, "unknown key", "unknown key");
    ck_assert_ptr_null(root_child);
}

/* INTERNAL TESTS:
 * --------------------------------------------------------------------------*/

Suite *sample_suite(void) {

    Suite *s;
    TCase *tc_core;
    s = suite_create("JSN");

    /* Core test case */
    tc_core = tcase_create("Parsing");
    tcase_add_test(tc_core, jsn_from_file_test);
    tcase_add_test(tc_core, jsn_from_file_unknown_file_test);
    tcase_add_test(tc_core, jsn_from_file_bad_file_test);
    tcase_add_test(tc_core, jsn_to_file_test);
    tcase_add_test(tc_core, jsn_get_test);
    tcase_add_test(tc_core, jsn_get_unknown_key_test);
    suite_add_tcase(s, tc_core);

    return s;
}

int main(void) {
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = sample_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_VERBOSE);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
