#include "json.h"
#include <ctype.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Backus–Naur form: */
/* Concept of the look ahead? */

/* TOKENIZER:
 * --------------------------------------------------------------------------*/
/* The tokenizer needs to return tokens of different types on demand. */

enum jsn_token_types
{
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

struct jsn_token
{
  unsigned int type;
  char *lexeme;
  unsigned long lexeme_size;
};

struct jsn_tokenizer
{
  char *src;
  unsigned int cursor;
};

struct jsn_tokenizer
jsn_tokenizer_init (const char *src)
{
  /* Determine the length of the provided source string. */
  unsigned int src_length = strlen (src) + 1;

  /* Set aside memory and make a copy. */
  char *src_cpy = strcpy (malloc (src_length * CHAR_BIT), src);

  /* Construct tokenizer. */
  struct jsn_tokenizer tokenizer;
  tokenizer.src = src_cpy;
  tokenizer.cursor = 0;

  return tokenizer;
};

void
jsn_token_lexeme_append (struct jsn_token *token, char c)
{
  /* Will chunk 50 * CHAR_BIT. */
  static const unsigned int chunk_size = 50;

  /* Handle dynamic memory allocation, in chunks. */
  if (token->lexeme == NULL)
    {
      char *lexeme_ext = malloc (chunk_size * CHAR_BIT);
      token->lexeme_size = chunk_size;
      token->lexeme = lexeme_ext;
    }
  else if (token->lexeme_size < (strlen (token->lexeme) + 1))
    {
      /* Keep pointer to old lexeme. */
      char *old_lexeme = token->lexeme;

      /* Allocate additional memory chunk. */
      unsigned long int lexeme_new_size = (token->lexeme_size + chunk_size);
      char *lexeme_ext = malloc (lexeme_new_size * CHAR_BIT);
      token->lexeme = memcpy (lexeme_ext, token->lexeme, token->lexeme_size * CHAR_BIT);

      /* Set the new size. */
      token->lexeme_size = lexeme_new_size;

      /* Free old memory. */
      free (old_lexeme);

      /**
       * \todo Remove print statement.
       */
      printf ("Allocated more memory! \n");
    }

  /* Get's the lexemes length. */
  unsigned long int lexeme_len = strlen (token->lexeme) + 1;

  /* Append the additional char. */
  token->lexeme[lexeme_len] = '\0';
  token->lexeme[lexeme_len - 1] = c;
}

struct jsn_token
jsn_tokenizer_get_next_token (struct jsn_tokenizer *tokenizer)
{
  /* Is the cursor on a number. */
  struct jsn_token token;
  token.type = JSN_TOC_NULL;
  token.lexeme = NULL;

  /** \todo Implementing regex patterns instead. */

  /* Get the number token. */
  if (isdigit (tokenizer->src[tokenizer->cursor]))
    {
      token.type = JSN_TOC_NUMBER;
      while (isdigit (tokenizer->src[tokenizer->cursor]))
        {
          /* Perform operation. */
          jsn_token_lexeme_append (&token, tokenizer->src[tokenizer->cursor]);
          /* Move the cursor up. */
          tokenizer->cursor++;
        }
      return token;
    }

  /* Handle strings. */
  if (tokenizer->src[tokenizer->cursor] == '"')
    {
      token.type = JSN_TOC_STRING;
      tokenizer->cursor++;

      while (isalpha (tokenizer->src[tokenizer->cursor]))
        {
          /* Perform operation. */
          jsn_token_lexeme_append (&token, tokenizer->src[tokenizer->cursor]);
          /* Move the cursor up. */
          tokenizer->cursor++;
        }

      return token;
    }

  /* Handle array opening. */
  if (tokenizer->src[tokenizer->cursor] == '[')
    {
      token.type = JSN_TOC_ARRAY_OPEN;
      jsn_token_lexeme_append (&token, tokenizer->src[tokenizer->cursor]);
      tokenizer->cursor++;
      return token;
    }

  /* Handle array closing. */
  if (tokenizer->src[tokenizer->cursor] == ']')
    {
      token.type = JSN_TOC_ARRAY_CLOSE;
      jsn_token_lexeme_append (&token, tokenizer->src[tokenizer->cursor]);
      tokenizer->cursor++;
      return token;
    }

  /* Handle comma separator. */
  if (tokenizer->src[tokenizer->cursor] == ',')
    {
      token.type = JSN_TOC_COMMA;
      jsn_token_lexeme_append (&token, tokenizer->src[tokenizer->cursor]);
      tokenizer->cursor++;
      return token;
    }

    /* Handle object opening. */
  if (tokenizer->src[tokenizer->cursor] == '{')
    {
      token.type = JSN_TOC_OBJECT_OPEN;
      jsn_token_lexeme_append (&token, tokenizer->src[tokenizer->cursor]);
      tokenizer->cursor++;
      return token;
    }

  /* Handle object closing. */
  if (tokenizer->src[tokenizer->cursor] == '}')
    {
      token.type = JSN_TOC_OBJECT_CLOSE;
      jsn_token_lexeme_append (&token, tokenizer->src[tokenizer->cursor]);
      tokenizer->cursor++;
      return token;
    }

  /* Handle colon.*/
  if (tokenizer->src[tokenizer->cursor] == ':')
    {
      token.type = JSN_TOC_COLON;
      jsn_token_lexeme_append (&token, tokenizer->src[tokenizer->cursor]);
      tokenizer->cursor++;
      return token;
    }

  /* Handle white spaces. */
  if (isspace (tokenizer->src[tokenizer->cursor]))
    {
      tokenizer->cursor++;
      return jsn_tokenizer_get_next_token (tokenizer);
    }

  return token;
}

/* DATA STRUCTURE:
 * --------------------------------------------------------------------------*/

/* Nodes */
enum jsn_node_type
{
  /** \todo Handle exponents (1e-005). */
  JSN_NODE_NUMBER,
  JSN_NODE_STRING,
  JSN_NODE_ARRAY,
  JSN_NODE_OBJECT,
};

struct jsn_node
{
  enum jsn_node_type type;
  char *value;
  char *key;
  unsigned int children_count;
  struct jsn_node **children;
};

void jsn_node_print (struct jsn_node *node, unsigned int indent);

struct jsn_node *
jsn_node_create (enum jsn_node_type type)
{
  /* Let's allocate some memory on the heap. */
  struct jsn_node *node = malloc (sizeof (struct jsn_node));

  node->type = type;
  node->value = NULL;
  node->children_count = 0;
  node->children = NULL;
  node->key = NULL;

  return node;
}

void
jsn_node_free (struct jsn_node *node)
{
  /** \todo implement a free here. */
}

void
jsn_node_copy (struct jsn_node *dest, struct jsn_node *src)
{
  /** \todo implement a deep copy here. */
}

void
jsn_node_append_child (struct jsn_node *parent, struct jsn_node *child)
{
  /** \todo Insure a deep copy is being made here of. */

  /* Increment children count. */
  parent->children_count++;

  /* Reallocate memory. */
  parent->children = realloc (parent->children, sizeof (struct jsn_node *)
                                                    * parent->children_count);

  /* Set the new node. */
  parent->children[parent->children_count - 1] = child;
}

void
jsn_node_print (struct jsn_node *node, unsigned int indent)
{
  /* Generate padding string, if needed. */
  char indent_str[indent + 1];
  unsigned int i;
  for (i = 0; i < indent; i++)
    {
      indent_str[i] = '.';
    }
  indent_str[indent] = '\0';

  /* Print out the current node. */
  printf ("%s[\n", indent_str);
  printf ("%sNode: \n", indent_str);
  printf ("%sKey: %s\n", indent_str, node->key);
  switch (node->type)
    {
    case JSN_NODE_ARRAY:
      printf ("%sType: %s\n", indent_str, "ARRAY");
      break;
    case JSN_NODE_STRING:
      printf ("%sType: %s\n", indent_str, "STRING");
      break;
    case JSN_NODE_OBJECT:
      printf ("%sType: %s\n", indent_str, "OBJECT");
      break;
    case JSN_NODE_NUMBER:
      printf ("%sType: %s\n", indent_str, "NUMBER");
      break;
    }
  printf ("%sChildren_Count: %u\n", indent_str, node->children_count);
  printf ("%sValue: %s\n", indent_str, node->value);

  /* Print children too. */
  {
    unsigned int i;
    for (i = 0; i < node->children_count; i++)
      {
        /* Only implement this after the child appender has been added. */
        jsn_node_print (node->children[i], indent + 4);
      }
  }
  printf ("%s]\n", indent_str);
}

/* PARSER:
 * --------------------------------------------------------------------------*/

/* Parser needs to make use of the tokenizer by asking for tokens to
   extract. The Parser needs to return a tree structure that holds all of the
   extracted tokens.

   The Parser makes use of the concept of a look ahead. The recursive decent
   parser is predictive a predictive parser. I means it can predict specific
   production based on looking ahead to the current token.

   We should start off by priming the parser with the look ahead token. Based
   on the first token we will be able to route the parsing process accordingly.
   Such parser corresponds to the LL1 automatic parser.
*/

struct jsn_node *jsn_parse_value (struct jsn_tokenizer *tokenizer,
                                  struct jsn_token token);

struct jsn_node *
jsn_parse_string (struct jsn_tokenizer *tokenizer, struct jsn_token token)
{
  /* The issue is that the token string pointer */
  struct jsn_node *node = jsn_node_create (JSN_NODE_STRING);
  node->value = token.lexeme;
  return node;
}

struct jsn_node *
jsn_parse_number (struct jsn_tokenizer *tokenizer, struct jsn_token token)
{
  /* The issue is that the token string pointer */
  struct jsn_node *node = jsn_node_create (JSN_NODE_NUMBER);
  node->value = token.lexeme;
  return node;
}

struct jsn_node *
jsn_parse_array (struct jsn_tokenizer *tokenizer, struct jsn_token token)
{
  /* Attempting recursive method. */
  struct jsn_node *node = jsn_node_create (JSN_NODE_ARRAY);

  /* While we are not at the end of the array, handle array tokens. */
  while (token.type != JSN_TOC_ARRAY_CLOSE)
    {
      token = jsn_tokenizer_get_next_token (tokenizer);

      /* Create the new child node (recursive). */
      struct jsn_node *child_node = jsn_parse_value (tokenizer, token);

      if (child_node != NULL)
        {
          jsn_node_append_child (node, child_node);
        }
    }

  return node;
}

struct jsn_node *
jsn_parse_object (struct jsn_tokenizer *tokenizer, struct jsn_token token)
{
  /* Attempting recursive method. */
  struct jsn_node *node = jsn_node_create (JSN_NODE_OBJECT);

  /** \todo Think about how this should be implemented.  */

  return node;
}

struct jsn_node *
jsn_parse_value (struct jsn_tokenizer *tokenizer, struct jsn_token token)
{
  switch (token.type)
    {
    case JSN_TOC_NUMBER:
      return jsn_parse_number (tokenizer, token);
      break;
    case JSN_TOC_STRING:
      return jsn_parse_string (tokenizer, token);
      break;
    case JSN_TOC_ARRAY_OPEN:
      return jsn_parse_array (tokenizer, token);
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
      perror ("Undefined token found.\n");
      exit (-1);
      break;
    }
}

struct jsn_node *
jsn_parse (const char *src)
{
  /* Initialize our tokenizer, for this specific source string. */
  struct jsn_tokenizer tokenizer = jsn_tokenizer_init (src);

  /* prime the tokenizer. */
  struct jsn_token token = jsn_tokenizer_get_next_token (&tokenizer);

  /* We should handle some parsing, recursively. */
  return jsn_parse_value (&tokenizer, token);
}

/* TESTING:
 * --------------------------------------------------------------------------*/

int
main (void)
{
  /* This is our sample array node. */
  struct jsn_node *node = jsn_parse ("[1,[1,2]]");
  jsn_node_print (node, 0);

  /* This is our sample object node. */
  struct jsn_node *node_obj = jsn_parse ("{ \"mykey\" : 1 }");
  jsn_node_print (node_obj, 0);

  /* success */
  return 0;
}
