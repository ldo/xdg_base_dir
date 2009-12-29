/*
	Test program for my xdg_base_dir.[ch] library. Invoke as follows:

	c_try op pathtype path

	where op indicates the operation to perform, viz:
	    read    -- find highest-priority existing file/dir path
	    write   -- create user-specific file path
	    findall -- find all existing file/dir paths
	pathtype indicates what type of path we're dealing with (config, data or cache),
	and path is the file/dir path string.

	Written by Lawrence D'Oliveiro <ldo@geek-central.gen.nz>.
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "xdg_base_dir.h"

int main
  (
	int argc,
	char ** argv
  )
  {
	int status = 0;
	const char * op;
	const char * pathtype;
	const char * itempath;
	const char * result = 0;
	do /*once*/
	  {
		if (argc != 4)
		  {
			fprintf(stderr, "usage: %s read|write|findall config|data|cache path\n", argv[0]);
			status = 1;
			break;
		  } /*if*/
		op = argv[1];
		pathtype = argv[2];
		itempath = argv[3];
		if
		  (
				strcmp(op, "read") != 0 && strcmp(op, "write") != 0 && strcmp(op, "findall") != 0
			||
					strcmp(pathtype, "config") != 0
				&&
					strcmp(pathtype, "data") != 0
				&&
					strcmp(pathtype, "cache") != 0
		  )
		  {
			fprintf(stderr,
				"op must be read, write or findall and pathtype must be config, data or cache\n");
			status = 1;
			break;
		  } /*if*/
		if (strcmp(op, "read") == 0)
		  {
			if (strcmp(pathtype, "config") == 0)
			  {
				result = xdg_find_first_config_path(itempath);
			  }
			else if (strcmp(pathtype, "data") == 0)
			  {
				result = xdg_find_first_data_path(itempath);
			  }
			else if (strcmp(pathtype, "cache") == 0)
			  {
				result = xdg_find_cache_path(itempath, false);
			  } /*if*/
			if (result == 0)
			  {
				fprintf(stderr, "error %d -- %s\n", errno, strerror(errno));
				status = 2;
				break;
			  } /*if*/
			fputs(result, stdout);
			fputs("\n", stdout);
		  }
		else if (strcmp(op, "write") == 0)
		  {
			if (strcmp(pathtype, "config") == 0)
			  {
				result = xdg_get_config_home(true);
			  }
			else if (strcmp(pathtype, "data") == 0)
			  {
				result = xdg_get_data_home(true);
			  }
			else if (strcmp(pathtype, "cache") == 0)
			  {
				result = xdg_get_cache_home(true);
			  } /*if*/
			if (result == 0)
			  {
				fprintf(stderr, "error %d -- %s\n", errno, strerror(errno));
				status = 2;
				break;
			  } /*if*/
			if (xdg_makedirsif(result) != 0)
			  {
				fprintf(stderr, "error %d making dirs -- %s\n", errno, strerror(errno));
				status = 2;
				break;
			  } /*if*/
			  {
				const size_t total_len = strlen(result) + 1 + strlen(itempath) + 1;
				char * const new_result = malloc(total_len);
				strncpy(new_result, result, total_len);
				strncat(new_result, "/", total_len);
				strncat(new_result, itempath, total_len);
				free((void *)result);
				result = new_result;
			  }
			  {
				FILE * const out = fopen(result, "w");
				if (out != 0)
				  {
					fclose(out);
				  }
				else
				  {
					fprintf(stderr, "error %d opening -- %s\n", errno, strerror(errno));
					status = 2;
					break;
				  } /*if*/
			  }
			fputs(result, stdout);
			fputs("\n", stdout);
		  }
		else if (strcmp(op, "findall") == 0)
		  {
			int another_item
			  (
				const char * path,
				void * unused
			  )
			  {
				fprintf(stdout, "* %s\n", path);
				return
					0;
			  } /*another_item*/;
			if (strcmp(pathtype, "config") == 0)
			  {
				status = xdg_find_all_config_path
				  (
					/*itempath =*/ itempath,
					/*action =*/ another_item,
					/*actionarg =*/ 0,
					/*forwards =*/ true
				  );
				if (status != 0)
				  {
					fprintf(stderr, "error %d -- %s\n", errno, strerror(errno));
					break;
				  } /*if*/
			  }
			else if (strcmp(pathtype, "data") == 0)
			  {
				status = xdg_find_all_data_path
				  (
					/*itempath =*/ itempath,
					/*action =*/ another_item,
					/*actionarg =*/ 0,
					/*forwards =*/ true
				  );
				if (status != 0)
				  {
					fprintf(stderr, "error %d -- %s\n", errno, strerror(errno));
					break;
				  } /*if*/
			  }
			else if (strcmp(pathtype, "cache") == 0)
			  {
				result = xdg_find_cache_path(itempath, true);
				if (result == 0)
				  {
					fprintf(stderr, "error %d -- %s\n", errno, strerror(errno));
					status = 2;
					break;
				  } /*if*/
			  } /*if*/
		  } /*if*/
	  }
	while (false);
	free((void *)result);
	return
		status;
  } /*main*/
