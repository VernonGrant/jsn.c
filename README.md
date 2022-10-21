# JSN.C

```text
       _  _____ _   _   _____
      | |/ ____| \ | | / ____|
      | | (___ |  \| || |
  _   | |\___ \| . ` || |
 | |__| |____) | |\  || |____
  \____/|_____/|_| \_(_)_____|
```

**JSN.c** is a simple JSON utility that's intended to be used for
parsing, generating and manipulating configuration files.

## Outstanding tasks (TODO)

- Writing all unit tests.
- Performance optimizations.
- Perform final code refactoring and cleanup.

## Usage

To use **JSN.c** you only need to make use of two files, `jsn.c` and
`jsn.h`. Here's a few usage examples to get you going.

### 1. Reading a value from JSON file

```C
#include <stdio.h>
#include <stdlib.h>
#include "jsn.h"

int main(int argc, char *argv[]) {
    double version_number = 0;

    // Parse the file.
    jsn_handle conf = jsn_from_file("./data/learn.json");

    // Get's the version number node.
    jsn_handle version_node = jsn_get(conf, 1, "version");

    // Get's the version number node's double value.
    version_number = jsn_get_value_double(version_node);

    // Frees the entire tree.
    jsn_free(conf);

    // Print out the version number.
    printf("The version number is %f\n", version_number);

    return EXIT_SUCCESS;
}
```

### 2. Reading and updating a JSON file

```C
#include <stdlib.h>
#include "jsn.h"

int main(int argc, char *argv[]) {
    // Parse the file.
    jsn_handle conf = jsn_from_file("./data/learn.json");

    // Get's the version number node.
    jsn_handle version_node = jsn_get(conf, 1, "version");

    // Set the node's new value.
    jsn_set_as_double(version_node, 1.0);

    // Write changes back to file.
    jsn_to_file(conf, "./data/learn.json");

    // Frees the entire tree.
    jsn_free(conf);

    return EXIT_SUCCESS;
}
```

### 3. Creating a JSON object via code and writing it to a file

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

    return EXIT_SUCCESS;
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

### 4. Looping through an arrays items

```C
#include <stdlib.h>
#include <stdio.h>
#include "jsn.h"

int main(int argc, char *argv[]) {

    // Parse the JSON file.
    jsn_handle root = jsn_from_file("./data/learn.json");

    // Let's get an array from the root object other > array-of-numbers.
    jsn_handle array = jsn_get(root, 2, "other", "array-of-numbers");

    // Let's loop through each array item, we know that each array item is of
    // type int.
    for (unsigned int i = 0; i < jsn_array_count(array); i++) {
        // Get the array item from the given index.
        jsn_handle array_item = jsn_get_array_item(array, i);

        // Print out the number.
        printf("This array item's number is: %u \n", jsn_get_value_int(array_item));
    }

    return EXIT_SUCCESS;
}
```

The above program will output:

```text
This array item's number is: 10
This array item's number is: 20
This array item's number is: 30
This array item's number is: 40
```

## Public Interface

### Parsing, printing and saving

- `void jsn_print(jsn_handle handle)`
    - Prints the JSON of the provided handle (node).

- `jsn_handle jsn_from_file(const char *file_path)`
    - Opens the given JSON file and parses it into a tree structure. It will
      call exit if there's any issues opening or parsing the file. Else it will
      return a handle to the root node.

- `void jsn_to_file(jsn_handle handle, const char *file_path)`
    - Will write the JSON of the given handle (node) to a file specified by the
      given path.

### Tree creation and deletion

- `jsn_handle jsn_create_object()`
    - Creates an empty object node and return's it's handle.

- `jsn_handle jsn_create_array()`
    - Creates an empty array node and return's it's handle.

- `jsn_handle jsn_create_integer(int value)`
    - Creates an integer node and return's it's handle.

- `jsn_handle jsn_create_double(double value)`
    - Creates an double node and return's it's handle.

- `jsn_handle jsn_create_string(const char *value)`
    - Creates an string node and return's it's handle.

- `jsn_handle jsn_create_boolean(bool value)`
    - Creates an boolean node and return's it's handle.

- `jsn_handle jsn_create_null()`
    - Creates an null node and return's it's handle.

- `void jsn_object_set(jsn_handle handle, const char \*key, jsn_handle node)`
    - Will append a node onto the provided object (handle) and associates it
      with the given key. The first argument (handle) must be of an object
      type.

- `void jsn_array_push(jsn_handle handle, jsn_handle node)`
    - Will append a node onto the end of an array. The first argument (handle)
      must be of an array type.

- `unsigned int jsn_array_count(jsn_handle handle)`
    - Returns the total number of children of the given handle.

- `void jsn_free(jsn_handle handle)`
    - Will recursively free the handle (node). Please note, that you should
      only every free the root node.

### Getting and setting

- `jsn_handle jsn_get(jsn_handle handle, unsigned int arg_count, ...)`
    - Returns a handle to an objects child node that matching the provided key
      hierarchy.

- `jsn_handle jsn_get_array_item(jsn_handle handle, unsigned int index)`
    - Returns a handle to an array child node, at the given index.

- `int jsn_get_value_int(jsn_handle handle)`
    - Get a nodes integer value.

- `bool jsn_get_value_bool(jsn_handle handle)`
    - Get a nodes boolean value.

- `double jsn_get_value_double(jsn_handle handle)`
    - Get a nodes double value.

- `const char *jsn_get_value_string(jsn_handle handle)`
    - Get a nodes string value.

- `void jsn_set_as_object(jsn_handle handle)`
    - Set's the given handle (node) as an object. Will change it's type if the
      handle is not of an object type.

- `void jsn_set_as_array(jsn_handle handle)`
    - Set's the given handle (node) as an array. Will change it's type if the
      handle is not of an array type.

- `void jsn_set_as_integer(jsn_handle handle, int value)`
    - Set's the given handle (node) as an integer. Will change it's type if the
      handle is not of an integer type.

- `void jsn_set_as_double(jsn_handle handle, double value)`
    - Set's the given handle (node) as an double. Will change it's type if the
      handle is not of an double type.

- `void jsn_set_as_boolean(jsn_handle handle, bool value)`
    - Set's the given handle (node) as an boolean. Will change it's type if the
      handle is not of an boolean type.

- `void jsn_set_as_string(jsn_handle handle, const char *value)`
    - Set's the given handle (node) as an string. Will change it's type if the
      handle is not of an string type.
