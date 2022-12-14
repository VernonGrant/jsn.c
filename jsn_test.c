/**
 * Author: Vernon Grant
 * Repository: https://github.com/VernonGrant/jsn.c
 * License: https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Created: 2022-10-21
 **/

#include "./jsn.h"
#include <check.h>
#include <stdio.h>
#include <stdlib.h>

/* CONSTANTS:
 * --------------------------------------------------------------------------*/

#define JSN_TESTING_DATA_FILE_COUNT 4
#define JSN_TESTING_DATA_FILES_PATHS                                           \
    (char[JSN_TESTING_DATA_FILE_COUNT][50]) {                                  \
        "./data/data_1.json", "./data/data_2.json", "./data/data_3.json",      \
            "./data/data_4.json"                                               \
    }

#define JSN_TESTING_BAD_DATA_FILE_COUNT 4
#define JSN_TESTING_BAD_DATA_FILES_PATHS                                       \
    (char[JSN_TESTING_BAD_DATA_FILE_COUNT][50]) {                              \
        "./data/data_bad_1.json", "./data/data_bad_2.json",                    \
            "./data/data_bad_3.json", "./data/data_bad_4.json"                 \
    }

/* PARSING, PRINTING AND SAVING
 * --------------------------------------------------------------------------*/

/**
 * Checks that all sample files can be parsed.
 */
START_TEST(jsn_from_file_test) {
    for (unsigned int i = 0; i < JSN_TESTING_DATA_FILE_COUNT; i++) {
        jsn_handle root_node = jsn_from_file(JSN_TESTING_DATA_FILES_PATHS[i]);
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
        jsn_to_file(root_node, "./data/data_written.json");
        jsn_free(root_node);
    }
}
END_TEST

/**
 * Checks that bad file paths will cause exit failure.
 */
START_TEST(jsn_from_file_unknown_file_test) {
    jsn_from_file("./data/data_100.json");
}
END_TEST

/**
 * Checks that file's with breakable syntax errors will cause exit failure.
 */
START_TEST(jsn_from_file_bad_file_test) {
    for (unsigned int i = 0; i < JSN_TESTING_BAD_DATA_FILE_COUNT; i++) {
        jsn_from_file(JSN_TESTING_BAD_DATA_FILES_PATHS[i]);
    }
}
END_TEST

/* GETTING AND SETTING
 * -------------------------------------------------------------------------*/

START_TEST(jsn_get_test) {
    jsn_handle root;

    // Root level nodes.
    root = jsn_from_file(JSN_TESTING_DATA_FILES_PATHS[0]);

    // Will fail if a key was not found.
    jsn_get(root, 1, "height");
    jsn_get(root, 1, "time");
    jsn_get(root, 1, "block_index");
    jsn_get(root, 1, "hash");

    // Nested nodes.
    root = jsn_from_file(JSN_TESTING_DATA_FILES_PATHS[1]);
    jsn_get(root, 2, "rates", "USD");
    jsn_get(root, 2, "rates", "AFN");

    // Free.
    jsn_free(root);
}

START_TEST(jsn_get_unknown_key_test) {
    jsn_handle root = jsn_from_file(JSN_TESTING_DATA_FILES_PATHS[0]);

    // These should all fail.
    jsn_get(root, 0);
    jsn_get(root, 1, "unknown key");
    jsn_get(root, 2, "unknown key", "unknown key");

    // Free.
    jsn_free(root);
}

START_TEST(jsn_object_set_test) {
    jsn_handle root = jsn_create_object();

    // Integer.
    jsn_object_set(root, "integer-key", jsn_create_integer(5));
    ck_assert_int_eq(jsn_get_value_int(jsn_get(root, 1, "integer-key")), 5);

    // Double.
    jsn_object_set(root, "double-key", jsn_create_double(1.10));
    ck_assert_int_eq(jsn_get_value_double(jsn_get(root, 1, "double-key")), 1.10);

    // Boolean.
    jsn_object_set(root, "boolean-key", jsn_create_boolean(true));
    ck_assert_int_eq(jsn_get_value_bool(jsn_get(root, 1, "boolean-key")), true);

    // Null.
    jsn_object_set(root, "null-key", jsn_create_null());
    ck_assert_int_eq(jsn_is_value_null(jsn_get(root, 1, "null-key")), 1);

    // String.
    jsn_object_set(root, "string-key", jsn_create_string("My string."));
    jsn_handle string_node = jsn_get(root, 1, "string-key");
    ck_assert_str_eq(jsn_get_value_string(string_node), "My string.");

    // Create and set a nested object.
    jsn_handle nested_object = jsn_create_object();
    jsn_object_set(nested_object, "n-integer-key", jsn_create_integer(5));
    jsn_object_set(nested_object, "n-double-key", jsn_create_double(1.10));
    jsn_object_set(nested_object, "n-boolean-key", jsn_create_boolean(false));
    jsn_object_set(nested_object, "n-null-key", jsn_create_null());
    jsn_object_set(nested_object, "n-string-key", jsn_create_string("My string."));
    jsn_object_set(root, "object-key", nested_object);

    // Nested integer.
    jsn_handle nested_integer = jsn_get(root, 2, "object-key", "n-integer-key");
    ck_assert_int_eq(jsn_get_value_int(nested_integer), 5);

    // Nested double.
    jsn_handle nested_double = jsn_get(root, 2, "object-key", "n-double-key");
    ck_assert_double_eq(jsn_get_value_double(nested_double), 1.10);

    // Nested boolean.
    jsn_handle nested_boolean = jsn_get(root, 2, "object-key", "n-boolean-key");
    ck_assert_int_eq(jsn_get_value_bool(nested_boolean), 0);

    // Nested null.
    jsn_handle nested_null = jsn_get(root, 2, "object-key", "n-null-key");
    ck_assert_int_eq(jsn_is_value_null(nested_null), 1);

    // Nested string.
    jsn_handle nested_string = jsn_get(root, 2, "object-key", "n-string-key");
    ck_assert_str_eq(jsn_get_value_string(nested_string), "My string.");

    // free root.
    jsn_free(root);
}

START_TEST(jsn_array_push_and_get_item_test) {
    jsn_handle root = jsn_create_object();

    // Create an array and append things to it.
    jsn_handle array = jsn_object_set(root, "array-prop", jsn_create_array());
    jsn_array_push(array, jsn_create_integer(1));
    jsn_array_push(array, jsn_create_integer(2));
    jsn_array_push(array, jsn_create_integer(3));
    jsn_array_push(array, jsn_create_integer(4));
    jsn_array_push(array, jsn_create_integer(5));

    jsn_handle array_retrieved = jsn_get(root, 1, "array-prop");
    ck_assert_int_eq(jsn_get_value_int(jsn_get_array_item(array_retrieved, 0)), 1);
    ck_assert_int_eq(jsn_get_value_int(jsn_get_array_item(array_retrieved, 1)), 2);
    ck_assert_int_eq(jsn_get_value_int(jsn_get_array_item(array_retrieved, 2)), 3);
    ck_assert_int_eq(jsn_get_value_int(jsn_get_array_item(array_retrieved, 3)), 4);
    ck_assert_int_eq(jsn_get_value_int(jsn_get_array_item(array_retrieved, 4)), 5);
}

/* INTERNAL TESTS:
 * --------------------------------------------------------------------------*/

Suite *sample_suite(void) {
    Suite *s;
    TCase *tc_core;
    s = suite_create("JSN");

    // Core test case
    tc_core = tcase_create("Parsing");
    tcase_add_test(tc_core, jsn_from_file_test);
    tcase_add_test(tc_core, jsn_to_file_test);

    // Getters and setters
    tcase_add_test(tc_core, jsn_get_test);
    tcase_add_test(tc_core, jsn_object_set_test);
    tcase_add_test(tc_core, jsn_array_push_and_get_item_test);

    // Exist tests
    tcase_add_exit_test(tc_core, jsn_get_unknown_key_test, 1);
    tcase_add_exit_test(tc_core, jsn_from_file_unknown_file_test, 1);
    tcase_add_exit_test(tc_core, jsn_from_file_bad_file_test, 1);

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
