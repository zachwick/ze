/**
 * @file util.c
 * @brief Miscellaneous utility implementations.
 * @ingroup util
 */
#include <dirent.h>

/**
 * @brief Scandir selector that accepts all entries.
 * @ingroup util
 *
 * Ignores its parameter and returns 1 to include every directory entry when
 * used with scandir().
 *
 * @param[in] empty Unused dirent pointer.
 * @return 1 always.
 */
int _true(const struct dirent *empty) {
  (void)empty;
  return 1;
}


