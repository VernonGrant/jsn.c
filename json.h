#ifndef JSON_H
#define JSON_H

#include <stdbool.h>

/**
 * Handle definition.
 */
struct jsn_node;
typedef struct jsn_node *jsn_handle;

/* DEBUGGING
 * ------------------------------------------------------------------------- */

void jsn_node_print(jsn_handle handle);

/* GENERATING NODE TREE
 * ------------------------------------------------------------------------- */

jsn_handle jsn_from_string(const char *src);
/* jsn_handle jsn_from_file(const char *path); */


/* GETTING AND SETTERS VALUES
 * ------------------------------------------------------------------------- */

// Integer nodes.
void jsn_set_value_int(jsn_handle handle, int value, ...);
int jsn_get_value_int(jsn_handle handle, ...);

// Double nodes.
void jsn_set_value_double(jsn_handle handle, double value, ...);

/* Final API
 * ------------------------------------------------------------------------- */

// Create a new node.
jsn_handle jsn_node_create_as_object();
jsn_handle jsn_node_create_as_array();
jsn_handle jsn_node_create_as_int(int value);
jsn_handle jsn_node_create_as_double(double value);
jsn_handle jsn_node_create_as_boolean(bool value);
jsn_handle jsn_node_create_as_string(const char* value);

// Get a node.
jsn_handle jsn_node_get(unsigned int arg_count, ...);               // Not Implemented!
jsn_handle jsn_node_get_array_node(unsigned int index, unsigned int arg_count, ...); // Not Implemented!

// Delete a node.
jsn_handle jsn_node_delete(jsn_handle handle);                      // Not Implemented!

// Set a nodes value.
void jsn_node_set_as_int(jsn_handle handle, int value);             // Not Implemented!
void jsn_node_set_as_double(jsn_handle handle, double value);       // Not Implemented!
void jsn_node_set_as_boolean(jsn_handle handle, bool value);        // Not Implemented!
void jsn_node_set_as_string(jsn_handle handle, const char *value);  // Not Implemented!

// Add a node to object.
void jsn_node_object_append(jsn_handle handle, const char *key, jsn_handle node);

// Add node to array.
void jsn_node_array_append(jsn_handle handle, jsn_handle node);

#endif
