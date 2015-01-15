#+
# Implementation of the XDG Base Directory specification
# <http://standards.freedesktop.org/basedir-spec/latest/>
# for Python 3.
#
# The routines in this module can be grouped into the following categories:
# * find all config/data files:
#     find_all_config_path, find_all_data_path
# * find highest-priority config/data file:
#     find_first_config_path, find_first_data_path
# * find location to create user-specific config/data/cache file:
#     get_config_home, get_data_home, get_cache_home, find_cache_path
# * utility:
#     makedirsif
#
# Strategies for dealing with multiple configuration/data files are up to you.
# Common strategies are:
# 1) Look only at the highest-priority config or data file and ignore any others.
# 2) Look at all config/data files, but process them in reverse order of priority
#    and merge the results, so settings in later, higher-priority files override
#    corresponding ones in earlier, lower-priority ones.
#
# Written by Lawrence D'Oliveiro <ldo@geek-central.gen.nz>.
#-

import os
import errno

def makedirsif(path) :
    """creates all the directories in path, if they don't already exist."""
    try :
        os.makedirs(path, 0o700)
    except OSError as Err :
        if Err.errno != errno.EEXIST :
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
    return tuple(os.environ.get("XDG_CONFIG_DIRS", "/etc").split(":"))
      # note spec actually says default should be /etc/xdg, but /etc is the
      # conventional location for system config files.
#end config_search_path

def data_search_path() :
    """returns the list of data directories to search (apart from the user area)."""
    return tuple(os.environ.get("XDG_DATA_DIRS", "/usr/local/share:/usr/share").split(":"))
#end data_search_path

def find_first_config_path(path) :
    """searches for path in all the config directory locations in order of decreasing
    priority, returning the expansion where it is first found, or None if not found."""
    paths_to_try = iter((get_config_home(),) + config_search_path())
        # highest priority first
    while True :
        this_path = next(paths_to_try, None)
        if this_path == None :
            break
        this_path = os.path.join(this_path, path)
        if os.path.exists(this_path) :
            break
    #end while
    return this_path
#end find_first_config_path

def find_all_config_path(path) :
    """searches for path in all the config directory locations, and returns a tuple
    of all instances found in order of increasing priority."""
    result = []
    for this_path in (get_config_home(),) + config_search_path() :
        this_path = os.path.join(this_path, path)
        if os.path.exists(this_path) :
            result.append(this_path)
        #end if
    #end for
    return tuple(result)
#end find_all_config_path

def find_first_data_path(path) :
    """searches for path in all the data directory locations in order of decreasing
    priority, returning the expansion where it is first found, or None if not found."""
    paths_to_try = iter((get_data_home(),) + data_search_path())
        # highest priority first
    while True :
        this_path = next(paths_to_try, None)
        if this_path == None :
            break
        this_path = os.path.join(this_path, path)
        if os.path.exists(this_path) :
            break
    #end while
    return this_path
#end find_first_data_path

def find_all_data_path(path) :
    """searches for path in all the data directory locations, and returns a tuple
    of all instances found in order of increasing priority."""
    result = []
    for this_path in (get_data_home(),) + data_search_path() :
        this_path = os.path.join(this_path, path)
        if os.path.exists(this_path) :
            result.append(this_path)
        #end if
    #end for
    return tuple(result)
#end find_all_data_path

def find_cache_path(path, create_if = False) :
    """returns an expansion for path in the cache directory area."""
    result = os.path.join(get_cache_home(create_if), path)
    if create_if :
        makedirsif(result)
    #end if
    return result
#end find_cache_path
