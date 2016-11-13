#include <stdio.h>
#include <limits.h>
#include <unistd.h>
#include <time.h>
#include "rimraf/rimraf.h"
#include "fs/fs.h"
#include "copy/copy.h"
#include "clib-package/clib-package.h"
#include "cache.h"


#define GET_PKG_CACHE(pkg) char pkg_cache[PATH_MAX]; \
                      package_cache_path(pkg_cache, pkg);

#define GET_BASE_PKG_CACHE(a, n, v) char pkg_cache[PATH_MAX]; \
                  base_package_cache_path(pkg_cache, a, n, v);

#define GET_JSON_CACHE(a, n, v) char json_cache[PATH_MAX]; \
                  json_cache_path(json_cache, a, n, v);

#ifdef _WIN32
#define BASE_DIR getenv("AppData")
#else
#define BASE_DIR getenv("HOME")
#endif


#define BASE_CACHE_PATTERN "%s/%s_%s_%s"
static char cache_dir[PATH_MAX];
static char search_cache[PATH_MAX];
static time_t expiration;


static void json_cache_path(char *pkg_cache, char *author, char *name, char *version)
{
    sprintf(pkg_cache, BASE_CACHE_PATTERN"/package.json", cache_dir, author, name, version);
}

static void base_package_cache_path(char *cache, char *author, char *name, char *version)
{
    sprintf(cache, BASE_CACHE_PATTERN, cache_dir, author, name, version);
}

static void package_cache_path(char *json_cache, clib_package_t *pkg)
{
    sprintf(json_cache, BASE_CACHE_PATTERN, cache_dir, pkg->author, pkg->name, pkg->version);
}

const char *clib_cache_dir(void)
{
    return cache_dir;
}

int clib_cache_init(time_t exp)
{
    expiration = exp;

    sprintf(cache_dir, "%s/.cache/clib/packages", BASE_DIR);
    sprintf(search_cache, "%s/search.html", cache_dir);

    if (0 != fs_exists(cache_dir)) {
        return mkdirp(cache_dir, 0700);
    }

    return 0;
}

static int is_expired(char *cache)
{
    fs_stats *stat = fs_stat(cache);

    if (!stat) {
        return -1;
    }

    time_t modified = stat->st_mtime;
    time_t now = time(NULL);
    free(stat);

    return now - modified >= expiration;
}

int clib_cache_has_json(char *author, char *name, char *version)
{
    GET_JSON_CACHE(author, name, version);

    return 0 == fs_exists(json_cache) && !is_expired(json_cache);
}

char *clib_cache_read_json(char *author, char *name, char *version)
{
    GET_JSON_CACHE(author, name, version);

    if (is_expired(json_cache)){
        return NULL;
    }

    return fs_read(json_cache);
}

int clib_cache_save_json(char *author, char *name, char *version, char *content)
{
    int err = 0;
    GET_BASE_PKG_CACHE(author, name, version);
    GET_JSON_CACHE(author, name, version);

    if (-1 == fs_exists(pkg_cache)) {
        if (0 != (err = fs_mkdir(pkg_cache, 0700))) {
            return err;
        }
    }
    return fs_write(json_cache, content);
}

int clib_cache_delete_json(char *author, char *name, char *version)
{
    GET_JSON_CACHE(author, name, version);

    return unlink(json_cache);
}

int clib_cache_has_search(void)
{
    return 0 == fs_exists(search_cache);
}

char *clib_cache_read_search(void)
{
    if (!clib_cache_has_search()){
        return NULL;
    }
    return fs_read(search_cache);
}

int clib_cache_save_search(char *content)
{
    return fs_write(search_cache, content);
}

int clib_cache_delete_search(void)
{
    return unlink(search_cache);
}

int clib_cache_has_package(clib_package_t *pkg)
{
    GET_PKG_CACHE(pkg);

    return 0 == fs_exists(pkg_cache) && !is_expired(pkg_cache);
}

int clib_cache_is_expired_package(clib_package_t *pkg)
{
    GET_PKG_CACHE(pkg);

    return is_expired(pkg_cache);
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

    if (is_expired(pkg_cache)) {
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
