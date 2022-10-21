/**
 * Author: Vernon Grant
 * Repository: https://github.com/VernonGrant/jsn.c
 * License: https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Created: 2022-10-21
 **/

#include "jsn.h"
#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* UTILITIES
 * --------------------------------------------------------------------------*/

void jsn_report_failure(const char *message) {
    // Report failures.
    printf("FAILURE: %s\n", message);
    exit(EXIT_FAILURE);
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
        // Allocate for source string.
        char *src = malloc(source_length * sizeof(char));

        // Check allocation success.
        if (src == NULL) {
            jsn_report_failure("Memory allocation failure.");
        }

        // Copy over source string.
        tokenizer.source = strcpy(src, source);
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

    // Check allocation success.
    if (node == NULL) {
        jsn_report_failure("Memory allocation failure.");
        return NULL;
    }

    // Set some sane defaults.
    node->key = NULL;
    node->type = type;
    node->children_count = 0;
    node->children = NULL;

    return node;
}

void jsn_append_node_child(struct jsn_node *parent, struct jsn_node *child) {
    // Increment children count.
    parent->children_count++;

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
                free(node->children[i]->value.value_string);
            }
            // If it has a key, free it.
            if (node->children[i]->key != NULL) {
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

/**
 * Will write out the node tree to a file stream, minified.
 */
void jsn_node_to_stream(jsn_handle handle, FILE *stream) {
    if (handle->key != NULL) {
        fprintf(stream, "\"%s\":", handle->key);
    }

    switch (handle->type) {
    case JSN_NODE_STRING:
        fprintf(stream, "\"%s\"", handle->value.value_string);
        break;
    case JSN_NODE_INTEGER:
        fprintf(stream, "%u", handle->value.value_integer);
        break;
    case JSN_NODE_DOUBLE:
        fprintf(stream, "%f", handle->value.value_double);
        break;
    case JSN_NODE_BOOLEAN: {
        if (handle->value.value_boolean == true) {
            fprintf(stream, "true");
        } else {
            fprintf(stream, "false");
        }
    } break;
    case JSN_NODE_NULL:
        fprintf(stream, "null");
        break;
    case JSN_NODE_ARRAY: {
        fprintf(stream, "[");
        for (unsigned int i = 0; i < handle->children_count; i++) {
            jsn_node_to_stream(handle->children[i], stream);
            if (i != (handle->children_count - 1)) {
                fprintf(stream, ",");
            }
        }
        fprintf(stream, "]");
    } break;
    case JSN_NODE_OBJECT: {
        fprintf(stream, "{");
        for (unsigned int i = 0; i < handle->children_count; i++) {
            jsn_node_to_stream(handle->children[i], stream);
            if (i != (handle->children_count - 1)) {
                fprintf(stream, ",");
            }
        }
        fprintf(stream, "}");
    } break;
    }
}

/* PARSER:
 * --------------------------------------------------------------------------*/

struct jsn_node *jsn_parse_value(struct jsn_tokenizer *tokenizer,
                                 struct jsn_token token);

struct jsn_node *jsn_parse_string(struct jsn_tokenizer *tokenizer,
                                  struct jsn_token token) {
    struct jsn_node *node = jsn_create_node(JSN_NODE_STRING);

    // Allocate string space.
    char *str = malloc((token.lexeme_length + 1) * sizeof(char));

    // Check allocation success.
    if (str == NULL) {
        jsn_report_failure("Memory allocation failure.");
        return NULL;
    }

    // Copy over the token string.
    node->value.value_string =
        strncpy(str, token.lexeme_start, token.lexeme_length);
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

        // Allocate memory.
        char *key = malloc((token_key.lexeme_length + 1) * sizeof(char));

        // Check allocation success.
        if (key == NULL) {
            jsn_report_failure("Memory allocation failure.");
            return NULL;
        }

        // Copy over key.
        child_node->key =
            strncpy(key, token_key.lexeme_start, token_key.lexeme_length);
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
    if (handle->key != NULL) {
        printf("\"%s\":", handle->key);
    }

    switch (handle->type) {
    case JSN_NODE_STRING:
        printf("\"%s\"", handle->value.value_string);
        break;
    case JSN_NODE_INTEGER:
        printf("%u", handle->value.value_integer);
        break;
    case JSN_NODE_DOUBLE:
        printf("%f", handle->value.value_double);
        break;
    case JSN_NODE_BOOLEAN: {
        if (handle->value.value_boolean == true) {
            printf("true");
        } else {
            printf("false");
        }
    } break;
    case JSN_NODE_NULL:
        printf("null");
        break;
    case JSN_NODE_ARRAY: {
        printf("[");
        for (unsigned int i = 0; i < handle->children_count; i++) {
            jsn_print(handle->children[i]);
            if (i != (handle->children_count - 1)) {
                printf(",");
            }
        }
        printf("]");
    } break;
    case JSN_NODE_OBJECT: {
        printf("{");
        for (unsigned int i = 0; i < handle->children_count; i++) {
            jsn_print(handle->children[i]);
            if (i != (handle->children_count - 1)) {
                printf(",");
            }
        }
        printf("}");
    } break;
    }
}

jsn_handle jsn_from_file(const char *file_path) {
    // TODO: Note there's a file size limit here for fseek and fread.
    // TODO: Will require a more sophisticated solution.
    FILE *file_ptr;

    // Open the file.
    file_ptr = fopen(file_path, "r");

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

    // Allocate and copy file source into a temp buffer.
    char *file_buffer = malloc(file_size * sizeof(char));

    // Check allocation success.
    if (file_buffer == NULL) {
        jsn_report_failure("Memory allocation failure.");
        return NULL;
    }

    // Read file into buffer.
    fread(file_buffer, file_size, 1, file_ptr);
    file_buffer[file_size - 1] = '\0';

    // Close the file steam.
    fclose(file_ptr);

    // Create tokenizer from buffer.
    struct jsn_tokenizer tokenizer =
        jsn_tokenizer_init(file_buffer, file_size, false);

    // Get the first token.
    struct jsn_token token = jsn_tokenizer_get_next_token(&tokenizer);

    // Start parsing, recursively.
    jsn_handle root_node = jsn_parse_value(&tokenizer, token);

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

void jsn_to_file(jsn_handle handle, const char *file_path) {
    FILE *file_ptr;

    // Open the file.
    file_ptr = fopen(file_path, "w");

    // Write the tree to a stream.
    jsn_node_to_stream(handle, file_ptr);

    // Handle writing to a file.
    fclose(file_ptr);
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

    // Allocate new string.
    char *str = malloc(strlen(value) * sizeof(char));

    // Check allocation success.
    if (str == NULL) {
        jsn_report_failure("Memory allocation failure.");
        return NULL;
    }

    // Copy value into allocated string.
    node->value.value_string = strcpy(str, value);

    return node;
}

jsn_handle jsn_create_null() {
    struct jsn_node *node = jsn_create_node(JSN_NODE_NULL);
    return node;
}

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
        jsn_report_failure("The given handle is not of ARRAY type.");
        return NULL;
    }

    // Make sure the provided index is not larger then the array itself.
    if ((index + 1) > handle->children_count) {
        jsn_report_failure("The given index is larger then the array.");
        return NULL;
    }

    // Check to make sure the array does in fact have children.
    if (handle->children_count == 0) {
        jsn_report_failure("The given handle has no children.");
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
        jsn_report_failure("The handle is not an object.");
        return;
    }

    // Already has a key so we need to free it.
    if (node->key != NULL) {
        free(node->key);
        node->key = NULL;
    }

    // Allocate for the nodes new key.
    char *str = malloc(strlen(key) * sizeof(char));

    // Check allocation success.
    if (str == NULL) {
        jsn_report_failure("Memory allocation failure.");
        return;
    }

    // Copy over the new key string.
    node->key = strcpy(str, key);

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
        jsn_report_failure("The given handle is not of ARRAY type.");
        return;
    }

    // Array children nodes, must not have keys. (Not Objects).
    if (node->key != NULL) {
        free(node->key);
        node->key = NULL;
    }

    // Append the node to the provided object.
    jsn_append_node_child(handle, node);
}

unsigned int jsn_array_count(jsn_handle handle) {
    // If the handle is not for an array, return zero.
    if (handle->type != JSN_NODE_ARRAY) {
        return 0;
    }

    return handle->children_count;
}

int jsn_get_value_int(jsn_handle handle) { return handle->value.value_integer; }

double jsn_get_value_double(jsn_handle handle) {
    return handle->value.value_double;
}

bool jsn_get_value_bool(jsn_handle handle) {
    return handle->value.value_boolean;
}

const char *jsn_get_value_string(jsn_handle handle) {
    return handle->value.value_string;
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

    // Allocate for new string.
    char *str = malloc(str_len * sizeof(char));

    // Check allocation success.
    if (str == NULL) {
        jsn_report_failure("Memory allocation failure.");
        return;
    }

    // Copy over the value.
    handle->value.value_string = strcpy(str, value);
}

void jsn_free(jsn_handle handle) { jsn_free_node(handle); }
