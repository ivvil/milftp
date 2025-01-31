#include <errno.h>
#include <libmill.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define CONN_ESTABLISHED 1
#define CONN_SUCCEEDED 2
#define CONN_FAILED 3

coroutine void dialogue(tcpsock as, chan ch) {
  chs(ch, int, CONN_ESTABLISHED);
  int64_t deadline = now() + 10000;

  tcpsend(as, "What's your name\r\n", 19, deadline);
  if (errno != 0)
    goto cleanup;

  tcpflush(as, -1);
  if (errno != 0)
    goto cleanup;

  char inbuf[256];
  size_t sz = tcprecvuntil(as, inbuf, sizeof(inbuf), "\r", 1, deadline);
  if (errno != 0)
    goto cleanup;

  inbuf[sz - 1] = 0;
  char outbuf[512];

  int rc = sprintf(outbuf, "Hello %s!\r\n", inbuf);

  tcpsend(as, outbuf, rc, deadline);
  if (errno != 0)
    goto cleanup;
  tcpflush(as, deadline);
  if (errno != 0)
    goto cleanup;

cleanup:
  if (errno == 0)
    chs(ch, int, CONN_SUCCEEDED);
  else
    chs(ch, int, CONN_FAILED);
  tcpclose(as);
}

coroutine void statistics(chan ch) {
  int conns = 0;
  int active = 0;
  int failed = 0;

  while (1) {
    int op = chr(ch, int);

    if (op == CONN_ESTABLISHED)
      ++conns, ++active;
    else
      --active;
    if (op == CONN_FAILED)
      ++failed;

    printf("Process ID: %d\n", (int)getpid());
    printf("Total number of connections: %d\n", conns);
    printf("Active connections: %d\n", active);
    printf("Failed connections: %d\n\n", failed);
  }
}

int main(int argc, char *argv[]) {

  int port = 5555;

#ifdef _WIN32
#include <sysinfoapi.h>
  SystemInfo sysinfo;
  GetSystemInfo(&sysinfo);
  int nproc = sysinfo.dwNumberOfProcessors;
#elif __linux__
#include <sys/sysinfo.h>
  int nproc = get_nprocs();
#else
  int nproc = 1;
#endif

  if (argc > 1)
    port = atoi(argv[1]);
  if (argc > 2)
    nproc = atoi(argv[2]);

  printf("Port: %d, Number of cores: %d\n\n", port, nproc);

  ipaddr addr = iplocal(NULL, port, 0);
  tcpsock ls = tcplisten(addr, 10);
  if (!ls) {
    perror("Can't open listening socket");
    return 1;
  }

  for (int i = 0; i < nproc; ++i) {
    pid_t pid = mfork();
    if (pid < 0) {
      perror("Can't create new process");
    }

    if (pid == 0) {
      break;
    }
  }

  chan ch = chmake(int, 0);
  go(statistics(ch));

  while (1) {
    tcpsock as = tcpaccept(ls, -1);
    if (!as)
      continue;

    go(dialogue(as, ch));
  }
}
