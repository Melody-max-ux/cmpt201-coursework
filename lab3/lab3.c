#define _POSIX_C_SOURCE 200809
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  char *struct_lineptr;
  size_t struct_cap;
  int struct_len;
} LINE;

int main() {
  LINE buffer[5] = {0};
  int i = 0;
  while (1) {
    if (i >= 5)
      i = 0;
    printf("Enter input: ");
    buffer[i].struct_len = getline(&buffer[i].struct_lineptr, &buffer[i].struct_cap, stdin);
    if ((buffer[i].struct_lineptr != NULL) &&
        (buffer[i].struct_lineptr[buffer[i].struct_len - 1] == '\n'))
      buffer[i].struct_lineptr[buffer[i].struct_len - 1] = '\0';

    if (strcmp(buffer[i].struct_lineptr, "print") == 0) {
      // print loop
      int j = i + 1;
      if (j >= 5)
        j = 0;
      for (int k = 0; k < 5; k++) {
        if ((buffer[j].struct_lineptr != NULL) && (buffer[j].struct_lineptr[0] != '\0'))
          printf("%s\n", buffer[j].struct_lineptr);
        j++;
        if (j >= 5)
          j = 0;
      }
    }

    if (strcmp(buffer[i].struct_lineptr, "clean") == 0) {
      // clean loop
      for (int j = 0; j < 5; j++) {
        free(buffer[j].struct_lineptr);
        buffer[j].struct_lineptr = NULL;
        buffer[j].struct_cap = 0;
        buffer[j].struct_len = 0;
      }
    }

    i++;
  }

  // free loop;
  for (int j = 0; j < 5; j++) {
    free(buffer[j].struct_lineptr);
    buffer[j].struct_lineptr = NULL;
    buffer[j].struct_cap = 0;
    buffer[j].struct_len = 0;
  }

  return 0;
}
