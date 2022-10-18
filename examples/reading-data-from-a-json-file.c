#include "../jsn.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    double version_number = 0;

    // Parse the file.
    jsn_handle conf = jsn_from_file("./data/learn.json");

    // Will be null if parsing failed.
    if (conf != NULL) {

        // Get's the version number node.
        jsn_handle version_node = jsn_get(conf, 1, "version");

        // Get's the version number node's double value.
        version_number = jsn_get_value_double(version_node);

        // Frees the entire tree.
        jsn_free(conf);
    }

    // Print out the version number.
    printf("The version number is %f\n", version_number);

    return 0;
}
