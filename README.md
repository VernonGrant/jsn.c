# JSN.c

**JSN.c** is intended to be used to read and manipulate small JSON configuration
files. Instead of focusing on memory usage and performance, the focus is to
create an easy to use interface.

## Usage

The following provides quick guides to help you get started. You can take a
look at the entire public interface here.

- [Reading data from a JSON file](#reading-data-from-a-json-file)
- [Changing a JSON file's data](#changing-a-json-files-data)


### Reading data from a JSON file

JSN.c consists of two files, the `jsn.c` and `jsn.h`. You can simply just
include these into your projects and reference the header file where needed.

In the blow example we parse a configuration file, get the node matching the
`version` key and then extract the node's value of type double.

```C
#include <stdio.h>
#include <stdlib.h>
#include "jsn.h"

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
```

### Changing a JSON file's data

Parse a file and update a tree's node and update the file once done.

```C
#include <stdlib.h>
#include "jsn.h"

int main(int argc, char *argv[]) {
    // Parse the file.
    jsn_handle conf = jsn_from_file("./data/learn.json");

    // Will be null if parsing failed.
    if (conf != NULL) {

        // Get's the version number node.
        jsn_handle version_node = jsn_get(conf, 1, "version");

        // Set the node's new value.
        jsn_set_as_double(version_node, 2.0);

        // Write changes back to file.
        jsn_to_file(conf, "./data/learn.json");

        // Frees the entire tree.
        jsn_free(conf);
    }

    return 0;
}
```

## Interface Overview


