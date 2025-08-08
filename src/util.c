/**
 * @file util.c
 * @brief Miscellaneous utility implementations.
 * @ingroup util
 */
#include <dirent.h>

int _true(const struct dirent *empty) {
  (void)empty;
  return 1;
}


