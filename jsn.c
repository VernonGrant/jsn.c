#include "jsn.h"

#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TODO: Remove this in production.
// Development use only.
#include "utils/debugging.h"

/* SETTINGS
 * --------------------------------------------------------------------------*/

/**
 * For performance reasons we allocate a single pool of memory where all token
 * lexeme's are stored. The lexeme pool excess defines an additional buffer,
 * that insure that there's more then enough memory available for all null
 * terminators.
 *
 * The default value of 100, should be enough for 99% of the cases.
 */

// TODO: Add some error handling CONSTANTS.

/* UTILITIES
 * --------------------------------------------------------------------------*/

void jsn_report_failure(const char *message) {
    printf("FAILURE: %s\n", message);
}

/* TOKENIZER
 * --------------------------------------------------------------------------*/

enum jsn_token_kind {
    JSN_TOC_INTEGER,
    JSN_TOC_DOUBLE,
    JSN_TOC_STRING,
    JSN_TOC_BOOLEAN,
    JSN_TOC_ARRAY_OPEN,
    JSN_TOC_ARRAY_CLOSE,
    JSN_TOC_OBJECT_OPEN,
    JSN_TOC_OBJECT_CLOSE,
    JSN_TOC_COMMA,
    JSN_TOC_COLON,
    JSN_TOC_NULL,
    JSN_TOC_UNKNOWN
};

struct jsn_token {
    enum jsn_token_kind type;
    char *lexeme_start;
    char *lexeme_end; // The ending address, becomes our null terminator.
    unsigned int lexeme_length;
};

struct jsn_tokenizer {
    // Points to source starting point.
    char *source;
    unsigned int source_cursor;
};

/**
 * Source string needs to have a null terminator.
 *
 * Initialized the tokenizer, the passed in char pointer should point to
 * malloced memory.
 */
struct jsn_tokenizer
jsn_tokenizer_init(char *source, unsigned int source_length, bool make_copy) {
    // Construct tokenizer.
    struct jsn_tokenizer tokenizer;

    if (make_copy) {
        // Allocate the memory for source.
        tokenizer.source = strcpy(malloc(source_length * CHAR_BIT), source);
    } else {
        // Set the shared memory to the provided pointer.
        tokenizer.source = source;
    }

    // Set source string starting pointer and copy it into the tokenizer.
    tokenizer.source_cursor = 0;

    return tokenizer;
};


static inline void jsn_token_set_lexeme_start(struct jsn_token *token,
                                              struct jsn_tokenizer *tokenizer) {
    token->lexeme_start = &tokenizer->source[tokenizer->source_cursor];
}

static inline void jsn_token_set_lexeme_end(struct jsn_token *token,
                                            struct jsn_tokenizer *tokenizer) {
    token->lexeme_end = &tokenizer->source[tokenizer->source_cursor];
    token->lexeme_length = token->lexeme_end - token->lexeme_start;
}

struct jsn_token jsn_tokenizer_get_next_token(struct jsn_tokenizer *tokenizer) {
    // Is the cursor on a number.
    struct jsn_token token;
    token.type = JSN_TOC_UNKNOWN;
    token.lexeme_start = NULL;
    token.lexeme_end = NULL;

    // Keep the current token here.
    char current_source_char = tokenizer->source[tokenizer->source_cursor];

    // Just skip spaces.
    if (isspace(current_source_char) != 0) {
        tokenizer->source_cursor++;
        return jsn_tokenizer_get_next_token(tokenizer);
    }

    // Handle strings.
    if (current_source_char == '"') {
        token.type = JSN_TOC_STRING;
        tokenizer->source_cursor++;

        // Set lexeme starting location.
        jsn_token_set_lexeme_start(&token, tokenizer);

        // TODO: Make performance improvements here.
        char current_char, next_char;

        // This will keep adding bytes until the end of string is reached.
        while (tokenizer->source[tokenizer->source_cursor] != '"') {
            current_char = tokenizer->source[tokenizer->source_cursor];
            next_char = tokenizer->source[tokenizer->source_cursor + 1];

            // Maybe handle this during output?, yes.
            // Handled escaping of backslashes.
            if (current_char == '\\') {

                // Skip escape char.
                if (next_char == '\\') {
                    tokenizer->source_cursor++;
                    tokenizer->source_cursor++;
                    continue;
                }

                // Skip escape char.
                if (next_char == '"') {
                    tokenizer->source_cursor++;
                    tokenizer->source_cursor++;
                    continue;
                }
            }

            // Perform operation.
            tokenizer->source_cursor++;
        }

        // Set lexeme ending null terminator.
        jsn_token_set_lexeme_end(&token, tokenizer);

        // Move the past the ending quote.
        tokenizer->source_cursor++;

        return token;
    }

    // Get the number token.
    if (isdigit(current_source_char) || current_source_char == '-') {
        token.type = JSN_TOC_INTEGER;

        // Set lexeme starting location.
        jsn_token_set_lexeme_start(&token, tokenizer);

        // Increment past the sign
        if (current_source_char == '-') {
            tokenizer->source_cursor++;
        }

        // TODO: Make performance improvements here.
        char current_char = tokenizer->source[tokenizer->source_cursor];

        while (isdigit(current_char) || current_char == '.') {
            // Set this if it's a double.
            if (current_char == '.') {
                token.type = JSN_TOC_DOUBLE;
            }

            tokenizer->source_cursor++;
            current_char = tokenizer->source[tokenizer->source_cursor];
        }

        // Set lexeme ending null terminator.
        jsn_token_set_lexeme_end(&token, tokenizer);
        return token;
    }

    // TODO: We are repeating node here, it's not needed.
    jsn_token_set_lexeme_start(&token, tokenizer);

    // TODO: Performance improvements possible here.
    // Check all other general token types.
    switch (current_source_char) {
    case 't':
        token.type = JSN_TOC_BOOLEAN;
        tokenizer->source_cursor += 4; // true, 1234
        break;
    case 'f':
        token.type = JSN_TOC_BOOLEAN;
        tokenizer->source_cursor += 5; // false, 12345
        break;
    case 'n':
        token.type = JSN_TOC_NULL;
        tokenizer->source_cursor += 4; // null, 1234
        return token;
    case '[':
        token.type = JSN_TOC_ARRAY_OPEN;
        tokenizer->source_cursor++;
        break;
    case ']':
        token.type = JSN_TOC_ARRAY_CLOSE;
        tokenizer->source_cursor++;
        break;
    case ',':
        token.type = JSN_TOC_COMMA;
        tokenizer->source_cursor++;
        break;
    case '{':
        token.type = JSN_TOC_OBJECT_OPEN;
        tokenizer->source_cursor++;
        break;
    case '}':
        token.type = JSN_TOC_OBJECT_CLOSE;
        tokenizer->source_cursor++;
        break;
    case ':':
        token.type = JSN_TOC_COLON;
        tokenizer->source_cursor++;
        break;
    default:
        jsn_report_failure("Unknown token found!");
        return token;
    }

    jsn_token_set_lexeme_end(&token, tokenizer);

    return token;
}

/* TREE DATA STRUCTURE:
 * --------------------------------------------------------------------------*/

enum jsn_node_type {
    JSN_NODE_NULL,
    JSN_NODE_INTEGER,
    JSN_NODE_DOUBLE,
    JSN_NODE_BOOLEAN,
    JSN_NODE_STRING,
    JSN_NODE_ARRAY,
    JSN_NODE_OBJECT,
};

union jsn_node_value {
    int value_integer;
    double value_double;
    bool value_boolean;
    char *value_string;
};

struct jsn_node {
    char *key;
    enum jsn_node_type type;
    union jsn_node_value value;
    unsigned int children_count;
    struct jsn_node **children;
};

struct jsn_node *jsn_create_node(enum jsn_node_type type) {
    // Let's allocate some memory on the heap.
    struct jsn_node *node = malloc(sizeof(struct jsn_node));

    // Set some sane defaults.
    node->key = NULL;
    node->type = type;
    node->children_count = 0;
    node->children = NULL;

    return node;
}

void jsn_append_node_child(struct jsn_node *parent, struct jsn_node *child) {
    // TODO: Why does this almost not even, effect performance?
    // Increment children count.
    parent->children_count++;

    // TODO: Can you call realloc on a null pointer?
    // Reallocate memory.
    unsigned int size = (sizeof(struct jsn_node *)) * (parent->children_count);
    parent->children = realloc(parent->children, size);

    // Set the new node.
    parent->children[parent->children_count - 1] = child;
}

/**
 * Will free all the nodes children and set the provided nodes children count
 * to 0.
 */
void jsn_free_node_children(struct jsn_node *node) {
    // They might have strings.
    // They might have keys.
    // Both of these need to be freed.
    // We only want to free it's children, not the node itself.

    // When the node has no children.
    if (node->children_count == 0) {
        return;
    }

    // For each child node, starting at the very end.
    unsigned int i;
    for (i = node->children_count - 1; i > -1; i--) {
        // If this node has no children, free it's taken resources.
        if (node->children[i]->children_count == 0) {
            // If it's a string, free it.
            if (node->children[i]->type == JSN_NODE_STRING) {
                // TODO: This will cause issues, only needed if not parsed.
                free(node->children[i]->value.value_string);
            }
            // If it has a key, free it.
            if (node->children[i]->key != NULL) {
                // TODO: This will cause issues, only needed if not parsed.
                free(node->children[i]->key);
            }
        } else {
            // This node also has it's own children, so recursively continue.
            jsn_free_node_children(node->children[i]);
            // Now free all the memory taken by this nodes children.
            free(node->children);
            // Reset nodes defaults.
            node->children = NULL;
            node->children_count = 0;
        }
    }

    // Now we can free this parents data.
    free(node->children);

    // Reset node defaults.
    node->children = NULL;
    node->children_count = 0;
}

void jsn_free_node_members(struct jsn_node *node, bool keep_key) {
    // If it's a string, free it.
    if (node->type == JSN_NODE_STRING) {
        free(node->value.value_string);
        node->value.value_string = NULL;
    }

    // If it has a key we also need to free that.
    if (node->key != NULL && keep_key == false) {
        free(node->key);
        node->key = NULL;
    }

    // We also need to free it's children.
    if (node->children_count != 0) {
        jsn_free_node_children(node);
    }
}

void jsn_free_node(struct jsn_node *node) {
    // Free all it's members.
    jsn_free_node_members(node, false);

    // TODO: Not sure about setting it to null.
    // And free the node itself.
    free(node);
    node = NULL;
}

struct jsn_node *jsn_get_node_direct_child(jsn_handle handle, const char *key) {
    // If a null node is given, just return.
    if (handle == NULL) {
        return NULL;
    }

    // Can optimize this to only check nodes with keys.
    for (unsigned int i = 0; i < handle->children_count; i++) {
        if (handle->children[i]->key != NULL) {
            if (strcmp(handle->children[i]->key, key) == 0) {
                return handle->children[i];
            }
        }
    }

    return NULL;
}

/**
 * Returns the index of the child node that has the given key. Returns -1 if
 * the there is no direct child with the given key.
 */
int jsn_get_node_direct_child_index(jsn_handle handle, const char *key) {
    for (unsigned int i = 0; i < handle->children_count; i++) {
        if (handle->children[i]->key != NULL) {
            if (strcmp(handle->children[i]->key, key) == 0) {
                return i;
            }
        }
    }

    return -1;
}

/* PARSER:
 * --------------------------------------------------------------------------*/

struct jsn_node *jsn_parse_value(struct jsn_tokenizer *tokenizer,
                                 struct jsn_token token);

struct jsn_node *jsn_parse_string(struct jsn_tokenizer *tokenizer,
                                  struct jsn_token token) {
    struct jsn_node *node = jsn_create_node(JSN_NODE_STRING);

    // Malloc and copy over the lexeme.
    node->value.value_string = malloc((token.lexeme_length + 1) * CHAR_BIT);
    strncpy(node->value.value_string, token.lexeme_start, token.lexeme_length);
    node->value.value_string[token.lexeme_length] = '\0';

    return node;
}

struct jsn_node *jsn_parse_null(struct jsn_tokenizer *tokenizer,
                                struct jsn_token token) {
    struct jsn_node *node = jsn_create_node(JSN_NODE_NULL);
    return node;
}

struct jsn_node *jsn_parse_integer(struct jsn_tokenizer *tokenizer,
                                  struct jsn_token token) {
    struct jsn_node *node;

    // Check if the lexeme is a double.
    char lexeme[token.lexeme_length + 1];
    strncpy(lexeme, token.lexeme_start, token.lexeme_length);
    lexeme[token.lexeme_length] = '\0';

    // Integer
    node = jsn_create_node(JSN_NODE_INTEGER);
    node->value.value_integer = atoi(lexeme);

    return node;
}

struct jsn_node *jsn_parse_double(struct jsn_tokenizer *tokenizer,
                                  struct jsn_token token) {
    struct jsn_node *node;

    // Check if the lexeme is a double.
    char lexeme[token.lexeme_length + 1];
    strncpy(lexeme, token.lexeme_start, token.lexeme_length);
    lexeme[token.lexeme_length] = '\0';

    // Double
    node = jsn_create_node(JSN_NODE_DOUBLE);
    node->value.value_double = strtod(lexeme, NULL);

    return node;
}


struct jsn_node *jsn_parse_boolean(struct jsn_tokenizer *tokenizer,
                                   struct jsn_token token) {
    struct jsn_node *node;
    node = jsn_create_node(JSN_NODE_BOOLEAN);

    if (token.lexeme_start[0] == 't') {
        node->value.value_boolean = true;
    } else {
        node->value.value_boolean = false;
    }
    return node;
}

struct jsn_node *jsn_parse_array(struct jsn_tokenizer *tokenizer,
                                 struct jsn_token token) {

    // Create our array node type.
    struct jsn_node *node = jsn_create_node(JSN_NODE_ARRAY);
    struct jsn_node *child_node;

    // While we are not at the end of the array, handle array tokens.
    while (token.type != JSN_TOC_ARRAY_CLOSE) {
        token = jsn_tokenizer_get_next_token(tokenizer);

        // Create the new child node (recursive call).
        child_node = jsn_parse_value(tokenizer, token);

        if (child_node != NULL) {
            jsn_append_node_child(node, child_node);
        }
    }

    return node;
}

struct jsn_node *jsn_parse_object(struct jsn_tokenizer *tokenizer,
                                  struct jsn_token token) {
    // Create our object node type.
    struct jsn_node *node = jsn_create_node(JSN_NODE_OBJECT);

    struct jsn_token token_key, token_colon, token_val;
    struct jsn_node *child_node;

    // While we haven't reached the end of the object.
    while (token.type != JSN_TOC_OBJECT_CLOSE) {

        // Get the key.
        token_key = jsn_tokenizer_get_next_token(tokenizer);

        // It's an empty token, just break and move on.
        if (token_key.type == JSN_TOC_OBJECT_CLOSE) {
            break;
        }

        if (token_key.type != JSN_TOC_STRING) {
            jsn_report_failure("Unknown token found!");
            return NULL;
        }

        // Get the colon.
        token_colon = jsn_tokenizer_get_next_token(tokenizer);
        if (token_colon.type != JSN_TOC_COLON) {
            jsn_report_failure("Unknown token found!");
            return NULL;
        }

        // Get the value and create the new child node (recursive call).
        token_val = jsn_tokenizer_get_next_token(tokenizer);
        child_node = jsn_parse_value(tokenizer, token_val);

        // Dynamically allocated key.
        child_node->key = malloc((token_key.lexeme_length + 1) * CHAR_BIT);
        strncpy(child_node->key, token_key.lexeme_start, token_key.lexeme_length);
        child_node->key[token_key.lexeme_length] = '\0';

        // Append the child node.
        jsn_append_node_child(node, child_node);

        /* Set the last token, if this token is a comma, then the next
           token should be a string key, if not it will report errors above
           on the next iteration. */
        token = jsn_tokenizer_get_next_token(tokenizer);
    }

    return node;
}

struct jsn_node *jsn_parse_value(struct jsn_tokenizer *tokenizer,
                                 struct jsn_token token) {

    // TODO: Need to find a way to return NULL, no matter how deep the parsing go's.
    switch (token.type) {
    case JSN_TOC_OBJECT_OPEN:
        return jsn_parse_object(tokenizer, token);
    case JSN_TOC_ARRAY_OPEN:
        return jsn_parse_array(tokenizer, token);
    case JSN_TOC_STRING:
        return jsn_parse_string(tokenizer, token);
    case JSN_TOC_INTEGER:
        return jsn_parse_integer(tokenizer, token);
    case JSN_TOC_DOUBLE:
        return jsn_parse_double(tokenizer, token);
    case JSN_TOC_BOOLEAN:
        return jsn_parse_boolean(tokenizer, token);
    case JSN_TOC_NULL:
        return jsn_parse_null(tokenizer, token);
    case JSN_TOC_ARRAY_CLOSE:
    case JSN_TOC_OBJECT_CLOSE:
    case JSN_TOC_COMMA:
        return NULL;
    default:
        jsn_report_failure("Unknown token found!");
        return NULL;
    }
}

/* Debug:
 * --------------------------------------------------------------------------*/

void jsn_node_print_tree(struct jsn_node *node, unsigned int indent) {
    // Generate padding string, if needed.
    char indent_str[indent + 1];
    for (unsigned int i = 0; i < indent; i++) {
        indent_str[i] = '.';
    }
    indent_str[indent] = '\0';

    // TODO: Make this actually print out JSON.
    // TODO: Optimize this by handling less print functions.

    // Print out the current node.
    printf("%s[\n", indent_str);
    printf("%sNode: \n", indent_str);
    printf("%sKey: %s\n", indent_str, node->key);
    switch (node->type) {
    case JSN_NODE_NULL:
        printf("%sType: %s\n", indent_str, "NULL");
        printf("%sValue: %s\n", indent_str, "null");
        break;
    case JSN_NODE_INTEGER:
        printf("%sType: %s\n", indent_str, "INTEGER");
        printf("%sValue: %i\n", indent_str, node->value.value_integer);
        break;
    case JSN_NODE_DOUBLE:
        printf("%sType: %s\n", indent_str, "DOUBLE");
        printf("%sValue: %f\n", indent_str, node->value.value_double);
        break;
    case JSN_NODE_BOOLEAN:
        printf("%sType: %s\n", indent_str, "BOOLEAN");
        printf("%sValue: %i\n", indent_str, node->value.value_boolean);
        break;
    case JSN_NODE_STRING:
        printf("%sType: %s\n", indent_str, "STRING");
        printf("%sValue: %s\n", indent_str, node->value.value_string);
        break;
    case JSN_NODE_ARRAY:
        printf("%sType: %s\n", indent_str, "ARRAY");
        break;
    case JSN_NODE_OBJECT:
        printf("%sType: %s\n", indent_str, "OBJECT");
        break;
    }
    printf("%sChildren_Count: %u\n", indent_str, node->children_count);

    // Print children too.
    for (unsigned int i = 0; i < node->children_count; i++) {
        // Print out node children (recursive call).
        jsn_node_print_tree(node->children[i], indent + 4);
    }
    printf("%s]\n", indent_str);
}

/* API:
 * --------------------------------------------------------------------------*/

void jsn_print(jsn_handle handle) {
    // TODO: Implement a proper printer, JSON type.
    jsn_node_print_tree(handle, 0);
}

// TODO: What is the meaning of const?
jsn_handle jsn_from_string(const char *src) {
    // The string will get copied, so the below cast is fine.
    // Initialize our tokenizer, for this specific source string.
    struct jsn_tokenizer tokenizer =
        jsn_tokenizer_init((char *)src, strlen(src), true);

    // Prime the tokenizer.
    struct jsn_token token = jsn_tokenizer_get_next_token(&tokenizer);

    // Start parsing, recursively.
    jsn_handle root_node = jsn_parse_value(&tokenizer, token);

    return root_node;
}

jsn_handle jsn_from_file(const char *file_path) {
    // TODO: Note there's a file size limit here for fseek and fread.
    // TODO: Will require a more sophisticated solution.
    FILE *file_ptr;

    // Open the file.
    file_ptr = fopen(file_path, "r");

    // TODO: We should handle the error
    // In case the file can't be read, report and return null.
    if (file_ptr == NULL) {
        jsn_report_failure("The file could not be opened, incorrect path?");
        return NULL;
    }

    // Let's get the size of the file in bytes.
    fseek(file_ptr, 0, SEEK_END);
    int file_size = ftell(file_ptr) + 1;

    // Rewind and allocate a local string.
    fseek(file_ptr, 0, SEEK_SET);

    // TODO: Handle allocation errors here.
    // Allocate and copy file source into a temp buffer.
    char *file_buffer = malloc(file_size * CHAR_BIT);
    fread(file_buffer, file_size, 1, file_ptr);
    file_buffer[file_size - 1] = '\0';

    // Close the file steam.
    fclose(file_ptr);

    // Create tokenizer from buffer.
    struct jsn_tokenizer tokenizer =
        jsn_tokenizer_init(file_buffer, file_size, false);
    // jsn_print_memory_usage("Memory used after tokenization init.");

    // Get the first token.
    struct jsn_token token = jsn_tokenizer_get_next_token(&tokenizer);

    // Start parsing, recursively.
    jsn_handle root_node = jsn_parse_value(&tokenizer, token);
    // jsn_print_memory_usage("Memory used after parsing.");

    // If the parser returned NULL, return NULL.
    if (root_node == NULL) {
        jsn_report_failure("File could not be parsed!");
        return NULL;
    }

    // Free the tokenizer source
    free(file_buffer);
    tokenizer.source = NULL;

    // The node will be null anyway, so just return it.
    return root_node;
}

jsn_handle jsn_create_object() {
    struct jsn_node *node = jsn_create_node(JSN_NODE_OBJECT);
    return node;
}

jsn_handle jsn_create_array() {
    struct jsn_node *node = jsn_create_node(JSN_NODE_ARRAY);
    return node;
}

jsn_handle jsn_create_integer(int value) {
    struct jsn_node *node = jsn_create_node(JSN_NODE_INTEGER);
    node->value.value_integer = value;
    return node;
}

jsn_handle jsn_create_double(double value) {
    struct jsn_node *node = jsn_create_node(JSN_NODE_DOUBLE);
    node->value.value_double = value;
    return node;
}

jsn_handle jsn_create_boolean(bool value) {
    struct jsn_node *node = jsn_create_node(JSN_NODE_BOOLEAN);
    node->value.value_boolean = value;
    return node;
}

jsn_handle jsn_create_string(const char *value) {
    struct jsn_node *node = jsn_create_node(JSN_NODE_STRING);

    // TODO: Handle allocation errors here.
    node->value.value_string = strcpy(malloc(strlen(value) * CHAR_BIT), value);
    return node;
}

// TODO: Might return a null pointer, best way to handle that?
jsn_handle jsn_get(jsn_handle handle, unsigned int arg_count, ...) {
    // Create our pointer for the selected node.
    struct jsn_node *selected = NULL;

    // Find the node using the given keys.
    va_list args;
    va_start(args, arg_count);
    for (unsigned int i = 0; i < arg_count; i++) {
        // Will ignore nodes that equal NULL.
        if (i == 0) {
            selected = jsn_get_node_direct_child(handle, va_arg(args, char *));
        } else {
            selected =
                jsn_get_node_direct_child(selected, va_arg(args, char *));
        }
    }
    va_end(args);

    return selected;
}

jsn_handle jsn_get_array_item(jsn_handle handle, unsigned int index) {
    // Make sure were dealing with an array item.
    if (handle->type != JSN_NODE_ARRAY) {
        // TODO: Define better error messages.
        jsn_report_failure("The handle passed to jsn_get_array_item isn't an array.");
        return NULL;
    }

    // Make sure the provided index is not larger then the array itself.
    if (handle->children_count > (index + 1)) {
        // TODO: Define better error messages.
        jsn_report_failure("The provided index is outside the arrays scope.");
        return NULL;
    }

    // Check to make sure the array does in fact have children.
    if (handle->children_count == 0) {
        // TODO: Define better error messages.
        jsn_report_failure("The array does not have any children.");
        return NULL;
    }

    return handle->children[index];
}

/**
 * Will append a node to an node of object type. If the object already has a
 * node with the same key, it will get replaced. Keep in mind the replaced
 * node will be freed and set to NULL.
 */
void jsn_object_set(jsn_handle handle, const char *key, jsn_handle node) {
    // Make sure were dealing with an object handle type here.
    if (handle->type != JSN_NODE_OBJECT) {
        jsn_report_failure("The handle passed to jsn_object_set isn't an object.");
    }

    // Already has a key so we need to free it.
    if (node->key != NULL) {
        free(node->key);
        node->key = NULL;
    }

    // TODO: Handle allocation errors here.
    // Allocate for the nodes new key.
    node->key = strcpy(malloc(strlen(key) * CHAR_BIT), key);

    // Get the index of the child node with the same key if it exists.
    int matching_child_index = jsn_get_node_direct_child_index(handle, key);

    // If a node with the same key exists, free and replace it.
    if (matching_child_index != -1) {
        // Keep a reference to the node that will be replaced.
        struct jsn_node *current_ref = handle->children[matching_child_index];

        // Replace the child node with the new one.
        handle->children[matching_child_index] = node;

        // Free the old node.
        jsn_free_node(current_ref);
    } else {
        // We should append a new node.
        jsn_append_node_child(handle, node);
    }
}

void jsn_array_push(jsn_handle handle, jsn_handle node) {
    // Make sure were dealing with an array handle type here.
    if (handle->type != JSN_NODE_ARRAY) {
        jsn_report_failure("The handle passed to object append isn't an array.");
    }

    // Array children nodes, must not have keys. (Not Objects).
    if (node->key != NULL) {
        free(node->key);
        node->key = NULL;
    }

    // Append the node to the provided object.
    jsn_append_node_child(handle, node);
}

void jsn_set_as_object(jsn_handle handle) {
    // Free the node's members.
    jsn_free_node_members(handle, true);

    // Set the node's new type
    handle->type = JSN_NODE_OBJECT;
}

void jsn_set_as_array(jsn_handle handle) {
    // Free the node's members.
    jsn_free_node_members(handle, true);

    // Set the node's new type
    handle->type = JSN_NODE_ARRAY;
}

void jsn_set_as_integer(jsn_handle handle, int value) {
    // Free the node's members.
    jsn_free_node_members(handle, true);

    // Set the node's new type and value.
    handle->type = JSN_NODE_INTEGER;
    handle->value.value_integer = value;
}

void jsn_set_as_double(jsn_handle handle, double value) {
    // Free the node's members.
    jsn_free_node_members(handle, true);

    // Set the node's new type and value.
    handle->type = JSN_NODE_DOUBLE;
    handle->value.value_double = value;
}

void jsn_set_as_boolean(jsn_handle handle, bool value) {
    // Free the node's members.
    jsn_free_node_members(handle, true);

    // Set the node's new type and value.
    handle->type = JSN_NODE_BOOLEAN;
    handle->value.value_boolean = value;
}

void jsn_set_as_string(jsn_handle handle, const char *value) {
    // Free the node's members.
    jsn_free_node_members(handle, true);

    // Set the node's new type and value.
    handle->type = JSN_NODE_STRING;
    unsigned int str_len = strlen(value) + 1;

    // TODO: Handle allocation errors here.
    handle->value.value_string = strcpy(malloc(str_len * CHAR_BIT), value);
}

void jsn_free(jsn_handle handle) {
    jsn_free_node(handle);
}

/* TESTING:
 * --------------------------------------------------------------------------*/

// int main(void) {
    // Building a JSON tree from code.
    // jsn_handle root = jsn_create_object();
    // jsn_object_set(root, "Jackie", jsn_create_integer(39));
    // jsn_object_set(root, "Vernon", jsn_create_integer(32));
    // jsn_object_set(root, "Lucy", jsn_create_integer(80));
    // jsn_print(root);

    // jsn_object_set(root, "JackieD", jsn_create_double(39));
    // jsn_object_set(root, "VernonD", jsn_create_double(32));
    // jsn_object_set(root, "LucyD", jsn_create_double(80));

    // jsn_object_set(root, "JackieB", jsn_create_boolean(true));
    // jsn_object_set(root, "VernonB", jsn_create_boolean(false));
    // jsn_object_set(root, "LucyB", jsn_create_boolean(false));

    // jsn_object_set(root, "JackieS", jsn_create_string("Hello, Jackie!"));
    // jsn_object_set(root, "VernonS", jsn_create_string("Hello, Vernon!"));
    // jsn_object_set(root, "LucyS", jsn_create_string("Hello, Lucy!"));

    // jsn_handle people = jsn_create_array();
    // jsn_array_push(people, jsn_create_integer(39));
    // jsn_array_push(people, jsn_create_double(39));
    // jsn_array_push(people, jsn_create_boolean(false));
    // jsn_array_push(people, jsn_create_string("Hello String!"));
    // jsn_object_set(root, "MyArray", people);
    // jsn_print(people);

    // Getting and setting the values of nodes and or changing their types.
    // jsn_handle team_members = jsn_create_object();
    // // When you try to append an existing node, it will get replaced.
    // jsn_object_set(team_members, "Member 1", jsn_create_integer(300));
    // jsn_object_set(team_members, "Member 2", jsn_create_integer(300));
    // jsn_object_set(team_members, "Member 3", jsn_create_integer(200));
    // jsn_print(team_members);

    // jsn_handle member = jsn_get(team_members, 1, "Member 1");
    // jsn_print(member);
    // jsn_set_as_integer(member, 5000);
    // jsn_print(member);
    // jsn_set_as_double(member, 5000);
    // jsn_print(member);
    // jsn_set_as_boolean(member, false);
    // jsn_print(member);
    // jsn_set_as_string(member, "My String Value");
    // jsn_print(member);
    // jsn_set_as_object(member);
    // jsn_print(member);
    // jsn_set_as_array(member);
    // jsn_print(member);

    // jsn_handle file_object =
    //     jsn_from_file("/home/vernon/Devenv/projects/json_c/data/citylots.json");
    // jsn_print(file_object);

    // jsn_benchmark_start();
    // jsn_handle file_object = jsn_from_file(
    //     "/home/vernon/Devenv/projects/json_c/data/testing-large.json");
    // jsn_benchmark_end("Parsing of 25MB, testing large JSON file.");
    // jsn_print(file_object);

    // jsn_benchmark_start();
    // jsn_handle file_object = jsn_from_file(
    //     "/home/vernon/Devenv/projects/json_c/data/selectors.json");
    // jsn_benchmark_end("Parsing of CSS selectors JSON file.");
    // jsn_print(file_object);

    // jsn_benchmark_start();
    // jsn_handle file_object = jsn_from_file(
    //     "/home/vernon/Devenv/projects/json_c/data/testing-1.json");
    // jsn_benchmark_end("Parsing of normally sized, JSON file.");
    // jsn_print(file_object);

    // jsn_handle member = jsn_get(team_members, 1, "Member 1");

    // printf("The size of a single jsn_node is: %lu, bytes!\n", sizeof( char *));
    // printf("The size of a single jsn_node is: %lu, bytes!\n", sizeof( enum jsn_node_type));
    // printf("The size of a single jsn_node is: %lu, bytes!\n", sizeof( union jsn_node_value));
    // printf("The size of a single jsn_node is: %lu, bytes!\n", sizeof( unsigned int ));
    // printf("The size of a single jsn_node is: %lu, bytes!\n", sizeof( struct jsn_node *));

    // jsn_benchmark_start();
    // jsn_handle file_object_1 = jsn_from_file("/home/vernon/Devenv/projects/json_c/data/citylots.json");
    // jsn_benchmark_end("Parsing of 180MB, city lots JSON file.");

    // // TODO: We should clear memory here.
    // jsn_handle file_object_2 =
    //     jsn_from_file("/home/vernon/Devenv/projects/json_c/data/citylots.json");
    // jsn_benchmark_end("Parsing of 2 * 180MB (360MB), city lots JSON file.");

    // jsn_benchmark_start();
    // jsn_handle file_object_2 =
    //     jsn_from_file("/home/vernon/Devenv/projects/json_c/data/citylots.json");
    // jsn_handle file_object_3 =
    //     jsn_from_file("/home/vernon/Devenv/projects/json_c/data/citylots.json");
    // jsn_handle file_object_4 =
    //     jsn_from_file("/home/vernon/Devenv/projects/json_c/data/citylots.json");
    // jsn_benchmark_end("Parsing of 3 * 180MB (540MB), city lots JSON file.");
    // jsn_handle file_object_5 =
    //     jsn_from_file("/home/vernon/Devenv/projects/json_c/data/citylots.json");
    // jsn_handle file_object_6 =
    //     jsn_from_file("/home/vernon/Devenv/projects/json_c/data/citylots.json");
    // jsn_free(file_object);

    // jsn_benchmark_start();
    // jsn_handle file_object_alt = jsn_from_file(
    //     "/home/vernon/Devenv/projects/json_c/data/latestblock.json");
    // jsn_benchmark_end("Parsing of long numbers only, small file JSON file.");

    // jsn_handle file_object_alt_alt = jsn_from_file(
    //     "/home/vernon/Devenv/projects/json_c/data/search.json");

    // jsn_handle file_object = jsn_from_file(
    // "/home/vernon/Devenv/projects/json_c/data/testing-1.json");
    // jsn_print(file_object);

    // Setting node values and changing their types.
    // jsn_handle handle_ages = jsn_from_string("{ \
    //     \"jackie\" : 39, \
    //     \"vernon\" : 32, \
    //     \"jhon\" : 32.5, \
    //     \"lucy\" : true, \
    //     \"Brandon\" : false, \
    //     \"Brandon Alt\" : \"Hello This is my string\" \
    // }");
    // jsn_print(handle_ages);

    // success
    // return 0;
// }
