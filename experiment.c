/**
 * Author: Vernon Grant
 * Repository: https://github.com/VernonGrant/jsn.c
 * License: https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Created: 2022-10-21
 **/

#include "jsn.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    char data_files[4][50] = {"./data/data_1.json", "./data/data_2.json",
                              "./data/data_3.json", "./data/data_4.json"};

    for (unsigned int i = 0; i < 4; i++) {
        jsn_handle root = jsn_from_file(data_files[i]);
        jsn_free(root);
    }

    printf("All good!\n");

    // // Parse the JSON file.
    // jsn_handle root = jsn_from_file("./data/learn.json");
    // // Let's get an array from the root object other > array-of-numbers.
    // jsn_handle array = jsn_get(root, 2, "other", "array-of-numbers");
    // // Let's loop through each array item, we know that each array item is of
    // // type int.
    // for (unsigned int i = 0; i < jsn_array_count(array); i++) {
    //     // Get the array item from the given index.
    //     jsn_handle array_item = jsn_get_array_item(array, i);
    //     // Print out the number.
    //     printf("This array item's number is: %u \n",
    //            jsn_get_value_int(array_item));
    // }

    return EXIT_SUCCESS;
}
