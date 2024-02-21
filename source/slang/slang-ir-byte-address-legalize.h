// slang-ir-byte-address-legalize.h
#pragma once

namespace Slang
{
class Session;
class TargetProgram;
struct IRModule;

struct ByteAddressBufferLegalizationOptions
{
    bool scalarizeVectorLoadStore = false;
    bool useBitCastFromUInt = false;
    bool translateToStructuredBufferOps = false;
};

    /// Legalize byte-address buffer `Load()` and `Store()` operations.
    ///
    /// This function translates load/store operations that involve
    /// aggregate types into primitive load-store operations on
    /// scalar or vector types.
    ///
void legalizeByteAddressBufferOps(
    Session*                                    session,
    TargetProgram*                              target,
    IRModule*                                   module,
    ByteAddressBufferLegalizationOptions const& options);
}

