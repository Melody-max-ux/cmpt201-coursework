// Structurally inspired by the canonical examples in man 3 getline and man 3 strtok_r.
#define _POSIX_C_SOURCE 200809
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
  printf("Please enter some text:");
  char *line = NULL;
  size_t len = 0;
  ssize_t nread;

  while ((nread = getline(&line, &len, stdin)) != -1) {
    if (nread > 0 && line[nread - 1] == '\n')
      line[nread - 1] = '\0';
    printf("Tokens:\n");
    char *str = NULL;
    char *saveptr = NULL;
    char *token = NULL;
    for (str = line;; str = NULL) {
      token = strtok_r(str, " ", &saveptr);
      if (token == NULL)
        break;
      printf("\t%s\n", token);
    }
    printf("Please enter some text:");
  }
  free(line);
  exit(EXIT_SUCCESS);
}
