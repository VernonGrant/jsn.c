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
