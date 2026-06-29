// `slang.h` is included unconditionally and *outside* this header's own include
// guard (`SLANG_REFLECTION_H`) on purpose, because the two headers are mutually
// dependent: the prototypes below need `slang.h`'s opaque reflection types and
// `SLANG_API`, while `slang.h`'s C++ reflection wrappers need these prototypes.
//
// `slang.h` includes this header from the middle of itself — after its
// reflection types are defined but before its wrapper classes — so letting
// `slang.h` drive the include order keeps the prototypes visible to those
// wrappers no matter which of the two headers a translation unit includes first:
//
//   * `slang.h`-first: `slang.h` reaches its `#include "slang-reflection.h"`,
//     this header is entered for the first time, and the prototypes are declared
//     ahead of the wrappers that follow.
//   * `slang-reflection.h`-first: this `#include "slang.h"` runs before our guard
//     is defined, so `slang.h` is processed top-to-bottom and re-enters this
//     header at its own include site; `SLANG_REFLECTION_H` is still undefined at
//     that point, so the prototypes are declared there. Control then returns here
//     with the guard now set, making the rest of this file a no-op.
//
// Recursion terminates because `slang.h` has its own `#ifndef SLANG_H` guard: the
// nested `#include "slang.h"` above is a no-op once `slang.h` is mid-inclusion.
#include "slang.h"

#ifndef SLANG_REFLECTION_H
#define SLANG_REFLECTION_H

/* SLANG REFLECTION API

This header declares the C functions that back Slang's reflection system: the
`spGetReflection` entry point and the `spReflection*` family. These are the C
entry points that the C++ reflection wrapper types in `slang.h` (such as
`slang::TypeReflection`, `slang::TypeLayoutReflection`, and `slang::ProgramLayout`)
call into.

These declarations are NOT deprecated. They previously lived in
`slang-deprecated.h`, which forced `slang.h` to include the deprecated header
purely to compile its own reflection wrappers. Moving the reflection C-API into
its own non-deprecated header lets `slang.h`'s reflection wrappers depend on
active API surface rather than on the deprecated header.
*/

#ifdef __cplusplus
extern "C"
{
#endif

    // get reflection data from a compilation request
    SLANG_API SlangReflection* spGetReflection(SlangCompileRequest* request);

    // User Attribute
    SLANG_API char const* spReflectionUserAttribute_GetName(SlangReflectionUserAttribute* attrib);
    SLANG_API unsigned int spReflectionUserAttribute_GetArgumentCount(
        SlangReflectionUserAttribute* attrib);
    SLANG_API SlangReflectionType* spReflectionUserAttribute_GetArgumentType(
        SlangReflectionUserAttribute* attrib,
        unsigned int index);
    SLANG_API SlangResult spReflectionUserAttribute_GetArgumentValueInt(
        SlangReflectionUserAttribute* attrib,
        unsigned int index,
        int* rs);
    SLANG_API SlangResult spReflectionUserAttribute_GetArgumentValueFloat(
        SlangReflectionUserAttribute* attrib,
        unsigned int index,
        float* rs);

    /** Returns the string-typed value of a user attribute argument
        The string returned is not null-terminated. The length of the string is returned via
       `outSize`. If index of out of range, or if the specified argument is not a string, the
       function will return nullptr.
    */
    SLANG_API const char* spReflectionUserAttribute_GetArgumentValueString(
        SlangReflectionUserAttribute* attrib,
        unsigned int index,
        size_t* outSize);

    // Type Reflection

    SLANG_API SlangTypeKind spReflectionType_GetKind(SlangReflectionType* type);
    SLANG_API unsigned int spReflectionType_GetUserAttributeCount(SlangReflectionType* type);
    SLANG_API SlangReflectionUserAttribute* spReflectionType_GetUserAttribute(
        SlangReflectionType* type,
        unsigned int index);
    SLANG_API SlangReflectionUserAttribute* spReflectionType_FindUserAttributeByName(
        SlangReflectionType* type,
        char const* name);
    SLANG_API SlangReflectionType* spReflectionType_applySpecializations(
        SlangReflectionType* type,
        SlangReflectionGeneric* generic);

    SLANG_API unsigned int spReflectionType_GetFieldCount(SlangReflectionType* type);
    SLANG_API SlangReflectionVariable* spReflectionType_GetFieldByIndex(
        SlangReflectionType* type,
        unsigned index);

    /** Returns the number of elements in the given type.

    This operation is valid for vector and array types. For other types it returns zero.

    When invoked on an unbounded-size array it will return `SLANG_UNBOUNDED_SIZE`,
    which is defined to be `~size_t(0)`.

    If the size of a type cannot be statically computed, perhaps because it depends on
    a generic parameter that has not been bound to a specific value, this function returns zero.

    Use spReflectionType_GetSpecializedElementCount if the size is dependent on
    a link time constant
    */
    SLANG_API size_t spReflectionType_GetElementCount(SlangReflectionType* type);

    /** The same as spReflectionType_GetElementCount except it takes into account specialization
     * information from the given reflection info
     */
    SLANG_API size_t spReflectionType_GetSpecializedElementCount(
        SlangReflectionType* type,
        SlangReflection* reflection);

    SLANG_API SlangReflectionType* spReflectionType_GetElementType(SlangReflectionType* type);

    SLANG_API unsigned int spReflectionType_GetRowCount(SlangReflectionType* type);
    SLANG_API unsigned int spReflectionType_GetColumnCount(SlangReflectionType* type);
    SLANG_API SlangScalarType spReflectionType_GetScalarType(SlangReflectionType* type);

    SLANG_API SlangResourceShape spReflectionType_GetResourceShape(SlangReflectionType* type);
    SLANG_API SlangResourceAccess spReflectionType_GetResourceAccess(SlangReflectionType* type);
    SLANG_API SlangReflectionType* spReflectionType_GetResourceResultType(
        SlangReflectionType* type);

    SLANG_API char const* spReflectionType_GetName(SlangReflectionType* type);
    SLANG_API SlangResult
    spReflectionType_GetFullName(SlangReflectionType* type, ISlangBlob** outNameBlob);
    SLANG_API SlangReflectionGeneric* spReflectionType_GetGenericContainer(
        SlangReflectionType* type);

    // Type Layout Reflection

    SLANG_API SlangReflectionType* spReflectionTypeLayout_GetType(SlangReflectionTypeLayout* type);
    SLANG_API SlangTypeKind spReflectionTypeLayout_getKind(SlangReflectionTypeLayout* type);
    /** Get the size of a type layout in the specified parameter category.
     *
     * Returns `SLANG_UNBOUNDED_SIZE` for unbounded resources (e.g., unsized arrays).
     * Returns `SLANG_UNKNOWN_SIZE` when the size depends on unresolved generic parameters or
     * link-time constants.
     */
    SLANG_API size_t spReflectionTypeLayout_GetSize(
        SlangReflectionTypeLayout* type,
        SlangParameterCategory category);

    /** Get the stride of a type layout in the specified parameter category.
     *
     * Returns `SLANG_UNBOUNDED_SIZE` for unbounded resources.
     * Returns `SLANG_UNKNOWN_SIZE` when stride depends on unresolved generic parameters or
     * link-time constants.
     */
    SLANG_API size_t spReflectionTypeLayout_GetStride(
        SlangReflectionTypeLayout* type,
        SlangParameterCategory category);
    SLANG_API int32_t spReflectionTypeLayout_getAlignment(
        SlangReflectionTypeLayout* type,
        SlangParameterCategory category);

    SLANG_API uint32_t spReflectionTypeLayout_GetFieldCount(SlangReflectionTypeLayout* type);
    SLANG_API SlangReflectionVariableLayout* spReflectionTypeLayout_GetFieldByIndex(
        SlangReflectionTypeLayout* type,
        unsigned index);

    SLANG_API SlangInt spReflectionTypeLayout_findFieldIndexByName(
        SlangReflectionTypeLayout* typeLayout,
        const char* nameBegin,
        const char* nameEnd);

    SLANG_API SlangReflectionVariableLayout* spReflectionTypeLayout_GetExplicitCounter(
        SlangReflectionTypeLayout* typeLayout);

    /** Get the stride between elements of an array type layout.
     *
     * Returns `SLANG_UNBOUNDED_SIZE` for unbounded resources.
     * Returns `SLANG_UNKNOWN_SIZE` when element stride depends on unresolved generic parameters or
     * link-time constants.
     */
    SLANG_API size_t spReflectionTypeLayout_GetElementStride(
        SlangReflectionTypeLayout* type,
        SlangParameterCategory category);
    SLANG_API SlangReflectionTypeLayout* spReflectionTypeLayout_GetElementTypeLayout(
        SlangReflectionTypeLayout* type);
    SLANG_API SlangReflectionVariableLayout* spReflectionTypeLayout_GetElementVarLayout(
        SlangReflectionTypeLayout* type);
    SLANG_API SlangReflectionVariableLayout* spReflectionTypeLayout_getContainerVarLayout(
        SlangReflectionTypeLayout* type);

    SLANG_API SlangParameterCategory
    spReflectionTypeLayout_GetParameterCategory(SlangReflectionTypeLayout* type);

    SLANG_API unsigned spReflectionTypeLayout_GetCategoryCount(SlangReflectionTypeLayout* type);
    SLANG_API SlangParameterCategory
    spReflectionTypeLayout_GetCategoryByIndex(SlangReflectionTypeLayout* type, unsigned index);

    SLANG_API SlangMatrixLayoutMode
    spReflectionTypeLayout_GetMatrixLayoutMode(SlangReflectionTypeLayout* type);

    SLANG_API int spReflectionTypeLayout_getGenericParamIndex(SlangReflectionTypeLayout* type);

    SLANG_API SlangReflectionTypeLayout* spReflectionTypeLayout_getPendingDataTypeLayout(
        SlangReflectionTypeLayout* type);

    SLANG_API SlangReflectionVariableLayout*
    spReflectionTypeLayout_getSpecializedTypePendingDataVarLayout(SlangReflectionTypeLayout* type);
    SLANG_API SlangInt spReflectionType_getSpecializedTypeArgCount(SlangReflectionType* type);
    SLANG_API SlangReflectionType* spReflectionType_getSpecializedTypeArgType(
        SlangReflectionType* type,
        SlangInt index);

    SLANG_API SlangInt
    spReflectionTypeLayout_getBindingRangeCount(SlangReflectionTypeLayout* typeLayout);
    SLANG_API SlangBindingType spReflectionTypeLayout_getBindingRangeType(
        SlangReflectionTypeLayout* typeLayout,
        SlangInt index);
    SLANG_API SlangInt spReflectionTypeLayout_isBindingRangeSpecializable(
        SlangReflectionTypeLayout* typeLayout,
        SlangInt index);
    /** Get the binding count for a binding range at the specified index.
     *
     * Returns `SLANG_UNBOUNDED_SIZE` for unbounded resources.
     * Returns `SLANG_UNKNOWN_SIZE` when the count depends on unresolved generic parameters or
     * link-time constants.
     */
    SLANG_API SlangInt spReflectionTypeLayout_getBindingRangeBindingCount(
        SlangReflectionTypeLayout* typeLayout,
        SlangInt index);
    SLANG_API SlangReflectionTypeLayout* spReflectionTypeLayout_getBindingRangeLeafTypeLayout(
        SlangReflectionTypeLayout* typeLayout,
        SlangInt index);
    SLANG_API SlangReflectionVariable* spReflectionTypeLayout_getBindingRangeLeafVariable(
        SlangReflectionTypeLayout* typeLayout,
        SlangInt index);
    SLANG_API SlangImageFormat spReflectionTypeLayout_getBindingRangeImageFormat(
        SlangReflectionTypeLayout* typeLayout,
        SlangInt index);
    SLANG_API SlangInt spReflectionTypeLayout_getFieldBindingRangeOffset(
        SlangReflectionTypeLayout* typeLayout,
        SlangInt fieldIndex);
    SLANG_API SlangInt spReflectionTypeLayout_getExplicitCounterBindingRangeOffset(
        SlangReflectionTypeLayout* inTypeLayout);

    SLANG_API SlangInt spReflectionTypeLayout_getBindingRangeDescriptorSetIndex(
        SlangReflectionTypeLayout* typeLayout,
        SlangInt index);
    SLANG_API SlangInt spReflectionTypeLayout_getBindingRangeFirstDescriptorRangeIndex(
        SlangReflectionTypeLayout* typeLayout,
        SlangInt index);
    SLANG_API SlangInt spReflectionTypeLayout_getBindingRangeDescriptorRangeCount(
        SlangReflectionTypeLayout* typeLayout,
        SlangInt index);

    SLANG_API SlangInt
    spReflectionTypeLayout_getDescriptorSetCount(SlangReflectionTypeLayout* typeLayout);
    SLANG_API SlangInt spReflectionTypeLayout_getDescriptorSetSpaceOffset(
        SlangReflectionTypeLayout* typeLayout,
        SlangInt setIndex);
    SLANG_API SlangInt spReflectionTypeLayout_getDescriptorSetDescriptorRangeCount(
        SlangReflectionTypeLayout* typeLayout,
        SlangInt setIndex);
    /** Get the index offset for a descriptor range within a descriptor set.
     *
     * Returns `SLANG_UNKNOWN_SIZE` when the offset depends on unresolved generic parameters or
     * link-time constants.
     */
    SLANG_API SlangInt spReflectionTypeLayout_getDescriptorSetDescriptorRangeIndexOffset(
        SlangReflectionTypeLayout* typeLayout,
        SlangInt setIndex,
        SlangInt rangeIndex);

    /** Get the descriptor count for a descriptor range within a descriptor set.
     *
     * Returns `SLANG_UNBOUNDED_SIZE` for unbounded resources.
     * Returns `SLANG_UNKNOWN_SIZE` when the count depends on unresolved generic parameters or
     * link-time constants.
     */
    SLANG_API SlangInt spReflectionTypeLayout_getDescriptorSetDescriptorRangeDescriptorCount(
        SlangReflectionTypeLayout* typeLayout,
        SlangInt setIndex,
        SlangInt rangeIndex);
    SLANG_API SlangBindingType spReflectionTypeLayout_getDescriptorSetDescriptorRangeType(
        SlangReflectionTypeLayout* typeLayout,
        SlangInt setIndex,
        SlangInt rangeIndex);
    SLANG_API SlangParameterCategory spReflectionTypeLayout_getDescriptorSetDescriptorRangeCategory(
        SlangReflectionTypeLayout* typeLayout,
        SlangInt setIndex,
        SlangInt rangeIndex);

    SLANG_API SlangInt
    spReflectionTypeLayout_getSubObjectRangeCount(SlangReflectionTypeLayout* typeLayout);
    SLANG_API SlangInt spReflectionTypeLayout_getSubObjectRangeBindingRangeIndex(
        SlangReflectionTypeLayout* typeLayout,
        SlangInt subObjectRangeIndex);
    /** Get the space offset for a sub-object range.
     *
     * Returns `SLANG_UNKNOWN_SIZE` when the offset depends on unresolved generic parameters or
     * link-time constants.
     */
    SLANG_API SlangInt spReflectionTypeLayout_getSubObjectRangeSpaceOffset(
        SlangReflectionTypeLayout* typeLayout,
        SlangInt subObjectRangeIndex);
    SLANG_API SlangReflectionVariableLayout* spReflectionTypeLayout_getSubObjectRangeOffset(
        SlangReflectionTypeLayout* typeLayout,
        SlangInt subObjectRangeIndex);

// These declarations are intentionally disabled (`#if 0`) and are preserved
// verbatim from their original home in `slang-deprecated.h`; this header's move
// does not revive or remove them.
#if 0
    SLANG_API SlangInt spReflectionTypeLayout_getSubObjectRangeCount(SlangReflectionTypeLayout* typeLayout);
    SLANG_API SlangInt spReflectionTypeLayout_getSubObjectRangeObjectCount(SlangReflectionTypeLayout* typeLayout, SlangInt index);
    SLANG_API SlangInt spReflectionTypeLayout_getSubObjectRangeBindingRangeIndex(SlangReflectionTypeLayout* typeLayout, SlangInt index);
    SLANG_API SlangReflectionTypeLayout* spReflectionTypeLayout_getSubObjectRangeTypeLayout(SlangReflectionTypeLayout* typeLayout, SlangInt index);

    SLANG_API SlangInt spReflectionTypeLayout_getSubObjectRangeDescriptorRangeCount(SlangReflectionTypeLayout* typeLayout, SlangInt subObjectRangeIndex);
    SLANG_API SlangBindingType spReflectionTypeLayout_getSubObjectRangeDescriptorRangeBindingType(SlangReflectionTypeLayout* typeLayout, SlangInt subObjectRangeIndex, SlangInt bindingRangeIndexInSubObject);
    SLANG_API SlangInt spReflectionTypeLayout_getSubObjectRangeDescriptorRangeBindingCount(SlangReflectionTypeLayout* typeLayout, SlangInt subObjectRangeIndex, SlangInt bindingRangeIndexInSubObject);
    SLANG_API SlangInt spReflectionTypeLayout_getSubObjectRangeDescriptorRangeIndexOffset(SlangReflectionTypeLayout* typeLayout, SlangInt subObjectRangeIndex, SlangInt bindingRangeIndexInSubObject);
    SLANG_API SlangInt spReflectionTypeLayout_getSubObjectRangeDescriptorRangeSpaceOffset(SlangReflectionTypeLayout* typeLayout, SlangInt subObjectRangeIndex, SlangInt bindingRangeIndexInSubObject);
#endif

    // Variable Reflection

    SLANG_API char const* spReflectionVariable_GetName(SlangReflectionVariable* var);
    SLANG_API SlangReflectionType* spReflectionVariable_GetType(SlangReflectionVariable* var);
    SLANG_API SlangReflectionModifier* spReflectionVariable_FindModifier(
        SlangReflectionVariable* var,
        SlangModifierID modifierID);
    SLANG_API unsigned int spReflectionVariable_GetUserAttributeCount(SlangReflectionVariable* var);
    SLANG_API SlangReflectionUserAttribute* spReflectionVariable_GetUserAttribute(
        SlangReflectionVariable* var,
        unsigned int index);
    SLANG_API SlangReflectionUserAttribute* spReflectionVariable_FindUserAttributeByName(
        SlangReflectionVariable* var,
        SlangSession* globalSession,
        char const* name);
    SLANG_API bool spReflectionVariable_HasDefaultValue(SlangReflectionVariable* inVar);
    SLANG_API SlangResult
    spReflectionVariable_GetDefaultValueInt(SlangReflectionVariable* inVar, int64_t* rs);
    SLANG_API SlangResult
    spReflectionVariable_GetDefaultValueFloat(SlangReflectionVariable* inVar, float* rs);
    SLANG_API SlangReflectionGeneric* spReflectionVariable_GetGenericContainer(
        SlangReflectionVariable* var);
    SLANG_API SlangReflectionVariable* spReflectionVariable_applySpecializations(
        SlangReflectionVariable* var,
        SlangReflectionGeneric* generic);

    // Variable Layout Reflection

    SLANG_API SlangReflectionVariable* spReflectionVariableLayout_GetVariable(
        SlangReflectionVariableLayout* var);

    SLANG_API SlangReflectionTypeLayout* spReflectionVariableLayout_GetTypeLayout(
        SlangReflectionVariableLayout* var);

    /** Get the offset of a variable in the specified parameter category.
     *
     * Returns `SLANG_UNKNOWN_SIZE` when the offset depends on unresolved generic parameters or
     * link-time constants.
     */
    SLANG_API size_t spReflectionVariableLayout_GetOffset(
        SlangReflectionVariableLayout* var,
        SlangParameterCategory category);

    /** Get the register space/set of a variable in the specified parameter category.
     *
     * Returns `SLANG_UNKNOWN_SIZE` when the space depends on unresolved generic parameters or
     * link-time constants.
     */
    SLANG_API size_t spReflectionVariableLayout_GetSpace(
        SlangReflectionVariableLayout* var,
        SlangParameterCategory category);
    SLANG_API SlangImageFormat
    spReflectionVariableLayout_GetImageFormat(SlangReflectionVariableLayout* var);

    SLANG_API char const* spReflectionVariableLayout_GetSemanticName(
        SlangReflectionVariableLayout* var);
    SLANG_API size_t
    spReflectionVariableLayout_GetSemanticIndex(SlangReflectionVariableLayout* var);


    // Function Reflection

    SLANG_API SlangReflectionDecl* spReflectionFunction_asDecl(SlangReflectionFunction* func);
    SLANG_API char const* spReflectionFunction_GetName(SlangReflectionFunction* func);
    SLANG_API SlangReflectionModifier* spReflectionFunction_FindModifier(
        SlangReflectionFunction* var,
        SlangModifierID modifierID);
    SLANG_API unsigned int spReflectionFunction_GetUserAttributeCount(
        SlangReflectionFunction* func);
    SLANG_API SlangReflectionUserAttribute* spReflectionFunction_GetUserAttribute(
        SlangReflectionFunction* func,
        unsigned int index);
    SLANG_API SlangReflectionUserAttribute* spReflectionFunction_FindUserAttributeByName(
        SlangReflectionFunction* func,
        SlangSession* globalSession,
        char const* name);
    SLANG_API unsigned int spReflectionFunction_GetParameterCount(SlangReflectionFunction* func);
    SLANG_API SlangReflectionVariable* spReflectionFunction_GetParameter(
        SlangReflectionFunction* func,
        unsigned index);
    SLANG_API SlangReflectionType* spReflectionFunction_GetResultType(
        SlangReflectionFunction* func);
    SLANG_API SlangReflectionGeneric* spReflectionFunction_GetGenericContainer(
        SlangReflectionFunction* func);
    SLANG_API SlangReflectionFunction* spReflectionFunction_applySpecializations(
        SlangReflectionFunction* func,
        SlangReflectionGeneric* generic);
    SLANG_API SlangReflectionFunction* spReflectionFunction_specializeWithArgTypes(
        SlangReflectionFunction* func,
        SlangInt argTypeCount,
        SlangReflectionType* const* argTypes);
    SLANG_API bool spReflectionFunction_isOverloaded(SlangReflectionFunction* func);
    SLANG_API unsigned int spReflectionFunction_getOverloadCount(SlangReflectionFunction* func);
    SLANG_API SlangReflectionFunction* spReflectionFunction_getOverload(
        SlangReflectionFunction* func,
        unsigned int index);

    // Abstract Decl Reflection

    SLANG_API unsigned int spReflectionDecl_getChildrenCount(SlangReflectionDecl* parentDecl);
    SLANG_API SlangReflectionDecl* spReflectionDecl_getChild(
        SlangReflectionDecl* parentDecl,
        unsigned int index);
    SLANG_API char const* spReflectionDecl_getName(SlangReflectionDecl* decl);
    SLANG_API SlangDeclKind spReflectionDecl_getKind(SlangReflectionDecl* decl);
    SLANG_API SlangReflectionFunction* spReflectionDecl_castToFunction(SlangReflectionDecl* decl);
    SLANG_API SlangReflectionVariable* spReflectionDecl_castToVariable(SlangReflectionDecl* decl);
    SLANG_API SlangReflectionGeneric* spReflectionDecl_castToGeneric(SlangReflectionDecl* decl);
    SLANG_API SlangReflectionType* spReflection_getTypeFromDecl(SlangReflectionDecl* decl);
    SLANG_API SlangReflectionDecl* spReflectionDecl_getParent(SlangReflectionDecl* decl);
    SLANG_API SlangReflectionModifier* spReflectionDecl_findModifier(
        SlangReflectionDecl* decl,
        SlangModifierID modifierID);

    // Generic Reflection

    SLANG_API SlangReflectionDecl* spReflectionGeneric_asDecl(SlangReflectionGeneric* generic);
    SLANG_API char const* spReflectionGeneric_GetName(SlangReflectionGeneric* generic);
    SLANG_API unsigned int spReflectionGeneric_GetTypeParameterCount(
        SlangReflectionGeneric* generic);
    SLANG_API SlangReflectionVariable* spReflectionGeneric_GetTypeParameter(
        SlangReflectionGeneric* generic,
        unsigned index);
    SLANG_API unsigned int spReflectionGeneric_GetValueParameterCount(
        SlangReflectionGeneric* generic);
    SLANG_API SlangReflectionVariable* spReflectionGeneric_GetValueParameter(
        SlangReflectionGeneric* generic,
        unsigned index);
    SLANG_API unsigned int spReflectionGeneric_GetTypeParameterConstraintCount(
        SlangReflectionGeneric* generic,
        SlangReflectionVariable* typeParam);
    SLANG_API SlangReflectionType* spReflectionGeneric_GetTypeParameterConstraintType(
        SlangReflectionGeneric* generic,
        SlangReflectionVariable* typeParam,
        unsigned index);
    SLANG_API SlangDeclKind spReflectionGeneric_GetInnerKind(SlangReflectionGeneric* generic);
    SLANG_API SlangReflectionDecl* spReflectionGeneric_GetInnerDecl(
        SlangReflectionGeneric* generic);
    SLANG_API SlangReflectionGeneric* spReflectionGeneric_GetOuterGenericContainer(
        SlangReflectionGeneric* generic);
    SLANG_API SlangReflectionType* spReflectionGeneric_GetConcreteType(
        SlangReflectionGeneric* generic,
        SlangReflectionVariable* typeParam);
    SLANG_API int64_t spReflectionGeneric_GetConcreteIntVal(
        SlangReflectionGeneric* generic,
        SlangReflectionVariable* valueParam);
    SLANG_API SlangReflectionGeneric* spReflectionGeneric_applySpecializations(
        SlangReflectionGeneric* currGeneric,
        SlangReflectionGeneric* generic);


    /** Get the stage that a variable belongs to (if any).

    A variable "belongs" to a specific stage when it is a varying input/output
    parameter either defined as part of the parameter list for an entry
    point *or* at the global scope of a stage-specific GLSL code file (e.g.,
    an `in` parameter in a GLSL `.vs` file belongs to the vertex stage).
    */
    SLANG_API SlangStage spReflectionVariableLayout_getStage(SlangReflectionVariableLayout* var);


    SLANG_API SlangReflectionVariableLayout* spReflectionVariableLayout_getPendingDataLayout(
        SlangReflectionVariableLayout* var);

    // Shader Parameter Reflection

    /** Get the binding index for a shader parameter.
     *
     * Returns `SLANG_UNKNOWN_SIZE` when the index depends on unresolved generic parameters or
     * link-time constants.
     */
    SLANG_API unsigned spReflectionParameter_GetBindingIndex(SlangReflectionParameter* parameter);

    /** Get the binding space for a shader parameter.
     *
     * Returns `SLANG_UNKNOWN_SIZE` when the space depends on unresolved generic parameters or
     * link-time constants.
     */
    SLANG_API unsigned spReflectionParameter_GetBindingSpace(SlangReflectionParameter* parameter);


    // Entry Point Reflection

    SLANG_API char const* spReflectionEntryPoint_getName(SlangReflectionEntryPoint* entryPoint);

    SLANG_API char const* spReflectionEntryPoint_getNameOverride(
        SlangReflectionEntryPoint* entryPoint);

    SLANG_API SlangReflectionFunction* spReflectionEntryPoint_getFunction(
        SlangReflectionEntryPoint* entryPoint);

    SLANG_API unsigned spReflectionEntryPoint_getParameterCount(
        SlangReflectionEntryPoint* entryPoint);

    SLANG_API SlangReflectionVariableLayout* spReflectionEntryPoint_getParameterByIndex(
        SlangReflectionEntryPoint* entryPoint,
        unsigned index);

    SLANG_API SlangStage spReflectionEntryPoint_getStage(SlangReflectionEntryPoint* entryPoint);

    SLANG_API void spReflectionEntryPoint_getComputeThreadGroupSize(
        SlangReflectionEntryPoint* entryPoint,
        SlangUInt axisCount,
        SlangUInt* outSizeAlongAxis);

    SLANG_API void spReflectionEntryPoint_getComputeWaveSize(
        SlangReflectionEntryPoint* entryPoint,
        SlangUInt* outWaveSize);

    SLANG_API int spReflectionEntryPoint_usesAnySampleRateInput(
        SlangReflectionEntryPoint* entryPoint);

    SLANG_API SlangReflectionVariableLayout* spReflectionEntryPoint_getVarLayout(
        SlangReflectionEntryPoint* entryPoint);

    SLANG_API SlangReflectionVariableLayout* spReflectionEntryPoint_getResultVarLayout(
        SlangReflectionEntryPoint* entryPoint);

    SLANG_API int spReflectionEntryPoint_hasDefaultConstantBuffer(
        SlangReflectionEntryPoint* entryPoint);

    // SlangReflectionTypeParameter
    SLANG_API char const* spReflectionTypeParameter_GetName(
        SlangReflectionTypeParameter* typeParam);
    SLANG_API unsigned spReflectionTypeParameter_GetIndex(SlangReflectionTypeParameter* typeParam);
    SLANG_API unsigned spReflectionTypeParameter_GetConstraintCount(
        SlangReflectionTypeParameter* typeParam);
    SLANG_API SlangReflectionType* spReflectionTypeParameter_GetConstraintByIndex(
        SlangReflectionTypeParameter* typeParam,
        unsigned int index);

    // Shader Reflection

    SLANG_API SlangResult spReflection_ToJson(
        SlangReflection* reflection,
        SlangCompileRequest* request,
        ISlangBlob** outBlob);

    SLANG_API unsigned spReflection_GetParameterCount(SlangReflection* reflection);
    SLANG_API SlangReflectionParameter* spReflection_GetParameterByIndex(
        SlangReflection* reflection,
        unsigned index);

    SLANG_API unsigned int spReflection_GetTypeParameterCount(SlangReflection* reflection);
    SLANG_API SlangReflectionTypeParameter* spReflection_GetTypeParameterByIndex(
        SlangReflection* reflection,
        unsigned int index);
    SLANG_API SlangReflectionTypeParameter* spReflection_FindTypeParameter(
        SlangReflection* reflection,
        char const* name);

    SLANG_API SlangReflectionType* spReflection_FindTypeByName(
        SlangReflection* reflection,
        char const* name);
    SLANG_API SlangReflectionTypeLayout* spReflection_GetTypeLayout(
        SlangReflection* reflection,
        SlangReflectionType* reflectionType,
        SlangLayoutRules rules);

    SLANG_API SlangReflectionFunction* spReflection_FindFunctionByName(
        SlangReflection* reflection,
        char const* name);
    SLANG_API SlangReflectionFunction* spReflection_FindFunctionByNameInType(
        SlangReflection* reflection,
        SlangReflectionType* reflType,
        char const* name);
    SLANG_API SlangReflectionVariable* spReflection_FindVarByNameInType(
        SlangReflection* reflection,
        SlangReflectionType* reflType,
        char const* name);
    SLANG_API SlangReflectionFunction* spReflection_TryResolveOverloadedFunction(
        SlangReflection* reflection,
        uint32_t candidateCount,
        SlangReflectionFunction** candidates);

    SLANG_API SlangUInt spReflection_getEntryPointCount(SlangReflection* reflection);
    SLANG_API SlangReflectionEntryPoint* spReflection_getEntryPointByIndex(
        SlangReflection* reflection,
        SlangUInt index);
    SLANG_API SlangReflectionEntryPoint* spReflection_findEntryPointByName(
        SlangReflection* reflection,
        char const* name);

    /** Get the binding index for the global constant buffer.
     *
     * Returns `SLANG_UNKNOWN_SIZE` when the binding depends on unresolved generic parameters or
     * link-time constants.
     */
    SLANG_API SlangUInt spReflection_getGlobalConstantBufferBinding(SlangReflection* reflection);

    /** Get the size of the global constant buffer.
     *
     * Returns `SLANG_UNBOUNDED_SIZE` for unbounded resources.
     * Returns `SLANG_UNKNOWN_SIZE` when the size depends on unresolved generic parameters or
     * link-time constants.
     */
    SLANG_API size_t spReflection_getGlobalConstantBufferSize(SlangReflection* reflection);

    SLANG_API SlangReflectionType* spReflection_specializeType(
        SlangReflection* reflection,
        SlangReflectionType* type,
        SlangInt specializationArgCount,
        SlangReflectionType* const* specializationArgs,
        ISlangBlob** outDiagnostics);

    SLANG_API SlangReflectionGeneric* spReflection_specializeGeneric(
        SlangReflection* inProgramLayout,
        SlangReflectionGeneric* generic,
        SlangInt argCount,
        SlangReflectionGenericArgType const* argTypes,
        SlangReflectionGenericArg const* args,
        ISlangBlob** outDiagnostics);

    SLANG_API bool spReflection_isSubType(
        SlangReflection* reflection,
        SlangReflectionType* subType,
        SlangReflectionType* superType);

    /// Get the number of hashed strings
    SLANG_API SlangUInt spReflection_getHashedStringCount(SlangReflection* reflection);

    /// Get a hashed string. The number of chars is written in outCount.
    /// The count does *NOT* including terminating 0. The returned string will be 0 terminated.
    SLANG_API const char* spReflection_getHashedString(
        SlangReflection* reflection,
        SlangUInt index,
        size_t* outCount);


    /// Get a type layout representing reflection information for the global-scope parameters.
    SLANG_API SlangReflectionTypeLayout* spReflection_getGlobalParamsTypeLayout(
        SlangReflection* reflection);

    /// Get a variable layout representing reflection information for the global-scope parameters.
    SLANG_API SlangReflectionVariableLayout* spReflection_getGlobalParamsVarLayout(
        SlangReflection* reflection);


    /** Get the descriptor set/space index reserved for the bindless resource heap.
     *
     * This is a layout/reflection reservation made before final target lowering and
     * optimization. It can remain non-negative even when the emitted target code no
     * longer uses a bindless heap/resource-handle path. Query `IBindlessResourceMetadata`
     * from target metadata to determine whether such a path survived in the compiled
     * target IR.
     *
     * Returns -1 only when no bindless heap space was reserved for the program layout.
     */
    SLANG_API SlangInt spReflection_getBindlessSpaceIndex(SlangReflection* reflection);

#ifdef __cplusplus
}
#endif

// `spReflection_GetSession` is declared *outside* the `extern "C"` block above
// because it returns a `slang::ISession*` — a C++ type — so it must keep C++
// linkage. It is only available when compiling as C++ (hence the `#ifdef
// __cplusplus`). This split is preserved exactly as it was in `slang-deprecated.h`.
#ifdef __cplusplus
SLANG_API slang::ISession* spReflection_GetSession(SlangReflection* reflection);
#endif

#endif // SLANG_REFLECTION_H
