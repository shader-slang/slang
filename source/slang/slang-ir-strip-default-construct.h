// slang-ir-strip-default-construct.h
#pragma once

namespace Slang
{
struct IRModule;

enum class RawDefaultConstructStoreMode
{
    StripStoreOnlyDefaultConstructs,
    MaterializeStoreOnlyDefaultConstructs,
};

/// Remove raw `kIROp_DefaultConstruct` instructions that are only used as
/// the RHS of `Store` instructions (along with those stores). Any
/// `DefaultConstruct` with non-store uses is re-emitted in materialized form
/// when possible so downstream emitters do not need native support for the
/// raw opcode.
void removeRawDefaultConstructors(
    IRModule* module,
    RawDefaultConstructStoreMode storeMode =
        RawDefaultConstructStoreMode::StripStoreOnlyDefaultConstructs);

} // namespace Slang
