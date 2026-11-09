// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
import trace_lambda_helpers;

[host]
void main()
{
    forward(() => 2u); forward(() => 3u);
    make<uint>();
    make<float>();
    print(42);
}
