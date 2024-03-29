/*
*  C Implementation: opts.c
*
* Option parser
*
* License: BSD-style license
* Original Copyright: Radek Podgorny <radek@podgorny.cz>,
*                     Bernd Schubert <bernd-schubert@gmx.de>
*
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
#include "stats.h"
#include "version.h"
#include "string.h"

/**
 * Set the maximum number of open files
 */
int set_max_open_files(const char *arg)
{
	struct rlimit rlim;
	unsigned long max_files;
	if (sscanf(arg, "max_files=%ld\n", &max_files) != 1) {
		fprintf(stderr, "%s Converting %s to number failed, aborting!\n",
			__func__, arg);
		exit(1);
	}
	rlim.rlim_cur = max_files;
	rlim.rlim_max = max_files;
	if (setrlimit(RLIMIT_NOFILE, &rlim)) {
		fprintf(stderr, "%s: Setting the maximum number of files failed: %s\n",
			__func__, strerror(errno));
		exit(1);
	}

	return 0;
}


uopt_t uopt;

void uopt_init() {
	memset(&uopt, 0, sizeof(uopt_t)); // initialize options with zeros first
	uopt.n_month = TDFS_N_DEFAULT;
}

/**
 * Take a relative path as argument and return the absolute path by using the
 * current working directory. The return string is malloc'ed with this function.
 */
char *make_absolute(char *relpath) {
	// Already an absolute path
	if (*relpath == '/') return relpath;

	char cwd[PATHLEN_MAX];
	if (!getcwd(cwd, PATHLEN_MAX)) {
		perror("Unable to get current working directory");
		return NULL;
	}

	size_t cwdlen = strlen(cwd);
	if (!cwdlen) {
		fprintf(stderr, "Zero-sized length of CWD!\n");
		return NULL;
	}

	// 2 due to: +1 for '/' between cwd and relpath
	//           +1 for trailing '/'
	int abslen = cwdlen + strlen(relpath) + 2;
	if (abslen > PATHLEN_MAX) {
		fprintf(stderr, "Absolute path too long!\n");
		return NULL;
	}

	char *abspath = malloc(abslen);
	if (abspath == NULL) {
		fprintf(stderr, "%s: malloc failed\n", __func__);
		exit(1); // still at early stage, we can abort
	}
	
	// the ending required slash is added later by add_trailing_slash()
	snprintf(abspath, abslen, "%s/%s", cwd, relpath);

	return abspath;
}

/**
 * Add a trailing slash at the end of a branch. So functions using this
 * path don't have to care about this slash themselves.
 **/
char *add_trailing_slash(char *path) {
	int len = strlen(path);
	if (path[len - 1] == '/') {
		return path; // no need to add a slash, already there
	}

	path = realloc(path, len + 2); // +1 for '/' and +1 for '\0'
	if (!path) {
		fprintf(stderr, "%s: realloc() failed, aborting\n", __func__);
		exit(1); // still very early stage, we can abort here
	}

	strcat(path, "/");
	return path;
}

/**
 * Add a given branch and its options to the array of available branches.
 * example branch string "branch1=RO" or "/path/path2=RW"
 */
static void add_branch(char *branch) {
	uopt.branches = realloc(uopt.branches, (uopt.nbranches+1) * sizeof(branch_entry_t));
	if (uopt.branches == NULL) {
		fprintf(stderr, "%s: realloc failed\n", __func__);
		exit(1); // still at early stage, we can't abort
	}

	char *res;
	char **ptr = (char **)&branch;

	res = strsep(ptr, "=");
	if (!res) return;

	// for string manipulations it is important to copy the string, otherwise
	// make_absolute() and add_trailing_slash() will corrupt our input (parse string)
	uopt.branches[uopt.nbranches].path = strdup(res);
	uopt.branches[uopt.nbranches].rw = 0;

	res = strsep(ptr, "=");
	if (res) {
		if (strcasecmp(res, "rw") == 0) {
			uopt.branches[uopt.nbranches].rw = 1;
		} else if (strcasecmp(res, "ro") == 0) {
			// no action needed here
		} else {
			fprintf(stderr, "Failed to parse RO/RW flag, setting RO.\n");
			// no action needed here either
		}
	}

	uopt.nbranches++;
}

/**
 * Options without any -X prefix, so these options define our branch paths.
 * example arg string: "branch1=RW:branch2=RO:branch3=RO"
 */
int parse_branches(const char *arg) {
	// the last argument is  our mountpoint, don't take it as branch!
	if (uopt.nbranches) return 0;

	// We don't free the buf as parts of it may go to branches
	char *buf = strdup(arg);
	char **ptr = (char **)&buf;
	char *branch;
	while ((branch = strsep(ptr, ROOT_SEP)) != NULL) {
		if (strlen(branch) == 0) continue;

		add_branch(branch);
	}

	free(buf);

	return uopt.nbranches;
}

/**
  * fuse passes arguments with the argument prefix, e.g.
  * "-o chroot=/path/to/chroot/" will give us "chroot=/path/to/chroot/"
  * and we need to cut off the "chroot=" part
  * NOTE: If the user specifies a relative path of the branches
  *       to the chroot, it is absolutely required
  *       -o chroot=path is provided before specifying the braches!
  */
char * get_chroot(const char *arg)
{
	char *str = index(arg, '=');

	if (!str) {
		fprintf(stderr, "-o chroot parameter not properly specified, aborting!\n");
		exit(1); // still early phase, we can abort
	}

	if (strlen(str) < 3) {
		fprintf(stderr, "Chroot path has not sufficient characters, aborting!\n");
		exit(1);
	}

	str++; // just jump over the '='

	// copy of the given parameter, just in case something messes around
	// with command line parameters later on
	str = strdup(str);
	if (!str) {
		fprintf(stderr, "strdup failed: %s Aborting!\n", strerror(errno));
		exit(1);
	}
	return str;
}

void print_help(const char *progname) {
	printf(
	"tdfs version "VERSION"\n"
	"by Yusuke DOI <doi@gohan.to>\n"
	"\n"
	"Usage: %s [options] workroot mountpoint\n"
	"\n"
	"general options:\n"
	"    -o opt,[opt...]        mount options\n"
	"    -h   --help            print help\n"
	"    -V   --version         print version\n"
	"\n"
	"TDFS options:\n"
        "    -o count=N             access newest N directories (default is %d)\n"
	"    -o stats               show statistics in the file 'stats' under the\n"
	"                           mountpoint\n"
	"    -o max_files=number    Increase the maximum number of open files\n"
	"\n",
	progname, TDFS_N_DEFAULT);
}

/**
  * This method is to post-process options once we know all of them
  */
void unionfs_post_opts(void) {
	// chdir to the given chroot, we
	if (uopt.chroot) {
		int res = chdir(uopt.chroot);
		if (res) {
			fprintf(stderr, "Chdir to %s failed: %s ! Aborting!\n",
				  uopt.chroot, strerror(errno));
			exit(1);
		}
	}

	// Make the pathes absolute and add trailing slashes
	int i;
	for (i = 0; i<uopt.nbranches; i++) {
		// if -ochroot= is specified, the path has to be given absolute
		// or relative to the chroot, so no need to make it absolute
		// also won't work, since we are not yet in the chroot here
		if (!uopt.chroot) {
			uopt.branches[i].path = make_absolute(uopt.branches[i].path);
		}
		uopt.branches[i].path = add_trailing_slash(uopt.branches[i].path);

		// Prevent accidental umounts. Especially system shutdown scripts tend
		// to umount everything they can. If we don't have an open file descriptor,
		// this might cause unexpected behaviour.
		char path[PATHLEN_MAX];

		if (!uopt.chroot) {
			BUILD_PATH(path, uopt.branches[i].path);
		} else {
			BUILD_PATH(path, uopt.chroot, uopt.branches[i].path);
		}

		if (mkdir(path, 0755) < 0){
			if (errno != EEXIST){
				perror("mkdir");
				fprintf(stderr, "path=%s\n", path);
				exit(1);
			}
		}
		
		int fd = open(path, O_RDONLY);
		if (fd == -1) {
			fprintf(stderr, "\nFailed to open %s: %s. Aborting!\n\n",
				path, strerror(errno));
			exit(1);
		}
		uopt.branches[i].fd = fd;
	}
}

/*
 * Local Variables:
 * c-basic-offset: 8
 * tab-width: 8
 * indent-tabs-mode: t
 * End:
 */
