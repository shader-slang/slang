## Configure CMake and Build

To configure cmake, run `cmake --preset default --fresh`.
To build, run `cmake --workflow --preset debug` or `cmake --workflow --preset release`.

## Formatting

Your PR needs to be formatted according to our coding style.
Run `./extras/formatting.sh` script to format your changes before creating a PR.

## Labeling your PR

All PRs needs to be labeled as either "pr: non-breaking" or "pr: breaking".
Label your PR as "pr: breaking" if you are introducing public API changes that breaks ABI compabibility,
or you are introducing changes to the Slang language that will cause the compiler to error out on existing Slang code.
It is rare for a PR to be a breaking change.

## Testing

Your PR should include a regression test for the bug you are fixing.
Normally, these tests present as a `.slang` file under `tests/` directory.
You will need to run your test with `slang-test tests/path/to/your-new-test.slang`. 