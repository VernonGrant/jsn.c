#ifndef JSON_H
#define JSON_H

#include <stdbool.h>

/**
 * Handle definition.
 */
struct jsn_node;
typedef struct jsn_node *jsn_handle;

/* The goal should be to:
   - Parse a JSON file.
   - Create elements.
   - Read elements.
   - Update elements.
   - Delete elements.
*/

/* Generating a tree of nodes from. */

jsn_handle jsn_from_file (const char *path);

jsn_handle jsn_from_string (const char *src);

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

/* jsn_set(handle, 10, "my-key"); */
/* jsn_set(handle, 10.1, "my-key"); */
/* jsn_set(handle, JSN_TRUE, "my-key"); */
/* jsn_set(handle, JSN_FALSE, "my-key"); */
/* jsn_set(handle, handle, "my-key"); */

#endif
