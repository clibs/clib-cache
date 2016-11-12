#ifndef CLIB_CACHE_H
#define CLIB_CACHE_H


#include <stdbool.h>
#include <stdint.h>
#include "clib-package/clib-package.h"


/**
 * Internal setup, creates the base cache dir if necessary
 *
 * @param expiration Cache expiration in seconds
 *
 * @return 0 on success, -1 otherwise
 */
int clib_cache_init(time_t expiration);

/**
 * @return The base base dir
 */
const char *clib_cache_dir(void);

/**
 * At this point the package object is not built yet, and can't rely on it
 *
 * @return 0/1 is the package is cached
 */
int clib_cache_has_json(char *author, char *name, char *version);

/**
 * @return The content of the cached package's package.json, or NULL on error
 */
char *clib_cache_read_json(char *author, char *name, char *version);

/**
 * @return 0/1 if the packe is cached
 */
int clib_cache_has_package(clib_package_t *pkg);

/**
 * @return 0/1 if the cached package modified date is more or less then the given expiration.
 *         -1 if the package is not cached
 */
int clib_cache_is_expired(clib_package_t *pkg);

/**
 * @param pkg_dir The downloaded package (e.g. ./deps/my_package).
 *                If the package was already cached, it will be deleted first, then saved
 *
 * @return 0 on success, -1 on error
 */
int clib_cache_save_package(clib_package_t *pkg, char *pkg_dir);

/**
 * @param target_dir Where the cached package should be copied
 *
 * @return 0 on success, -1 on error, if the package is not found in the cache.
 *         If the cached package is expired, it will be deleted, and -2 returned
 */
int clib_cache_load_package(clib_package_t *pkg, char *target_dir);

/**
 * @return 0 on success, -1 on error
 */
int clib_cache_delete_package(clib_package_t *pkg);


#endif
