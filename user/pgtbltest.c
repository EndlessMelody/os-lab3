// Course-aligned local tests for Project 3; not an official MIT grader.
#include "kernel/types.h"
#include "kernel/riscv.h"
#include "user/user.h"

static void
fail(char *message)
{
  printf("PGTBL_FAIL: %s\n", message);
  exit(1);
}

static void
ugetpid_test(void)
{
  if(ugetpid() != getpid())
    fail("parent ugetpid differs from getpid");
  // Repeated allocation and slot reuse exercise per-process page ownership.
  for(int i = 0; i < 64; i++){
    int pid = fork(), status;
    if(pid < 0)
      fail("fork");
    if(pid == 0){
      if(ugetpid() != getpid())
        fail("child ugetpid differs from getpid");
      exit(0);
    }
    if(wait(&status) != pid || status != 0)
      fail("child ugetpid or cleanup");
  }
  printf("ugetpid_test: OK\n");
}

static void
pgaccess_test(void)
{
  char *allocation = malloc(33 * PGSIZE);
  if(allocation == 0)
    fail("malloc");
  volatile char *pages = (char *)PGROUNDUP((uint64)allocation);
  uint mask = 0;
  if(pgaccess((void *)pages, 32, &mask) < 0)
    fail("initial pgaccess");
  pages[PGSIZE] = 1;
  pages[2 * PGSIZE] = 1;
  pages[30 * PGSIZE] = 1;
  if(pgaccess((void *)pages, 32, &mask) < 0)
    fail("pgaccess");
  if(mask != ((1U << 1) | (1U << 2) | (1U << 30)))
    fail("accessed-page mask");
  if(pgaccess((void *)pages, 32, &mask) < 0 || mask != 0)
    fail("access bits were not cleared");
  free(allocation);
  printf("pgaccess_test: OK\n");
}

int
main(int argc, char **argv)
{
  if(argc > 2 || (argc == 2 && strcmp(argv[1], "ugetpid") != 0 &&
                 strcmp(argv[1], "pgaccess") != 0)){
    fprintf(2, "usage: pgtbltest [ugetpid|pgaccess]\n");
    exit(1);
  }
  if(argc == 1 || strcmp(argv[1], "ugetpid") == 0)
    ugetpid_test();
  if(argc == 1 || strcmp(argv[1], "pgaccess") == 0)
    pgaccess_test();
  exit(0);
}
