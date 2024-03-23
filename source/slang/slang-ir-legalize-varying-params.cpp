// slang-ir-legalize-varying-params.cpp
#include "slang-ir-legalize-varying-params.h"

#include "slang-ir-insts.h"

namespace Slang
{

// This pass implements logic to "legalize" the varying parameter
// signature of an entry point.
//
// The traditional Slang/HLSL model is to have varying input parameters
// be marked with "semantics" that can either mark them as user-defined
// or system-value parameters. In addition the result (return value)
// of the function can be marked, and effectively works like an `out`
// parameter.
//
// Other targets have very different models for how varying parameters
// are passed:
//
// * GLSL/SPIR-V declare user-defined varying input/output as global variables,
//   and system-defined varying parameters are available as magic built-in variables.
//
// * CUDA compute kernels expose varying inputs as magic built-in
//   variables like `threadIdx`.
//
// * Our CPU compilation path requires the caller to pass in a `ComputeThreadVaryingInput`
//   that specifies the values of the critical varying parameters for compute shaders.
//
// While these targets differ in how they prefer to represent varying parameters,
// they share the common theme that they cannot work with the varying parameter
// signature of functions as written in vanilla HLSL.
//
// This pass in this file is responsible for walking the parameters (and result)
// of each entry point in an IR module and transforming them into a form that
// is legal for each target. The shared logic deals with many aspects of the
// HLSL/Slang model for varying parameters that need to be "desugared" for these
// targets:
//
// * Slang allows either an `out` parameter or the result (return value) of the
//   entry point to be used interchangeably, so ensuring both cases are treated
//   the same is handled here.
//
// * Slang allows a varying parameter to use a `struct` or array type, so that
//   we need to recursively process elements and/or fields to find the leaf
//   varying parameters as they will be understood by other targets.
//
// * As an extension of the above, `struct`-type varying parameters in Slang
//   may mix user-defined and system-defined inputs/outputs.
//
// * Slang allows for `inout` varying parameters, which need to desugar into
//   distinct `in` and `out` parameters for targets like GLSL.


#define SYSTEM_VALUE_SEMANTIC_NAMES(M)              \
    M(DispatchThreadID,     SV_DispatchThreadID)    \
    M(GroupID,              SV_GroupID)             \
    M(GroupThreadID,        SV_GroupThreadID)       \
    M(GroupThreadIndex,     SV_GroupIndex)          \
    /* end */

    /// A known system-value semantic name that can be applied to a parameter
    ///
enum class SystemValueSemanticName
{
    None = 0,

    // TODO: Should this enumeration be responsible for differentiating
    // cases where the same semantic name string is allowed in multiple stages,
    // or as both input/output in a single stage, and those different uses
    // might result in different meanings? The alternative is to always
    // pass around the semantic name, stage, and direction together so
    // that code can tell those special cases apart.

#define CASE(ID, NAME) ID,
SYSTEM_VALUE_SEMANTIC_NAMES(CASE)
#undef CASE

    // TODO: There are many more system-value semantic names that we
    // can/should handle here, but for now I've restricted this list
    // to those that are necessary for translating compute shaders.
};

    /// A placeholder that represents the value of a legalized varying
    /// parameter, for the purposes of substituting it into IR code.
    ///
struct LegalizedVaryingVal
{
public:
    enum class Flavor
    {
        None,       ///< No value (conceptually a literal of type `void`)

        Value,      ///< A simple value represented as a single `IRInst*`

        Address,    ///< A location in memory, identified by an address in an `IRInst*`
    };

    LegalizedVaryingVal()
    {}

    static LegalizedVaryingVal makeValue(IRInst* irInst)
    {
        return LegalizedVaryingVal(Flavor::Value, irInst);
    }

    static LegalizedVaryingVal makeAddress(IRInst* irInst)
    {
        return LegalizedVaryingVal(Flavor::Address, irInst);
    }

    Flavor getFlavor() const { return m_flavor; }

    IRInst* getValue() const
    {
        SLANG_ASSERT(getFlavor() == Flavor::Value);
        return m_irInst;
    }

    IRInst* getAddress() const
    {
        SLANG_ASSERT(getFlavor() == Flavor::Address);
        return m_irInst;
    }

private:
    LegalizedVaryingVal(Flavor flavor, IRInst* irInst)
        : m_flavor(flavor)
        , m_irInst(irInst)
    {}

    Flavor  m_flavor = Flavor::None;
    IRInst* m_irInst = nullptr;
};

    /// Materialize the value of `val` as a single IR instruction.
    ///
    /// Any IR code that is needed to materialize the value will be emitted to `builder`.
IRInst* materialize(IRBuilder& builder, LegalizedVaryingVal const& val)
{
    switch( val.getFlavor() )
    {
    case LegalizedVaryingVal::Flavor::None:
        return nullptr; // TODO: should use a `void` literal

    case LegalizedVaryingVal::Flavor::Value:
        return val.getValue();

    case LegalizedVaryingVal::Flavor::Address:
        return builder.emitLoad(val.getAddress());

    default:
        SLANG_UNEXPECTED("unimplemented");
        break;
    }
}

void assign(IRBuilder& builder, LegalizedVaryingVal const& dest, LegalizedVaryingVal const& src)
{
    switch( dest.getFlavor() )
    {
    case LegalizedVaryingVal::Flavor::None:
        break;

    case LegalizedVaryingVal::Flavor::Address:
        builder.emitStore(dest.getAddress(), materialize(builder, src));
        break;

    default:
        SLANG_UNEXPECTED("unimplemented");
        break;
    }
}

void assign(IRBuilder& builder, LegalizedVaryingVal const& dest, IRInst* src)
{
    assign(builder, dest, LegalizedVaryingVal::makeValue(src));
}

    /// Context for the IR pass that legalizing entry-point
    /// varying parameters for a target.
    ///
    /// This is an abstract base type that needs to be inherited
    /// to implement the appropriate policy for a particular
    /// compilation target.
    ///
struct EntryPointVaryingParamLegalizeContext
{
    // This pass will be invoked on an entire module, and will
    // process all entry points in that module.
    //
public:
    void processModule(IRModule* module, DiagnosticSink* sink)
    {
        m_module = module;
        m_sink = sink;

        // We will use multiple IR builders during the legalization
        // process, to avoid having state changes on one builder
        // affect other builders that might be in use.
        //

        // Once the basic initialization is done, we will allow
        // the subtype to implement its own initialization logic
        // that should occur at the start of processing a module.
        //
        beginModuleImpl();

        // We now search for entry-point definitions in the IR module.
        // All entry points should appear at the global scope.
        //
        for(auto inst : module->getGlobalInsts())
        {
            // Entry points are IR functions.
            //
            auto func = as<IRFunc>(inst);
            if(!func)
                continue;

            // Entry point functions must have the `[entryPoint]` decoration.
            //
            auto entryPointDecor = func->findDecoration<IREntryPointDecoration>();
            if(!entryPointDecor)
                continue;

            // Once we find an entry point we process it immediately.
            //
            processEntryPoint(func, entryPointDecor);
        }
    }

protected:

    // As discussed in `processModule()`, a subtype can overide
    // the `beginModuleImpl()` method to perform work that should
    // only happen once per module that is processed.
    //
    virtual void beginModuleImpl()
    {}

    // We have both per-module and per-entry-point state that
    // needs to be managed. The former is set up in `processModule()`,
    // while the latter is used during `processEntryPoint`.
    //
    // Note: It would be possible in principle to remove some
    // the statefullness from this pass by factoring the
    // per-module and per-entry-point logic into distinct types,
    // but then every target-specific implementation would
    // need to comprise two types with complicated interdependencies.
    // The current solution of a single type with statefullness
    // seems easier to manage.

    IRModule*           m_module        = nullptr;
    DiagnosticSink*     m_sink          = nullptr;

    IRFunc*     m_entryPointFunc    = nullptr;
    IRBlock*    m_firstBlock        = nullptr;
    IRInst*     m_firstOrdinaryInst = nullptr;
    Stage       m_stage             = Stage::Unknown;


    void processEntryPoint(IRFunc* entryPointFunc, IREntryPointDecoration* entryPointDecor)
    {
        m_entryPointFunc = entryPointFunc;

        // Before diving into the work of processing an entry point, we start by
        // extracting a bunch of information about the entry point that will
        // be useful to the downstream logic.
        //
        m_stage = entryPointDecor->getProfile().getStage();
        m_firstBlock = entryPointFunc->getFirstBlock();
        m_firstOrdinaryInst = m_firstBlock ? m_firstBlock->getFirstOrdinaryInst() : nullptr;

        auto entryPointLayoutDecoration = entryPointFunc->findDecoration<IRLayoutDecoration>();
        SLANG_ASSERT(entryPointLayoutDecoration);

        auto entryPointLayout = as<IREntryPointLayout>(entryPointLayoutDecoration->getLayout());
        SLANG_ASSERT(entryPointLayout);

        // Note: Of particular importance is that we extract the first/last parameters
        // of the function *before* we allow the subtype to perform per-entry-point
        // setup operations. This ensures that if the subtype adds new parameters to
        // the beginnign or end of the parameter list, those new parameters won't
        // be processed.
        //
        IRParam* firstOriginalParam = m_firstBlock ? m_firstBlock->getFirstParam() : nullptr;
        IRParam* lastOriginalParam = m_firstBlock ? m_firstBlock->getLastParam() : nullptr;

        // We allow the subtype to perform whatever setup or code generation
        // it wants to on a per-entry-point basis. In some cases this might
        // inject code into the start of the function to provide the value
        // of certain system-value parameters.
        //
        beginEntryPointImpl();

        // We now proceed to the meat of the work.
        //
        // We start by considering the result of the entry point function
        // if it is non-`void`.
        //
        auto resultType = entryPointFunc->getResultType();
        if( !as<IRVoidType>(resultType) )
        {
            // We need to translate the existing function result type
            // into zero or more varying parameters that are legal for
            // the target. An entry point function result should be
            // processed in a way that semantically matches an `out` parameter.
            //
            auto legalResult = createLegalVaryingVal(
                resultType,
                entryPointLayout->getResultLayout(),
                LayoutResourceKind::VaryingOutput);

            // Now that we have a representation of the value(s) that will
            // be used to hold the entry-point result we need to transform
            // any `returnVal(r)` instructions in the function body to
            // instead assign `r` to `legalResult` and then `returnVoid`.
            //
            IRBuilder builder(m_module);
            for( auto block : entryPointFunc->getBlocks() )
            {
                auto returnValInst = as<IRReturn>(block->getTerminator());
                if(!returnValInst)
                    continue;

                // We have a `returnVal` instruction that returns `resultVal`.
                //
                auto resultVal = returnValInst->getVal();

                // To replace the existing `returnVal` instruction we will
                // emit an assignment to the new legalized result (whether
                // a global variable, `out` parameter, etc.) and a `returnVoid`.
                //
                builder.setInsertBefore(returnValInst);
                assign(builder, legalResult, resultVal);
                builder.emitReturn();

                returnValInst->removeAndDeallocate();
            }
        }

        // The parameters of the entry-point function will be processed in
        // order to legalize them. We need to be careful when iterating
        // over the parameters for a few reasons:
        //
        // * The subtype-specific setup logic could have introduce parameters
        //   at the beginning or end of the list. We defend against that by
        //   capturing `firstOriginalParam` and `lastOriginalParam` at the
        //   start of this function, and only iterating over that range.
        //
        // * Somehow we might have an entry point declaration but not a definition
        //   this is unlikely but defended against because `firstOriginalParam`
        //   and `lastOriginalParam` will be null in that case.
        //
        // * We will often be removing the parameters once we have legalized
        //   them, so we will modify the list while traversing it. We defend
        //   against this by capturing `nextParam` at the start of each iteration
        //   so that we move to the same parameter next, even if the current
        //   parameter got removed.
        //
        // * The subtype-specific logic for legalizing a specific parameter
        //   might decide to insert new parameters to replace it. This is another
        //   case of modifying the parameter list while iterating it, and we
        //   defend against it with `nextParam` just like we do for the problem
        //  of deletion.
        //
        IRParam* nextParam = nullptr;
        for( auto param = firstOriginalParam; param; param = nextParam )
        {
            nextParam = param->getNextParam();

            processParam(param);

            if(param == lastOriginalParam)
                break;
        }
    }

    virtual void beginEntryPointImpl() {}

    // The next level down is the per-parameter processing logic, which
    // like the per-module and per-entry-point levels maintains its own
    // state to simplify the code (avoiding lots of long parameters lists).

    IRParam* m_param = nullptr;
    IRVarLayout* m_paramLayout = nullptr;

    void processParam(IRParam* param)
    {
        m_param = param;

        // We expect and require all entry-point parameters to have layout
        // information assocaited with them at this point.
        //
        auto paramLayoutDecoration = param->findDecoration<IRLayoutDecoration>();
        SLANG_ASSERT(paramLayoutDecoration);
        m_paramLayout = as<IRVarLayout>(paramLayoutDecoration->getLayout());
        SLANG_ASSERT(m_paramLayout);

        if(!isVaryingParameter(m_paramLayout))
            return;

        // TODO: The GLSL-specific variant of this pass has several
        // special cases that handle entry-point parameters for things like
        // GS output streams and input primitive topology.

        // TODO: The GLSL-specific variant of this pass has special cases
        // to deal with user-defined varying input to RT shaders, since
        // these don't translate to globals in the same way as all other
        // GLSL varying inputs.

        // We need to start by detecting whether the parameter represents
        // an `in` or an `out`/`inout` parameter, since that will determine
        // the strategy we take.
        //
        auto paramType = param->getDataType();
        if(auto inOutType = as<IRInOutType>(paramType))
        {
            processInOutParam(param, inOutType);
        }
        else if(auto outType = as<IROutType>(paramType))
        {
            processOutParam(param, outType);
        }
        else
        {
            processInParam(param, paramType);
        }
    }

    // We anticipate that some targets may need to customize the handling
    // of `out` and `inout` varying parameters, so we have `virtual` methods
    // to handle those cases, which just delegate to a default implementation
    // that provides baseline behavior that should in theory work for
    // multiple targets.
    //
    virtual void processInOutParam(IRParam* param, IRInOutType* inOutType)
    {
        processMutableParam(param, inOutType);
    }
    virtual void processOutParam(IRParam* param, IROutType* inOutType)
    {
        processMutableParam(param, inOutType);
    }

    void processMutableParam(IRParam* param, IROutTypeBase* paramPtrType)
    {
        // The deafult handling of any mutable (`out` or `inout`) parameter
        // will be to introduce a local variable of the corresponding
        // type and to use that in place of the actual parameter during
        // exeuction of the function.

        // The replacement variable will have the type of the original
        // parameter (the `T` in `Out<T>` or `InOut<T>`).
        //
        auto valueType = paramPtrType->getValueType();

        // The replacement variable will be declared at the top of
        // the function.
        //
        IRBuilder builder(m_module);
        builder.setInsertBefore(m_firstOrdinaryInst);

        auto localVar = builder.emitVar(valueType);
        auto localVal = LegalizedVaryingVal::makeAddress(localVar);

        if( const auto inOutType = as<IRInOutType>(paramPtrType) )
        {
            // If the parameter was an `inout` and not just an `out`
            // parameter, we will create one more more legal `in`
            // parameters to represent the incoming value,
            // and then assign from those legalized input(s)
            // into our local variable at the start of the function.
            //
            auto inputVal = createLegalVaryingVal(
                valueType,
                m_paramLayout,
                LayoutResourceKind::VaryingInput);
            assign(builder, localVal, inputVal);
        }

        // Because the `out` or `inout` parameter is represented
        // as a pointer, and our local variabel is also a pointer
        // we can directly replace all uses of the original parameter
        // with uses of the variable.
        //
        param->replaceUsesWith(localVar);

        // For both `out` and `inout` parameters, we need to
        // introduce one or more legalized `out` parameters
        // to represent the outgoing value.
        //
        auto outputVal = createLegalVaryingVal(
            valueType,
            m_paramLayout,
            LayoutResourceKind::VaryingOutput);

        // In order to have changes to our local variable become
        // visible in the legalized outputs, we need to assign
        // from the local variable to the output as the last
        // operation before any `return` instructions.
        //
        for( auto block : m_entryPointFunc->getBlocks() )
        {
            auto returnInst = as<IRReturn>(block->getTerminator());
            if(!returnInst)
                continue;

            builder.setInsertBefore(returnInst);
            assign(builder, outputVal, localVal);
        }

        // Once we are done replacing the original parameter,
        // we can remove it from the function.
        //
        param->removeAndDeallocate();
    }

    void processInParam(IRParam* param, IRType* paramType)
    {
        // Legalizing an `in` parameter is easier than a mutable parameter.

        // We start by creating one or more legalized `in` parameters
        // to represent the incoming value.
        //
        auto legalVal = createLegalVaryingVal(
            paramType,
            m_paramLayout,
            LayoutResourceKind::VaryingInput);

        // Next, we "materialize" the legalized value to produce
        // an `IRInst*` that represents it.
        //
        // Note: We materialize each input parameter once, at the top
        // of the entry point. Making a copy in this way could
        // introduce overhead if an input parameter is an array,
        // since all indexing operations will now refer to a copy
        // of the original array.
        //
        // TODO: We could in theory iterate over all uses of
        // `param` and introduce a custom replacement for each.
        // Such a replacement strategy could produce better code
        // for things like indexing into varying arrays, but at the
        // cost of more accesses to the input parameter data.
        //
        IRBuilder builder(m_module);
        builder.setInsertBefore(m_firstOrdinaryInst);
        IRInst* materialized = materialize(builder, legalVal);

        // The materialized value can be used to completely
        // replace the original parameter.
        //
        param->replaceUsesWith(materialized);
        param->removeAndDeallocate();
    }

    // Depending on the "direction" of the parameter (`in`, `out`, `inout`)
    // we may need to create one or legalized variables to represented it.
    //
    // We now turn our attention to the problem of creating a legalized
    // value (wrapping zero or more variables/parameters) to represent
    // a varying parameter of a given type for a specific direction:
    // either input or output, but not both.
    //
    LegalizedVaryingVal createLegalVaryingVal(IRType* type, IRVarLayout* varLayout, LayoutResourceKind kind)
    {
        // The process we are going to use for creating legalized
        // values is going to involve recursion over the `type`
        // of the parameter, and there is a lot of state that
        // we need to carry along the way.
        //
        // Rather than have our core recursive function have
        // many parameters that need to be followed through
        // all the recursive call sites, we are going to wrap
        // the relevant data up in a `struct` and pass all
        // the information down as a bundle.

        auto typeLayout = varLayout->getTypeLayout();

        VaryingParamInfo info;
        info.type       = type;
        info.varLayout  = varLayout;
        info.typeLayout = typeLayout;
        info.kind       = kind;

        return _createLegalVaryingVal(info);
    }

    // While recursing through the type of a varying parameter,
    // we may need to make a recursive call on the element type
    // of an array, while still tracking the fact that any
    // leaf parameter we encounter needs to have the "outer
    // array brackets" taken into account when giving it a type.
    //
    // For those purposes we have the `VaryingArrayDeclaratorInfo`
    // type that keeps track of outer layers of array-ness
    // for a parameter during our recursive walk.
    //
    // It is stored as a stack-allocated linked list, where the list flows
    // up through the call stack.
    //
    struct VaryingArrayDeclaratorInfo
    {
        IRInst*                     elementCount    = nullptr;
        VaryingArrayDeclaratorInfo* next            = nullptr;
    };

    // Here is the declaration of the bundled information we care
    // about when declaring a varying parameter.
    //
    struct VaryingParamInfo
    {
        // We obviously care about the type of the parameter we
        // need to legalize, as well as the layout of that type.
        //
        IRType*                     type                        = nullptr;
        IRTypeLayout*               typeLayout                  = nullptr;

        // We also care about the variable layout information for
        // the parameter, because that includes things like the semantic
        // name/index, as well as any binding information that was
        // computed (e.g., for the `location` of GLSL user-defined
        // varying parameters).
        //
        // Note: the `varLayout` member may not represent a layout for
        // a variable of the given `type`, because we might be peeling
        // away layers of array-ness. Consider:
        //
        //      int stuff[3] : STUFF
        //
        // When processing the parameter `stuff`, we start with `type`
        // being `int[3]`, but then we will recurse on `int`. At that
        // point the `varLayout` will still refer to `stuff` with its
        // semantic of `STUFF`, but the `type` and `typeLayout` will
        // refer to the `int` type.
        //
        IRVarLayout*                varLayout                   = nullptr;

        // As discussed above, sometimes `varLayout` will refer to an
        // outer declaration of array type, while `type` and `typeLayout`
        // refer to an element type (perhaps nested).
        //
        // The `arrayDeclarators` field stores a linked list representing
        // outer layers of "array brackets" that surround the variable/field
        // of `type`.
        //
        // If code decides to construct a leaf parameter based on `type`,
        // then it will need to use these `arrayDeclarators` to wrap the
        // type up to make it correct.
        //
        VaryingArrayDeclaratorInfo* arrayDeclarators            = nullptr;

        // In some cases the decision-making about how to lower a parameter
        // will depend on the kind of varying parameter (input or output).
        //
        // TODO: We may find that there are cases where a target wants to
        // support true `inout` varying parameters, and `LayoutResourceKind`
        // cannot currently handle those.
        //
        LayoutResourceKind          kind                        = LayoutResourceKind::None;

        // When we arrive at a leaf parameter/field, we can identify whether
        // it is a user-defined or system-value varying based on its semantic name.
        //
        // For convenience, target-specific subtypes only need to understand
        // the enumerated `systemValueSemanticName` rather than needing to
        // implement their own parsing of semantic name strings.
        //
        SystemValueSemanticName     systemValueSemanticName     = SystemValueSemanticName::None;
    };

    LegalizedVaryingVal _createLegalVaryingVal(VaryingParamInfo const& info)
    {
        // By default, when we seek to creating a legalized value
        // for a varying parameter, we will look at its type to
        // decide what to do.
        //
        // For most basic types, we will immediately delegate to the
        // base case (which will use target-specific logic).
        //
        // Note: The logic here will always fully scalarize the input
        // type, gernerated multiple SOA declarations if the input
        // was AOS. That choice is required for some cases in GLSL,
        // and seems to be a reasonable default policy, but it could
        // lead to some performance issues for shaders that rely
        // on varying arrays.
        //
        // TODO: Consider whether some carefully designed early-out
        // checks could avoid full scalarization when it is possible
        // to avoid. Those early-out cases would probably need to
        // align with the layout logic that is assigning `location`s
        // to varying parameters.
        //
        auto type = info.type;
        if (as<IRVoidType>(type))
        {
            return createSimpleLegalVaryingVal(info);
        }
        else if( as<IRBasicType>(type) )
        {
            return createSimpleLegalVaryingVal(info);
        }
        else if( as<IRVectorType>(type) )
        {
            return createSimpleLegalVaryingVal(info);
        }
        else if( as<IRMatrixType>(type) )
        {
            // Note: For now we are handling matrix types in a varying
            // parameter list as if they were ordinary types like
            // scalars and vectors. This works well enough for simple
            // stuff, and is unlikely to see much use anyway.
            //
            // TODO: A more correct implementation will probably treat
            // a matrix-type varying parameter as if it was syntax
            // sugar for an array of rows.
            //
            return createSimpleLegalVaryingVal(info);
        }
        else if( auto arrayType = as<IRArrayType>(type) )
        {
            // A varying parameter of array type is an interesting beast,
            // because depending on the element type of the array we
            // might end up needing to generate multiple parameters in
            // struct-of-arrays (SOA) fashion. This will notably
            // come up in the case where the element type is a `struct`,
            // with fields that mix both user-defined and system-value
            // semantics.
            //
            auto elementType = arrayType->getElementType();
            auto elementCount = arrayType->getElementCount();
            auto arrayLayout = as<IRArrayTypeLayout>(info.typeLayout);
            SLANG_ASSERT(arrayLayout);
            auto elementTypeLayout = arrayLayout->getElementTypeLayout();

            // We are going to recursively apply legalization to the
            // element type of the array, but when doing so we will
            // pass down information about the outer "array brackets"
            // that this type represented.
            //
            VaryingArrayDeclaratorInfo arrayDeclarator;
            arrayDeclarator.elementCount = elementCount;
            arrayDeclarator.next = info.arrayDeclarators;

            VaryingParamInfo elementInfo = info;
            elementInfo.type = elementType;
            elementInfo.typeLayout = elementTypeLayout;
            elementInfo.arrayDeclarators = &arrayDeclarator;

            return _createLegalVaryingVal(elementInfo);
        }
        else if( auto streamType = as<IRHLSLStreamOutputType>(type))
        {
            // Handling a geometry shader stream output type like
            // `TriangleStream<T>` is similar to handling an array,
            // but we do *not* pass down a "declarator" to note
            // the wrapping type.
            //
            // This choice is appropriate for GLSL because geometry
            // shader outputs are just declared as their per-vertex
            // types and not wrapped in array or stream types.
            //
            // TODO: If we ever need to legalize geometry shaders for
            // a target with different rules we might need to revisit
            // this choice.
            //
            auto elementType = streamType->getElementType();
            auto streamLayout = as<IRStreamOutputTypeLayout>(info.typeLayout);
            SLANG_ASSERT(streamLayout);
            auto elementTypeLayout = streamLayout->getElementTypeLayout();

            VaryingParamInfo elementInfo = info;
            elementInfo.type = elementType;
            elementInfo.typeLayout = elementTypeLayout;

            return _createLegalVaryingVal(elementInfo);
        }
        // Note: This file is currently missing the case for handling a varying `struct`.
        // The relevant logic is present in `slang-ir-glsl-legalize`, but it would add
        // a lot of complexity to this file to include it now.
        //
        // The main consequence of this choice is that this pass doesn't support varying
        // parameters wrapped in `struct`s for the targets that require this pass
        // (currently CPU and CUDA).
        //
        // TODO: Copy over the relevant logic from the GLSL-specific pass, as part of
        // readying this file to handle the needs of all targets.
        //
        else
        {
            // When no special case matches, we assume the parameter
            // has a simple type that we can handle directly.
            //
            return createSimpleLegalVaryingVal(info);
        }
    }

    LegalizedVaryingVal createSimpleLegalVaryingVal(VaryingParamInfo const& info)
    {
        // At this point we've bottomed out in the type-based recursion
        // and we have a leaf parameter of some simple type that should
        // also have a single semantic name/index to work with.

        // TODO: This seems like the right place to "wrap" the type back
        // up in layers of array-ness based on the outer array brackets
        // that were accumulated.

        // Our first order of business will be to check whether the
        // parameter represents a system-value parameter.
        //
        auto varLayout = info.varLayout;
        auto semanticInst = varLayout->findSystemValueSemanticAttr();
        if( semanticInst )
        {
            // We will compare the semantic name against our list of
            // system-value semantics using conversion to lower-case
            // to achieve a case-insensitive comparison (this is
            // necessary because semantics in HLSL/Slang do not
            // treat case as significant).
            //
            // TODO: It would be nice to have a case-insensitive
            // comparsion operation on `UnownedStringSlice` to
            // avoid all the `String`s we crete and thren throw
            // away here.
            //
            String semanticNameSpelling = semanticInst->getName();
            auto semanticName = semanticNameSpelling.toLower();

            SystemValueSemanticName systemValueSemanticName = SystemValueSemanticName::None;

        #define CASE(ID, NAME)                                          \
            if(semanticName == String(#NAME).toLower())                 \
            {                                                           \
                systemValueSemanticName = SystemValueSemanticName::ID;  \
            }                                                           \
            else

            SYSTEM_VALUE_SEMANTIC_NAMES(CASE)
        #undef CASE
            {
                // no match
            }

            if( systemValueSemanticName != SystemValueSemanticName::None )
            {
                // If the leaf parameter has a system-value semantic, then
                // we need to translate the system value in whatever way
                // is appropraite for the target.
                //
                // TODO: The logic here is missing the behavior from the
                // GLSL-specific pass that handles type conversion when
                // a user-declared system-value parameter might not
                // match the type that was expected exactly (e.g., they
                // declare a `uint2` but the parameter is a `uint3`).
                //
                VaryingParamInfo systemValueParamInfo = info;
                systemValueParamInfo.systemValueSemanticName = systemValueSemanticName;
                return createLegalSystemVaryingValImpl(systemValueParamInfo);
            }

            // TODO: We should seemingly do something if the semantic name
            // implies a system-value semantic (starts with `SV_`) but we
            // didn't find a match.
            //
            // In practice, this is probably something that should be handled
            // at the layout level (`slang-parameter-binding.cpp`), and the
            // layout for a parameter should include the `SystemValueSemanticName`
            // as an enumerated value rather than a string (so that downstream
            // code doesn't have to get into the business of parsing it).
        }

        // If there was semantic applied to the parameter *or* the semantic
        // wasn't recognized as a system-value semantic, then we need
        // to do whatever target-specific logic is required to legalize
        // a user-defined varying parameter.
        //
        return createLegalUserVaryingValImpl(info);
    }

    // The base type will provide default implementations of the logic
    // for creating user-defined and system-value varyings, but in
    // each case the default logic will simply diagnose an error.
    //
    // For targets that support either case, it is essential to
    // override these methods with appropriate logic.

    virtual LegalizedVaryingVal createLegalUserVaryingValImpl(VaryingParamInfo const& info)
    {
        return diagnoseUnsupportedUserVal(info);
    }

    virtual LegalizedVaryingVal createLegalSystemVaryingValImpl(VaryingParamInfo const& info)
    {
        return diagnoseUnsupportedSystemVal(info);
    }

    // As a utility for target-specific subtypes, we define a routine
    // to diagnose the case of a system-value semantic that isn't
    // understood by the target.

    LegalizedVaryingVal diagnoseUnsupportedSystemVal(VaryingParamInfo const& info)
    {
        SLANG_UNUSED(info);

        m_sink->diagnose(m_param, Diagnostics::unimplemented, "this target doesn't support this system-defined varying parameter");

        return LegalizedVaryingVal();
    }

    LegalizedVaryingVal diagnoseUnsupportedUserVal(VaryingParamInfo const& info)
    {
        SLANG_UNUSED(info);

        m_sink->diagnose(m_param, Diagnostics::unimplemented, "this target doesn't support this user-defined varying parameter");

        return LegalizedVaryingVal();
    }

    // There are some cases of system-value inputs that can be derived
    // from other inputs; notably compute shaders support `SV_DispatchThreadID`
    // and `SV_GroupIndex` which can both be derived from the more primitive
    // `SV_GroupID` and `SV_GroupThreadID`, together with the extents
    // of the thread group (which are specified with `[numthreads(...)]`).
    //
    // As a utilty to target-specific subtypes, we define helpers for
    // calculating the value of these derived system values from the
    // more primitive ones.

        /// Emit code to calculate `SV_DispatchThreadID`
    IRInst* emitCalcDispatchThreadID(
        IRBuilder&  builder,
        IRType*     type,
        IRInst*     groupID,
        IRInst*     groupThreadID,
        IRInst*     groupExtents)
    {
        // The dispatch thread ID can be computed as:
        //
        //      dispatchThreadID = groupID*groupExtents + groupThreadID
        //
        // where `groupExtents` is the X,Y,Z extents of
        // each thread group in threads (as given by
        // `[numthreads(X,Y,Z)]`).

        return builder.emitAdd(type,
            builder.emitMul(type,
                groupID,
                groupExtents),
            groupThreadID);
    }

        /// Emit code to calculate `SV_GroupIndex`
    IRInst* emitCalcGroupThreadIndex(
        IRBuilder&  builder,
        IRInst*     groupThreadID,
        IRInst*     groupExtents)
    {
        auto intType = builder.getIntType();
        auto uintType = builder.getBasicType(BaseType::UInt);

        // The group thread index can be computed as:
        //
        //      groupThreadIndex = groupThreadID.x
        //                       + groupThreadID.y*groupExtents.x
        //                       + groupThreadID.z*groupExtents.x*groupExtents.z;
        //
        // or equivalently (with one less multiply):
        //
        //      groupThreadIndex = (groupThreadID.z  * groupExtents.y
        //                        + groupThreadID.y) * groupExtents.x
        //                        + groupThreadID.x;
        //

        // `offset = groupThreadID.z`
        auto zAxis = builder.getIntValue(intType, 2);
        IRInst* offset = builder.emitElementExtract(uintType, groupThreadID, zAxis);

        // `offset *= groupExtents.y`
        // `offset += groupExtents.y`
        auto yAxis = builder.getIntValue(intType, 1);
        offset = builder.emitMul(uintType, offset, builder.emitElementExtract(uintType, groupExtents, yAxis));
        offset = builder.emitAdd(uintType, offset, builder.emitElementExtract(uintType, groupThreadID, yAxis));

        // `offset *= groupExtents.x`
        // `offset += groupExtents.x`
        auto xAxis = builder.getIntValue(intType, 0);
        offset = builder.emitMul(uintType, offset, builder.emitElementExtract(uintType, groupExtents, xAxis));
        offset = builder.emitAdd(uintType, offset, builder.emitElementExtract(uintType, groupThreadID, xAxis));

        return offset;
    }

    // Several of the derived calcluations rely on having
    // access to the "group extents" of a compute shader.
    // That information is expected to be present on
    // the entry point as a `[numthreads(...)]` attribute,
    // and we define a convenience routine for accessing
    // that information.

    IRInst* emitCalcGroupExtents(
        IRBuilder&      builder,
        IRVectorType*   type)
    {
        if(auto numThreadsDecor = m_entryPointFunc->findDecoration<IRNumThreadsDecoration>())
        {
            static const int kAxisCount = 3;
            IRInst* groupExtentAlongAxis[kAxisCount] = {};

            for( int axis = 0; axis < kAxisCount; axis++ )
            {
                auto litValue = as<IRIntLit>(numThreadsDecor->getExtentAlongAxis(axis));
                if(!litValue)
                    return nullptr;

                groupExtentAlongAxis[axis] = builder.getIntValue(type->getElementType(), litValue->getValue());
            }

            return builder.emitMakeVector(type, kAxisCount, groupExtentAlongAxis);
        }

        // TODO: We may want to implement a backup option here,
        // in case we ever want to support compute shaders with
        // dynamic/flexible group size on targets that allow it.
        //
        SLANG_UNEXPECTED("Expected '[numthreads(...)]' attribute on compute entry point.");
        UNREACHABLE_RETURN(nullptr);
    }
};

// With the target-independent core of the pass out of the way, we can
// turn our attention to the target-specific subtypes that handle
// translation of "leaf" varying parameters.

struct CUDAEntryPointVaryingParamLegalizeContext : EntryPointVaryingParamLegalizeContext
{
    // CUDA compute kernels don't support user-defined varying
    // input or output, and there are only a few system-value
    // varying inputs to deal with.
    //
    // CUDA provides built-in global parameters `threadIdx`,
    // `blockIdx`, and `blockDim` that we can make use of.
    //
    IRGlobalParam* threadIdxGlobalParam = nullptr;
    IRGlobalParam* blockIdxGlobalParam = nullptr;
    IRGlobalParam* blockDimGlobalParam = nullptr;

    // All of our system values will be exposed with the
    // `uint3` type, and we'll cache a pointer to that
    // type to void looking it up repeatedly.
    //
    IRType* uint3Type = nullptr;

    // Scans through and returns the first typeLayout attribute of non-zero size.
    static LayoutResourceKind getLayoutResourceKind(IRTypeLayout* typeLayout) {
        for (auto attr : typeLayout->getSizeAttrs()) {
            if (attr->getSize() != 0) return attr->getResourceKind();
        }
        return LayoutResourceKind::None;
    }

    IRInst* emitOptiXAttributeFetch(int& ioBaseAttributeIndex, IRType* typeToFetch, IRBuilder* builder) {
        if (auto structType = as<IRStructType>(typeToFetch))
        {
            List<IRInst*> fieldVals;
            for (auto field : structType->getFields())
            {
                auto fieldType = field->getFieldType();
                auto fieldVal = emitOptiXAttributeFetch(ioBaseAttributeIndex, fieldType, builder);
                if (!fieldVal)
                    return nullptr;

                fieldVals.add(fieldVal);
            }
            return builder->emitMakeStruct(typeToFetch, fieldVals);
        }
        else if (auto arrayType = as<IRArrayTypeBase>(typeToFetch))
        {
            auto elementCountInst = as<IRIntLit>(arrayType->getElementCount());
            IRIntegerValue elementCount = elementCountInst->getValue();
            auto elementType = arrayType->getElementType();
            List<IRInst*> elementVals;
            for (IRIntegerValue ii = 0; ii < elementCount; ++ii)
            {
                auto elementVal = emitOptiXAttributeFetch(ioBaseAttributeIndex, elementType, builder);
                if (!elementVal)
                    return nullptr;
                elementVals.add(elementVal);
            }
            return builder->emitMakeArray(typeToFetch, elementVals.getCount(), elementVals.getBuffer());
        }
        else if (auto matType = as<IRMatrixType>(typeToFetch))
        {
            auto rowCountInst = as<IRIntLit>(matType->getRowCount());
            if (rowCountInst)
            {
                auto rowType = builder->getVectorType(matType->getElementType(), matType->getColumnCount());
                IRType* elementType = rowType;
                IRIntegerValue elementCount = rowCountInst->getValue();
                List<IRInst*> elementVals;
                for (IRIntegerValue ii = 0; ii < elementCount; ++ii)
                {
                    auto elementVal = emitOptiXAttributeFetch(ioBaseAttributeIndex, elementType, builder);
                    if (!elementVal)
                        return nullptr;
                    elementVals.add(elementVal);
                }
                return builder->emitIntrinsicInst(typeToFetch, kIROp_MakeMatrix, elementVals.getCount(), elementVals.getBuffer());
            }
        }
        else if (auto vecType = as<IRVectorType>(typeToFetch))
        {
            auto elementCountInst = as<IRIntLit>(vecType->getElementCount());
            IRIntegerValue elementCount = elementCountInst->getValue();
            IRType* elementType = vecType->getElementType();
            List<IRInst*> elementVals;
            for (IRIntegerValue ii = 0; ii < elementCount; ++ii)
            {
                auto elementVal = emitOptiXAttributeFetch(ioBaseAttributeIndex, elementType, builder);
                if (!elementVal)
                    return nullptr;
                elementVals.add(elementVal);
            }
            return builder->emitMakeVector(typeToFetch, elementVals.getCount(), elementVals.getBuffer());
        }
        else if (const auto basicType = as<IRBasicType>(typeToFetch))
        {
            IRIntegerValue idx = ioBaseAttributeIndex;
            auto idxInst = builder->getIntValue(builder->getIntType(), idx);
            ioBaseAttributeIndex++;
            IRInst* args[] = { typeToFetch, idxInst };
            IRInst* getAttr = builder->emitIntrinsicInst(typeToFetch, kIROp_GetOptiXHitAttribute, 2, args);
            return getAttr;
        }

        return nullptr;
    }

    void beginModuleImpl() SLANG_OVERRIDE
    {
        // Because many of the varying parameters are defined
        // as magic globals in CUDA, we can introduce their
        // definitions once per module, instead of once per
        // entry point.
        //
        IRBuilder builder(m_module);
        builder.setInsertInto(m_module->getModuleInst());

        // We begin by looking up the `uint` and `uint3` types.
        //
        auto uintType = builder.getBasicType(BaseType::UInt);
        uint3Type = builder.getVectorType(uintType, builder.getIntValue(builder.getIntType(), 3));

        // Next we create IR type and variable layouts that
        // we can use to mark the global parameters like
        // `threadIdx` as varying parameters instead of
        // uniform.
        //
        IRTypeLayout::Builder typeLayoutBuilder(&builder);
        typeLayoutBuilder.addResourceUsage(LayoutResourceKind::VaryingInput, 1);
        auto typeLayout = typeLayoutBuilder.build();

        IRVarLayout::Builder varLayoutBuilder(&builder, typeLayout);
        varLayoutBuilder.findOrAddResourceInfo(LayoutResourceKind::VaryingInput);
        auto varLayout = varLayoutBuilder.build();

        // Finaly, we construct global parameters to represent
        // `threadIdx`, `blockIdx`, and `blockDim`.
        //
        // Each of these parameters is given a target-intrinsic
        // decoration that ensures that (1) it will not get a declaration
        // emitted in output code, and (2) it will be referenced
        // by exactly the desired name (with no attempt to generate
        // a unique name).

        threadIdxGlobalParam = builder.createGlobalParam(uint3Type);
        builder.addTargetIntrinsicDecoration(threadIdxGlobalParam, CapabilitySet::makeEmpty(), UnownedTerminatedStringSlice("threadIdx"));
        builder.addLayoutDecoration(threadIdxGlobalParam, varLayout);

        blockIdxGlobalParam = builder.createGlobalParam(uint3Type);
        builder.addTargetIntrinsicDecoration(blockIdxGlobalParam, CapabilitySet::makeEmpty(), UnownedTerminatedStringSlice("blockIdx"));
        builder.addLayoutDecoration(blockIdxGlobalParam, varLayout);

        blockDimGlobalParam = builder.createGlobalParam(uint3Type);
        builder.addTargetIntrinsicDecoration(blockDimGlobalParam, CapabilitySet::makeEmpty(), UnownedTerminatedStringSlice("blockDim"));
        builder.addLayoutDecoration(blockDimGlobalParam, varLayout);
    }

    // While CUDA provides many useful system values
    // as built-in globals, it does not provide the
    // equivalent of `SV_DispatchThreadID` or
    // `SV_GroupIndex` as a built-in.
    //
    // We will instead synthesize those values on
    // entry to each kernel.

    IRInst* groupThreadIndex = nullptr;
    IRInst* dispatchThreadID = nullptr;
    void beginEntryPointImpl() SLANG_OVERRIDE
    {
        IRBuilder builder(m_module);
        builder.setInsertBefore(m_firstOrdinaryInst);

        // Note that we can use the built-in `blockDim`
        // variable to determine the group extents,
        // instead of inspecting the `[numthreads(...)]`
        // attribute.
        //
        // This choice makes our output more idomatic
        // as CUDA code, but might also cost a small
        // amount of performance by not folding in
        // the known constant values from `numthreads`.
        //
        // TODO: Add logic to use the values from
        // `numthreads` if it is present, but to fall
        // back to `blockDim` if not?

        dispatchThreadID = emitCalcDispatchThreadID(
            builder,
            uint3Type,
            blockIdxGlobalParam,
            threadIdxGlobalParam,
            blockDimGlobalParam);

        groupThreadIndex = emitCalcGroupThreadIndex(
            builder,
            threadIdxGlobalParam,
            blockDimGlobalParam);

        // Note: we don't pay attention to whether the
        // kernel actually makes use of either of these
        // system values when we synthesize them.
        //
        // We can get away with this because we know
        // that subsequent DCE passes will eliminate
        // the computations if they aren't used.
        //
        // The main alternative would be to compute
        // these values lazily, when they are first
        // referenced. While that is possible, it
        // requires more (and more subtle) code in this pass.
    }

    LegalizedVaryingVal createLegalSystemVaryingValImpl(VaryingParamInfo const& info) SLANG_OVERRIDE
    {
        // Because all of the relevant values are either
        // ambiently available in CUDA, or were computed
        // eagerly in the entry block to the kernel
        // function, we can easily return the right
        // value to use for a system-value parameter.

        switch( info.systemValueSemanticName )
        {
        case SystemValueSemanticName::GroupID:          return LegalizedVaryingVal::makeValue(blockIdxGlobalParam);
        case SystemValueSemanticName::GroupThreadID:    return LegalizedVaryingVal::makeValue(threadIdxGlobalParam);
        case SystemValueSemanticName::GroupThreadIndex: return LegalizedVaryingVal::makeValue(groupThreadIndex);
        case SystemValueSemanticName::DispatchThreadID: return LegalizedVaryingVal::makeValue(dispatchThreadID);
        default:
            return diagnoseUnsupportedSystemVal(info);
        }
    }

    LegalizedVaryingVal createLegalUserVaryingValImpl(VaryingParamInfo const& info) SLANG_OVERRIDE
    {
        auto layoutResourceKind = getLayoutResourceKind(info.typeLayout);
        switch (layoutResourceKind)
        {
        case LayoutResourceKind::RayPayload: {
            IRBuilder builder(m_module);
            builder.setInsertBefore(m_firstOrdinaryInst);
            IRPtrType* ptrType = builder.getPtrType(info.type);
            IRInst* getRayPayload = builder.emitIntrinsicInst(ptrType, kIROp_GetOptiXRayPayloadPtr, 0, nullptr);
            return LegalizedVaryingVal::makeAddress(getRayPayload);
            // Todo: compute how many registers are required for the current payload. 
            // If more than 32, use the above logic. 
            // Otherwise, either use the optix_get_payload or optix_set_payload 
            // intrinsics depending on input/output
            /*if (info.kind == LayoutResourceKind::VaryingInput) {
            }
            else if (info.kind == LayoutResourceKind::VaryingOutput) {
            }
            else {
                return diagnoseUnsupportedUserVal(info);
            }*/ 
        }
        case LayoutResourceKind::HitAttributes: {
            IRBuilder builder(m_module);
            builder.setInsertBefore(m_firstOrdinaryInst);
            int ioBaseAttributeIndex = 0;
            IRInst* getHitAttributes = emitOptiXAttributeFetch(/*ioBaseAttributeIndex*/ ioBaseAttributeIndex, /* type to fetch */info.type, /*the builder in use*/ &builder);
            if (ioBaseAttributeIndex > 8) {
                m_sink->diagnose(m_param, Diagnostics::unexpected, "the supplied hit attribute exceeds the maximum hit attribute structure size (32 bytes)");
                return LegalizedVaryingVal();
            }
            return LegalizedVaryingVal::makeValue(getHitAttributes);
        }
        default:
            return diagnoseUnsupportedUserVal(info);
        }
    }
};


struct CPUEntryPointVaryingParamLegalizeContext : EntryPointVaryingParamLegalizeContext
{
    // Slang translates compute shaders for CPU such that they always have an
    // initial parameter that is a `ComputeThreadVaryingInput*`, and that
    // type provides the essential parameters (`SV_GroupID` and `SV_GroupThreadID`
    // as fields).
    //
    // Our legalization pass for CPU this begins with the per-module logic
    // to synthesize an IR definition of that type and its fields, so that
    // we can use it across entry points.

    IRType*         uintType        = nullptr;
    IRVectorType*   uint3Type       = nullptr;
    IRType*         uint3PtrType    = nullptr;

    IRStructType*   varyingInputStructType = nullptr;
    IRPtrType*      varyingInputStructPtrType = nullptr;

    IRStructKey*    groupIDKey = nullptr;
    IRStructKey*    groupThreadIDKey = nullptr;

    void beginModuleImpl() SLANG_OVERRIDE
    {
        IRBuilder builder(m_module);
        builder.setInsertInto(m_module->getModuleInst());

        uintType = builder.getBasicType(BaseType::UInt);
        uint3Type = builder.getVectorType(uintType, builder.getIntValue(builder.getIntType(), 3));
        uint3PtrType = builder.getPtrType(uint3Type);

        // As we construct the `ComputeThreadVaryingInput` type and its fields,
        // we mark them all as target intrinsics, which means that their
        // declarations will *not* be reproduced in the output code, instead
        // coming from the "prelude" file that already defines this type.

        varyingInputStructType = builder.createStructType();
        varyingInputStructPtrType = builder.getPtrType(varyingInputStructType);

        builder.addTargetIntrinsicDecoration(varyingInputStructType, CapabilitySet::makeEmpty(), UnownedTerminatedStringSlice("ComputeThreadVaryingInput"));

        groupIDKey = builder.createStructKey();
        builder.addTargetIntrinsicDecoration(groupIDKey, CapabilitySet::makeEmpty(), UnownedTerminatedStringSlice("groupID"));
        builder.createStructField(varyingInputStructType, groupIDKey, uint3Type);

        groupThreadIDKey = builder.createStructKey();
        builder.addTargetIntrinsicDecoration(groupThreadIDKey, CapabilitySet::makeEmpty(), UnownedTerminatedStringSlice("groupThreadID"));
        builder.createStructField(varyingInputStructType, groupThreadIDKey, uint3Type);
    }

    // While the declaration of the `ComputeVaryingThreadInput` type
    // can be shared across all entry points, each entry point must
    // declare its own parameter to receive the varying parameters.
    //
    // We will extract the relevant fields from the `ComputeVaryingThreadInput`
    // at the start of kernel execution (rather than repeatedly load them
    // at each use site), and will also eagerly compute the derived
    // values for `SV_DispatchThreadID` and `SV_GroupIndex`.

    IRInst* groupID = nullptr;
    IRInst* groupThreadID = nullptr;
    IRInst* groupExtents = nullptr;
    IRInst* dispatchThreadID = nullptr;
    IRInst* groupThreadIndex = nullptr;

    void beginEntryPointImpl() SLANG_OVERRIDE
    {
        groupID = nullptr;
        groupThreadID = nullptr;
        dispatchThreadID = nullptr;

        IRBuilder builder(m_module);

        auto varyingInputParam = builder.createParam(varyingInputStructPtrType);
        varyingInputParam->insertBefore(m_firstBlock->getFirstChild());

        builder.setInsertBefore(m_firstOrdinaryInst);

        groupID = builder.emitLoad(
            builder.emitFieldAddress(uint3PtrType, varyingInputParam, groupIDKey));

        groupThreadID = builder.emitLoad(
            builder.emitFieldAddress(uint3PtrType, varyingInputParam, groupThreadIDKey));

        // Note: we need to rely on the presence of the `[numthreads(...)]` attribute
        // to tell us the size of the compute thread group, which we will then use
        // when computing the dispatch thread ID and group thread index.
        //
        // TODO: If we ever wanted to support flexible thread-group sizes for our
        // CPU target, we'd need to change it so that the thread-group size can
        // be passed in as part of `ComputeVaryingThreadInput`.
        //
        groupExtents = emitCalcGroupExtents(builder, uint3Type);

        dispatchThreadID = emitCalcDispatchThreadID(builder, uint3Type, groupID, groupThreadID, groupExtents);

        groupThreadIndex = emitCalcGroupThreadIndex(builder, groupThreadID, groupExtents);
    }

    LegalizedVaryingVal createLegalSystemVaryingValImpl(VaryingParamInfo const& info) SLANG_OVERRIDE
    {
        // Because all of the relvant system values were synthesized
        // into the first block of the entry-point function, we can
        // just return them wherever they are referenced.
        //
        // Note that any values that were synthesized but then are
        // not referened will simply be eliminated as dead code
        // in later passes.

        switch( info.systemValueSemanticName )
        {
        case SystemValueSemanticName::GroupID:          return LegalizedVaryingVal::makeValue(groupID);
        case SystemValueSemanticName::GroupThreadID:    return LegalizedVaryingVal::makeValue(groupThreadID);
        case SystemValueSemanticName::GroupThreadIndex: return LegalizedVaryingVal::makeValue(groupThreadIndex);
        case SystemValueSemanticName::DispatchThreadID: return LegalizedVaryingVal::makeValue(dispatchThreadID);

        default:
            return diagnoseUnsupportedSystemVal(info);
        }
    }
};

void legalizeEntryPointVaryingParamsForCPU(
    IRModule*               module,
    DiagnosticSink*         sink)
{
    CPUEntryPointVaryingParamLegalizeContext context;
    context.processModule(module, sink);
}

void legalizeEntryPointVaryingParamsForCUDA(
    IRModule*               module,
    DiagnosticSink*         sink)
{
    CUDAEntryPointVaryingParamLegalizeContext context;
    context.processModule(module, sink);
}

}
