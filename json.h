#ifndef JSON_H
#define JSON_H

#include <stdbool.h>

/* HANDLE DEFINITION.
 * ------------------------------------------------------------------------- */

struct jsn_node;
typedef struct jsn_node *jsn_handle;

/* DEBUGGING
 * ------------------------------------------------------------------------- */

void jsn_print(jsn_handle handle);

// TODO: Define debugging constants here, so we can hard exit on failures.

/* GENERATING NODE TREE
 * ------------------------------------------------------------------------- */

jsn_handle jsn_from_string(const char *src);
jsn_handle jsn_from_file(const char *file_path);

char *jsn_to_string(jsn_handle handle); // Not Implemented
void jsn_to_file(jsn_handle handle, const char *file_path); // Not Implemented

/* FINAL INTERFACE
 * ------------------------------------------------------------------------- */

// Create a new node.
jsn_handle jsn_create_object();
jsn_handle jsn_create_array();
jsn_handle jsn_create_integer(int value);
jsn_handle jsn_create_double(double value);
jsn_handle jsn_create_boolean(bool value);
jsn_handle jsn_create_string(const char *value);

// Add a node to object.
void jsn_object_set(jsn_handle handle, const char *key, jsn_handle node);

// Add node to array.
void jsn_array_push(jsn_handle handle, jsn_handle node);

// Get a node.
jsn_handle jsn_get(jsn_handle handle, unsigned int arg_count, ...);
jsn_handle jsn_get_array_item(jsn_handle handle, unsigned int index);

// Set a nodes value, change the nodes type if needed.
void jsn_set_as_object(jsn_handle handle);
void jsn_set_as_array(jsn_handle handle);
void jsn_set_as_integer(jsn_handle handle, int value);
void jsn_set_as_double(jsn_handle handle, double value);
void jsn_set_as_boolean(jsn_handle handle, bool value);
void jsn_set_as_string(jsn_handle handle, const char *value);

// Free a node.
void jsn_free(jsn_handle handle);

#endif
