#define _DEFAULT_SOURCE
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
int BUF_SIZE = 50;
int EXTRA_SIZE = 256;

struct header {
  uint64_t size;
  struct header *next;
};

void handle_error(const char *msg) {
  write(STDOUT_FILENO, "Error message: ", 15);
  size_t len = 0;
  while (msg[len] != '\0')
    len++;
  write(STDOUT_FILENO, msg, len);
  write(STDOUT_FILENO, "\n", 1);
  _exit(1);
}

void print_out(char *format, void *data, size_t data_size) {
  char buf[BUF_SIZE];
  ssize_t len = snprintf(buf, BUF_SIZE, format, *(uint64_t *)data);
  if (len < 0) {
    handle_error("snprintf");
  }
  write(STDOUT_FILENO, buf, len);
}

int main(void) {
  void *first_block = sbrk(EXTRA_SIZE);
  void *second_block = (char *)first_block + EXTRA_SIZE / 2;
  struct header *h1 = (struct header *)first_block;
  struct header *h2 = (struct header *)second_block;
  h1->size = EXTRA_SIZE / 2;
  h1->next = NULL;
  h2->size = EXTRA_SIZE / 2;
  h2->next = h1;
  memset((char *)h1 + sizeof(struct header), 0, h1->size - sizeof(struct header));
  // second block
  memset((char *)h2 + sizeof(struct header), 1, h2->size - sizeof(struct header));
  print_out("first block:       %p\n", &first_block, sizeof(first_block));
  print_out("second block:      %p\n", &second_block, sizeof(second_block));
  print_out("first block size:  %lu\n", &(h1->size), sizeof(h1->size));
  print_out("first block next:  %p\n", &(h1->next), sizeof(h1->next));
  print_out("second block size: %lu\n", &(h2->size), sizeof(h2->size));
  print_out("second block next: %p\n", &(h2->next), sizeof(h2->next));
  for (char *i = (char *)first_block + sizeof(struct header); i < (char *)first_block + h1->size;
       i++) {
    uint64_t val = (uint64_t)*i;
    print_out("%lu\n", &val, sizeof(val));
  }

  for (char *i = (char *)second_block + sizeof(struct header); i < (char *)second_block + h2->size;
       i++) {
    uint64_t val = (uint64_t)*i;
    print_out("%lu\n", &val, sizeof(val));
  }

  return 0;
}
