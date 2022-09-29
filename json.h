#ifndef JSON_H
#define JSON_H

#include <stdbool.h>

/**
 * Handle definition.
 */
struct jsn_node;
typedef struct jsn_node *jsn_handle;

/* Generating a tree of nodes from. */

jsn_handle jsn_from_file (const char *path);

jsn_handle jsn_from_string (const char *src);

/* Exporting a tree of nodes. */

void jsn_to_file (jsn_handle handle);

char *jsn_to_string (jsn_handle handle);

/* Debugging. */

void jsn_print(jsn_handle handle);

/* Setting the values of nodes. */

#define JSN_TRUE (_Bool)1
#define JSN_FALSE (_Bool)0

// clang-format off
#define jsn_set(handle, value, ...)                                            \
_Generic((value),                                                              \
           default : jsn_set_int,                                              \
           double : jsn_set_double,                                            \
           _Bool : jsn_set_bool,                                               \
           jsn_handle : jsn_set_collection                                     \
           ) (handle, value __VA_OPT__ (, ) __VA_ARGS__)

// clang-format on
#endif
