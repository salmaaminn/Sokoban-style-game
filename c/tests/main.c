#include <check.h>
#include <stdlib.h>

Suite *playerSuite(void);
Suite *roomSuite(void);
Suite *game_engineSuite(void);
Suite *renderSuite(void);
Suite *loaderSuite(void);
//more suites

int main(void)
{
    Suite *suites[] = {playerSuite(), roomSuite(), game_engineSuite(),renderSuite(),loaderSuite(),NULL};

    SRunner *runner = srunner_create(suites[0]);
    for (int i = 1; suites[i] != NULL; ++i) {
        srunner_add_suite(runner, suites[i]);
    }

    srunner_run_all(runner, CK_NORMAL);
    int failed = srunner_ntests_failed(runner);
    srunner_free(runner);

    return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
