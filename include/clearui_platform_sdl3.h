/**
 * SDL3 platform adapter for ClearUI.
 * Link with cui_platform_sdl3.c and SDL3 when building with WITH_SDL3=1.
 */
#ifndef CLEARUI_PLATFORM_SDL3_H
#define CLEARUI_PLATFORM_SDL3_H

#include "clearui_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

const cui_platform *cui_platform_sdl3_get(void);

#ifdef __cplusplus
}
#endif

#endif /* CLEARUI_PLATFORM_SDL3_H */
