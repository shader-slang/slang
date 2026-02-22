The goal of the capabitilies tests is as follows:

- Verify that features that are supposed to enabled on specific
  backend versions compile successfully
- Verify that features that are not supposed to work trigger slang
  compiler diagnostics accordingly

This verifies that the [require(...)] and static_assert() statements
gating feature enablement are present and match with the
documentation. See, e.g., texture type vs minimum backend version
table in Slang Standard Library Reference / _Texture [1].

These tests are not intended to verify functionality.

An individual feature test is usually executed 3 times per backend:

- Positive simple test that runs on the minimum supported backend version
- Functional test the backend in the loop
- Negative simple test that runs on the version just prior to the supported version.


References:

[1] https://docs.shader-slang.org/en/latest/external/core-module-reference/types/0texture-01/index.html
