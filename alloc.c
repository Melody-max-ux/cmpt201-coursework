#define _DEFAULT_SOURCE
#include "alloc.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int find_free_chunk(void **, uint64_t);
struct header *find_top_free_chunk(void);
void find_neighbor(struct header *target, struct header **left, struct header **right,
                   struct header **l_prev, struct header **r_prev);

static struct header *list_head;
static void *inital_brk;
static void *curr_brk;
static int limit;
static enum algs curr_alg;
static int brk_inital = 0;

void *alloc(int size) {
  uint64_t chunk_size = (uint64_t)(size + sizeof(struct header));
  int is_first = 1;
  if (is_first) {
    if (!brk_inital) {
      inital_brk = sbrk(0);
      brk_inital = 1;
    }
    sbrk(INCREMENT);
    curr_brk = (void *)((char *)inital_brk + INCREMENT);
    is_first = 0;
    list_head = inital_brk;
    list_head->size = INCREMENT;
    list_head->next = NULL;
  }
  void *result = NULL;
  // put request size of whole chunk: size + sizeof header
  if (find_free_chunk(&result, chunk_size) && result != NULL) {
    return (void *)((char *)result + sizeof(struct header));
  } else {
    /*increse brk, look for lst free chunk and combine
     *set new space in this free chunk
     * */
    struct header *top_free_chunk = NULL;
    int incre_count = 0;
    uint64_t top_free_chunk_size = 0;
    top_free_chunk = find_top_free_chunk();
    if (top_free_chunk != NULL) {
      top_free_chunk_size = top_free_chunk->size;
    }
    while ((top_free_chunk_size + incre_count * INCREMENT) < chunk_size) {
      incre_count++;
    }
    if (incre_count * INCREMENT + curr_brk - inital_brk > limit) {
      return NULL;
    } else {
      // within limit
      struct header *new_chunk = (struct header *)sbrk(incre_count * INCREMENT);
      new_chunk->size = incre_count * INCREMENT;
      new_chunk->next = NULL;
      if (top_free_chunk == NULL) {
        new_chunk->next = list_head;
        list_head = new_chunk;
      } else {
        top_free_chunk->size += incre_count * INCREMENT;
      }
      if (find_free_chunk(&result, chunk_size) && result != NULL) {
        return (void *)((char *)result + sizeof(struct header));
      } else {
        write(STDERR_FILENO, "after sbrk(), still can not find chunk\n", 39);
      }
    }
  }
}

struct header *find_top_free_chunk(void) {
  if (list_head != NULL) {
    struct header *curr = list_head;
    while (curr != NULL) {
      if ((void *)((char *)curr + curr->size) == curr_brk) {
        return curr;
      }
      curr = curr->next;
    }
    return NULL;
  }
  return NULL;
}

/*if find the chunk,cut the chunk or not , return 1
 * if can not find approprite chunk , return -1
 */
int find_free_chunk(void **ptr, uint64_t size) {
  struct header *prev = NULL;
  struct header *curr = list_head;
  struct header *target = NULL;
  struct header *tar_prev = NULL;
  // locate the chunk fit
  switch (curr_alg) {
  case FIRST_FIT:
  default:
    while (curr != NULL) {
      if (curr->size >= size) {
        target = curr;
        tar_prev = prev;
        break;
      }
      prev = curr;
      curr = curr->next;
    }
    break;
  case BEST_FIT:
    uint64_t best_fit_size = UINT64_MAX;
    while (curr != NULL) {
      if (curr->size >= size && curr->size < best_fit_size) {
        target = curr;
        tar_prev = prev;
        best_fit_size = curr->size;
      }
      prev = curr;
      curr = curr->next;
    }
    break;
  case WORST_FIT:
    uint64_t worst_fit_size = 0;
    while (curr != NULL) {
      if (curr->size >= size && curr->size > worst_fit_size) {
        target = curr;
        tar_prev = prev;
        worst_fit_size = curr->size;
      }
      prev = curr;
      curr = curr->next;
    }
    break;
  }
  // if not find a chunk,return -1
  if (target == NULL) {
    return -1;
  } else {
    // find a chunk target
    // check the size of the chunk ,wether need to cut
    if (target->size <= size + (uint64_t)sizeof(struct header)) {
      // no need cut, hand the whole chunk
      *ptr = target;
      // if target is head
      if (tar_prev == NULL) {
        list_head = target->next;
      } else {
        // if target is not head
        tar_prev->next = target->next;
      }
      return 1;
    } else {
      // need cut
      target->size = target->size - size;
      *ptr = (void *)((char *)target + target->size);
      ((struct header *)*ptr)->size = size;
      return 1;
    }
  }
}

void dealloc(void *p) {
  struct header *de_chunk = (struct header *)((char *)p - sizeof(struct header));
  // set two neighbor chunk
  struct header *left_ngb = NULL;
  struct header *left_prev = NULL;
  struct header *right_ngb = NULL;
  struct header *right_prev = NULL;
  find_neighbor(de_chunk, &left_ngb, &right_ngb, &left_prev, &right_prev);
  if (left_ngb != NULL && right_ngb != NULL) {
    if (left_prev == NULL) {
      right_prev->next = right_ngb->next;
      left_ngb->size += de_chunk->size + right_ngb->size;
    } else if (right_prev == NULL) {
      list_head = right_ngb->next;
      left_ngb->size += de_chunk->size + right_ngb->size;
    } else if (left_prev != NULL && right_prev != NULL) {
      right_prev->next = right_ngb->next;
      left_ngb->size += de_chunk->size + right_ngb->size;
    }
  } else if (left_ngb != NULL && right_ngb == NULL) {
    left_ngb->size += de_chunk->size;
  } else if (left_ngb == NULL && right_ngb != NULL) {
    if (right_prev == NULL) {
      de_chunk->next = right_ngb->next;
    } else if (right_prev != NULL) {
      right_prev->next = right_ngb->next;
    }
    list_head = de_chunk;
    de_chunk->size += right_ngb->size;
  }
}

void find_neighbor(struct header *target, struct header **left, struct header **right,
                   struct header **l_prev, struct header **r_prev) {
  struct header *curr = list_head;
  struct header *prev = NULL;
  while (curr != NULL) {
    // check left
    if ((void *)((char *)curr + curr->size) == (void *)target) {
      *left = curr;
      *l_prev = prev;
    }
    // check right
    if ((void *)((char *)target + target->size) == (void *)curr) {
      *right = curr;
      *r_prev = prev;
    }
    // next
    prev = curr;
    curr = curr->next;
  }
}

void allocopt(enum algs a, int n) {

  if (!brk_inital) {
    inital_brk = sbrk(0);
    brk_inital = 1;
  } else {
    if (!brk(inital_brk)) {
      write(STDERR_FILENO, "brk() fail, reset fail\n", 24);
      exit(1);
    }
  }
  curr_brk = inital_brk;
  list_head = NULL;
  limit = n;
  curr_alg = a;
}

struct allocinfo allocinfo() {
  struct allocinfo info;
  info.free_size = 0;
  info.free_chunks = 0;
  info.largest_free_chunk_size = 0;
  info.smallest_free_chunk_size = 0;

  if (list_head != NULL) {
    info.free_size = list_head->size - sizeof(struct header);
    info.free_chunks = 1;
    info.largest_free_chunk_size = list_head->size;
    info.smallest_free_chunk_size = list_head->size;
  }
  struct header *curr = list_head->next;
  while (curr != NULL) {
    info.free_size += curr->size - sizeof(struct header);
    info.free_chunks++;
    if (curr->size > info.largest_free_chunk_size) {
      info.largest_free_chunk_size = curr->size;
    } else if (curr->size < info.smallest_free_chunk_size) {
      info.smallest_free_chunk_size = curr->size;
    }
    curr = curr->next;
  }
  return info;
}
