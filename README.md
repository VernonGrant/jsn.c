# JSN.c

**JSN.c** is intended to be used to read and manipulate small JSON configuration
files. Instead of focusing on memory usage and performance, the focus is to
create an easy to use interface.

## Usage

The following provides quick guides to help you get started. You can take a
look at the entire public interface here.

### Reading a value from JSON file

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

### Reading and updating a JSON file

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
        jsn_set_as_double(version_node, 1.0);

        // Write changes back to file.
        jsn_to_file(conf, "./data/learn.json");

        // Frees the entire tree.
        jsn_free(conf);
    }

    return 0;
}
```

### Creating a structure and writing it to a file

```C
#include "jsn.h"
#include <stdlib.h>

int main(int argc, char *argv[]) {
    // Create the main object.
    jsn_handle main_object = jsn_create_object();

    // Set some root properties.
    jsn_object_set(main_object, "version", jsn_create_double(1.0));
    jsn_object_set(main_object, "name", jsn_create_string("Project Name"));

    // Create a array node and add some children.
    jsn_handle inner_array = jsn_create_array();
    for (unsigned int i = 0; i < 10; i++) {
        jsn_array_push(inner_array, jsn_create_integer(i));
    }

    // Append the array to the main object.
    jsn_object_set(main_object, "numbers-array", inner_array);

    // Create a new object.
    jsn_handle inner_object = jsn_create_object();

    // Set some properties on the above inner object.
    jsn_object_set(inner_object, "release", jsn_create_boolean(true));
    jsn_object_set(inner_object, "dependencies", jsn_create_boolean(false));
    jsn_object_set(inner_object, "children", jsn_create_null());

    // Append the inner object to the main object.
    jsn_object_set(main_object, "inner-object", inner_object);

    // Write the main object to a file.
    jsn_to_file(main_object, "./data/data_written.json");

    // Print out the JSON.
    jsn_print(main_object);

    // We're done, so let's free the used memory. We only need to free the
    // main object, as all other node's have already been appended to the main
    // object, they will all get freed recursively.
    jsn_free(main_object);

    return 0;
}
```

The results of the above code.

```JSON
{
	"version": 1.000000,
	"name": "Project Name",
	"numbers-array": [0, 1, 2, 3, 4, 5, 6, 7, 8, 9],
	"inner-object": {
		"release": true,
		"dependencies": false,
		"children": null
	}
}
```

