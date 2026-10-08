# Our CI

There are github actions for building and testing slang.

## Tests

Most configurations run a restricted set of tests, however on some self hosted
runners we run the full test suite, as well as running Falcor's test suite with
the new slang build.

## Building LLVM

We require a static build of LLVM for building slang-llvm. CI downloads the installed
LLVM libraries from the publicly readable `slang-ci-cache` Google Cloud Storage
bucket. The key is `llvm-{os}-{compiler}-{platform}-{hash}`, where the hash includes
`external/build-llvm.sh` and `external/llvm-*.patch`. Downloads need no credentials,
including for fork PRs. A missing prebuilt falls back to building LLVM from source;
native Windows ARM64 cold builds can take nearly two hours.

The [Populate LLVM prebuilts workflow](../.github/workflows/llvm-populate.yml)
runs when the LLVM build script or patches change on `master`, and when the
population workflow or cache action changes. It checks the six main CI keys:
Linux GCC x86_64, ARM64, and WASM; macOS Clang ARM64; and Windows MSVC x86_64 and
ARM64. The WASM entry builds host LLVM, preserving the key requested by the WASM
Slang build. Each native builder has a four-hour timeout and stages an archive
only on a cache miss. A separate Ubuntu job authenticates, installs the Cloud
SDK, publishes completed archives, and downloads each public object to check it
matches the staged archive byte for byte. Publication is
restricted to the upstream repository's `master` ref.

To repair missing prebuilts, a maintainer can select **Populate LLVM prebuilts**
in GitHub Actions and run it on `master`, or use:

```sh
gh workflow run llvm-populate.yml --repo shader-slang/slang --ref master
```

Check the platform build jobs and the **Publish and verify new prebuilts** step.
The restore step logs the full key and public URL under
`https://storage.googleapis.com/slang-ci-cache/llvm-prebuilts/`. A cache hit
requires no upload. If one platform fails, completed platform archives can still
be published; fix the failed build or upload and dispatch the workflow again.
Rerunning a PR cannot publish a missing prebuilt, because its ref is not `master`.
Recipe changes on a PR can still require a cold build before they reach `master`.

LLVM itself is built in Release mode, including for Windows Debug Slang builds
because of the differences in Debug/Release standard libraries. We cache the
installed build product rather than using sccache for LLVM compilation. See the
[Slang CI LLVM-cache documentation](https://slang.gitlab-master-pages.nvidia.com/slang-ci-docs/caching/llvm-cache)
for the infrastructure details.

## sccache

> Due to reliability issues, we are not currently using sccache, this is
> historical/aspirational.

The CI actions use sccache, keyed on compiler and platform, this runs on all
configurations and significantly speeds up small source change builds. This
cache can be safely missed without a large impact on build times.
