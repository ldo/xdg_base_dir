#+
# Implementation of the XDG Base Directory specification
# <http://standards.freedesktop.org/basedir-spec/latest/>.
#
# Written by Lawrence D'Oliveiro <ldo@geek-central.gen.nz>.
#-

import os
import errno

def makedirsif(path) :
	"""creates all the directories in path, if they don't already exist."""
	try :
		os.makedirs(path, 0700)
	except OSError, (ErrNo, Msg) :
		if ErrNo != errno.EEXIST :
			raise
		#end if
	#end try
#end makedirsif

def get_config_home(makedirs = False) :
	"""returns the directory for holding user-specific config files."""
	result = os.environ.get("XDG_CONFIG_HOME")
	if result == None :
		result = os.path.join(os.environ["HOME"], ".config")
	#end if
	if makedirs :
		makedirsif(result)
	#end if
	return result
#end get_config_home

def get_data_home(makedirs = False) :
	"""returns the directory for holding user-specific data files."""
	result = os.environ.get("XDG_DATA_HOME")
	if result == None :
		result = os.path.join(os.environ["HOME"], ".local/share")
	#end if
	if makedirs :
		makedirsif(result)
	#end if
	return result
#end get_data_home

def get_cache_home(makedirs = False) :
	"""returns the directory for holding user-specific cache files."""
	result = os.environ.get("XDG_CACHE_HOME")
	if result == None :
		result = os.path.join(os.environ["HOME"], ".cache")
	#end if
	if makedirs :
		makedirsif(result)
	#end if
	return result
#end get_cache_home

def config_search_path() :
	"""returns the list of config directories to search (apart from the user area)."""
	return os.environ.get("XDG_CONFIG_DIRS", "/etc").split(":")
	  # note spec actually says default should be /etc/xdg
#end config_search_path

def data_search_path() :
	"""returns the list of data directories to search (apart from the user area)."""
	return os.environ.get("XDG_DATA_DIRS", "/usr/local/share:/usr/share").split(":")
#end data_search_path

def find_config_path(path, create_if = False) :
	"""searches for path in all the config directory locations, returning the
	expansion where it is found, or an expansion for creating it in the per-user
	area if not found."""
	paths_to_try = iter([get_config_home()] + config_search_path())
		# highest priority first
	while True :
		try :
			this_path = paths_to_try.next()
		except StopIteration :
			this_path = os.path.join(get_config_home(create_if), path)
			if create_if :
				makedirsif(this_path)
			break
		#end try
		this_path = os.path.join(this_path, path)
		if os.path.exists(this_path) :
			break
	#end while
	return this_path
#end find_config_path

def find_data_path(path, create_if = False) :
	"""searches for path in all the data directory locations, returning the
	expansion where it is found, or an expansion for creating it in the per-user
	area if not found."""
	paths_to_try = iter([get_data_home()] + data_search_path())
		# highest priority first
	while True :
		try :
			this_path = paths_to_try.next()
		except StopIteration :
			this_path = os.path.join(get_data_home(create_if), path)
			if create_if :
				makedirsif(this_path)
			#end if
			break
		#end try
		this_path = os.path.join(this_path, path)
		if os.path.exists(this_path) :
			break
	#end while
	return this_path
#end find_data_path

def find_cache_path(path, create_if = False) :
	"""returns an expansion for path in the cache directory area."""
	result = os.path.join(get_cache_home(create_if), path)
	if create_if :
		makedirsif(result)
	#end if
	return result
#end find_cache_path
