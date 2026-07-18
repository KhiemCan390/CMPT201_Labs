// Lab 8 - Starting Code for sorting data ie ehreads using uthash
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <uthash.h>
#define THREAD_COUNT 3
typedef const char *word_t;

typedef struct {
  word_t word;
  size_t count;
  UT_hash_handle hh;
} word_count_entry_t;

word_count_entry_t *create_entry(word_t, size_t);

typedef word_count_entry_t *count_map_t;

// Each thread needs to know

typedef struct {
  count_map_t *map;      // which map to update
  word_t *words;         // where its chunk begins
  size_t num_words;      // the number of words in its chunk
  pthread_mutex_t *lock; // which mutex protect the map
} count_thread_args_t;

count_thread_args_t *pack_args(count_map_t *map, word_t *words, size_t num_words,
                               pthread_mutex_t *lock);

static void add_word_counts_in_chunk(count_map_t *map, word_t *words, size_t num_words,
                                     pthread_mutex_t *lock) {
  // --------- Task 4 --------- \\
  // Make this function thread-safe by using the lock

  for (size_t i = 0; i < num_words; i++) {
    word_count_entry_t *w = NULL;
    // The hash-table entry is word_count_entry_t
    // where word_t word is the key, size_t count is the value, and UT_hash_handle_hh is
    // uthash bookkepping
    // HASH_FIND_STR(*map, words[i],w) means search the hash table *map for an entry whose string
    // key equals words[i], and store the pointer at the result pointer w. Consider the argument
    // count_map_t *map, we are given typedef wrod_count_entry_t *count_map_t thus map is a pointer
    // points to a count_map_t var which points to a word_count_entry_t, thus by deref map i.e. *map
    // gives the pointer points to a word_count_entry_t. In this case, it is the pointer points to
    // the first element of the hash table, whhich we need to pass to HASH_FIND_STR and HASH_ADD_STR

    pthread_mutex_lock(lock);
    HASH_FIND_STR(*map, words[i], w);

    if (w) {
      // if the word is already in the table
      w->count++;
    } else {
      // create a new entry and set its count to 1
      w = create_entry(words[i], 1);
      // add the complete structure pointed by w to the hash table *map, using w->word as its string
      // key (word here is a field name, which tells uthash to use w->word)
      HASH_ADD_STR(*map, word, w);
    }
    pthread_mutex_unlock(lock);
  }
}

static void *counter_thread_func(void *);

static count_map_t count_words_parallel(word_t *words, size_t num_words) {
  // --------- Task 2 --------- \\
  // Implement this function
  // Hints:
  // - Use counter_thread_func for pthread_create(..) and modify as needed.
  // - Store the threads and their arguments so you can manage them later
  // - Initialize and pass the mutex to protect the critical sections (Task 4)

  // My notes:
  //
  // typedef const char *word_t, means word_t is a pointer points to the first char of a null-ter
  //-minated string e.g. word_t word = "hello";
  // Therefore, word_t *words is a pointer points to a word_t, but here it means it points to the
  // first element of an array with element type = word_t. Therefore, *words is the first word_t
  // element, which points to the first char of the first string.
  //
  // num_words is just the number of elements in that array.

  count_map_t map = NULL;      // the uthash table-head pointer
  pthread_mutex_t count_mutex; // declaring the mutex object

  pthread_t threads[THREAD_COUNT];                 // stores the 3 thread identifiers
  count_thread_args_t *threads_args[THREAD_COUNT]; // stores th argument structure for each thread

  size_t chunk_size = num_words / THREAD_COUNT; // 13/3 = 4, and the final threads have 5 words

  // TODO: Perform initialization (on the mutex object)
  // The mutex is to ensure that threads can perform safely, e.g.
  // to prevent adding duplicate entries to the hash table.
  pthread_mutex_init(&count_mutex, NULL);
  // end of TODO

  // Launch threads
  for (size_t i = 0; i < THREAD_COUNT; i++) {
    word_t *thread_arg_words = words + i * chunk_size; // pointer arithmetic,
                                                       // computing the begin of each chunk
                                                       // e.g. i = 1, words + 1 * 4 = &words[4]
    size_t thread_arg_num_words =
        chunk_size + (i == THREAD_COUNT - 1 ? num_words % THREAD_COUNT : 0);
    // if this thread is the last thread, i.e. i == 2, then the number of element is 4 + (13%3) = 5
    // otherwise 4

    // TODO: Prepare the arguments and launch the threads
    count_thread_args_t *args =
        pack_args(&map, thread_arg_words, thread_arg_num_words, &count_mutex);
    threads_args[i] = args; // saving the argument structure so we can free them later
    pthread_create(&threads[i], NULL, counter_thread_func, args);
    // The arguments are : wherte to store the thread id, default thread attributes, function for
    // the thread to run, args are arguments passed to that function.
    // end of TODO
  }

  // TODO: Wait for threads to finish
  // have to join them, otherwise they may jump immediately to return map while threads haven't
  // complete running
  for (size_t i = 0; i < THREAD_COUNT; i++) {
    pthread_join(threads[i], NULL);
  }
  // end of TODO
  //  TODO: Cleanup
  for (int i = 0; i < THREAD_COUNT; i++) {
    free(threads_args[i]); // since pack_args uses malloc
  }
  // This doesn't free the input array, the hash table, or destroy the mutex
  return map;
}

// Takes in an array of words of size num_words and
// returns a hash table where the key is the word
// and the value is the number of occurrences
static word_count_entry_t *count_words_seq(word_t *words, size_t num_words) {
  word_count_entry_t *map = NULL;

  // Pass all the words as a single chunk
  add_word_counts_in_chunk(&map, words, num_words, NULL);

  return map;
}

int sort_func(word_count_entry_t *a, word_count_entry_t *b);

void print_counts(count_map_t);
void delete_table(count_map_t);

int main(void) {
  word_t words_in[13] = {"the",  "quick", "brown", "fox", "jumps", "over", "the",
                         "lazy", "dog",   "the",   "the", "fox",   "brown"};
  const size_t words_in_len = 13;
  count_map_t word_map = NULL;

  // Task 2: Replace this function call with the parallelized version.
  // word_map = count_words_seq(words_in, words_in_len);
  word_map = count_words_parallel(words_in, words_in_len);

  // Print table
  if (word_map) {
    // --------- Task 1 --------- \\
    // Sort the table by the sort function in uthash using `sort_func`.
    // TODO
    HASH_SORT(word_map, sort_func);
    // end of TODO
    print_counts(word_map);
  }

  // Cleanup
  if (word_map) {
    delete_table(word_map);
  }

  return 0;
}

word_count_entry_t *create_entry(word_t word, size_t count) {
  word_count_entry_t *ptr = malloc(sizeof(word_count_entry_t));
  ptr->word = word;
  ptr->count = count;
  return ptr;
}

int sort_func(word_count_entry_t *a, word_count_entry_t *b) { return strcmp(a->word, b->word); }

void print_counts(count_map_t word_map) {
  printf("%-32s%-10s\n", "Word", "Count");
  word_count_entry_t *current, *tmp;
  HASH_ITER(hh, word_map, current, tmp) { printf("%-32s%-10zu\n", current->word, current->count); }
}

void delete_table(count_map_t word_map) {
  word_count_entry_t *current, *tmp;
  HASH_ITER(hh, word_map, current, tmp) {
    HASH_DEL(word_map, current);
    free(current);
  }
}

count_thread_args_t *pack_args(count_map_t *map, word_t *words, size_t num_words,
                               pthread_mutex_t *lock) {
  // we pass &map because map is a pointer to the first element, and we need to change it when the
  // first entry is added.
  count_thread_args_t *args = malloc(sizeof(count_thread_args_t));
  args->map = map;
  args->words = words;
  args->num_words = num_words;
  args->lock = lock;
  return args;
}

static void *counter_thread_func(void *param) {
  // Call count_words_in_chunk with the appropriate arguments
  count_thread_args_t *args = (count_thread_args_t *)param; // have to cast back
  add_word_counts_in_chunk(args->map, args->words, args->num_words, args->lock);

  return NULL;
}
