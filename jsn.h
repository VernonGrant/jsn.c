#ifndef JSON_H
#define JSON_H

#include <stdbool.h>

/* HANDLE DEFINITION.
 * ------------------------------------------------------------------------- */

/**
 * A handle is just a pointe to a node's struct. The tree structure is made out
 * of many nodes.
 */
typedef struct jsn_node *jsn_handle;


/* PARSING, SAVING AND OUTPUTTING FUNCTIONS
 * ------------------------------------------------------------------------- */

/**
 * Prints the JSON of the provided handle (node).
 */
void jsn_print(jsn_handle handle);

/**
 * Opens the given JSON file and parses it into a tree structure. It will
 * call exit if there's any issues opening or parsing the file. Else it will
 * return a handle to the root node.
 */
jsn_handle jsn_from_file(const char *file_path);

/**
 * Will write the JSON of the given handle (node) to a file specified by the
 * given path.
 */
void jsn_to_file(jsn_handle handle, const char *file_path);


/* TREE CREATION AND DELETION FUNCTIONS
 * ------------------------------------------------------------------------- */

/**
 * Creates an empty object node and return's it's handle.
 */
jsn_handle jsn_create_object();

/**
 * Creates an empty array node and return's it's handle.
 */
jsn_handle jsn_create_array();

/**
 * Creates an integer node and return's it's handle.
 */
jsn_handle jsn_create_integer(int value);

/**
 * Creates an double node and return's it's handle.
 */
jsn_handle jsn_create_double(double value);

/**
 * Creates an string node and return's it's handle.
 */
jsn_handle jsn_create_string(const char *value);

/**
 * Creates an boolean node and return's it's handle.
 */
jsn_handle jsn_create_boolean(bool value);

/**
 * Creates an null node and return's it's handle.
 */
jsn_handle jsn_create_null();

/**
 * Will append a node onto the provided object (handle) and associates it with
 * the given key. The first argument (handle) must be of an object type.
 */
void jsn_object_set(jsn_handle handle, const char *key, jsn_handle node);

/**
 * Will append a node onto the end of an array. The first argument (handle)
 * must be of an array type.
 */
void jsn_array_push(jsn_handle handle, jsn_handle node);

/**
 * Will recursively free the handle (node). Please note, that you should only
 * every free the root node.
 */
void jsn_free(jsn_handle handle);

/* GETTING AND SETTING FUNCTIONS
 * ------------------------------------------------------------------------- */

/**
 * Returns a handle to an objects child node that matching the provided key
 * hierarchy.
 */
jsn_handle jsn_get(jsn_handle handle, unsigned int arg_count, ...);

/**
 * Returns a handle to an array child node, at the given index.
 */
jsn_handle jsn_get_array_item(jsn_handle handle, unsigned int index);

/**
 * Get a nodes integer value.
 */
int jsn_get_value_int(jsn_handle handle);

/**
 * Get a nodes boolean value.
 */
bool jsn_get_value_bool(jsn_handle handle);

/**
 * Get a nodes double value.
 */
double jsn_get_value_double(jsn_handle handle);

/**
 * Get a nodes string value.
 */
const char *jsn_get_value_string(jsn_handle handle);

/**
 * Set's the given handle (node) as an object. Will mutate it's type if the
 * handle is not of an object type.
 */
void jsn_set_as_object(jsn_handle handle);

/**
 * Set's the given handle (node) as an array. Will mutate it's type if the
 * handle is not of an array type.
 */
void jsn_set_as_array(jsn_handle handle);

/**
 * Set's the given handle (node) as an integer. Will mutate it's type if the
 * handle is not of an integer type.
 */
void jsn_set_as_integer(jsn_handle handle, int value);

/**
 * Set's the given handle (node) as an double. Will mutate it's type if the
 * handle is not of an double type.
 */
void jsn_set_as_double(jsn_handle handle, double value);

/**
 * Set's the given handle (node) as an boolean. Will mutate it's type if the
 * handle is not of an boolean type.
 */
void jsn_set_as_boolean(jsn_handle handle, bool value);

/**
 * Set's the given handle (node) as an string. Will mutate it's type if the
 * handle is not of an string type.
 */
void jsn_set_as_string(jsn_handle handle, const char *value);

#endif
