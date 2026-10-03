# Standalone CPU RayQuery

This example compiles a Slang compute shader to ordinary C++ and calls it directly from the host.
It does not use slang-rhi, TinyBVH, or a Slang runtime library. Slang is needed only to generate
the C++ source.

The shader renders a Cornell box with red and green walls, two boxes, and a ceiling area light.
It uses five diffuse bounces and explicit light sampling for soft shadows and indirect color
bleeding. The host writes a tone-mapped PPM image using standard C file I/O.

`TriangleScene` implements the CPU prelude's `IRaytracingAccelerationStructure` interface by
scanning 36 triangles in a `for` loop. The shader uses the same `RayQuery` operations as a GPU
shader, including closest-hit queries for shading and accept-first-hit queries for shadows.
The host passes a borrowed pointer to its scene in the shader globals and supplies the group
range to `computeMain`.

The provider keeps the next triangle index in each query, with no mutable shared traversal state
or per-query allocations. It has no instances or procedural geometry.

Build this directory as a separate CMake project, using a source-built `slangc` matching the
prelude headers in this checkout:

```sh
cmake -S examples/cpu-ray-query -B build/cpu-ray-query-example \
    -DSLANGC_EXECUTABLE=/absolute/path/to/slangc -DCMAKE_BUILD_TYPE=Release
cmake --build build/cpu-ray-query-example --config Release
ctest --test-dir build/cpu-ray-query-example -C Release --output-on-failure
```

This example does not change or depend on Slang's top-level build and test configuration.

Run the executable to render at 256 x 256 with 64 samples per pixel by default. Optional
arguments select the output path, square image size, and samples per pixel:

```sh
build/cpu-ray-query-example/cpu-ray-query cornell-box.ppm 512 256
```

With a multi-configuration generator, the executable is in the `Release` subdirectory.
`--test` checks opaque closest hits, non-opaque candidate commits, ignored candidates, abort,
misses, instance masks, ray bounds, accept-first-hit, triangle skipping, and face culling.
It reuses a query and checks that the final candidate expires on the next `Proceed`.
It also renders a small image and checks for finite radiance, both colored walls, and the light.

Alternatively, from this directory on macOS or Linux, use a source-built `slangc` matching the
prelude headers in this checkout:

```sh
slangc shader.slang -target cpp -entry computeMain -o /tmp/cpu-ray-query-shader.cpp
c++ -O2 -std=c++17 -I../../prelude main.cpp /tmp/cpu-ray-query-shader.cpp -o /tmp/cpu-ray-query
/tmp/cpu-ray-query
```

See [the CPU provider contract](../../docs/cpu-target.md#cpu-rayquery-providers) for the fields,
candidate transitions, and lifetime rules a custom provider must implement.
