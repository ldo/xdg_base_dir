/*
	Implementation of the XDG Base Directory specification
	<http://standards.freedesktop.org/basedir-spec/latest/>.

	The routines in this module can be grouped into the following categories:
	* find all config/data files:
	    xdg_find_all_config_path, xdg_find_all_data_path
	* find highest-priority config/data file:
	    xdg_find_first_config_path, xdg_find_first_data_path
	* find location to create user-specific config/data/cache file:
	    xdg_get_config_home, xdg_get_data_home, xdg_get_cache_home, xdg_find_cache_path
	* utility:
	    xdg_makedirsif

	Strategies for dealing with multiple configuration/data files are up to you.
	Common strategies are:
	1) Look only at the highest-priority config or data file and ignore any others.
	2) Look at all config/data files, but process them in reverse order of priority
	   and merge the results, so settings in later, higher-priority files override
	   corresponding ones in earlier, lower-priority ones.

	Written by Lawrence D'Oliveiro <ldo@geek-central.gen.nz>.
*/	

#include <stdbool.h>
#include <sys/types.h>

int xdg_makedirsif
  (
	const char * path
  );
  /* creates all the directories in path, if they don't already exist. Returns
	nonzero and sets errno on error. */

char * xdg_make_home_relative
  (
	const char * path
  );
  /* prepends the value of $HOME onto path (assumed not to begin with a slash), or
	NULL on error (out of memory, $HOME not defined or not absolute). Caller must dispose
	of the result pointer. */

char * xdg_get_config_home
  (
	bool makedirs
  );
  /* returns the directory for holding user-specific config files, or NULL on
	error. Caller must dispose of the result pointer. */

char * xdg_get_data_home
  (
	bool makedirs
  );
  /* returns the directory for holding user-specific data files, or NULL on
	error. Caller must dispose of the result pointer. */

char * xdg_get_data_home
  (
	bool makedirs
  );
  /* returns the directory for holding user-specific data files, or NULL on
	error. Caller must dispose of the result pointer. */

char * xdg_get_cache_home
  (
	bool makedirs
  );
  /* returns the directory for holding user-specific cache files, or NULL on
	error. Caller must dispose of the result pointer. */

char * xdg_config_search_path(void);
  /* returns a string containing the colon-separated list of config directories to search
	(apart from the user area). Caller must dispose of the result pointer. */

char * xdg_data_search_path(void);
  /* returns a string containing the colon-separated list of data directories to search
	(apart from the user area). Caller must dispose of the result pointer. */

typedef int (*xdg_path_component_action)
  (
	const unsigned char * path, /* storage belongs to me, make a copy if you want to keep it */
	size_t path_len, /* length of path string */
	void * arg /* meaning is up to you */
  );
  /* return nonzero to abort the scan */

int xdg_for_each_path_component
  (
	const unsigned char * path,
	size_t path_len,
	xdg_path_component_action action,
	void * actionarg,
	bool forwards /* false to do in reverse */
  );
  /* splits the string path with len path_len at any colon separators, calling
	action for each component found, in forward or reverse order as specified.
	Returns nonzero on error, or if action returned nonzero. */

typedef int (*xdg_item_path_action)
  (
	const char * path, /* a complete expanded pathname */
	void * arg /* meaning is up to you */
  );
  /* return nonzero to abort the scan */

char * xdg_find_first_config_path
  (
	const char * itempath
  );
  /* searches for itempath in all the config directory locations in order of decreasing
	priority, returning the expansion where it is first found, or NULL if not found.
	Caller must dispose of the result pointer. */

int xdg_find_all_config_path
  (
	const char * itempath, /* relative path of item to look for in each directory */
	xdg_item_path_action action,
	void * actionarg,
	bool forwards /* false to do in reverse */
  );
  /* searches for itempath in all the config directory locations, and invokes the
	specified action for each instance found. Returns nonzero on error, or if action
	returned nonzero. */

char * xdg_find_first_data_path
  (
	const char * itempath
  );
  /* searches for itempath in all the data directory locations in order of decreasing
	priority, returning the expansion where it is first found, or NULL if not found.
	Caller must dispose of the result pointer. */

int xdg_find_all_data_path
  (
	const char * itempath, /* relative path of item to look for in each directory */
	xdg_item_path_action action,
	void * actionarg,
	bool forwards /* false to do in reverse */
  );
  /* searches for itempath in all the data directory locations, and invokes the
	specified action for each instance found. Returns nonzero on error, or if action
	returned nonzero. */

char * xdg_find_cache_path
  (
	const char * itempath,
	bool create_if
  );
  /* returns an expansion for itempath in the cache directory area. Caller must
	dispose of the result pointer. */
