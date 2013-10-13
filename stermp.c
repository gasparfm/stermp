/* ---------------------------------------------------------- */
/* | Nombre: stermp.c ( Simple Terminal Play)               | */
/* | Autor:  Gaspar Fernández (blakeyes@totaki.com)         | */
/* | Fecha:  05 Enero 2009                                  | */
/* | Web:    http://www.totaki.com/poesiabinaria/           | */
/* ---------------------------------------------------------- */
/* | Description:                                           | */
/* |   Colors and position handling for terminal programs   | */
/* |  made for easy terminal projects, to make it as        | */
/* |  similar as possible to conio.h, and make those old    | */
/* |  programs portable.                                    | */
/* ---------------------------------------------------------- */
/* | Descripción:                                           | */
/* |   Manejo de colores y posicionamiento de texto en un   | */
/* |  terminal para nostálgicos de conio.h                  | */
/* ---------------------------------------------------------- */
/* The MIT License (MIT) */

/* Copyright (c) 2013 Gaspar Fernández */

/* Permission is hereby granted, free of charge, to any person obtaining a copy of */
/* this software and associated documentation files (the "Software"), to deal in */
/* the Software without restriction, including without limitation the rights to */
/* use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of */
/* the Software, and to permit persons to whom the Software is furnished to do so, */
/* subject to the following conditions: */

/* The above copyright notice and this permission notice shall be included in all */
/* copies or substantial portions of the Software. */

/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR */
/* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS */
/* FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR */
/* COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER */
/* IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN */
/* CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "stermp.h"
#include <errno.h>
#include <fcntl.h>
#include <signal.h>

#define KBHIT_USLEEP() {			\
    if (kbh_usleep)				\
      usleep(kbh_usleep);			\
  }
#define KBHIT_PRE() {							\
    termnoecho(1);							\
    fcntl(STDIN_FILENO, F_SETFL, fdflags | O_NONBLOCK); /* Now we activate NON_BLOCK flag to not stop execution */ \
    KBHIT_USLEEP();							\
  }
/* Equivalencias de colores */
static const char ansicolors[16] = {30, 34, 32, 36, 31, 35, 33, 37, 0, 34, 32, 36, 31, 35, 33, 37};

struct termios  orig_tty;	/* Initial terminal attributes */
struct termios	save_tty;	/* Backgup of terminal attributes */
int             orig_fdf;	/* Initial flag values */
int             fdflags; 	/* Flag values */

int             kbh_usleep=100000;	/* No lighten CPU usage and avoid lots of kbhits() all the time, we usleep after each one of them */
int             __init=0;	/* Is it initialized? */

void clrscr()
{
  printf("\033[2J");   // Clears screen
  printf("\033[1;1H"); // Go to row 1, column 1
}

void gotoxy(int x, int y)
{
  printf("\033[%d;%dH", y, x); 	/* Put cursor in X, Y */
}

void textcolor(int color)
{
  int atrval=0;
  if (color & UNDERLINE)
    atrval=UNDERLINE_ATTR;
  else if (color & BLINK)
    atrval=BLINK_ATTR;
  else if (color>=BRILLO_MIN)
    atrval=BRILLO_ATTR;

  color=ansicolors[color & 15];
  
  printf("\033[%d;%dm", atrval, color);
}

void textbackground(int color)
{
  if (color<BRILLO_MIN)
    {				    /* Only some colors can be at background */
      color=ansicolors[color & 15]+10;
  
      printf("\033[%dm", color);
    }
}

void restore_color()
{
  printf("\033[0m");
}

int wherex()
{
  coord_t coord;

  coord=wherexy(0);
  return coord.x;
}

int wherey()
{
  coord_t coords;

  coords=wherexy(0);
  return coords.y;
}

int screenwidth()
{
  coord_t coords;

  coords=wherexy(1);
  return coords.x;
}

int screenheight()
{
  coord_t coords;

  coords=wherexy(1);
  return coords.y;
}

coord_t wherexy(int abs)
{
  coord_t coords;
  char buf[12];
  char *p;
  fflush(stdout);		/* Flush stdout  */

  termnoecho(1);
  if (abs)			/* To get screen dimensions */
    {
  	fprintf(stdout, "\033[s");	  /* Saves current position */
  	fprintf(stdout, "\033[r");	  /* Avoids scroll */
  	fprintf(stdout, "\033[255;255H"); /* Go to the bottom right position */
    }

  fprintf(stdout, "\033[6n");	/* Gets data */
  fflush(stdout);
  while (read(STDIN_FILENO, buf, sizeof(buf))==0);

  sscanf(buf, "\033[%d;%dR", &coords.y, &coords.x);

  if (abs)
    {
  	fprintf(stdout, "\033[u"); /* Restores cursor position */
    }

  restore_terminal();		/* Restores terminal properties */

  return coords;
}

void termnoecho(int canon)
{
  struct termios   tty;

  if (__init)
    save_term_info();

  tty = save_tty;
  
  if (canon)
    tty.c_lflag &= ~(ICANON);
  tty.c_lflag &= ~(ECHO|ECHOE|ECHOK|ECHONL);
  tty.c_cc[VMIN]  = 1;
  tty.c_cc[VTIME] = 1;
  
  if ( tcsetattr(STDIN_FILENO, TCSANOW, &tty) == -1 )
    error ("ERROR: I can't set terminal attributes.");
}

int getch()
{
  int key;

  termnoecho(1);
  key=getch();

  restore_terminal();
}

int getche()
{
  int key;

  termnoecho(0);
  key=getch();

  restore_terminal();
}

void kbhit_pre()
{
  KBHIT_PRE();
}

int __kbhit()
{
  int key;

  key=getchar();

  if (key!=EOF)
      return key;
}

int kbhit2()
{
  int key;

  KBHIT_PRE();
  key=getchar();
  restore_terminal();
  if (key!=EOF)
      return key;

  return 0;
}

int kbhit()
{
  int key;

  KBHIT_PRE();
  key=getchar();
  restore_terminal();
  if (key!=EOF)
    {
      ungetc(key, stdin);
      return 1;
    }
  return 0;
}

void restore_terminal()
{
  restore_term(&save_tty, &fdflags);
}

void restore_term(struct termios *tty, int *stdinflags)
{
  if ( tcsetattr(STDIN_FILENO, TCSANOW, tty) == -1 )
    error ("ERROR: I can't set terminal attributes.");

  if ( fcntl(STDIN_FILENO, F_SETFL, stdinflags) == -1 )
    error ("ERROR: I can't set stdin flags.");
}

void error(char *err)
{
  fprintf(stderr, "%s errno=%d (%s)\n", err, errno, strerror(errno));
  exit (EXIT_CODE);
}

void term_info(struct termios *tty, int *stdinflags)
{
  if ( tcgetattr(STDIN_FILENO, tty) == -1 )
    error ("ERROR: I Can't get terminal attributes.");

  *stdinflags = fcntl(STDIN_FILENO, F_GETFL, 0);
}

void save_term_info()
{
  term_info (&save_tty, &fdflags); /* Store flags in temporary variable */
}

void sigio(int s)
{
}

void term_init()
{
  /* Save initial values */
  term_info (&orig_tty, &orig_fdf);
  /* Backup values */
  save_tty=orig_tty;
  fdflags=orig_fdf;

  if (signal(SIGIO, sigio)==SIG_ERR)
    error("ERROR: Can't block SIGIO signal");

  __init=1;
}

void term_defaults()
{
  restore_term(&orig_tty, &orig_fdf);
}
