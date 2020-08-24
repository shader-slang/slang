// nvapi-include.h
#pragma once

// A helper that makes the NVAPI available across targets

#ifdef GFX_NVAPI
// On windows if we include NVAPI, we must include windows.h first

#   ifdef _WIN32
#       define WIN32_LEAN_AND_MEAN
#       define NOMINMAX
#       include <Windows.h>
#       undef WIN32_LEAN_AND_MEAN
#   undef NOMINMAX
#   endif

#   include <nvapi.h>
#   include <nvShaderExtnEnums.h>

#endif

