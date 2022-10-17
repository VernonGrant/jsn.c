#include <check.h>
#include <stdlib.h>
#include <stdio.h>
#include "./jsn.h"

/* CONSTANTS:
 * --------------------------------------------------------------------------*/

#define JSN_TESTING_DATA_FILE_COUNT 4
#define JSN_TESTING_DATA_FILES_PATHS (char[4][50]){ \
        "./data/data_1.json", \
        "./data/data_2.json", \
        "./data/data_3.json", \
        "./data/data_4.json" \
    }

#define JSN_TESTING_BAD_DATA_FILE_COUNT 1
#define JSN_TESTING_BAD_DATA_FILES_PATHS (char[1][50]){ \
        "./data/data_bad_1.json", \
    }

/* PARSER FROM TESTS:
 * --------------------------------------------------------------------------*/

START_TEST (jsn_from_file_test)
{
    // Insure that all data files can be parsed.
    for (unsigned int i = 0;  i < JSN_TESTING_DATA_FILE_COUNT; i++) {
        jsn_handle root_node = jsn_from_file(JSN_TESTING_DATA_FILES_PATHS[i]);
        ck_assert_ptr_nonnull(root_node);
        jsn_free(root_node);
    }
}
END_TEST

/**
 * Checks that bad file paths returns NULL.
 */
START_TEST (jsn_from_file_unknown_file_test)
{
    jsn_handle root= jsn_from_file("./data/data_100.json");
    ck_assert_ptr_null(root);
}
END_TEST

/**
 * Checks that file's with syntax errors returns NULL.
 */
START_TEST (jsn_from_file_bad_file_test)
{
    for (unsigned int i = 0;  i < JSN_TESTING_BAD_DATA_FILE_COUNT; i++) {
        jsn_handle root = jsn_from_file(JSN_TESTING_BAD_DATA_FILES_PATHS[i]);
        ck_assert_ptr_null(root);
    }
}
END_TEST


/* PUBLIC API TESTS:
 * -------------------------------------------------------------------------*/

START_TEST (jsn_get_test)
{
    // Parse some number heavy JSON files.
    // jsn_handle numbers_1 = jsn_from_file("./data/numbers_1.json");
    // jsn_handle height_node = jsn_get(numbers_1, 1, "height");
    // ck_assert_ptr_nonnull(height_node);

}


/* INTERNAL TESTS:
 * --------------------------------------------------------------------------*/

Suite * sample_suite(void)
{

    Suite *s;
    TCase *tc_core;
    s = suite_create("JSN");

    /* Core test case */
    tc_core = tcase_create("Parsing");
    tcase_add_test(tc_core, jsn_from_file_test);
    tcase_add_test(tc_core, jsn_from_file_unknown_file_test);
    tcase_add_test(tc_core, jsn_from_file_bad_file_test);
    tcase_add_test(tc_core, jsn_get_test);
    suite_add_tcase(s, tc_core);

    return s;
}

int main(void)
{
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
