/*
*
* This is offered under a BSD-style license. This means you can use the code for whatever you
* desire in any way you may want but you MUST NOT forget to give me appropriate credits when
* spreading your work which is based on mine. Something like "original implementation by Yusuke
* DOI" should be fine.
*
* License: BSD-style license
* Copyright: Yusuke DOI <doi@gohan.to>
*/


#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>

#include "unionfs.h"
#include "opts.h"
#include "tdfs_opts.h"
#include "debug.h"
#include "stats.h"


static struct fuse_opt tdfs_opts[] = {
	FUSE_OPT_KEY("--help", KEY_HELP),
	FUSE_OPT_KEY("--version", KEY_VERSION),
	FUSE_OPT_KEY("-h", KEY_HELP),
	FUSE_OPT_KEY("-V", KEY_VERSION),
	FUSE_OPT_KEY("stats", KEY_STATS),
	//FUSE_OPT_KEY("statfs_omit_ro", KEY_STATFS_OMIT_RO),
	//FUSE_OPT_KEY("chroot=%s,", KEY_CHROOT),
	FUSE_OPT_KEY("max_files=%s", KEY_MAX_FILES),
	FUSE_OPT_END
};

int main(int argc, char **argv){
  struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

  int res = debug_init();
  if (res != 0) return res;

  uopt_init();
  
  if (fuse_opt_parse(&args, NULL, tdfs_opts, tdfs_opt_proc) == -1) return 1;

  if (!uopt.doexit) {
    if (uopt.nbranches == 0) {
      printf("You need to specify at least one branch!\n");
      return 1;
    }

    if (uopt.stats_enabled) stats_init(&stats);
  }

  // enable fuse permission checks, we need to set this, even we we are
  // not root, since we don't have our own access() function
  if (fuse_opt_add_arg(&args, "-odefault_permissions")) {
    fprintf(stderr, "Severe failure, can't enable permssion checks, aborting!\n");
    exit(1);
  }
  unionfs_post_opts();

  umask(0);
  res = fuse_main(args.argc, args.argv, &unionfs_oper, NULL);
  return uopt.doexit ? uopt.retval : res;
}
