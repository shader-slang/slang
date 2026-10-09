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

The [Populate sccache workflow](../.github/workflows/sccache-populate.yml)
checks LLVM prebuilts independently of Slang's object caches on its schedule,
manual dispatch, and changes to the LLVM recipe or population implementation on
`master`. It checks the six main CI keys:
Linux GCC x86_64, ARM64, and WASM; macOS Clang ARM64; and Windows MSVC x86_64 and
ARM64. The WASM entry builds host LLVM, preserving the key requested by the WASM
Slang build. Only missing keys start native builders, each with a four-hour
timeout. They stage new archives for an Ubuntu job that authenticates, installs the Cloud
SDK, publishes completed archives, and downloads each public object to check it
matches the staged archive byte for byte. Publication is
restricted to the upstream repository's `master` ref.

Slang cache-warming builds wait for LLVM population to complete, then run only
when their commit-based sccache entries are missing. A complete sccache no longer
skips missing LLVM prebuilts. If an LLVM build fails, successful platforms can
still be published, but the workflow reports incomplete population and does not
start the LLVM-dependent Slang builds. The next scheduled run retries missing keys.

To repair missing prebuilts, a maintainer can select **Populate sccache**
in GitHub Actions and run it on `master`, or use:

```sh
gh workflow run sccache-populate.yml --repo shader-slang/slang --ref master
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

The CI actions use sccache with a local disk backend saved to and restored from
GitHub Actions cache, keyed on compiler, platform, configuration, and commit.
`sccache-populate.yml` warms these caches on master after ensuring the current
LLVM prebuilts are available. Cache misses fall back to ordinary compilation.
