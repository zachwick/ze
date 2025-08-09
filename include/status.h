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

void editorSetStatusMessage(const char *fmt, ...);
void scmEditorSetStatusMessage(SCM message);

/** @} */


