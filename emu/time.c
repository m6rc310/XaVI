#include <unistd.h>
#include <sys/time.h>
#include <signal.h>

void updateScreen ();

struct itimerval t;
struct sigaction sa;

#define COUNT_SCREEN 20
int countScreen=1;

void action (int a) {
  countScreen++;
  if (countScreen==COUNT_SCREEN) {
    updateScreen ();
    countScreen=1;
  }
}

int initTime () {
 
  t.it_interval.tv_sec=0;
  t.it_interval.tv_usec=1000;
  t.it_value.tv_sec=0;
  t.it_value.tv_usec=1000;

  sigemptyset(&sa.sa_mask);
  sa.sa_handler=action;
  sa.sa_flags=0;
  sigaction(SIGALRM,&sa,NULL);
  setitimer (ITIMER_REAL,&t,NULL);
}
