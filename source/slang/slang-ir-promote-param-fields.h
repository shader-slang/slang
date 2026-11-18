// slang-ir-promote-param-fields.h
#pragma once

namespace Slang
{
struct IRModule;
class TargetRequest;

/// Rewrite functions that take a large aggregate (`struct` or fixed-size array) `in`
/// parameter but only read a small part of it, so that they take just the fields
/// they read as separate parameters.
///
/// Call sites extract those fields from the argument instead of passing the whole
/// aggregate. This avoids materializing a full copy of the aggregate at every call
/// site on targets where `in` aggregates are later passed through a temporary
/// (see `transformParamsToConstRef`).
///
void promoteParamFields(IRModule* module, TargetRequest* targetReq);

} // namespace Slang
