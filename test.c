#include <stdio.h>
#include <time.h>
#include "stermp.h"

void update_time()
{
  struct tm *tm;
  time_t _time;
  char texto[50];
  textcolor(YELLOW);
  _time=time(NULL);
  tm = localtime(&_time);
  strftime(texto,50,"%d/%m/%Y %H:%M:%S", tm);
  gotoxy(1,1);
  printf("%s    ", texto);
}

int main()
{
  int x,y;
  int ancho, alto;
  int tecla;
  term_init();

  ancho = screenwidth();
  alto = screenheight();
  /* Rellenamos de verde la pantalla */
  textbackground(GREEN);
  clrscr();

  textbackground(BLUE);
  /* Rellenamos de azul la primera fila */
  for (x=0; x<ancho; x++)
    printf(" ");

  gotoxy(1,alto);
  /* Rellenamos de azul la última fila */
  for (x=0; x<ancho; x++)
    printf(" ");

  gotoxy(2,2);
  while ((tecla=kbhit2())==0)
      update_time();    


  printf("You have pressed: %d\n", tecla);

  term_defaults();
 
}

