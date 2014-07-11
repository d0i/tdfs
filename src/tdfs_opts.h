/*
* License: BSD-style license
* Copyright: Yusuke DOI <doi@gohan.to>
*/

#ifndef TDFS_OPTS_H
#define TDFS_OPTS_H


#include <fuse.h>
#include <stdbool.h>

#include "unionfs.h"

int tdfs_opt_proc(void *data, const char *arg, int key, struct fuse_args *outargs);
#endif
