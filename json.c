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
    /* exit (0); */
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
    /* Determine the length of the provided source string. */
    unsigned int src_length = strlen(src) + 1;

    /* Set aside memory and make a copy. */
    char *src_cpy = strcpy(malloc(src_length * CHAR_BIT), src);

    /* Construct tokenizer. */
    struct jsn_tokenizer tokenizer;
    tokenizer.src = src_cpy;
    tokenizer.cursor = 0;

    return tokenizer;
};

/* TODO: Make this a utility function, as the only thing it does dynamically
 * expands the memory of a string. */
void jsn_token_lexeme_append(struct jsn_token *token, char c) {
    /* Will chunk 50 * CHAR_BIT. */
    static const unsigned int chunk_size = 50;

    /* Handle dynamic memory allocation, in chunks. */
    if (token->lexeme == NULL) {
        char *lexeme_ext = malloc(chunk_size * CHAR_BIT);
        token->lexeme_len = chunk_size;
        token->lexeme = lexeme_ext;
    } else if (token->lexeme_len < (strlen(token->lexeme) + 1)) {
        /* Keep pointer to old lexeme. */
        char *old_lexeme = token->lexeme;

        /* Allocate additional memory chunk. */
        unsigned long int lexeme_new_len = (token->lexeme_len + chunk_size);
        char *lexeme_ext = malloc(lexeme_new_len * CHAR_BIT);
        token->lexeme =
            memcpy(lexeme_ext, token->lexeme, token->lexeme_len * CHAR_BIT);

        /* Set the new length. */
        token->lexeme_len = lexeme_new_len;

        /* Free old memory. */
        free(old_lexeme);

        /**
         * TODO: Remove print statement.
         */
        printf("Allocated more memory! \n");
    }

    /* Get's the lexemes length. */
    unsigned long int lexeme_len = strlen(token->lexeme) + 1;

    /* Append the additional char. */
    token->lexeme[lexeme_len] = '\0';
    token->lexeme[lexeme_len - 1] = c;
}

struct jsn_token jsn_tokenizer_get_next_token(struct jsn_tokenizer *tokenizer) {
    /* Is the cursor on a number. */
    struct jsn_token token;
    token.type = JSN_TOC_NULL;
    token.lexeme = NULL;

    /** TODO: Implementing regex patterns instead. */

    /* Get the number token. */
    if (isdigit(tokenizer->src[tokenizer->cursor])) {
        /** \todo Handle different number types.  */

        token.type = JSN_TOC_NUMBER;
        while (isdigit(tokenizer->src[tokenizer->cursor]) ||
               tokenizer->src[tokenizer->cursor] == '.') {
            /* Perform operation. */
            jsn_token_lexeme_append(&token, tokenizer->src[tokenizer->cursor]);
            /* Move the cursor up. */
            tokenizer->cursor++;
        }
        return token;
    }

    /** TODO: Optimize the tokenizing process, below. */

    /* Handle strings. */
    if (tokenizer->src[tokenizer->cursor] == '"') {
        token.type = JSN_TOC_STRING;
        tokenizer->cursor++;

        /** TODO: Handle skipping escaped values.  */

        /* This will keep adding bytes until the end of string is reached.
         */
        /* while (isalpha (tokenizer->src[tokenizer->cursor])) */
        while (tokenizer->src[tokenizer->cursor] != '"') {
            /* Perform operation. */
            jsn_token_lexeme_append(&token, tokenizer->src[tokenizer->cursor]);
            /* Move the cursor up. */
            tokenizer->cursor++;
        }

        /* Move the past the ending quote. */
        tokenizer->cursor++;

        return token;
    }

    /* Handle array opening. */
    if (tokenizer->src[tokenizer->cursor] == '[') {
        token.type = JSN_TOC_ARRAY_OPEN;
        jsn_token_lexeme_append(&token, tokenizer->src[tokenizer->cursor]);
        tokenizer->cursor++;
        return token;
    }

    /* Handle array closing. */
    if (tokenizer->src[tokenizer->cursor] == ']') {
        token.type = JSN_TOC_ARRAY_CLOSE;
        jsn_token_lexeme_append(&token, tokenizer->src[tokenizer->cursor]);
        tokenizer->cursor++;
        return token;
    }

    /* Handle comma separator. */
    if (tokenizer->src[tokenizer->cursor] == ',') {
        token.type = JSN_TOC_COMMA;
        jsn_token_lexeme_append(&token, tokenizer->src[tokenizer->cursor]);
        tokenizer->cursor++;
        return token;
    }

    /* Handle object opening. */
    if (tokenizer->src[tokenizer->cursor] == '{') {
        token.type = JSN_TOC_OBJECT_OPEN;
        jsn_token_lexeme_append(&token, tokenizer->src[tokenizer->cursor]);
        tokenizer->cursor++;
        return token;
    }

    /* Handle object closing. */
    if (tokenizer->src[tokenizer->cursor] == '}') {
        token.type = JSN_TOC_OBJECT_CLOSE;
        jsn_token_lexeme_append(&token, tokenizer->src[tokenizer->cursor]);
        tokenizer->cursor++;
        return token;
    }

    /* Handle colon.*/
    if (tokenizer->src[tokenizer->cursor] == ':') {
        token.type = JSN_TOC_COLON;
        jsn_token_lexeme_append(&token, tokenizer->src[tokenizer->cursor]);
        tokenizer->cursor++;
        return token;
    }

    /* Handle white spaces. */
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
    int value_int;
    double value_double;
    bool value_bool;
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
    /* Let's allocate some memory on the heap. */
    struct jsn_node *node = malloc(sizeof(struct jsn_node));

    /* Set some sane defaults. */
    node->type = type;
    node->children_count = 0;
    node->children = NULL;
    node->key = NULL;

    return node;
}

void jsn_append_node_child(struct jsn_node *parent, struct jsn_node *child) {
    /* Increment children count. */
    parent->children_count++;

    /* Reallocate memory. */
    unsigned int size = (sizeof(struct jsn_node *)) * (parent->children_count);
    parent->children = realloc(parent->children, size);

    /* Set the new node. */
    parent->children[parent->children_count - 1] = child;
}

/* TODO: Might not be needed. */
struct jsn_node *jsn_get_node_direct_child(jsn_handle handle, const char *key) {
    /* TODO: What if this node has no children? */
    /* TODO: Maybe we need to dynamically create this node? */

    /* If a null node is given, just return.*/
    if (handle == NULL) {
        return NULL;
    }

    /* Can optimize this to only check nodes with keys. */
    unsigned int i;
    for (i = 0; i < handle->children_count; i++) {
        if (handle->children[i]->key != NULL) {
            if (strcmp(handle->children[i]->key, key) == 0) {
                return handle->children[i];
            }
        }
    }

    return NULL;
}


/* PARSER:
 * --------------------------------------------------------------------------*/

struct jsn_node *jsn_parse_value(struct jsn_tokenizer *tokenizer,
                                 struct jsn_token token);

struct jsn_node *jsn_parse_string(struct jsn_tokenizer *tokenizer,
                                  struct jsn_token token) {
    struct jsn_node *node = jsn_create_node(JSN_NODE_STRING);
    /* node->value = token.lexeme; */
    node->value.value_string = token.lexeme;
    return node;
}

struct jsn_node *jsn_parse_number(struct jsn_tokenizer *tokenizer,
                                  struct jsn_token token) {
    struct jsn_node *node;

    /* We check if the token lexeme contains a period, then we know that it's a
     * double */
    if (strchr(token.lexeme, '.')) {
        /* Double */
        node = jsn_create_node(JSN_NODE_DOUBLE);
        node->value.value_double = strtod(token.lexeme, NULL);
    } else {
        /* Integer */
        node = jsn_create_node(JSN_NODE_INTEGER);
        node->value.value_int = atoi(token.lexeme);
    }

    return node;
}

struct jsn_node *jsn_parse_array(struct jsn_tokenizer *tokenizer,
                                 struct jsn_token token) {
    /* Create our array node type. */
    struct jsn_node *node = jsn_create_node(JSN_NODE_ARRAY);

    /* While we are not at the end of the array, handle array tokens. */
    while (token.type != JSN_TOC_ARRAY_CLOSE) {
        token = jsn_tokenizer_get_next_token(tokenizer);

        /* Create the new child node (recursive call). */
        struct jsn_node *child_node = jsn_parse_value(tokenizer, token);

        if (child_node != NULL) {
            jsn_append_node_child(node, child_node);
        }
    }

    return node;
}

struct jsn_node *jsn_parse_object(struct jsn_tokenizer *tokenizer,
                                  struct jsn_token token) {
    /* Create our object node type.. */
    struct jsn_node *node = jsn_create_node(JSN_NODE_OBJECT);

    /* While we haven't reached the end of the object. */
    while (token.type != JSN_TOC_OBJECT_CLOSE) {
        /* Get the key. */
        struct jsn_token token_key = jsn_tokenizer_get_next_token(tokenizer);
        if (token_key.type != JSN_TOC_STRING) {
            /** \todo implement proper error handling. */
            perror("Undefined token as Object node key, maybe you "
                   "left an "
                   "extra comma after an object value.\n");
            exit(-1);
        }

        /* Get the colon. */
        struct jsn_token token_cln = jsn_tokenizer_get_next_token(tokenizer);
        if (token_cln.type != JSN_TOC_COLON) {
            /** \todo implement proper error handling. */
            perror("Undefined token after key, did you forget a colon "
                   "somewhere?.\n");
            exit(-1);
        }

        /* Get the value and create the new child node (recursive call). */
        struct jsn_token token_val = jsn_tokenizer_get_next_token(tokenizer);
        struct jsn_node *child_node = jsn_parse_value(tokenizer, token_val);
        child_node->key = token_key.lexeme;

        /* Append the child node. */
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
        /** \todo implement proper error handling. */
        perror("Undefined token found.\n");
        exit(-1);
        break;
    }
}

/* Debug:
 * --------------------------------------------------------------------------*/

void jsn_node_print_intented(struct jsn_node *node, unsigned int indent) {
    /* Generate padding string, if needed. */
    char indent_str[indent + 1];
    unsigned int i;
    for (i = 0; i < indent; i++) {
        indent_str[i] = '.';
    }
    indent_str[indent] = '\0';

    /* Print out the current node. */
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
        printf("%sValue: %i\n", indent_str, node->value.value_int);
        break;
    case JSN_NODE_DOUBLE:
        printf("%sType: %s\n", indent_str, "DOUBLE");
        printf("%sValue: %f\n", indent_str, node->value.value_double);
        break;
    case JSN_NODE_BOOLEAN:
        printf("%sType: %s\n", indent_str, "BOOLEAN");
        printf("%sValue: %i\n", indent_str, node->value.value_bool);
        break;
    case JSN_NODE_ARRAY:
        printf("%sType: %s\n", indent_str, "ARRAY");
        break;
    case JSN_NODE_OBJECT:
        printf("%sType: %s\n", indent_str, "OBJECT");
        break;
    }
    printf("%sChildren_Count: %u\n", indent_str, node->children_count);

    /* Print children too. */
    {
        unsigned int i;
        for (i = 0; i < node->children_count; i++) {
            /* Print out node children (recursive call). */
            jsn_node_print_intented(node->children[i], indent + 4);
        }
    }
    printf("%s]\n", indent_str);
}

/* API:
 * --------------------------------------------------------------------------*/

void jsn_node_print(jsn_handle handle) {
    jsn_node_print_intented(handle, 0);
}

jsn_handle jsn_form_string(const char *src) {
    /* Initialize our tokenizer, for this specific source string. */
    struct jsn_tokenizer tokenizer = jsn_tokenizer_init(src);

    /* Prime the tokenizer. */
    struct jsn_token token = jsn_tokenizer_get_next_token(&tokenizer);

    /* Start parsing, recursively. */
    return jsn_parse_value(&tokenizer, token);
}

jsn_handle jsn_node_create_as_object() {
    struct jsn_node *node = jsn_create_node(JSN_NODE_OBJECT);
    return node;
}

jsn_handle jsn_node_create_as_array() {
    struct jsn_node *node = jsn_create_node(JSN_NODE_ARRAY);
    return node;
}

jsn_handle jsn_node_create_as_int(int value) {
    struct jsn_node *node = jsn_create_node(JSN_NODE_INTEGER);
    node->value.value_int = value;
    return node;
}

jsn_handle jsn_node_create_as_double(double value) {
    struct jsn_node *node = jsn_create_node(JSN_NODE_DOUBLE);
    node->value.value_double = value;
    return node;
}

jsn_handle jsn_node_create_as_boolean(bool value) {
    struct jsn_node *node = jsn_create_node(JSN_NODE_BOOLEAN);
    node->value.value_bool = value;
    return node;
}

jsn_handle jsn_node_create_as_string(const char *value) {
    struct jsn_node *node = jsn_create_node(JSN_NODE_STRING);
    node->value.value_string = strcpy(malloc(strlen(value) * CHAR_BIT), value);
    return node;
}

void jsn_node_object_append(jsn_handle handle, const char *key, jsn_handle node) {
    /* Make sure were dealing with an object handle type here. */
    if (handle->type != JSN_NODE_OBJECT) {
        jsn_assert("The handle passed to object append isn't an object.");
    }

    /* Already has a key so we need to free it. */
    if (node->key != NULL) {
        free(node->key);
        node->key = NULL;
    }

    /* Malloc and set the node's new key. */
    node->key = strcpy(malloc(strlen(key) * CHAR_BIT), key);

    /* Append the node to the provided object */
    jsn_append_node_child(handle, node);
}

void jsn_node_array_append(jsn_handle handle, jsn_handle node) {
    /* Make sure were dealing with an array handle type here. */
    if (handle->type != JSN_NODE_ARRAY) {
        jsn_assert("The handle passed to object append isn't an array.");
    }

    /* TODO: Put in type checking gaurds here. */

    /* If the node has a key, remove it. */
    if (node->key != NULL) {
        free(node->key);
        node->key = NULL;
    }

    /* Append the node to the provided object */
    jsn_append_node_child(handle, node);
}

/* TESTING:
 * --------------------------------------------------------------------------*/

int main(void) {

    /* Building a JSON tree. */
    jsn_handle root = jsn_node_create_as_object();

    jsn_node_object_append(root, "Jackie", jsn_node_create_as_int(39));
    jsn_node_object_append(root, "Vernon", jsn_node_create_as_int(32));
    jsn_node_object_append(root, "Lucy", jsn_node_create_as_int(80));

    jsn_node_object_append(root, "JackieD", jsn_node_create_as_double(39));
    jsn_node_object_append(root, "VernonD", jsn_node_create_as_double(32));
    jsn_node_object_append(root, "LucyD", jsn_node_create_as_double(80));

    jsn_node_object_append(root, "JackieB", jsn_node_create_as_boolean(true));
    jsn_node_object_append(root, "VernonB", jsn_node_create_as_boolean(false));
    jsn_node_object_append(root, "LucyB", jsn_node_create_as_boolean(false));

    jsn_node_object_append(root, "JackieS", jsn_node_create_as_string("Hello, Jackie!"));
    jsn_node_object_append(root, "VernonS", jsn_node_create_as_string("Hello, Vernon!"));
    jsn_node_object_append(root, "LucyS", jsn_node_create_as_string("Hello, Lucy!"));

    jsn_handle people = jsn_node_create_as_array();
    jsn_node_array_append(people, jsn_node_create_as_int(39));
    jsn_node_array_append(people, jsn_node_create_as_double(39));
    jsn_node_array_append(people, jsn_node_create_as_boolean(false));
    jsn_node_array_append(people, jsn_node_create_as_string("Hello String!"));
    jsn_node_object_append(root, "MyArray", people);

    jsn_node_print(root);

    /* jsn_handle handle_ages = jsn_form_string("{ \ */
    /* \"jackie\" : 39, \ */
    /* \"vernon\" : 32, \ */
    /* \"jhon\" : 32.5, \ */
    /* \"lucy\" : 80 \ */
    /* }"); */

    /* Setting integer values */
    /* jsn_print(handle_ages); */
    /* jsn_set_value_int(handle_ages, 139, "vernon"); */
    /* jsn_set_value_int(handle_ages, 139, "lucy"); */
    /* jsn_set_value_int(handle_ages, 139, "jackie"); */

    /* jsn_print(handle_ages); */
    /* jsn_set_value_int(handle_ages, 45, "Unknown Key Here"); */
    /* jsn_set_value_int(handle_ages, 45, "vernon-" "other key"); */
    /* jsn_set_value_int(handle_ages, 45, "vernon-" "other key"); */
    /* jsn_print(handle_ages); */

    /* Getting integer values */
    /* printf("The key (%s) has the value: %i \n", "vernon", jsn_get_value_int(handle_ages, "vernon")); */
    /* printf("The key (%s) has the value: %i \n", "jhon", jsn_get_value_int(handle_ages, "jhon")); */

    /* Setting double values */
    /* jsn_set_value_double(handle_ages, 30000.0, "jhon"); */
    /* jsn_print(handle_ages); */
    /* jsn_print(handle_ages); */

    /* This is our sample object node. */
    /* jsn_handle handle = jsn_form_string ("{ \ */
    /* \"mykey\" : 123, \ */
    /* \"my other key\" : \"this is a UTF8 string! cónstàñt 家長專區.\", \ */
    /* \"my othere key\" : [1,2,3,4,5,6], \ */
    /* \"my otheree key\" : {\"this is an inner obj\" : 3000} \ */
    /* }"); */
    /* jsn_print (handle); */
    /* This is our sample array node. */
    /* jsn_handle handle_array = jsn_form_string ("[1,[1,2]]"); */
    /* jsn_set_value (handle, handle_array, "my-key"); */

    /* Node = key -> value */
    /* jsn_node_print (handle, 0); */
    /* jsn_set_value (handle, 10, "my-key"); */
    /* jsn_set_value (handle, 10.1, "my-key"); */
    /* jsn_set_value (handle, JSN_TRUE, "my-key"); */
    /* jsn_set_value (handle, JSN_FALSE, "my-key"); */

    /* success */
    return 0;
}
