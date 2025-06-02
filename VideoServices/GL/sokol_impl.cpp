#define SOKOL_DEBUG
#ifdef __EMSCRIPTEN__
    #define SOKOL_GLES3
#else
#define SOKOL_GLCORE
#endif

#define SOKOL_LOG_IMPL
#include "sokol_log.h"
#define SOKOL_GFX_IMPL
#include "sokol_gfx.h"

#include "imgui.h"
#define SOKOL_IMGUI_NO_SOKOL_APP
#define SOKOL_IMGUI_IMPL
#include "util/sokol_imgui.h"
