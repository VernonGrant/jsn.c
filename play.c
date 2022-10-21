#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include "jsn.h"

int main(int argc, char *argv[]) {
   jsn_handle handle = jsn_from_file("./data/data_bad_1.json");

   jsn_print(handle);
    // printf("Char bit size: %lu\n", sizeof(char));
    // printf("Char bit size: %u\n", CHAR_BIT);
    return 0;
}
