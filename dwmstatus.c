#define _DEFAULT_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <locale.h>
#include <fcntl.h>
#include <X11/Xlib.h>

#define RED   "\033[0;31m"
#define PATH_BATT_CAPACITY "/sys/class/power_supply/BAT0/capacity"

char *tzhelsinki = "Europe/Helsinki";

static Display *dpy;

char *
smprintf(char *fmt, ...)
{
  va_list fmtargs;
  char *ret;
  int len;
  va_start(fmtargs, fmt);
  len = vsnprintf(NULL, 0, fmt, fmtargs);
  va_end(fmtargs);
  ret = malloc(++len);
  if (ret == NULL) {
	  perror("malloc");
	  exit(1);
  }
  va_start(fmtargs, fmt);
  vsnprintf(ret, len, fmt, fmtargs);
  va_end(fmtargs);

  return ret;
}

void
settz(char *tzname)
{
  setenv("TZ", tzname, 1);
}

char *
mktimes(char *fmt, char *tzname)
{
  char buf[129];
  time_t tim;
  struct tm *timtm;

  memset(buf, 0, sizeof(buf));
  settz(tzname);
  tim = time(NULL);
  timtm = localtime(&tim);
  if (timtm == NULL) {
	  perror("localtime");
	  exit(1);
  }

  if (!strftime(buf, sizeof(buf)-1, fmt, timtm)) {
  	fprintf(stderr, "strftime == 0\n");
  	exit(1);
  }

  return smprintf("%s", buf);
}

void
setstatus(char *str)
{
  XStoreName(dpy, DefaultRootWindow(dpy), str);
  XSync(dpy, False);
}

char *
loadavg(void)
{
  double avgs[3];

  if (getloadavg(avgs, 3) < 0) {
	perror("getloadavg");
	exit(1);
  }

  return smprintf("%.2f %.2f %.2f", avgs[0], avgs[1], avgs[2]);
}

int
batterylife(void) {

  char *res;
  res = malloc(sizeof(char)*20);
  int fd, life;
  char inum[4];

  if (-1 == (fd = open(PATH_BATT_CAPACITY,O_RDONLY))) {
      perror("Could not open battery info for reading");
      exit(-1);
  }
  read( fd, &inum, sizeof(inum) );
  close(fd);

  sscanf(inum, "%d", &life);
  if (life >= 84) {
	  sprintf(res, "%lc", 0x2587);
  } else if (life < 84 && life >= 68) {
	  sprintf(res, "%lc", 0x2586);
  } else if (life < 68 && life >= 52) {
  	sprintf(res, "%lc", 0x2585);
} else if (life < 52 && life >= 36) {
  	sprintf(res, "%lc", 0x2583);
  } else if (life < 36 && life >= 16) {
  	sprintf(res, "%lc", 0x2582);
  } else if (life < 16) {
    sprintf(res, "%lc", 0x2581);
  }

  return (int) life; 
}

int
main(void)
{
  setlocale(LC_ALL, "");

  char *status;
  // char *avgs;
  char *tmhki;
  int battery;
  
  if (!(dpy = XOpenDisplay(NULL))) {
  	fprintf(stderr, "dwmstatus: cannot open display.\n");
  	return 1;
  }
	
  for (;;sleep(20)) {
  	// avgs = loadavg();
  	battery = batterylife();
  	tmhki = mktimes(" %Y-%m-%d %H:%M", tzhelsinki);
  	status = smprintf("%s %i", tmhki ,battery);
  	setstatus(status);
  }
  free(&battery);
	free(tmhki);
	free(status);
  XCloseDisplay(dpy);
	
  return 0;
}

