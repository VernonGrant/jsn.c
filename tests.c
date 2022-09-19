#include <check.h>
#include <stdlib.h>

START_TEST (sample_test_here)
{
  ck_assert_int_eq(5, 5);
}
END_TEST

START_TEST (sample_test_here_here)
{
  ck_assert_int_eq(5, 5);
}
END_TEST

Suite * sample_suite(void)
{
    Suite *s;
    TCase *tc_core;
    s = suite_create("Nodes");

    /* Core test case */
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, sample_test_here);
    tcase_add_test(tc_core, sample_test_here);
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
