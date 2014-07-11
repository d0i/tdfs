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
#include <sys/time.h>
#include <sys/resource.h>

#include "opts.h"
#include "tdfs_opts.h"
#include "stats.h"
#include "version.h"
#include "string.h"

#error "not yet implemented"
int tdfs_opt_proc(void *data, const char *arg, int key, struct fuse_args *outargs) {
  (void)data;

  int res = 0; // for general purposes
  uopt.cow_enabled = true;

  switch (key) {
  case FUSE_OPT_KEY_NONOPT:
    res = parse_branches(arg);
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
  default:
    uopt.retval = 1;
    return 1;
  }
}
