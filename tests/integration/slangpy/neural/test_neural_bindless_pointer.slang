// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// Neural integration smoke test for "bindless pointer" style parameters.
// This uses raw pointer parameters (passed from Python via Buffer.device_address).

import slangpy;
import neural;

int read_int_ptr(int* ptr)
{
    return ptr[0];
}
