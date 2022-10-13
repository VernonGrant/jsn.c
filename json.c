#include "json.h"

#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* UTILITIES
 * --------------------------------------------------------------------------*/

void jsn_notice(const char *message) {
    printf("NOTICE: %s\n", message);
    // exit (1);
}

void jsn_failure(const char *message) {
    printf("FAILURE: %s\n", message);
    exit(1);
}

#include <time.h>

static clock_t benchmark_clock;

void jsn_benchmark_start() {
    benchmark_clock = clock();
    printf("Benchmarking:\n");
}

void jsn_benchmark_end(const char *log_prefix) {
    double delta = (double)(clock() - benchmark_clock);
    double elapsed_time = delta / CLOCKS_PER_SEC;

    // Print out benchmark results.
    printf("Benchmark Result:\n");
    printf("---------------------------------------------------------------\n");
    printf("It took %f, seconds!\n", elapsed_time);
    printf("---------------------------------------------------------------\n");

    // Write the result to a file.
    FILE *f =
        fopen("/home/vernon/Devenv/projects/json_c/benchmarking.txt", "a");

    if (f == NULL) {
        printf("Error opening file!\n");
        exit(1);
    }

    // get the time.
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    fprintf(f, "%d-%02d-%02d %02d:%02d:%02d | ", tm.tm_year + 1900,
            tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    fprintf(f, "%s | %f seconds.\n", log_prefix, elapsed_time);
    fclose(f);
}

/* TOKENIZER
 * --------------------------------------------------------------------------*/

enum jsn_token_kind {
    JSN_TOC_UNKNOWN,
    JSN_TOC_NULL,
    JSN_TOC_NUMBER,
    JSN_TOC_STRING,
    JSN_TOC_BOOLEAN,
    JSN_TOC_ARRAY_OPEN,
    JSN_TOC_ARRAY_CLOSE,
    JSN_TOC_OBJECT_OPEN,
    JSN_TOC_OBJECT_CLOSE,
    JSN_TOC_COMMA,
    JSN_TOC_COLON
};

struct jsn_token {
    enum jsn_token_kind type;
    char *lexeme;
};

struct jsn_tokenizer {
    // Holds a copy of the provided source code.
    char *src;
    unsigned int src_cursor;

    // TODO: Think where this memory should be freed.
    // A memory pool that holds all initial lexeme strings.
    char *lexeme_memory_pool;
    unsigned int lexeme_memory_pool_cursor;
    unsigned int lexeme_memory_pool_size;
};

/**
 * Initialized the tokenizer, the passed in char pointer should point to
 * malloced memory.
 */
struct jsn_tokenizer jsn_tokenizer_init(const char *src, unsigned int src_len) {
    // Construct tokenizer.
    struct jsn_tokenizer tokenizer;

    // Allocate and copy the string over to the tokenizer.
    tokenizer.src = strcpy(malloc(src_len + 1 * CHAR_BIT), src);
    tokenizer.src[src_len + 1] = '\0';
    tokenizer.src_cursor = 0;

    /* Lexeme memory pool, we basically allocate a large pool of memory for all
     * initial lexeme.*/
    tokenizer.lexeme_memory_pool = malloc((src_len * 2) * CHAR_BIT);
    tokenizer.lexeme_memory_pool_size = src_len * 2;
    tokenizer.lexeme_memory_pool_cursor = 0;

    return tokenizer;
};

/**
 * Only call if you want to free the source memory.
 */
void jsn_tokenizer_free_src(struct jsn_tokenizer *tokenizer) {
    free(tokenizer->src);
    tokenizer->src = NULL;
}

/* TODO: Make this a utility function, as the only thing it does dynamically
 * expands the memory of a string. */
static inline void jsn_token_lexeme_append(struct jsn_token *token,
                                           struct jsn_tokenizer *tokenizer) {
    // Add this char to the lexeme memory pool.
    tokenizer->lexeme_memory_pool[tokenizer->lexeme_memory_pool_cursor] =
        tokenizer->src[tokenizer->src_cursor];
    tokenizer->lexeme_memory_pool_cursor++;

    // Increment the tokenizer src.
    tokenizer->src_cursor++;
}

// TODO: Implement lexeme_end function.
static inline void jsn_token_lexeme_start(struct jsn_token *token,
                                          struct jsn_tokenizer *tokenizer) {
    token->lexeme =
        &tokenizer->lexeme_memory_pool[tokenizer->lexeme_memory_pool_cursor];
}

static inline void jsn_token_lexeme_end(struct jsn_token *token,
                                        struct jsn_tokenizer *tokenizer) {
    // Set the null terminator.
    tokenizer->lexeme_memory_pool[tokenizer->lexeme_memory_pool_cursor] = '\0';
    tokenizer->lexeme_memory_pool_cursor++;
}

struct jsn_token jsn_tokenizer_get_next_token(struct jsn_tokenizer *tokenizer) {
    // TODO: Optimize the tokenizing process, below.
    // TODO: Implementing regex patterns instead.

    // Is the cursor on a number.
    struct jsn_token token;
    token.type = JSN_TOC_UNKNOWN;
    token.lexeme = NULL;

    // Just skip spaces.
    if (isspace(tokenizer->src[tokenizer->src_cursor])) {
        tokenizer->src_cursor++;
        return jsn_tokenizer_get_next_token(tokenizer);
    }

    // Handle strings.
    if (tokenizer->src[tokenizer->src_cursor] == '"') {
        token.type = JSN_TOC_STRING;
        tokenizer->src_cursor++;

        // Set lexeme starting location.
        jsn_token_lexeme_start(&token, tokenizer);

        // This will keep adding bytes until the end of string is reached.
        while (tokenizer->src[tokenizer->src_cursor] != '"') {

            // Handled escaped backslashes.
            if (tokenizer->src[tokenizer->src_cursor] == '\\' &&
                tokenizer->src[tokenizer->src_cursor + 1] == '\\') {
                tokenizer->src_cursor++;
                jsn_token_lexeme_append(&token, tokenizer);
                continue;
            }

            // Handled escaped quotes.
            if (tokenizer->src[tokenizer->src_cursor] == '\\' &&
                tokenizer->src[tokenizer->src_cursor + 1] == '"') {
                tokenizer->src_cursor++;
                jsn_token_lexeme_append(&token, tokenizer);
                continue;
            }

            // Perform operation.
            jsn_token_lexeme_append(&token, tokenizer);
        }

        // Set lexeme ending null terminator.
        jsn_token_lexeme_end(&token, tokenizer);

        // Move the past the ending quote.
        tokenizer->src_cursor++;

        return token;
    }

    // Get the number token.
    if (isdigit(tokenizer->src[tokenizer->src_cursor]) ||
        tokenizer->src[tokenizer->src_cursor] == '-') {
        token.type = JSN_TOC_NUMBER;

        // Set lexeme starting location.
        jsn_token_lexeme_start(&token, tokenizer);

        while (isdigit(tokenizer->src[tokenizer->src_cursor]) ||
               tokenizer->src[tokenizer->src_cursor] == '-' ||
               tokenizer->src[tokenizer->src_cursor] == '.') {
            jsn_token_lexeme_append(&token, tokenizer);
        }

        // Set lexeme ending null terminator.
        jsn_token_lexeme_end(&token, tokenizer);

        return token;
    }

    // Handle boolean types
    if (tokenizer->src[tokenizer->src_cursor] == 't' ||
        tokenizer->src[tokenizer->src_cursor] == 'f') {

        // Here we can assume the this must be a boolean value.
        token.type = JSN_TOC_BOOLEAN;

        // Set lexeme starting location.
        jsn_token_lexeme_start(&token, tokenizer);

        // If true or false.
        if (tokenizer->src[tokenizer->src_cursor] == 't') {
            // true, 0123
            for (char i = 0; i < 4; i++) {
                jsn_token_lexeme_append(&token, tokenizer);
            }
        } else {
            // false, 01234
            for (char i = 0; i < 5; i++) {
                jsn_token_lexeme_append(&token, tokenizer);
            }
        }

        // Set lexeme ending null terminator.
        jsn_token_lexeme_end(&token, tokenizer);

        return token;
    }

    // Handle null
    if (tokenizer->src[tokenizer->src_cursor] == 'n') {
        // Here we can assume the this must be a null value.
        token.type = JSN_TOC_NULL;

        // Set lexeme starting location.
        jsn_token_lexeme_start(&token, tokenizer);

        // null, 0123
        for (char i = 0; i < 4; i++) {
            jsn_token_lexeme_append(&token, tokenizer);
        }

        // Set lexeme ending null terminator.
        jsn_token_lexeme_end(&token, tokenizer);

        return token;
    }

    // Check all other general token types.
    switch (tokenizer->src[tokenizer->src_cursor]) {
    case '[':
        token.type = JSN_TOC_ARRAY_OPEN;
        break;
    case ']':
        token.type = JSN_TOC_ARRAY_CLOSE;
        break;
    case ',':
        token.type = JSN_TOC_COMMA;
        break;
    case '{':
        token.type = JSN_TOC_OBJECT_OPEN;
        break;
    case '}':
        token.type = JSN_TOC_OBJECT_CLOSE;
        break;
    case ':':
        token.type = JSN_TOC_COLON;
        break;
    default:
        jsn_failure("Unknown token, found.");
        return token;
    }

    // Set lexeme starting location.
    jsn_token_lexeme_start(&token, tokenizer);

    // If one of the above cases where true.
    jsn_token_lexeme_append(&token, tokenizer);

    // Set lexeme ending null terminator.
    jsn_token_lexeme_end(&token, tokenizer);

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
    enum jsn_node_type type;
    union jsn_node_value value;
    char *key;
    unsigned int children_count;
    struct jsn_node **children;
};

static inline struct jsn_node *jsn_create_node(enum jsn_node_type type) {
    // Let's allocate some memory on the heap.
    struct jsn_node *node = malloc(sizeof(struct jsn_node));

    // Set some sane defaults.
    node->type = type;
    node->children_count = 0;
    node->children = NULL;
    node->key = NULL;

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
    }

    // If it has a key we also need to free that.
    if (node->key != NULL && keep_key == false) {
        free(node->key);
        node->key = NULL;
    }

    // We also need to free it's children.
    if (node->children_count > 0) {
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

    node->value.value_string = token.lexeme;
    return node;
}

struct jsn_node *jsn_parse_null(struct jsn_tokenizer *tokenizer,
                                struct jsn_token token) {
    struct jsn_node *node = jsn_create_node(JSN_NODE_NULL);
    return node;
}

struct jsn_node *jsn_parse_number(struct jsn_tokenizer *tokenizer,
                                  struct jsn_token token) {
    struct jsn_node *node;

    // We check if the token lexeme contains a period, then we know that it's a
    // double
    if (strchr(token.lexeme, '.')) {
        // Double
        node = jsn_create_node(JSN_NODE_DOUBLE);
        node->value.value_double = strtod(token.lexeme, NULL);
    } else {
        // Integer
        node = jsn_create_node(JSN_NODE_INTEGER);
        node->value.value_integer = atoi(token.lexeme);
    }

    return node;
}

struct jsn_node *jsn_parse_boolean(struct jsn_tokenizer *tokenizer,
                                   struct jsn_token token) {
    struct jsn_node *node;

    if (strcmp(token.lexeme, "true") == 0) {
        // True
        node = jsn_create_node(JSN_NODE_BOOLEAN);
        node->value.value_boolean = true;
    } else {
        // False
        node = jsn_create_node(JSN_NODE_BOOLEAN);
        node->value.value_boolean = false;
    }

    return node;
}

struct jsn_node *jsn_parse_array(struct jsn_tokenizer *tokenizer,
                                 struct jsn_token token) {
    // Create our array node type.
    struct jsn_node *node = jsn_create_node(JSN_NODE_ARRAY);

    // While we are not at the end of the array, handle array tokens.
    while (token.type != JSN_TOC_ARRAY_CLOSE) {
        token = jsn_tokenizer_get_next_token(tokenizer);

        // Create the new child node (recursive call).
        struct jsn_node *child_node = jsn_parse_value(tokenizer, token);

        if (child_node != NULL) {
            jsn_append_node_child(node, child_node);
        }
    }

    return node;
}

struct jsn_node *jsn_parse_object(struct jsn_tokenizer *tokenizer,
                                  struct jsn_token token) {
    // Create our object node type..
    struct jsn_node *node = jsn_create_node(JSN_NODE_OBJECT);

    // While we haven't reached the end of the object.
    while (token.type != JSN_TOC_OBJECT_CLOSE) {

        // Get the key.
        struct jsn_token token_key = jsn_tokenizer_get_next_token(tokenizer);

        // It's an empty token, just break and move on.
        if (token_key.type == JSN_TOC_OBJECT_CLOSE) {
            break;
        }

        if (token_key.type != JSN_TOC_STRING) {
            jsn_failure("Unknown token found.");
        }

        // Get the colon.
        struct jsn_token token_cln = jsn_tokenizer_get_next_token(tokenizer);
        if (token_cln.type != JSN_TOC_COLON) {
            jsn_failure("Unknown token found.");
        }

        // Get the value and create the new child node (recursive call).
        struct jsn_token token_val = jsn_tokenizer_get_next_token(tokenizer);
        struct jsn_node *child_node = jsn_parse_value(tokenizer, token_val);
        child_node->key = token_key.lexeme;

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
    case JSN_TOC_NULL:
        return jsn_parse_null(tokenizer, token);
    case JSN_TOC_NUMBER:
        return jsn_parse_number(tokenizer, token);
    case JSN_TOC_STRING:
        return jsn_parse_string(tokenizer, token);
    case JSN_TOC_BOOLEAN:
        return jsn_parse_boolean(tokenizer, token);
    case JSN_TOC_ARRAY_OPEN:
        return jsn_parse_array(tokenizer, token);
    case JSN_TOC_OBJECT_OPEN:
        return jsn_parse_object(tokenizer, token);
    case JSN_TOC_ARRAY_CLOSE:
    case JSN_TOC_OBJECT_CLOSE:
    case JSN_TOC_COMMA:
        return NULL;
    default:
        jsn_failure("Undefined token found.");
        break;
    }

    return NULL;
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
    // Initialize our tokenizer, for this specific source string.
    struct jsn_tokenizer tokenizer = jsn_tokenizer_init(src, strlen(src) + 1);

    // Prime the tokenizer.
    struct jsn_token token = jsn_tokenizer_get_next_token(&tokenizer);

    // Start parsing, recursively.
    jsn_handle root_node = jsn_parse_value(&tokenizer, token);

    // Frees the tokenizer source.
    jsn_tokenizer_free_src(&tokenizer);

    return root_node;
}

jsn_handle jsn_from_file(const char *file_path) {
    // TODO: Note there's a file size limit here for fseek and fread.
    // TODO: Will require a more sophisticated solution.

    FILE *file_ptr;

    // Open the file.
    file_ptr = fopen(file_path, "r");

    // In case the file can't be read, report and return null.
    if (file_ptr == NULL) {
        fclose(file_ptr);
        jsn_notice("The file could not be opened.");
        return NULL;
    }

    // Let's get the size of the file in bytes.
    fseek(file_ptr, 0, SEEK_END);
    int file_size = ftell(file_ptr) + 1;

    // Rewind and allocate a local string.
    fseek(file_ptr, 0, SEEK_SET);

    // Allocate and copy file source into a temp buffer.
    char *file_buffer = malloc(file_size * CHAR_BIT);
    fread(file_buffer, file_size, 1, file_ptr);
    file_buffer[file_size - 1] = '\0';

    // Close the file steam.
    fclose(file_ptr);

    // Create the tokenizer from and point the src to the above buffer..
    struct jsn_tokenizer tokenizer = jsn_tokenizer_init(file_buffer, file_size);

    // Start parsing, recursively.
    jsn_handle root_node =
        jsn_parse_value(&tokenizer, jsn_tokenizer_get_next_token(&tokenizer));

    // Completed, so we can not free the tokenizer src.
    jsn_tokenizer_free_src(&tokenizer);

    if (root_node == NULL) {
        jsn_notice("The file could not be parsed, it might contain issues.");
        return NULL;
    }

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
        jsn_notice("The handle passed to jsn_get_array_item isn't an array.");
        return NULL;
    }

    // Make sure the provided index is not larger then the array itself.
    if (handle->children_count > (index + 1)) {
        // TODO: Define better error messages.
        jsn_notice("The provided index is outside the arrays scope.");
        return NULL;
    }

    // Check to make sure the array does in fact have children.
    if (handle->children_count == 0) {
        // TODO: Define better error messages.
        jsn_notice("The array does not have any children.");
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
        jsn_notice("The handle passed to jsn_object_set isn't an object.");
    }

    // Already has a key so we need to free it.
    if (node->key != NULL) {
        free(node->key);
        node->key = NULL;
    }

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
        jsn_notice("The handle passed to object append isn't an array.");
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
    handle->value.value_string = strcpy(malloc(str_len * CHAR_BIT), value);
}

void jsn_free(jsn_handle handle) { jsn_free_node(handle); }

/* TESTING:
 * --------------------------------------------------------------------------*/

int main(void) {
    // Building a JSON tree from code.
    // jsn_handle root = jsn_create_object();

    // jsn_object_set(root, "Jackie", jsn_create_integer(39));
    // jsn_object_set(root, "Vernon", jsn_create_integer(32));
    // jsn_object_set(root, "Lucy", jsn_create_integer(80));

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

    // jsn_benchmark_start();
    // jsn_handle file_object =
    //     jsn_from_file("/home/vernon/Devenv/projects/json_c/data/citylots.json");
    // jsn_benchmark_end("Parsing of 180MB, city lots JSON file.");

    jsn_benchmark_start();
    jsn_handle file_object =
        jsn_from_file("/home/vernon/Devenv/projects/json_c/data/testing-large.json");
    jsn_benchmark_end("Parsing of 25MB, testing large JSON file.");
    // jsn_print(file_object);

    // jsn_benchmark_start();
    // jsn_handle file_object =
    //     jsn_from_file("/home/vernon/Devenv/projects/json_c/data/testing-1.json");
    // jsn_benchmark_end("Parsing of small, testing 1 JSON file.");
    // jsn_print(file_object);

    // jsn_handle member = jsn_get(team_members, 1, "Member 1");

    // jsn_benchmark_start();
    // jsn_handle file_object_1 =
    //     jsn_from_file("/home/vernon/Devenv/projects/json_c/data/citylots.json");
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

    // jsn_handle file_object_alt = jsn_from_file(
    //     "/home/vernon/Devenv/projects/json_c/data/latestblock.json");

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
    return 0;
}
