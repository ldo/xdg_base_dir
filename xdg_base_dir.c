/*
	Implementation of the XDG Base Directory specification
	<http://standards.freedesktop.org/basedir-spec/latest/>.

	Note the use of GNU/GCC-specific extensions: strchrnul, strndup
	and local routine declarations.

	Written by Lawrence D'Oliveiro <ldo@geek-central.gen.nz>.
*/	

#define _GNU_SOURCE
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include "xdg_base_dir.h"

int xdg_makedirsif
  (
	const char * path
  )
  /* creates all the directories in path, if they don't already exist. Returns
	nonzero and sets errno on error. */
  {
	int status;
	const char * pathrest;
	pathrest = path;
	for (;;)
	  {
		const char * const segend = strchrnul(pathrest, '/');
		if (segend > pathrest)
		  {
			const char * const curdir = strndup(path, segend - path);
			if (curdir == 0)
			  {
				status = -1; /* assume errno is set to ENOMEM */
				break;
			  } /*if*/
			status = mkdir(curdir, 0700);
			free((void *)curdir);
			if (status != 0 && errno != EEXIST)
				break;
		  } /*if*/
		if (*segend == '\0')
		  {
			status = 0;
			break;
		  } /*if*/
		pathrest = segend + 1;
	  } /*for*/
	return
		status;
  } /*xdg_makedirsif*/

char * xdg_make_home_relative
  (
	const char * path
  )
  /* prepends the value of $HOME onto path (assumed not to begin with a slash), or
	NULL on error (out of memory, $HOME not defined or not absolute). Caller must dispose
	of the result pointer. */
  {
	const char * const home = getenv("HOME");
	char * result = 0;
	size_t result_len;
	do /*once*/
	  {
		if (home == 0 || home[0] != '/')
		  {
			errno = ENOENT;
			break;
		  } /*if*/
	  /* assert strlen(home) > 0 */
		result_len = strlen(home) + 1 + strlen(path) + 1; /* worst case */
		result = malloc(result_len);
		if (result == 0)
			break;
		strncpy(result, home, result_len);
		if (result[strlen(result) - 1] != '/')
		  {
			strncat(result, "/", result_len);
		  } /*if*/
		strncat(result, path, result_len);
	  }
	while (false);
	return
		result;
  } /*xdg_make_home_relative*/

char * xdg_get_config_home
  (
	bool makedirs
  )
  /* returns the directory for holding user-specific config files, or NULL on
	error. Caller must dispose of the result pointer. */
  {
	char * result;
	void * to_dispose = 0;
	result = getenv("XDG_CONFIG_HOME");
	if (result == 0)
	  {
		to_dispose = xdg_make_home_relative(".config");
		result = to_dispose;
	  } /*if*/
	if (makedirs && xdg_makedirsif(result) != 0)
	  {
		free(to_dispose);
		result = 0;
	  } /*if*/
	if (result != 0 && to_dispose == 0)
	  {
		result = strdup(result);
	  } /*if*/
	return
		result;
  } /*xdg_get_config_home*/

char * xdg_get_data_home
  (
	bool makedirs
  )
  /* returns the directory for holding user-specific data files, or NULL on
	error. Caller must dispose of the result pointer. */
  {
	char * result;
	void * to_dispose = 0;
	result = getenv("XDG_DATA_HOME");
	if (result == 0)
	  {
		to_dispose = xdg_make_home_relative(".local/share");
		result = to_dispose;
	  } /*if*/
	if (makedirs && xdg_makedirsif(result) != 0)
	  {
		free(to_dispose);
		result = 0;
	  } /*if*/
	if (result != 0 && to_dispose == 0)
	  {
		result = strdup(result);
	  } /*if*/
	return
		result;
  } /*xdg_get_data_home*/

char * xdg_get_cache_home
  (
	bool makedirs
  )
  /* returns the directory for holding user-specific cache files, or NULL on
	error. Caller must dispose of the result pointer. */
  {
	char * result;
	void * to_dispose = 0;
	result = getenv("XDG_CACHE_HOME");
	if (result == 0)
	  {
		to_dispose = xdg_make_home_relative(".cache");
		result = to_dispose;
	  } /*if*/
	if (makedirs && xdg_makedirsif(result) != 0)
	  {
		free(to_dispose);
		result = 0;
	  } /*if*/
	if (result != 0 && to_dispose == 0)
	  {
		result = strdup(result);
	  } /*if*/
	return
		result;
  } /*xdg_get_cache_home*/

char * xdg_config_search_path(void)
  /* returns a string containing the colon-separated list of config directories to search
	(apart from the user area). Caller must dispose of the result pointer. */
  {
	char * result = getenv("XDG_CONFIG_DIRS");
	if (result == 0)
	  {
		result = "/etc";
		  /* note spec actually says default should be /etc/xdg, but /etc is the
			conventional location for system config files. */
	  } /*if*/
	return strdup(result);
  } /*xdg_config_search_path*/

char * xdg_data_search_path(void)
  /* returns a string containing the colon-separated list of data directories to search
	(apart from the user area). Caller must dispose of the result pointer. */
  {
	char * result = getenv("XDG_DATA_DIRS");
	if (result == 0)
	  {
		result = "/usr/local/share:/usr/share";
	  } /*if*/
	return
		strdup(result);
  } /*xdg_data_search_path*/

int xdg_for_each_path_component
  (
	const unsigned char * path,
	size_t path_len,
	xdg_path_component_action action,
	void * actionarg,
	bool forwards /* false to do in reverse */
  )
  /* splits the string path with len path_len at any colon separators, calling
	action for each component found, in forward or reverse order as specified.
	Returns nonzero on error, or if action returned nonzero. */
  {
	int status;
	const unsigned char * const path_end = path + path_len;
	const unsigned char * path_prev;
	const unsigned char * path_next;
	if (forwards)
	  {
		path_prev = path;
		for (;;)
		  {
			path_next = path_prev;
			for (;;)
			  {
				if (path_next == path_end)
					break;
				if (*path_next == ':')
					break;
				++path_next;
			  } /*for*/
			status = action(path_prev, path_next - path_prev, actionarg);
			if (status != 0)
				break;
			if (path_next == path_end)
				break;
			path_prev = path_next + 1;
		  } /*for*/
	  }
	else /* backwards */
	  {
		path_next = path_end;
		for (;;)
		  {
			path_prev = path_next;
			for (;;)
			  {
				if (path_prev == path)
					break;
				--path_prev;
				if (*path_prev == ':')
				  {
					++path_prev;
					break;
				  } /*if*/
			  } /*for*/
			status = action(path_prev, path_next - path_prev, actionarg);
			if (status != 0)
				break;
			if (path_prev == path)
				break;
			path_next = path_prev - 1;
		  } /*for*/
	  } /*if*/
	return status;
  } /*xdg_for_each_path_component*/

static int xdg_for_each_found
  (
	const char * itempath, /* relative path of item to look for in each directory */
	bool config, /* true for config, false for data */
	xdg_item_path_action action,
	void * actionarg,
	bool forwards /* false to do in reverse */
  )
  {
	int status = 0;
	int try_component
	  (
		const unsigned char * dirpath,
		size_t dirpath_len,
		void * unused
	  )
	  /* generates the full item path, and passes it to the caller's action
		if it is accessible. */
	  {
		char * thispath;
		const size_t thispath_maxlen = dirpath_len + 1 + strlen(itempath) + 1;
		struct stat statinfo;
		int status = 0;
		do /*once*/
		  {
			thispath = malloc(thispath_maxlen);
			if (thispath == 0)
			  {
				status = -1;
				break;
			  } /*if*/
			memcpy(thispath, dirpath, dirpath_len);
			thispath[dirpath_len] = 0;
			if (dirpath_len != 0 && dirpath[dirpath_len - 1] != '/')
			  {
				strncat(thispath, "/", thispath_maxlen);
			  } /*if*/
			strncat(thispath, itempath, thispath_maxlen);
			if (stat(thispath, &statinfo) == 0)
			  {
				status = action(thispath, actionarg);
			  }
			else
			  {
				errno = 0; /* ignore stat result */
			  } /*if*/
		  }
		while (false);
		free(thispath);
		return
			status;
	  } /*try_component*/;

	const char * const home_path = config ? xdg_get_config_home(false) : xdg_get_data_home(false);
	const char * const search_path = config ? xdg_config_search_path() : xdg_data_search_path();
	do /*once*/
	  {
		if (forwards)
		  {
			status = try_component((const unsigned char *)home_path, strlen(home_path), 0);
			if (status != 0)
				break;
		  } /*if*/
		status = xdg_for_each_path_component
		  (
			/*path =*/ (const unsigned char *)search_path,
			/*path_len =*/ strlen(search_path),
			/*action =*/ (xdg_path_component_action)try_component,
			/*actionarg =*/ 0,
			/*forwards =*/ forwards
		  );
		if (status != 0)
			break;
		if (!forwards)
		  {
			status = try_component((const unsigned char *)home_path, strlen(home_path), 0);
			if (status != 0)
				break;
		  } /*if*/
	  }
	while (false);
	free((void *)home_path);
	free((void *)search_path);
	return
		status;
  } /*xdg_for_each_found*/

static char * xdg_find_first_path
  (
	const char * itempath, /* assumed relative */
	bool config /* true for config, false for data */
  )
  /* common internal routine for both xdg_find_first_config_path and xdg_find_first_data_path. */
  {
	char * result = 0;
	errno = 0;
	int save_item
	  (
		const char * itempath,
		void * unused
	  )
	  {
		result = strdup(itempath);
		return
			result != 0;
	  } /*save_item*/;
	(void)xdg_for_each_found
	  (
		/*itempath =*/ itempath,
		/*config =*/ config,
		/*action =*/ save_item,
		/*actionarg =*/ 0,
		/*forwards =*/ true
	  );
	if (result == 0 && errno == 0)
	  {
		errno = ENOENT;
	  } /*if*/
	return
		result;
  } /*xdg_find_first_path*/

char * xdg_find_first_config_path
  (
	const char * itempath
  )
  /* searches for itempath in all the config directory locations in order of decreasing
	priority, returning the expansion where it is first found, or NULL if not found.
	Caller must dispose of the result pointer. */
  {
	return
		xdg_find_first_path(itempath, true);
  } /*xdg_find_first_config_path*/

int xdg_find_all_config_path
  (
	const char * itempath, /* relative path of item to look for in each directory */
	xdg_item_path_action action,
	void * actionarg,
	bool forwards /* false to do in reverse */
  )
  /* searches for itempath in all the config directory locations, and invokes the
	specified action for each instance found. Returns nonzero on error, or if action
	returned nonzero. */
  {
	return
		xdg_for_each_found
		  (
			/*itempath =*/ itempath,
			/*config =*/ true,
			/*action =*/ action,
			/*actionarg =*/ actionarg,
			/*forwards =*/ forwards
		  );
  } /*xdg_find_all_config_path*/

char * xdg_find_first_data_path
  (
	const char * itempath
  )
  /* searches for itempath in all the data directory locations in order of decreasing
	priority, returning the expansion where it is first found, or NULL if not found.
	Caller must dispose of the result pointer. */
  {
	return
		xdg_find_first_path(itempath, false);
  } /*xdg_find_first_data_path*/

int xdg_find_all_data_path
  (
	const char * itempath, /* relative path of item to look for in each directory */
	xdg_item_path_action action,
	void * actionarg,
	bool forwards /* false to do in reverse */
  )
  /* searches for itempath in all the data directory locations, and invokes the
	specified action for each instance found. Returns nonzero on error, or if action
	returned nonzero. */
  {
	return
		xdg_for_each_found
		  (
			/*itempath =*/ itempath,
			/*config =*/ false,
			/*action =*/ action,
			/*actionarg =*/ actionarg,
			/*forwards =*/ forwards
		  );
  } /*xdg_find_all_data_path*/

char * xdg_find_cache_path
  (
	const char * itempath,
	bool create_if
  )
  /* returns an expansion for itempath in the cache directory area. Caller must
	dispose of the result pointer. */
  {
	const char * const cache_home = xdg_get_cache_home(false);
	char * result = 0;
	if (cache_home != 0)
	  {
		const size_t result_maxlen = strlen(cache_home) + 1 + strlen(itempath) + 1;
		result = malloc(result_maxlen);
		if (result != 0)
		  {
			strncpy(result, cache_home, result_maxlen);
			if (result[0] != '\0' && result[strlen(result) - 1] != '/')
			  {
				strncat(result, "/", result_maxlen);
			  } /*if*/
			strncat(result, itempath, result_maxlen);
			if (create_if && xdg_makedirsif(result) != 0)
			  {
				free(result);
				result = 0;
			  } /*if*/
		  } /*if*/
		free((void *)cache_home);
	  } /*if*/
	return
		result;
  } /*xdg_find_cache_path*/
