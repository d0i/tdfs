/*
* License: BSD-style license
* Original Copyright: Radek Podgorny <radek@podgorny.cz>,
*                     Bernd Schubert <bernd-schubert@gmx.de>
* Copyright: Yusuke DOI <doi@gohan.to>
*/

#ifndef OPTS_H
#define OPTS_H


#include <fuse.h>
#include <stdbool.h>

#include "unionfs.h"


#define ROOT_SEP ":"

typedef struct {
	int nbranches;
	branch_entry_t *branches;

	bool stats_enabled;
	bool cow_enabled;
	bool statfs_omit_ro;
	int doexit;
	int retval;
	char *chroot; 		// chroot we might go into
	int n_month; // how many months we look back?

} uopt_t;

#define TDFS_N_DEFAULT 3

enum {
	KEY_STATS,
	KEY_COW,
	//KEY_STATFS_OMIT_RO,
	//KEY_NOINITGROUPS,
	//KEY_CHROOT,
	KEY_MAX_FILES,
	KEY_N,
	KEY_HELP,
	KEY_VERSION
};


extern uopt_t uopt;


int set_max_open_files(const char *arg);
void uopt_init();
void unionfs_post_opts();
int parse_branches(const char *arg);
void print_help(const char *progname);

#endif

/*
 * Local Variables:
 * c-basic-offset: 8
 * tab-width: 8
 * indent-tabs-mode: t
 * End:
 */
