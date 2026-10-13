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
`master`. It checks five main CI keys:
Linux GCC x86_64 and ARM64; macOS Clang ARM64; and Windows MSVC x86_64 and ARM64.
WASM disables LLVM, so its setup does not download or populate an LLVM archive. Only missing keys start native builders, each with a four-hour
timeout. They stage new archives for an Ubuntu job that authenticates, installs the Cloud
SDK, publishes completed archives, and downloads each public object to check it
matches the staged archive byte for byte. Publication is
restricted to the upstream repository's `master` ref.

Slang cache-warming builds run when their commit-based sccache entries are missing
and their own platform's LLVM archive is publicly available after publication. A
complete sccache no longer skips missing LLVM prebuilts. Failed LLVM platforms
still make the separate population report fail, while successfully cached or
published platforms can warm Slang caches. An unrelated platform failure does
not suppress that warming. The next scheduled run retries missing LLVM keys.

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
Release prepares its tools with `common-setup`, then calls `setup-llvm-from-gcs`
with `upload-if-missing: false`. It can download cached LLVM or build it on a miss,
but does not attempt unauthenticated GCS publication.
The population workflow publishes the missing archives independently.

Population verifies the installed LLVM and Clang packages by compiling, linking,
and running a small LLVM consumer before staging an archive. It links the LLVM
and Clang component set used by slang-llvm. Linux x86_64 repeats that check in
both the normal GCC build container and the Clang sanitizer container. The recipe
explicitly disables optional zstd support so the build does not acquire a zstd
dependency from packages on the build host. Other optional dependencies retain
the existing LLVM defaults and are covered by the consumer checks.

Windows x64 LLVM population uses the GCP build pool and explicitly selects MSVC
14.43 to match the existing consumers. A newer producer toolset can introduce
standard-library symbols that their older linker cannot resolve.

LLVM itself is built in Release mode, including for Windows Debug Slang builds
because of the differences in Debug/Release standard libraries. We cache the
installed build product rather than using sccache for LLVM compilation.

## sccache

The CI actions use sccache with a local disk backend saved to and restored from
GitHub Actions cache, keyed on compiler, platform, configuration, and commit.
`sccache-populate.yml` warms these caches on master after ensuring the current
LLVM prebuilts are available. Cache misses fall back to ordinary compilation.
