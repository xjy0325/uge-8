/* Title: FrankenText
 * Author: Gökçe Aydos(85%) and Xingjia Yu(15%)
 * Description: A program that generates random sentences
based on the book Frankenstein by Mary Shelley.
 */
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// define the max length of all words
#define MAX_WORD_COUNT 15'000

// define the max length of all successor tokens
#define MAX_SUCCESSOR_COUNT MAX_WORD_COUNT / 2

char book[] = {
#embed "pg84.txt" /// Stores the content of the file as an array of chars.
    , '\0'};      /// Makes `book` a string.

/// Array of tokens registered so far.
/// No duplicates are allowed.
char *tokens[MAX_WORD_COUNT];
/// `tokens`'s current size
size_t tokens_size = 0;

/// Array of successor tokens registered so far.
/// One token can have many successor tokens. `succs[x]` corresponds to
/// `token[x]`'s successors.
/// We store directly tokens instead of token_ids, because we will directly
/// print them. If we wanted to delete the book, then it would make more sense
/// to store `token_id`s
char *succs[MAX_WORD_COUNT][MAX_SUCCESSOR_COUNT];
/// `succs`'s current size
size_t succs_sizes[MAX_WORD_COUNT];

/// Overwrites non-printable characters in `book` with a space.
/// Non-printable characters may lead to duplicates like
/// `"\xefthe" and "the"` even both print `the`.
void replace_non_printable_chars_with_space() {
  // loop over the whole book
  for (size_t i = 0; i < sizeof(book); i++) {

    // look up the non-printable char with isprint function
    // and overwrite them with space
    if (isprint(book[i]) == 0) {
      book[i] = ' ';
    }
  }
}

/// Returns the id (index) of the token, creating it if necessary.
///
/// Returns token id if token exists in \c tokens, otherwise creates a new entry
/// in \c tokens and returns its token id.
///
/// \param token token to look up (or insert)
/// \return Index of `token` in \c tokens array
size_t token_id(char *token) {
  // initial id every time
  size_t id;

  // loop over all previous tokens
  for (id = 0; id < tokens_size; ++id) {

    // use function strcmp to look up if there exists a same token already
    if (strcmp(tokens[id], token) == 0) {
      // if it exists, print the token's id (as look up function)
      return id;
    }
  }

  // create a new token and corresponding new id
  tokens[id] = token;
  ++tokens_size;

  return id;
}

/// Appends the token \c succ to the successors list of \c token.
// Xingjia: Does not check for duplicate successors
void append_to_succs(char *token, char *succ) {
  // Make a pointer that points to
  // the counter of how many successors this token
  auto next_empty_index_ptr = &succs_sizes[token_id(token)];

  // control array overflow
  if (*next_empty_index_ptr >= MAX_SUCCESSOR_COUNT) {
    printf("Successor array full.");
    exit(EXIT_FAILURE);
  }

  // ++ the counter of successors and add a new successor token to the token'
  // (*next_empty_index_ptr)++ implicate increment to succs_sizes
  succs[token_id(token)][(*next_empty_index_ptr)++] = succ;
}

/// Creates tokens on \c book and fills \c tokens and \c succs using
/// the functions \c token_id and \c append_to_succs.
void tokenize_and_fill_succs(char *delimiters, char *str) {
  // create two variables to distinguish between
  // the previous token and successor token
  char *LastToken = 0;
  // generate the very first token
  char *NewToken = strtok(str, delimiters);

  // check if it get to the end of the book
  while (NewToken) {

    // tokenize and get id or get an old id
    token_id(NewToken);

    // fill successor tokens array
    // exept the first time loop where no previous token
    if (LastToken != 0) {
      append_to_succs(LastToken, NewToken);
    }

    // move to next tokens
    LastToken = NewToken;
    NewToken = strtok(NULL, delimiters);
  }
}

/// Return last character of a string
char last_char(char *str) {
  // check it is a NULL string og empty string
  if (str == NULL || *str == '\0') {
    return '\0'; // return 0 not NULL
  }

  // scanning the string and return the last char
  while (1) {
    if (*(++str) == '\0') {
      return *(str - 1);
    }
  }
}

/// Returns whether the token ends with `!`, `?` or `.`.
bool token_ends_a_sentence(char *token) {
  if (last_char(token) == '!' || last_char(token) == '?' ||
      last_char(token) == '.') {
    return 1;
  }
  return 0;
}

/// Returns a random `token_id` that corresponds to a `token` that starts with
/// a capital letter. Uses \c tokens and \c tokens_size.
size_t random_token_id_that_starts_a_sentence() {
  // a temp id value
  int id = 0;

  // pick randomly id until its token starting with capital letter
  do {
    id = rand() % tokens_size;
  } while (!isupper(*tokens[id]));

  return id;
}

/// Generates a random sentence using \c tokens, \c succs, and \c succs_sizes.
/// The sentence array will be filled up to \c sentence_size-1 characters
/// using random tokens until:
/// - a token is found where \c token_ends_a_sentence
/// - or more tokens cannot be concatenated to the \c sentence anymore.
/// Returns the filled sentence array.
///
/// @param sentence array what will be used for the sentence.
//
//                  Will be overwritten. Does not have to be initialized.
/// @param sentence_size
/// @return input sentence pointer
char *generate_sentence(char *sentence, size_t sentence_size) {
  // generate the first token which starts with a capital letter
  size_t current_token_id = random_token_id_that_starts_a_sentence();
  auto token = tokens[current_token_id];

  // initialize our sentence
  sentence[0] = '\0';

  // add the first word to the sentence
  strcat(sentence, token);

  // stop when this token is '!' '?' or '.'
  if (token_ends_a_sentence(token))
    return sentence;

  // variable to store sentence length for the next iteration.
  // Used to stop the loop if the length exceeds sentence size
  size_t sentence_len_next;

  // Concatenates random successors to the sentence as long as
  // `sentence` can hold them.
  do {

    // variable stand for the counter of successors of their token
    int SuccLen = succs_sizes[current_token_id];

    // in case there is no successor
    if (SuccLen == 0)
      return sentence;

    // randomly pick a successor and put it behind the sentence
    // and the successor becomes the new token
    token = succs[current_token_id][rand() % SuccLen];

    // add a space behind and the new token to the end of the sentence
    strcat(sentence, " ");
    strcat(sentence, token);

    // check if the token can end a sentence
    if (token_ends_a_sentence(token))
      return sentence;

    // get the new id of the token
    current_token_id = token_id(token);

    // calculate the available length to the sentence
    sentence_len_next = strlen(sentence);
  } while (sentence_len_next < sentence_size - 1);
  return sentence;
}

int main() {
  // clean the txt
  replace_non_printable_chars_with_space();

  // tokenize the book and find all successors
  char *delimiters = " \n\r";
  tokenize_and_fill_succs(delimiters, book);

  // array to hold the sentence
  char sentence[1000];

  srand(time(nullptr)); // Be random each time we run the program

  // Generate and print sentences until we find a question sentence.
  do {
    generate_sentence(sentence, sizeof sentence);
  } while (last_char(sentence) != '?');
  puts(sentence);
  puts("");

  // Initialize `sentence` and then generate sentences until we find a
  // sentence ending with an exclamation mark.
  do {
    generate_sentence(sentence, sizeof sentence);
  } while (last_char(sentence) != '!');
  puts(sentence);
}