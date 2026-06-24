/*
    Este codigo es parte de "XaVI"
    Copyright (C) 2026  Marcelo Reina Aguilar

    Este programa es software libre: usted puede redistribuirlo y/o modificarlo
    bajo los términos de la Licencia Pública General de GNU publicada por
    la Fundación para el Software Libre, ya sea la versión 3 de la Licencia,
    o (a su elección) cualquier versión posterior.

    Este programa se distribuye con la esperanza de que sea útil, pero
    SIN NINGUNA GARANTÍA; ni siquiera la garantía implícita de
    COMERCIABILIDAD o IDONEIDAD PARA UN FIN DETERMINADO. Consulte la
    Licencia Pública General de GNU para más detalles.

    Debería haber recibido una copia de la Licencia Pública General de GNU
    junto con este programa. En caso contrario, consulte <https://www.gnu.org/licenses/>.
*/

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
