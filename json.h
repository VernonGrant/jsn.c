#ifndef JSON_H
#define JSON_H

/**
 * Handle definition.
 */
struct jsn_node;
typedef struct jsn_node *jsn_handle;

/* GENERATING NODE TREE
 * ------------------------------------------------------------------------- */

jsn_handle jsn_from_string (const char *src);

/* DEBUGGING
 * ------------------------------------------------------------------------- */

void jsn_print (jsn_handle handle);

/* SETTERS
 * ------------------------------------------------------------------------- */

/* Setting the values of nodes. */

/* #define JSN_TRUE (_Bool)1 */
/* #define JSN_FALSE (_Bool)0 */

// We can define this to be used to set a number.
// clang-format off.
/* #define jsn_set_value(handle, value, ...)                                     \ */
/*   _Generic((value), default                                                   \ */
/*            : jsn_set_int, double                                              \ */
/*            : jsn_set_double, _Bool                                            \ */
/*            : jsn_set_bool, jsn_handle                                         \ */
/*            : jsn_set_collection) (handle, value __VA_OPT__ (, ) __VA_ARGS__) */

void jsn_set_value_int(jsn_handle handle, int value, ...);

/* GETTERS
 * ------------------------------------------------------------------------- */

//int jsn_get_value_int(jsn_handle handle, ...);

// How about get?

// clang-format on.
#endif
