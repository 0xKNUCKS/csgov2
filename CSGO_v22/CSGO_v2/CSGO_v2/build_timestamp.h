#pragma once

// This file is force-recompiled every build (pre-build step touches it)
// so __DATE__ / __TIME__ always reflect the actual build time.
inline const char* BUILD_TIMESTAMP = __DATE__ " - " __TIME__;
