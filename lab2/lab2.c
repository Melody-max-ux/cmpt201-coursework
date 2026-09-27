#define _POSIX_C_SOURCE 200809
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  printf("Enter programs to run.\n");
  char *line = NULL;
  size_t len = 0;
  ssize_t nread;

  while ((nread = getline(&line, &len, stdin)) != -1) {
    if (nread > 0 && line[nread - 1] == '\n')
      line[nread - 1] = '\0';
    pid_t pid;
    pid = fork();
    if (pid == -1) {
      perror("fork");
      exit(EXIT_FAILURE);
    }
    if (pid == 0) {
      char *slash;
      if (execlp(line, ((slash = strrchr(line, '/'))) ? slash + 1 : line, (char *)NULL) == -1) {
        perror("exec");
        exit(EXIT_FAILURE);
      }
      exit(EXIT_SUCCESS);
    }
    if (pid > 0) {
      int wstatus;
      int pid_c = waitpid(pid, &wstatus, 0);
    }
    printf("Enter programs to run.\n");
  }
  free(line);
}
