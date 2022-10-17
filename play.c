#include "./jsn.h"
#include <stdlib.h>

int main(int argc, char *argv[])
{
    jsn_handle root = jsn_from_file("./data/data_error.json");
    if (root != NULL) {
        jsn_print(root);
    }

    // jsn_handle numbers_1 = jsn_from_file("./data/numbers_1.json");
    // jsn_free(numbers_1);

    // jsn_handle numbers_2 = jsn_from_file("./data/numbers_2.json");
    // jsn_free(numbers_2);

    return 0;
}
