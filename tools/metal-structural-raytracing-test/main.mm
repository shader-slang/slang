#include "metal-test-host.h"

#include <cstdio>

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::fprintf(stderr, "usage: %s <generated-metal-directory>\n", argv[0]);
        return 2;
    }

    return runMetalStructuralRayTracingTests(argv[1]) ? 0 : 1;
}
