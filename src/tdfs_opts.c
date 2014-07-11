/*
* License: BSD-style license
* Original Copyright: Radek Podgorny <radek@podgorny.cz>,
*                     Bernd Schubert <bernd-schubert@gmx.de>
* Copyright: Yusuke DOI <doi@gohan.to>
*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "opts.h"
#include "tdfs_opts.h"
#include "stats.h"
#include "version.h"
#include "string.h"


/**
 * workroot should have YYYY/MM format
 */
static int parse_workroot(const char *workroot){
  // the last argument is  our mountpoint, don't take it as branch!
  if (uopt.nbranches) return 0;
  if (uopt.n_month <= 0) {fprintf(stderr, "month shall be set\n");}

  int i;
  int year = 0;
  int month = 0;
  char dummy_branch_str[1024];
  char *wp = dummy_branch_str;
  char *ep = dummy_branch_str + sizeof(dummy_branch_str);
  char *p; // tmp
  int wr = 0;

  // make a dummy YYYY/MM strings -- FIXME this is stupid --
  struct tm *now;
  time_t t = time(NULL);
  now = localtime(&t);
  year = now->tm_year + 1900; 
  month = now->tm_mon + 1; // 0 is January
  if ((p = getenv("TDFS_DEBUG_SET_YEAR")) != NULL){
    year = atoi(p);
  }
  if ((p = getenv("TDFS_DEBUG_SET_MONTH")) != NULL){
    month = atoi(p);
  }
  if (year <= 1900 || month < 1 || month > 12){
    fprintf(stderr, "malformed year-month: Y:%d, M:%d\n", year, month);
    exit(1);
  }
  wr = snprintf(wp, ep-wp, "%s/%04d/%02d=RW", workroot, year, month);
  wp += wr;
  for (i = 1; i < uopt.n_month; i++){
    month --;
    if (month == 0){
      month = 12;
      year --;
    }
    wr = snprintf(wp, ep-wp, ":%s/%04d/%02d=RO", workroot, year, month);
    if (wr >= ep-wp){
      fprintf(stderr, "too long period (internal error)\n");
      fprintf(stderr, "dummy_branch_str = %s\n", dummy_branch_str);
      exit(1);
    }
    wp += wr;
  }

  return parse_branches(dummy_branch_str);
}

// parse nmonth or return -1;
int parse_nmonth(const char *arg){
  int nmonth = -1;
  if (sscanf(arg, "count=%d\n", &nmonth) != 1){
    fprintf(stderr, "%s failed to parse arg('%s'), aborting!\n",
	    __func__, arg);
    exit(1);
  }
  if (nmonth <= 0){
    fprintf(stderr, "N should be positive. Aborting!\n");
    exit(1);
  }
  return nmonth;
}

int tdfs_opt_proc(void *data, const char *arg, int key, struct fuse_args *outargs) {
  (void)data;

  int res = 0; // for general purposes
  uopt.cow_enabled = true;

  switch (key) {
  case FUSE_OPT_KEY_NONOPT:
    res = parse_workroot(arg);
    if (res > 0) return 0;
    uopt.retval = 1;
    return 1;
  case KEY_STATS:
    uopt.stats_enabled = 1;
    return 0;
  case KEY_MAX_FILES:
    set_max_open_files(arg);
    return 0;
  case KEY_HELP:
    print_help(outargs->argv[0]);
    fuse_opt_add_arg(outargs, "-ho");
    uopt.doexit = 1;
    return 0;
  case KEY_VERSION:
    printf("tdfs version: "VERSION"\n");
    uopt.doexit = 1;
    return 1;
  case KEY_N:
    uopt.n_month = parse_nmonth(arg);
    return 0;
  default:
    uopt.retval = 1;
    return 1;
  }
}
