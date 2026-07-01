#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define ASSERT(expr)                                                                               \
  {                                                                                                \
    if (!(expr)) {                                                                                 \
      fprintf(stderr, "Assertion failed: %s\n", #expr);                                            \
      fprintf(stderr, "File: %s, Line: %d\n", __FILE__, __LINE__);                                 \
      exit(1);                                                                                     \
    }                                                                                              \
  }

#define TEST(expr)                                                                                 \
  {                                                                                                \
    if (!(expr)) {                                                                                 \
      fprintf(stderr, "Test failed: %s\n", #expr);                                                 \
      exit(1);                                                                                     \
    }                                                                                              \
  }

typedef struct node {
  uint64_t data;
  struct node *next;
} node_t;

typedef struct info {
  uint64_t sum;
} info_t;

node_t *head = NULL;
info_t info = {0};

void insert_sorted(uint64_t data) {
  node_t *new_node = malloc(sizeof(node_t));
  new_node->data = data;
  new_node->next = NULL;

  if (head == NULL) {
    head = new_node;
  } else if (data < head->data) {
    new_node->next = head;
    head = new_node;
  } else {
    node_t *curr = head;
    node_t *prev = NULL;

    while (curr != NULL) {
      if (data < curr->data) {
        break;
      }

      prev = curr;
      curr = curr->next;
    }

    prev->next = new_node;
    if (curr != NULL) {
      // original bug:
      // new_node->next = curr->next;
      // this bug still passes 2 tests below, even when the linked list misses a node
      new_node->next = curr;
    }
  }
  info.sum += data;
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

int sum_linked_list(void) {
  node_t *curr = head;
  int sum = 0;
  while (curr != NULL) {
    sum += curr->data;
    curr = curr->next;
  }
  return sum;
}

int main() {
  insert_sorted(1);
  ASSERT(info.sum == sum_linked_list());
  insert_sorted(3);
  ASSERT(info.sum == sum_linked_list());
  insert_sorted(5);
  ASSERT(info.sum == sum_linked_list());
  insert_sorted(2);
  ASSERT(info.sum == sum_linked_list());
  // printf("Hello");
  TEST(info.sum == 1 + 3 + 5 + 2);
  ASSERT(index_of(1) == 0);
  ASSERT(index_of(2) == 1);
  ASSERT(index_of(3) == 2);
  ASSERT(index_of(5) == 3);
  ASSERT(sum_linked_list() == 1 + 3 + 5 + 2);
  TEST(index_of(2) == 1);
  // Extra tests to show that we have all nodes in the linked list
  TEST(index_of(3) == 2);
  TEST(index_of(5) == 3);

  return 0;
}
