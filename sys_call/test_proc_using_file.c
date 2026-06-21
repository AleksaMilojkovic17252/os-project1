#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <pwd.h>
#include <errno.h>

struct proc_info {
  int pid;
  int prio;
  int nice;
  unsigned int uid;
  unsigned long mem_kb;
  unsigned long long cpu_time_ns;
  char comm[16];
};

#define SYS_PROC_USING_FILE 463
#define MAX_RESULTS 1024

int main(int argc, char **argv)
{
  struct proc_info buf[MAX_RESULTS];
  long n;
  long i;
  
  if(argc < 2)
  {
    printf("Usage %s <file_path>\n", argv[0]);
    return 1;
  }
  
  n = syscall(SYS_PROC_USING_FILE, argv[1], buf, MAX_RESULTS);
  
  if(n<0)
  {
    printf("syscall failed");
    return 1;
  }
  
  printf("%ld process(es) currently using: %s\n\n", n, argv[1]);
  
  if(n == 0) 
  {
    printf("No processes have this file opened");
    return 0;
  }
  
  printf("%-8s %-5s %-5s %-12s %-10s %-18s %s\n", "PID", "PRIO", "NICE", "USER", "MEM(kB)", "CPU(ns)", "COMMAND");
  printf("%-8s %-5s %-5s %-12s %-10s %-18s %s\n", "---", "----", "----", "----", "-------", "-------", "-------");
  
  for (i = 0; i < n; i++) {
    struct passwd *pw = getpwuid(buf[i].uid);
    printf("%-8d %-5d %-5d %-12s %-10lu %-18llu %s\n", buf[i].pid, buf[i].prio, buf[i].nice, pw ? pw->pw_name : "?", buf[i].mem_kb, buf[i].cpu_time_ns, buf[i].comm);
  }
  return 0;
}