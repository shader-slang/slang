#include "core/slang-io.h"
#include "slang/slang-known-builtin-decl-name.h"

#include <stdio.h>

using namespace Slang;

/// Expands the ray-tracing module's template using the compiler's builtin declaration IDs.
///
/// For example, changing the position of RayTracingStageContext in KnownBuiltinDeclName updates
/// the generated Slang enum without maintaining another numeric constant in the module source.
static String generateRayTracingBuiltins()
{
    StringBuilder source;
    source << "// Generated from raytracing-builtins.meta.slang; do not edit.\n";
#define SLANG_RAW(TEXT) source << TEXT;
#define SLANG_SPLICE(EXPR) source << (EXPR);
#include "raytracing-builtins.meta.slang.h"
#undef SLANG_SPLICE
#undef SLANG_RAW
    return source.produceString();
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <raytracing-builtins-output.slang>\n", argv[0]);
        return 1;
    }

    if (SLANG_FAILED(File::writeAllText(argv[1], generateRayTracingBuiltins())))
    {
        fprintf(stderr, "unable to write standard module source: %s\n", argv[1]);
        return 1;
    }
    return 0;
}
