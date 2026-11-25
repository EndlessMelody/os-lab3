// Black-box checks of the shared PID page, its permissions and ownership.
#include "kernel/types.h"
#include "kernel/riscv.h"
#include "kernel/memlayout.h"
#include "kernel/elf.h"
#include "kernel/fcntl.h"
#include "kernel/sysinfo.h"
#include "kernel/syscall.h"
#include "user/user.h"

static void
check(int ok, char *message)
{
  if(!ok){
    fprintf(2, "UGETPID_FAIL: %s\n", message);
    exit(1);
  }
}

static uint64
freebytes(void)
{
  struct sysinfo info;
  check(sysinfo(&info) == 0, "sysinfo");
  return info.freemem;
}

static void
identity(void)
{
  check(ugetpid() == getpid(), "shared PID matches syscall");
  volatile uchar *page = (uchar *)USYSCALL;
  for(int i = sizeof(struct usyscall); i < PGSIZE; i++)
    check(page[i] == 0, "shared page padding is zero");
}

static void
reap(int expected)
{
  int status;
  check(wait(&status) == expected && status == 0, "child completed successfully");
}

static void
fastpath(void)
{
  int pid = getpid();
  check(trace(1U << SYS_getpid) == 0, "trace getpid calls");
  for(int i = 0; i < 128; i++)
    check(ugetpid() == pid, "fast path PID");
  check(trace(0) == 0, "disable tracing");
  printf("UGETPID_FASTPATH_OK\n");
}

static void
lifecycle(void)
{
  identity();
  uint64 before = freebytes();
  for(int i = 0; i < 96; i++){
    int pid = fork();
    check(pid >= 0, "fork for process-slot reuse");
    if(pid == 0){
      identity();
      exit(0);
    }
    reap(pid);
    identity();
  }
  check(freebytes() == before, "fork/exit/wait releases all pages");
  printf("UGETPID_LIFECYCLE_OK\n");
}

static void
concurrent(void)
{
  enum { N = 8 };
  int ready[2], gate[2], pids[N];
  uint64 before = freebytes();
  check(pipe(ready) == 0 && pipe(gate) == 0, "create synchronization pipes");
  for(int i = 0; i < N; i++){
    pids[i] = fork();
    check(pids[i] >= 0, "fork concurrent children");
    if(pids[i] == 0){
      close(ready[0]);
      close(gate[1]);
      identity();
      int pid = ugetpid();
      char token;
      check(write(ready[1], &pid, sizeof(pid)) == sizeof(pid), "report child PID");
      check(read(gate[0], &token, 1) == 1, "wait for parent");
      identity();
      exit(0);
    }
  }
  close(ready[1]);
  close(gate[0]);
  for(int i = 0; i < N; i++){
    int pid, found = 0;
    check(read(ready[0], &pid, sizeof(pid)) == sizeof(pid), "receive child PID");
    for(int j = 0; j < N; j++)
      if(pids[j] == pid){
        pids[j] = -1;
        found = 1;
        break;
      }
    check(found, "live children have distinct PIDs");
    identity();
  }
  check(write(gate[1], "12345678", N) == N, "release children");
  close(ready[0]);
  close(gate[1]);
  for(int i = 0; i < N; i++){
    int status;
    check(wait(&status) > 0 && status == 0, "reap concurrent child");
  }
  check(freebytes() == before, "concurrent children and pipes leave no leaked pages");
  printf("UGETPID_CONCURRENT_OK\n");
}

static void
exec_child(char stage)
{
  identity();
  if(stage > 'A'){
    char next[] = { stage - 1, 0 };
    char *args[] = { "ugetpidcheck", "exec-child", next, 0 };
    exec(args[0], args);
    check(0, "exec next image");
  }
  exit(0);
}

static void
exec_test(void)
{
  // Valid ELF header, followed by an invalid load segment: exec has already
  // created its replacement page table when it rejects filesz > memsz.
  struct elfhdr elf;
  struct proghdr segment;
  memset(&elf, 0, sizeof(elf));
  memset(&segment, 0, sizeof(segment));
  elf.magic = ELF_MAGIC;
  elf.phoff = sizeof(elf);
  elf.phnum = 1;
  segment.type = ELF_PROG_LOAD;
  segment.filesz = 1;
  int fd = open("badpidelf", O_CREATE | O_TRUNC | O_WRONLY);
  check(fd >= 0, "create invalid ELF");
  check(write(fd, &elf, sizeof(elf)) == sizeof(elf), "write ELF header");
  check(write(fd, &segment, sizeof(segment)) == sizeof(segment), "write invalid segment");
  close(fd);
  uint64 before = freebytes();
  char *badargs[] = { "badpidelf", 0 };
  for(int i = 0; i < 32; i++){
    check(exec(badargs[0], badargs) == -1, "invalid ELF rejected");
    identity();
  }
  check(freebytes() == before, "failed exec preserves PID page without leaks");
  int pid = fork();
  check(pid >= 0, "fork exec chain");
  if(pid == 0)
    exec_child('Q'); // Sixteen exec replacements, preserving the same process.
  reap(pid);
  identity();
  check(freebytes() == before, "successful exec and reap leave no leaked pages");
  check(unlink("badpidelf") == 0, "remove invalid ELF");
  printf("UGETPID_EXEC_OK\n");
}

static void
permissions(int execute)
{
  int pid = fork(), status;
  check(pid >= 0, "fork permission check");
  if(pid == 0){
    if(execute)
      ((void (*)(void))USYSCALL)();
    else
      ((volatile struct usyscall *)USYSCALL)->pid = -123;
    exit(1); // Access must fault; a normal return is failure.
  }
  check(wait(&status) == pid && status == -1, "forbidden access kills child");
  identity();
  if(!execute){
    int fds[2];
    char byte = 'x';
    check(pipe(fds) == 0, "create copyout check pipe");
    check(write(fds[1], &byte, 1) == 1, "write copyout check byte");
    // xv6 piperead returns the number of bytes copied (zero on this failure).
    check(read(fds[0], (void *)USYSCALL, 1) <= 0, "kernel copyout rejects read-only page");
    close(fds[0]);
    close(fds[1]);
    identity();
  }
  printf("UGETPID_%s_OK\n", execute ? "NX" : "READONLY");
}

static void
oom(void)
{
  uint64 before = freebytes();
  int pid = fork();
  check(pid >= 0, "fork memory pressure worker");
  if(pid == 0){
    while(sbrk(PGSIZE) != (char *)-1)
      ;
    // Few spare pages force failures at different allocation stages of fork:
    // trapframe, PID page, page-table root/levels, then copying user memory.
    for(int spare = 0; spare < 10; spare++){
      if(spare)
        check(sbrk(-PGSIZE) != (char *)-1, "release a spare page");
      uint64 available = freebytes();
      int child = fork();
      if(child == 0)
        exit(1);
      if(child > 0)
        reap(child);
      check(child == -1, "fork fails under memory pressure");
      check(freebytes() == available, "failed fork releases partial allocations");
      identity();
      printf("UGETPID_OOM_SPARE: %ld bytes\n", available);
    }
    exit(0);
  }
  reap(pid);
  check(freebytes() == before, "memory pressure worker releases all pages");
  identity();
  printf("UGETPID_OOM_OK\n");
}

int
main(int argc, char **argv)
{
  check(argc >= 2, "usage: ugetpidcheck group");
  if(strcmp(argv[1], "exec-child") == 0 && argc == 3)
    exec_child(argv[2][0]);
  else if(strcmp(argv[1], "fastpath") == 0)
    fastpath();
  else if(strcmp(argv[1], "lifecycle") == 0)
    lifecycle();
  else if(strcmp(argv[1], "concurrent") == 0)
    concurrent();
  else if(strcmp(argv[1], "exec") == 0)
    exec_test();
  else if(strcmp(argv[1], "readonly") == 0)
    permissions(0);
  else if(strcmp(argv[1], "nx") == 0)
    permissions(1);
  else if(strcmp(argv[1], "oom") == 0)
    oom();
  else
    check(0, "unknown test group");
  exit(0);
}
