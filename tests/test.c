#include <stdlib.h>
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

static void test_save(char *author, char *name, char *version, char *pkg_dir)
{
    mu_assert_int_eq(0, clib_cache_save_package(author, name, version, "./deps/copy"));
    mu_assert(1 == clib_cache_has_package(author, name, version), "Package should be cached");
    mu_assert(0 == clib_cache_is_expired_package(author, name, version), "This shouldn't be expired yet");

    assert_cached_dir(pkg_dir, 0);
    assert_cached_files(pkg_dir);
}

static void assert_loaded_files(void)
{
    assert_exists("./copy/copy.c");
    assert_exists("./copy/copy.h");
    assert_exists("./copy/package.json");
}

static void test_load(char *author, char *name, char *version)
{
    mu_assert_int_eq(0, clib_cache_load_package(author, name, version, "./copy"));
    assert_loaded_files();
}

static void test_delete(char *author, char *name, char *version, char *pkg_dir)
{
    mu_assert_int_eq(0, clib_cache_delete_package(author, name, version));
    assert_cached_dir(pkg_dir, -1);
    mu_assert(0 == clib_cache_has_package(author, name, version), "Package should be deleted from cached");
    mu_assert(0 == clib_cache_has_json(author, name, version),
              "Package.json should be deleted from cached");

    mu_assert_int_eq(-1, clib_cache_load_package(author, name, version, "./copy"));
    mu_assert(NULL == clib_cache_read_json(author, name, version),
              "Package.json should be deleted from cached");
}

static void test_expiration(char *author, char *name, char *version)
{
    mu_assert_int_eq(0, clib_cache_save_package(author, name, version, "./deps/copy"));
    sleep(1);

    mu_assert_int_eq(1, clib_cache_is_expired_package(author, name, version));
    mu_assert_int_eq(0, clib_cache_has_package(author, name, version));
    mu_assert_int_eq(-2, clib_cache_load_package(author, name, version, "./copy"));

    mu_assert_int_eq(0, clib_cache_has_package(author, name, version));
    mu_assert_int_eq(-1, clib_cache_load_package(author, name, version, "./copy"));

    mu_assert(0 == clib_cache_has_json(author, name, version), "Package.json should be expired");
    mu_assert(NULL == clib_cache_read_json(author, name, version), "Package.json should be expired");
}

MU_TEST(test_cache)
{
    char *author = "author";
    char *name = "pkg";
    char *version = "1.2.0";
    mu_assert_int_eq(0, clib_cache_init(1));

    char pkg_dir[PATH_MAX];
    sprintf(pkg_dir, "%s/author_pkg_1.2.0", clib_cache_dir());

    test_save(author, name, version, pkg_dir);
    test_load(author, name, version);
    test_delete(author, name, version, pkg_dir);
    test_expiration(author, name, version);

    rimraf("./copy");
}

MU_TEST(test_json_save)
{
    char *cached;

    mu_assert_int_eq(0, clib_cache_has_json("a", "n", "v"));
    mu_assert(NULL == clib_cache_read_json("a", "n", "v"), "Json should be NULL");

    mu_assert_int_eq(2, clib_cache_save_json("a", "n", "v", "{}"));
    mu_assert_int_eq(0, strcmp("{}", cached = clib_cache_read_json("a", "n", "v")));
    free(cached);
    mu_assert_int_eq(1, clib_cache_has_json("a", "n", "v"));

    mu_assert_int_eq(0, clib_cache_delete_json("a", "n", "v"));
    mu_assert_int_eq(0, clib_cache_has_json("a", "n", "v"));
    mu_assert(NULL == clib_cache_read_json("a", "n", "v"), "Json should be NULL");
}

MU_TEST(test_search)
{
    char *cached;

    mu_assert_int_eq(0, clib_cache_has_search());
    mu_assert(NULL == clib_cache_read_search(), "Search cache should be NULL");

    mu_assert_int_eq(13, clib_cache_save_search("<html></html>"));
    mu_assert_int_eq(1, clib_cache_has_search());
    mu_assert_int_eq(0, strcmp("<html></html>", cached = clib_cache_read_search()));
    free(cached);

    mu_assert_int_eq(0, clib_cache_delete_search());
    mu_assert_int_eq(0, clib_cache_has_search());
    mu_assert(NULL == clib_cache_read_search(), "Search cache should be NULL");
}

int main(void)
{
    MU_RUN_TEST(test_cache);
    MU_RUN_TEST(test_json_save);
    MU_RUN_TEST(test_search);

    MU_REPORT();

    return 0;
}
