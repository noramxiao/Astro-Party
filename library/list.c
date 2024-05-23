#include "list.h"
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

const size_t GROWTH_FACTOR = 2;

typedef struct list {
  size_t size;
  size_t capacity;
  void **data;
  free_func_t freer;
} list_t;

list_t *list_init(size_t initial_size, free_func_t freer) {
  list_t *ret = malloc(sizeof(list_t));
  assert(ret != NULL);
  ret->data = malloc(initial_size * sizeof(void *));
  assert(ret->data != NULL);
  ret->size = 0;
  ret->capacity = initial_size;
  ret->freer = freer;
  return ret;
}

void list_free(list_t *list) {
  if (list->freer != NULL) {
    for (size_t i = 0; i < list->size; i++) {
      list->freer(list->data[i]);
    }
  }

  free(list->data);
  free(list);
}

size_t list_size(list_t *list) { return list->size; }

void *list_get(list_t *list, size_t index) {
  assert(index < list->size && index >= 0);
  return list->data[index];
}

void list_add(list_t *list, void *value) {
  if (list->size == list->capacity) {
    void **new_data = malloc(GROWTH_FACTOR * list->capacity * sizeof(void *));
    assert(new_data != NULL);
    for (size_t i = 0; i < list->size; i++) {
      new_data[i] = list->data[i];
    }
    list->capacity = GROWTH_FACTOR * list->capacity;
    free(list->data);
    list->data = new_data;
  }

  assert(list->size < list->capacity);
  assert(value != NULL);
  list->data[list->size] = value;
  list->size++;
}

void *list_remove(list_t *list, size_t index) {
  size_t size = list->size;
  assert(size > 0);
  assert(index >= 0 && index < size);
  void **data = list->data;

  void *ret = data[index];
  for (size_t i = index; i < size - 1; i++) {
    data[i] = data[i + 1];
  }
  list->size--;
  return ret;
}
