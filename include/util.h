/**
 * @file util.h
 * @brief Miscellaneous utilities.
 * @defgroup util Utilities
 * @ingroup core
 * @{
 */
#pragma once

#include <dirent.h>

/** Scandir selector that accepts all entries. */
int _true(const struct dirent *empty);

/** @} */


