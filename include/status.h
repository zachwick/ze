/**
 * @file status.h
 * @brief Status/message bar helpers and Scheme binding.
 * @defgroup status Status bar
 * @ingroup core
 * @{
 */
#pragma once

#include "ze.h"
#include <libguile.h>

/** Set a printf-style status message shown transiently at the bottom. */
void editorSetStatusMessage(const char *fmt, ...);
/** Set the status message from a Scheme string. */
void scmEditorSetStatusMessage(SCM message);

/** @} */


