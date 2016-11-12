#include <stdio.h>
#include <limits.h>
#include <time.h>
#include "rimraf/rimraf.h"
#include "fs/fs.h"
#include "mkdirp/mkdirp.h"
#include "copy/copy.h"
#include "clib-package/clib-package.h"
#include "cache.h"


#define GET_PKG_CACHE(pkg) char pkg_cache[PATH_MAX]; \
                      cache_path(pkg, pkg_cache);


#ifdef _WIN32
#define BASE_DIR getenv("AppData")
#else
#define BASE_DIR getenv("HOME")
#endif

static char cache_dir[PATH_MAX];
static time_t expiration;


static void cache_path(clib_package_t *pkg, char *path)
{
    sprintf(path, "%s/%s_%s_%s", cache_dir, pkg->author, pkg->name, pkg->version);
}

const char *clib_cache_dir(void)
{
    return cache_dir;
}

int clib_cache_init(time_t exp)
{
    expiration = exp;

    sprintf(cache_dir, "%s/.cache/clib/packages", BASE_DIR);

    if (0 != fs_exists(cache_dir)) {
        return mkdirp(cache_dir, 0700);
    }

    return 0;
}

int clib_cache_has_package(clib_package_t *pkg)
{
    GET_PKG_CACHE(pkg);

    return 0 == fs_exists(pkg_cache);
}

static int is_expired_package(char *pkg_cache)
{
    fs_stats *stat = fs_stat(pkg_cache);

    if (!stat) {
        return -1;
    }

    time_t modified = stat->st_mtime;
    time_t now = time(NULL);
    free(stat);

    return now - modified >= expiration;
}

int clib_cache_is_expired(clib_package_t *pkg)
{
    GET_PKG_CACHE(pkg);

    return is_expired_package(pkg_cache);
}

int clib_cache_save_package(clib_package_t *pkg, char *pkg_dir)
{
    GET_PKG_CACHE(pkg);

    if (0 == fs_exists(pkg_cache)) {
        rimraf(pkg_cache);
    }

    return copy_dir(pkg_dir, pkg_cache);
}

int clib_cache_load_package(clib_package_t *pkg, char *target_dir)
{
    GET_PKG_CACHE(pkg);

    if (-1 == fs_exists(pkg_cache)) {
        return -1;
    }

    if (is_expired_package(pkg_cache)) {
        rimraf(pkg_cache);

        return -2;
    }

    return copy_dir(pkg_cache, target_dir);
}

int clib_cache_delete_package(clib_package_t *pkg)
{
    GET_PKG_CACHE(pkg);

    return rimraf(pkg_cache);
}
