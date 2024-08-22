# Our CI

There are github actions for testing building and testing slang.

## Tests

Most configurations run a restricted set of tests, however on some self hosted runners we run the full test suite, as well as running Falcor's test suite with the new slang build.

## Building LLVM

We require a static build of LLVM for building slang-llvm, we build and cache this in all workflow runs. Since this changes infrequently, the cache is almost always hit. A cold build takes about an hour on the slowest platform. The cached output is a few hundred MB, so conceivably if we add many more platforms we might be caching more than the 10GB github allowance, which would necessitate being a bit more complicated in building and tracking outputs here.

For slang-llvm, this is handled the same as any other dependency, except on Windows Debug builds, where we are required by the differences in Debug/Release standard libraries to always make a release build, this is noted in the ci action yaml file.

## sccache

The CI actions use sccache, keyed on compiler and platform, this runs on all configurations and significantly speeds up small source change builds. This cache can be safely missed without a large impact on build times.

