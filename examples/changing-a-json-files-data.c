#include <stdlib.h>
#include "../jsn.h"

int main(int argc, char *argv[]) {
    // Parse the file.
    jsn_handle conf = jsn_from_file("./data/learn.json");

    // Will be null if parsing failed.
    if (conf != NULL) {

        // Get's the version number node.
        jsn_handle version_node = jsn_get(conf, 1, "version");

        // Set the node's new value.
        jsn_set_as_double(version_node, 1.0);

        // Write changes back to file.
        jsn_to_file(conf, "./data/learn.json");

        // Frees the entire tree.
        jsn_free(conf);
    }

    return 0;
}
