#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
int main(int argc, char **argv)
{
  const char *path = (argc > 1) ? argv[1] : "/tmp/testfile";
  int n = (argc > 2) ? atoi(argv[2]) : 200;
  int fd, i;

  fd = open(path, O_RDWR | O_CREAT, 0644);
  if (fd < 0) { perror("open/create"); return 1; }
  close(fd);

  /* all children in same process group */
  setpgid(0, 0);

  for (i = 0; i < n; i++) 
  {
    pid_t child = fork();
    if (child < 0) 
    {
      perror("fork");
      break;
    }
    if (child == 0) 
    {
      /* Child: open the file and pause forever (keeping fd open) */
      int cfd = open(path, O_RDONLY);
      if (cfd < 0) 
      {
        _exit(1);
      }
      pause();
      _exit(0);
    }
  }

  printf("Spawned %d children, all holding '%s' open.\n", i, path);
  printf("Process group ID: %d\n", getpgrp());
  printf("\nNow run your syscall test or load the module.\n");
  printf("Press Enter when done to kill all children...\n");
  getchar();

  killpg(getpgrp(), SIGTERM);
  printf("All children terminated.\n");
  return 0;
}