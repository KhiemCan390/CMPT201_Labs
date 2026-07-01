#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

//\ is required at the end of each line of the macro's def
// # is the stringification operator, which turns the macro's argument to a string literal
// also don't comment in the macro's def
#define ASSERT(expr)                                                                               \
  {                                                                                                \
    if (!(expr)) {                                                                                 \
      fprintf(stderr, "Assertion failed: %s\n", #expr);                                            \
      exit(1);                                                                                     \
    }                                                                                              \
  }

#define TEST(expr)                                                                                 \
  {                                                                                                \
    if (!(expr)) {                                                                                 \
      fprintf(stderr, "Test failed: %s\n", #expr);                                                 \
      exit(1);                                                                                     \
    } else {                                                                                       \
      printf("Test passed: %s\n", #expr);                                                          \
    }                                                                                              \
  }

typedef struct node {
  uint64_t data;
  struct node *next;
} node_t;

node_t *head = NULL;

void insert_sorted(uint64_t data) {
  node_t *new_node = malloc(sizeof(node_t));
  new_node->data = data;
  new_node->next = NULL;

  if (head == NULL) {
    head = new_node;
  } else {
    node_t *curr = head;
    node_t *prev = NULL;

    bool inserted = false;
    while (curr != NULL && !inserted) {
      if (data < curr->data) {
        // Bug fix: if data = 0 i.e. smaller than the head
        // the "prev->next" causes sigfault
        if (curr == head) {
          new_node->next = head;
          head = new_node;
          inserted = true;
        } else {
          prev->next = new_node;
          new_node->next = curr;
          inserted = true;
        }
      }
      prev = curr;
      curr = curr->next;
    }
    // Bug fix: the loop above won't add a new node if the new node's data is
    // larger than any existing nodes' data
    if (!inserted) {
      prev->next = new_node;
    }
  }
}

int index_of(uint64_t data) {
  node_t *curr = head;
  int index = 0;

  while (curr != NULL) {
    if (curr->data == data) {
      return index;
    }

    curr = curr->next;
    index++;
  }

  return -1;
}

int main() {
  insert_sorted(1);
  insert_sorted(2);
  insert_sorted(5);
  insert_sorted(3);

  TEST(index_of(3) == 2);

  insert_sorted(0);
  insert_sorted(4);

  TEST(index_of(4) == 4);

  return 0;
}
