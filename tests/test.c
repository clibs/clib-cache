#include <clib-package/clib-package.h>
#include <limits.h>
#include <string.h>
#include "rimraf/rimraf.h"
#include "minunit/minunit.h"
#include "fs/fs.h"
#include "cache.h"


#define assert_exists(f) mu_assert_int_eq(0, fs_exists(f));


static void assert_cached_dir(char *pkg_dir, int exists)
{
    char *msg = 0 == exists ? "Package dir should exist" : "Package dir should not exist";

    mu_assert(exists == fs_exists(pkg_dir), msg);
}

static void assert_cached_file(char *pkg_dir, char *file)
{
    char path[PATH_MAX];
    sprintf(path, "%s/%s", pkg_dir, file);

    assert_exists(path);
}

static void assert_cached_files(char *pkg_dir)
{
    assert_cached_file(pkg_dir, "copy.c");
    assert_cached_file(pkg_dir, "copy.h");
    assert_cached_file(pkg_dir, "package.json");
}

static void test_save(clib_package_t *pkg, char *pkg_dir)
{
    mu_assert_int_eq(0, clib_cache_save_package(pkg, "./deps/copy"));
    mu_assert(1 == clib_cache_has_package(pkg), "Package should be cached");
    mu_assert(0 == clib_cache_is_expired(pkg), "This shouldn't be expired yet");

    assert_cached_dir(pkg_dir, 0);
    assert_cached_files(pkg_dir);

    mu_assert_int_eq(1, clib_cache_has_json(pkg->author, pkg->name, pkg->version));

    char *json = fs_read("./deps/copy/package.json");
    char *cached = clib_cache_read_json(pkg->author, pkg->name, pkg->version);

    mu_assert(0 == strcmp(json, cached), "Actual and cached package.json are not equal");
}

static void assert_loaded_files(void)
{
    assert_exists("./copy/copy.c");
    assert_exists("./copy/copy.h");
    assert_exists("./copy/package.json");
}

static void test_load(clib_package_t *pkg)
{
    mu_assert_int_eq(0, clib_cache_load_package(pkg, "./copy"));
    assert_loaded_files();
}

static void test_delete(clib_package_t *pkg, char *pkg_dir)
{
    mu_assert_int_eq(0, clib_cache_delete_package(pkg));
    assert_cached_dir(pkg_dir, -1);
    mu_assert(0 == clib_cache_has_package(pkg), "Package should be deleted from cached");
    mu_assert(0 == clib_cache_has_json(pkg->author, pkg->name, pkg->version), "Package.json should be deleted from cached");

    mu_assert_int_eq(-1, clib_cache_load_package(pkg, "./copy"));
    mu_assert(NULL == clib_cache_read_json(pkg->author, pkg->name, pkg->version), "Package.json should be deleted from cached");
}

static void test_expiration(clib_package_t *pkg)
{
    mu_assert_int_eq(0, clib_cache_save_package(pkg, "./deps/copy"));
    sleep(1);

    mu_assert_int_eq(1, clib_cache_is_expired(pkg));
    mu_assert_int_eq(0, clib_cache_has_package(pkg));
    mu_assert_int_eq(-2, clib_cache_load_package(pkg, "./copy"));

    mu_assert_int_eq(0, clib_cache_has_package(pkg));
    mu_assert_int_eq(-1, clib_cache_load_package(pkg, "./copy"));

    mu_assert(0 == clib_cache_has_json(pkg->author, pkg->name, pkg->version), "Package.json should be expired");
    mu_assert(NULL == clib_cache_read_json(pkg->author, pkg->name, pkg->version), "Package.json should be expired");
}

MU_TEST(test_cache)
{
    clib_package_t pkg;
    pkg.author = "author";
    pkg.name = "pkg";
    pkg.version = "1.2.0";
    mu_assert_int_eq(0, clib_cache_init(1));

    char pkg_dir[PATH_MAX];
    sprintf(pkg_dir, "%s/author_pkg_1.2.0", clib_cache_dir());

    test_save(&pkg, pkg_dir);
    test_load(&pkg);
    test_delete(&pkg, pkg_dir);
    test_expiration(&pkg);

    rimraf("./copy");
}

int main(void)
{
    MU_RUN_TEST(test_cache);
    MU_REPORT();

    return 0;
}
