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

void jsn_assert(const char *message) {
    printf("ASSERTION: %s\n", message);
    // exit (0);
}

/* TOKENIZER
 * --------------------------------------------------------------------------*/

enum jsn_token_kind {
    JSN_TOC_NULL,
    JSN_TOC_NUMBER,
    JSN_TOC_STRING,
    JSN_TOC_ARRAY_OPEN,
    JSN_TOC_ARRAY_CLOSE,
    JSN_TOC_OBJECT_OPEN,
    JSN_TOC_OBJECT_CLOSE,
    JSN_TOC_COMMA,
    JSN_TOC_COLON
};

struct jsn_token {
    unsigned int type;
    unsigned long lexeme_len;
    char *lexeme;
};

struct jsn_tokenizer {
    char *src;
    unsigned int cursor;
};

struct jsn_tokenizer jsn_tokenizer_init(const char *src) {
    // Determine the length of the provided source string.
    unsigned int src_length = strlen(src) + 1;

    // Set aside memory and make a copy.
    char *src_cpy = strcpy(malloc(src_length * CHAR_BIT), src);

    // Construct tokenizer.
    struct jsn_tokenizer tokenizer;
    tokenizer.src = src_cpy;
    tokenizer.cursor = 0;

    return tokenizer;
};

/* TODO: Make this a utility function, as the only thing it does dynamically
 * expands the memory of a string. */
void jsn_token_lexeme_append(struct jsn_token *token, char c) {
    // Will chunk 50 * CHAR_BIT.
    static const unsigned int chunk_size = 50;

    // Handle dynamic memory allocation, in chunks.
    if (token->lexeme == NULL) {
        char *lexeme_ext = malloc(chunk_size * CHAR_BIT);
        token->lexeme_len = chunk_size;
        token->lexeme = lexeme_ext;
    } else if (token->lexeme_len < (strlen(token->lexeme) + 1)) {
        // Keep pointer to old lexeme.
        char *old_lexeme = token->lexeme;

        // Allocate additional memory chunk.
        unsigned long int lexeme_new_len = (token->lexeme_len + chunk_size);
        char *lexeme_ext = malloc(lexeme_new_len * CHAR_BIT);
        token->lexeme =
            memcpy(lexeme_ext, token->lexeme, token->lexeme_len * CHAR_BIT);

        // Set the new length.
        token->lexeme_len = lexeme_new_len;

        // Free old memory.
        free(old_lexeme);

        // TODO: Remove print statement.
        printf("Allocated more memory! \n");
    }

    // Get's the lexemes length.
    unsigned long int lexeme_len = strlen(token->lexeme) + 1;

    // Append the additional char.
    token->lexeme[lexeme_len] = '\0';
    token->lexeme[lexeme_len - 1] = c;
}

struct jsn_token jsn_tokenizer_get_next_token(struct jsn_tokenizer *tokenizer) {
    // Is the cursor on a number.
    struct jsn_token token;
    token.type = JSN_TOC_NULL;
    token.lexeme = NULL;

    // TODO: Implementing regex patterns instead.

    // Get the number token.
    if (isdigit(tokenizer->src[tokenizer->cursor])) {
        // \todo Handle different number types.

        token.type = JSN_TOC_NUMBER;
        while (isdigit(tokenizer->src[tokenizer->cursor]) ||
               tokenizer->src[tokenizer->cursor] == '.') {
            // Perform operation.
            jsn_token_lexeme_append(&token, tokenizer->src[tokenizer->cursor]);
            // Move the cursor up.
            tokenizer->cursor++;
        }
        return token;
    }

    // TODO: Optimize the tokenizing process, below.

    // Handle strings.
    if (tokenizer->src[tokenizer->cursor] == '"') {
        token.type = JSN_TOC_STRING;
        tokenizer->cursor++;

        // TODO: Handle skipping escaped values.

        // This will keep adding bytes until the end of string is reached.
        // while (isalpha (tokenizer->src[tokenizer->cursor]))
        while (tokenizer->src[tokenizer->cursor] != '"') {
            // Perform operation.
            jsn_token_lexeme_append(&token, tokenizer->src[tokenizer->cursor]);
            // Move the cursor up.
            tokenizer->cursor++;
        }

        // Move the past the ending quote.
        tokenizer->cursor++;

        return token;
    }

    // Handle array opening.
    if (tokenizer->src[tokenizer->cursor] == '[') {
        token.type = JSN_TOC_ARRAY_OPEN;
        jsn_token_lexeme_append(&token, tokenizer->src[tokenizer->cursor]);
        tokenizer->cursor++;
        return token;
    }

    // Handle array closing.
    if (tokenizer->src[tokenizer->cursor] == ']') {
        token.type = JSN_TOC_ARRAY_CLOSE;
        jsn_token_lexeme_append(&token, tokenizer->src[tokenizer->cursor]);
        tokenizer->cursor++;
        return token;
    }

    // Handle comma separator.
    if (tokenizer->src[tokenizer->cursor] == ',') {
        token.type = JSN_TOC_COMMA;
        jsn_token_lexeme_append(&token, tokenizer->src[tokenizer->cursor]);
        tokenizer->cursor++;
        return token;
    }

    // Handle object opening.
    if (tokenizer->src[tokenizer->cursor] == '{') {
        token.type = JSN_TOC_OBJECT_OPEN;
        jsn_token_lexeme_append(&token, tokenizer->src[tokenizer->cursor]);
        tokenizer->cursor++;
        return token;
    }

    // Handle object closing.
    if (tokenizer->src[tokenizer->cursor] == '}') {
        token.type = JSN_TOC_OBJECT_CLOSE;
        jsn_token_lexeme_append(&token, tokenizer->src[tokenizer->cursor]);
        tokenizer->cursor++;
        return token;
    }

    // Handle colon.
    if (tokenizer->src[tokenizer->cursor] == ':') {
        token.type = JSN_TOC_COLON;
        jsn_token_lexeme_append(&token, tokenizer->src[tokenizer->cursor]);
        tokenizer->cursor++;
        return token;
    }

    // Handle white spaces.
    if (isspace(tokenizer->src[tokenizer->cursor])) {
        tokenizer->cursor++;
        return jsn_tokenizer_get_next_token(tokenizer);
    }

    return token;
}

/* TREE DATA STRUCTURE:
 * --------------------------------------------------------------------------*/

enum jsn_node_type {
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

struct jsn_node *jsn_create_node(enum jsn_node_type type) {
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

    // TODO: Not sure about this.
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
        if (token_key.type != JSN_TOC_STRING) {
            // TODO: implement proper error handling.
            perror("Undefined token as Object node key, maybe you "
                   "left an "
                   "extra comma after an object value.\n");
            exit(-1);
        }

        // Get the colon.
        struct jsn_token token_cln = jsn_tokenizer_get_next_token(tokenizer);
        if (token_cln.type != JSN_TOC_COLON) {
            // TODO: implement proper error handling.
            perror("Undefined token after key, did you forget a colon "
                   "somewhere?.\n");
            exit(-1);
        }

        // Get the value and create the new child node (recursive call).
        struct jsn_token token_val = jsn_tokenizer_get_next_token(tokenizer);
        struct jsn_node *child_node = jsn_parse_value(tokenizer, token_val);
        child_node->key = token_key.lexeme;

        // Append the child node.
        jsn_append_node_child(node, child_node);

        /* Set the last token, if this token is a comma, then the next
           token should be a string key, if not it will report errors above
           on the next iteration.  */
        token = jsn_tokenizer_get_next_token(tokenizer);
    }

    return node;
}

struct jsn_node *jsn_parse_value(struct jsn_tokenizer *tokenizer,
                                 struct jsn_token token) {
    switch (token.type) {
    case JSN_TOC_NUMBER:
        return jsn_parse_number(tokenizer, token);
        break;
    case JSN_TOC_STRING:
        return jsn_parse_string(tokenizer, token);
        break;
    case JSN_TOC_ARRAY_OPEN:
        return jsn_parse_array(tokenizer, token);
        break;
    case JSN_TOC_ARRAY_CLOSE:
        return NULL;
        break;
    case JSN_TOC_OBJECT_OPEN:
        return jsn_parse_object(tokenizer, token);
        break;
    case JSN_TOC_OBJECT_CLOSE:
        return NULL;
        break;
    case JSN_TOC_COMMA:
        return NULL;
        break;
    default:
        // TODO: implement proper error handling.
        perror("Undefined token found.\n");
        exit(-1);
        break;
    }
}

/* Debug:
 * --------------------------------------------------------------------------*/

void jsn_node_print_intented(struct jsn_node *node, unsigned int indent) {
    // Generate padding string, if needed.
    char indent_str[indent + 1];
    unsigned int i;
    for (i = 0; i < indent; i++) {
        indent_str[i] = '.';
    }
    indent_str[indent] = '\0';

    // Print out the current node.
    printf("%s[\n", indent_str);
    printf("%sNode: \n", indent_str);
    printf("%sKey: %s\n", indent_str, node->key);
    switch (node->type) {
    case JSN_NODE_STRING:
        printf("%sType: %s\n", indent_str, "STRING");
        printf("%sValue: %s\n", indent_str, node->value.value_string);
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
    case JSN_NODE_ARRAY:
        printf("%sType: %s\n", indent_str, "ARRAY");
        break;
    case JSN_NODE_OBJECT:
        printf("%sType: %s\n", indent_str, "OBJECT");
        break;
    }
    printf("%sChildren_Count: %u\n", indent_str, node->children_count);

    // Print children too.
    {
        unsigned int i;
        for (i = 0; i < node->children_count; i++) {
            // Print out node children (recursive call).
            jsn_node_print_intented(node->children[i], indent + 4);
        }
    }
    printf("%s]\n", indent_str);
}

/* API:
 * --------------------------------------------------------------------------*/

void jsn_print(jsn_handle handle) { jsn_node_print_intented(handle, 0); }

jsn_handle jsn_form_string(const char *src) {
    // Initialize our tokenizer, for this specific source string.
    struct jsn_tokenizer tokenizer = jsn_tokenizer_init(src);

    // Prime the tokenizer.
    struct jsn_token token = jsn_tokenizer_get_next_token(&tokenizer);

    // Start parsing, recursively.
    return jsn_parse_value(&tokenizer, token);
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

/**
 * Will append a node to an node of object type. If the object already has a
 * node with the same key, it will get replaced. Keep in mind the replaced
 * node will be freed and set to NULL.
 */
void jsn_object_set(jsn_handle handle, const char *key, jsn_handle node) {
    // Make sure were dealing with an object handle type here.
    if (handle->type != JSN_NODE_OBJECT) {
        jsn_assert(
            "The handle passed to jsn_node_object_append isn't an object.");
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
        jsn_assert("The handle passed to object append isn't an array.");
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

void jsn_delete(jsn_handle handle) { jsn_free_node(handle); }

/* TESTING:
 * --------------------------------------------------------------------------*/

int main(void) {
    // Building a JSON tree from code.
    jsn_handle root = jsn_create_object();

    jsn_object_set(root, "Jackie", jsn_create_integer(39));
    jsn_object_set(root, "Vernon", jsn_create_integer(32));
    jsn_object_set(root, "Lucy", jsn_create_integer(80));

    jsn_object_set(root, "JackieD", jsn_create_double(39));
    jsn_object_set(root, "VernonD", jsn_create_double(32));
    jsn_object_set(root, "LucyD", jsn_create_double(80));

    jsn_object_set(root, "JackieB", jsn_create_boolean(true));
    jsn_object_set(root, "VernonB", jsn_create_boolean(false));
    jsn_object_set(root, "LucyB", jsn_create_boolean(false));

    jsn_object_set(root, "JackieS", jsn_create_string("Hello, Jackie!"));
    jsn_object_set(root, "VernonS", jsn_create_string("Hello, Vernon!"));
    jsn_object_set(root, "LucyS", jsn_create_string("Hello, Lucy!"));

    jsn_handle people = jsn_create_array();
    jsn_array_push(people, jsn_create_integer(39));
    jsn_array_push(people, jsn_create_double(39));
    jsn_array_push(people, jsn_create_boolean(false));
    jsn_array_push(people, jsn_create_string("Hello String!"));
    jsn_object_set(root, "MyArray", people);
    jsn_print(root);

    // Getting and setting the values of nodes and or changing their types.
    jsn_handle team_members = jsn_create_object();
    // When you try to append an existing node, it will get replaced.
    jsn_object_set(team_members, "Member 1", jsn_create_integer(300));
    jsn_object_set(team_members, "Member 2", jsn_create_integer(300));
    jsn_object_set(team_members, "Member 3", jsn_create_integer(200));
    jsn_print(team_members);

    jsn_handle member = jsn_get(team_members, 1, "Member 1");
    jsn_print(member);
    jsn_set_as_integer(member, 5000);
    jsn_print(member);
    jsn_set_as_double(member, 5000);
    jsn_print(member);
    jsn_set_as_boolean(member, false);
    jsn_print(member);
    jsn_set_as_string(member, "My String Value");
    jsn_print(member);
    jsn_set_as_object(member);
    jsn_print(member);
    jsn_set_as_array(member);
    jsn_print(member);

    // Setting node values and changing their types.
    // jsn_handle handle_ages = jsn_form_string("{ \
    //     \"jackie\" : 39, \
    //     \"vernon\" : 32, \
    //     \"jhon\" : 32.5, \
    //     \"lucy\" : 80 \
    // }");

    // This is our sample object node.
    // jsn_handle handle = jsn_form_string ("{ \
    //     \"mykey\" : 123, \
    //     \"my other key\" : \"this is a UTF8 string! cónstàñt 家長專區.\", \
    //     \"my othere key\" : [1,2,3,4,5,6], \
    //     \"my otheree key\" : {\"this is an inner obj\" : 3000} \
    // }");

    // success
    return 0;
}
