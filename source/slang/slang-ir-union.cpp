// slang-ir-union.cpp
#include "slang-ir-union.h"

#include "slang-ir.h"
#include "slang-ir-insts.h"

namespace Slang {

// This file will implement a pass to replace any union types (currently
// just tagged unions) with plain `struct` types that attempt to provide
// equivalent semantics. This will necessarily be a bit fragile, and there
// will be fundamental limits to what the translation can support without
// improved features in the target shading languages/ILs.

struct DesugarUnionTypesContext
{
    // We'll start with some basic state that we need to get the job done.
    //
    // This includes the IR module we are to process, as well as IR building
    // state that we will initialize once and then use throughout the pass.
    //
    IRModule* module;
    SharedIRBuilder sharedBuilderStorage;
    IRBuilder builderStorage;
    IRBuilder* getBuilder() { return &builderStorage; }

    // Because we will be replacing instructions that refer to unions with
    // different logic, we'll want to remove the original instructions.
    // However, we need to be careful about modifying the IR tree while also
    // iterating it, and to keep things simple for ourselves we'll go ahead
    // and build up a list of instruction to remove along the way, and then
    // remove them all at the end.
    //
    List<IRInst*> instsToRemove;

    // The overall flow of the pass is pretty simple, so we will walk through it now.
    //
    void processModule()
    {
        // We start by initializing our IR building state.
        //
        sharedBuilderStorage.session = module->session;
        sharedBuilderStorage.module = module;
        builderStorage.sharedBuilder = &sharedBuilderStorage;

        // Next, we will search for any instruction that create or use
        // union types, and process them accordingingly (usually by
        // constructing a new instruction to replace them).
        //
        processInstRec(module->getModuleInst());

        // Along the way we will build up a list of the tagged union
        // types that we encountered, but we will refrain from replacing
        // them until we are done (so that we always know that the instructions
        // we process above refer to the original type, and not its
        // replacement.
        //
        for( auto info : taggedUnionInfos )
        {
            auto taggedUnionType = info->taggedUnionType;
            auto replacementInst = info->replacementInst;

            // TODO: We should consider transferring decorations from the source
            // type to the destination, but doing so carelessly could create
            // problems, since an IR struct type shouldn't have, e.g., a
            // `TaggedUnionTypeLayout` attached to it.

            taggedUnionType->replaceUsesWith(replacementInst);
            taggedUnionType->removeAndDeallocate();
        }

        // As described previously, we build up the `instsToRemove` list as
        // we iterate so that we can remove them all here and not risk
        // modifying the IR tree while also walking it.
        //
        // TODO: This might be overkill and we could conceivably just be
        // a bit careful in `processInstRec`.
        //
        for(auto inst : instsToRemove)
        {
            inst->removeAndDeallocate();
        }
    }

    // In order to replace a (tagged) union type, we will need to know
    // something about it, and we will use the `TaggedUnionInfo` type
    // to collect all the relevant information.
    //
    struct TaggedUnionInfo : public RefObject
    {
        // We obviously need to know the tagged union itself, and
        // we will also use this structure to track the instruction
        // (an IR struct type) that will replace it.
        //
        IRTaggedUnionType*  taggedUnionType;
        IRInst*             replacementInst;

        // In order to compute a suitable layout for the replacement
        // `struct` type we need to know how the tagged union itself
        // would be laid out in memory, so we require that all tagged
        // unions in the generated IR have an associated (target-specific)
        // layout.
        //
        IRTaggedUnionTypeLayout* taggedUnionTypeLayout;

        // The basic approach we will use 16-byte chunks (represented as an array
        // of `uint4`s) to reprent the "bulk" of a type, and then use a single field
        // that could be up to 12 bytes to represent the "rest" of the type.
        //
        // Note that there are deeply ingrained assumptions here that all types
        // are at least four bytes in size (so that unions cannot easily
        // accomodate `half` value), and that any types *larger* than four bytes
        // will need to be loaded/stored via multiple 4-byte loads/stores.
        //
        // With the basic idea out of the way, we need an IR level field
        // in our struct to hold the bulk data, which comprises a "key" for
        // looking up the field, and the type of the field itself. We also
        // keep track of how many bytes we put in our bulk storage.
        //
        // The bulk field might be:
        //
        // - null, if none of the case types was 16 bytes or more
        // - a single `uint4` for between 16 and 31 (inclusive) bytes
        // - an array of `uint4`s for 32 or more bytes
        //
        UInt64  bulkSize = 0;
        IRInst* bulkFieldKey = nullptr;
        IRType* bulkFieldType = nullptr;

        // The same basic idea then applies to the rest of the data.
        //
        // The "rest" field will be either be absent (if the size of the
        // type was evently divisible by 16), a scalar `uint`, or else
        // a 2- or 3-component vector of `uint`.
        //
        UInt64  restSize = 0;
        IRInst* restFieldKey = nullptr;
        IRType* restFieldType = nullptr;

        // Finally, since we are currently working with tagged unions,
        // we need a field to hold the tag, which will always be allocated
        // after the fields that hold the bulk/rest of the payload.
        //
        // This field is always a single `uint`.
        //
        // TODO: if/when we support untagged unions, they could be handled
        // by having this field be null.
        //
        IRInst* tagFieldKey;
    };

    // We will build up a list of all the tagged union types we encounter,
    // so that we can replace them with the synthesized types when we are done.
    //
    List<RefPtr<TaggedUnionInfo>> taggedUnionInfos;

    // It is possible that we will see the same tagged union type referenced
    // many times in the IR, but we only want to synthesize the information
    // above (including the various IR structures) once, so we also maintain
    // a map from the original IR type to the corresponding information.
    //
    Dictionary<IRInst*, TaggedUnionInfo*> mapIRTypeToTaggedUnionInfo;

    // We will process all instructions in the module in a single recursive walk.
    //
    void processInstRec(IRInst* inst)
    {
        processInst(inst);

        for( auto child : inst->getChildren() )
        {
            processInstRec(child);
        }
    }
    //
    // At each instruction, we will check if it is one of the union-related instructions
    // we need to replace, and process it accordingly.
    //
    void processInst(IRInst* inst)
    {
        switch( inst->getOp() )
        {
        default:
            // Any instruction not listed below either doesn't involve union types,
            // or handles them in a hands-off fashion that we don't need to care about.
            //
            // E.g., a `load` of a union type from a constant buffer will turn into
            // a load of the replacement `struct` type once we are done, and nothing
            // needs to be done to the `load` instruction.
            //
            break;

        case kIROp_TaggedUnionType:
            {
                // We clearly need to process the tagged union type itself, but the actual
                // work is handled by other functions. All we need to do here is ensure
                // that the information for this type gets generated, and then we can
                // rely on the main `processModule` function to do the actual replacement later.
                //
                auto type = cast<IRTaggedUnionType>(inst);
                getTaggedUnionInfo(type);
            }
            break;

        case kIROp_ExtractTaggedUnionTag:
            {
                // The case of extracting the tag from a tagged union is relatively
                // simple, because the replacement type will have a dedicated field or it.
                //
                // We start by finding the tagged union value the instruction is operating
                // on, and then looking up the information for its type (which had
                // better be a tagged union type).
                //
                auto taggedUnionVal = inst->getOperand(0);
                auto taggedUnionInfo = getTaggedUnionInfo(taggedUnionVal->getDataType());

                // Because the replacement type will have an explicit field for the tag,
                // we can simply emit a single field-extract instruction to read its value
                // out.
                //
                auto builder = getBuilder();
                builder->setInsertBefore(inst);
                auto replacement = builder->emitFieldExtract(
                    inst->getFullType(),
                    taggedUnionVal,
                    taggedUnionInfo->tagFieldKey);

                // Now we can replace anything that used the original instruction with
                // the new field-extract operation, and add this instruction to the
                // list for later removal.
                //
                inst->replaceUsesWith(replacement);
                instsToRemove.add(inst);
            }
            break;

        case kIROp_ExtractTaggedUnionPayload:
            {
                // The most interesting case is when we are trying to extract a particular
                // payload (one of the case types) from a union. We may need to extract
                // one or more fields from the data stored in the union's replacement
                // type (the bulk/rest fields), and we may also have to convert them
                // to the type expected via bit-casts.

                // We can start things off easily enough by extracting the tagged union
                // value being operated on, as well as the information for its type.
                //
                auto taggedUnionVal = inst->getOperand(0);
                auto taggedUnionInfo = getTaggedUnionInfo(taggedUnionVal->getDataType());

                // Next we need to figure out which case is being extracted from the union.
                // The operand for the case tag should be a literal by construction.
                //
                auto caseTagVal = inst->getOperand(1);
                auto caseTagConst = as<IRIntLit>(caseTagVal);
                SLANG_ASSERT(caseTagConst);

                // The case type we are extracting will be the result type of the instruciton.
                //
                auto caseType = inst->getDataType();
                //
                // The tag value itself will be the index of the case type in the union
                // type (and its layout).
                //
                auto caseTagIndex = UInt(caseTagConst->getValue());

                // We can use the case tag value to look up the layout for the particular
                // case type we are extracting (this will allow us to resolve byte offsets
                // for fields, etc.).
                //
                auto taggedUnionTypeLayout = taggedUnionInfo->taggedUnionTypeLayout;
                SLANG_ASSERT(caseTagIndex < UInt(taggedUnionTypeLayout->getCaseCount()));
                auto caseTypeLayout = taggedUnionTypeLayout->getCaseTypeLayout(caseTagIndex);

                // At this point we know the type we are trying to extract, as well
                // as its layout. We will defer the actual implementation of extraction
                // to a (recursive) subroutine that can extract a (sub-)field from the
                // union at a given byte offset. Since we are extracting a full case
                // right now, the byte offset will be zero.
                //
                auto payloadVal = extractPayload(
                    taggedUnionInfo,
                    taggedUnionVal,
                    caseType,
                    caseTypeLayout,
                    0);

                // TODO: There is a significant flaw in the above approach when
                // the case type might be (or contain) an array. If we have a setup
                // like the following:
                //
                //      union SomeUnion { float someCase[100]; ... }
                //      ...
                //      float result = someUnion.someCase[someIndex];
                //
                // The current logic would desugar this into something like:
                //
                //      struct SomeUnion { uint4 bulk[100]; ... }
                //      ...
                //      float[] tmp = { asfloat(someUnion.bulk[0].x), asfloat(someUnion.bulk[1].x), ... }
                //      float result = tmp[someIndex];
                //
                // The result is that we copy an entire 100-element array into local memory
                // just to fetch a single element, when it would be much nicer to just do:
                //
                //      float result = asfloat(someUnion.bulk[someIndex].x);
                //
                // Achieving the latter code requires that rather than blindly translate
                // the `extractTaggedUnionPayload` instruction into a semantically equiavlent
                // value (which might lead to a big copy in the end), we should transitively
                // chase down any "access chains" off of `inst` and see what leaf values are
                // actually needed, and generated more tailored extraction logic for just
                // the elements/fields that actually get referenced.
                //
                // The more refined approach can be built on top of many of the same primitives,
                // so for now we will resign ourselves to the simpler but potentially less
                // efficient approach.

                // Now that we've extracted the value for the payload from the fields of
                // the replacement struct, we can use that extracted value to replace
                // this instruction, and schedule the original instruction for removal.
                //
                inst->replaceUsesWith(payloadVal);
                instsToRemove.add(inst);
            }
            break;
        }
    }

    // The `extractPayload` operation is the most important bit of translation we
    // need to do to make unions work. We have as input the following:
    //
    IRInst* extractPayload(

        // - Information about a tagged union type and its layout.
        TaggedUnionInfo*    taggedUnionInfo,

        // - A single value of that tagged unon type.
        IRInst*             taggedUnionVal,

        // - Type type of some "payload" field we want to extract from the union.
        IRType*             payloadType,

        // - The memory layout of that payload type.
        IRTypeLayout*       payloadTypeLayout,

        // - The byte offset at which we want to fetch the payload.
        UInt64              payloadOffset)
    {
        // We are going to be building some IR code no matter what.
        //
        auto builder = getBuilder();

        // The basic approach here will be to look at the type we
        // are trying to extract from the union, and whenever possible
        // recursively walk its structure so that we can express things
        // in terms of extraction of smaller/simpler types.
        //
        if( auto irStructType = as<IRStructType>(payloadType) )
        {
            // A structure type is a nice recursive case: we simply
            // want to extract each of its field recursively, and
            // then construct a fresh value of the `struct` type.

            // In all of the cases of this function we expect/require
            // there to be complete type layout information for the
            // types involved.
            //
            auto structTypeLayout = as<IRStructTypeLayout>(payloadTypeLayout);
            SLANG_ASSERT(structTypeLayout);

            // We are going to emit code to extract each of the fields
            // and collect them to use as operands to a `makeStruct`.
            //
            List<IRInst*> fieldVals;

            // We need to walk over the fields in the order the IR expects them
            UInt fieldCounter = 0;
            for( auto irField : irStructType->getFields() )
            {
                IRType* fieldType = irField->getFieldType();

                // TODO: We need to confirm/enforce that the fields of the
                // IR struct and the fields of the layout still align.
                //
                UInt fieldIndex = fieldCounter++;
                auto fieldLayout = structTypeLayout->getFieldLayout(fieldIndex);
                auto fieldTypeLayout = fieldLayout->getTypeLayout();

                // The offset of the field can be computed from the base
                // offset passed in, plus the reflection data for the field.
                //
                UInt64 fieldOffset = payloadOffset;
                if(auto resInfo = fieldLayout->findOffsetAttr(LayoutResourceKind::Uniform))
                    fieldOffset += resInfo->getOffset();

                // We make a recursive call to extract each field, expecting
                // that this will bottom out eventually.
                //
                IRInst* fieldVal = extractPayload(
                    taggedUnionInfo,
                    taggedUnionVal,
                    fieldType,
                    fieldTypeLayout,
                    fieldOffset);
                fieldVals.add(fieldVal);
            }

            // The final value is then just a new struct constructed from
            // the extracted field values.
            //
            auto payloadVal = builder->emitMakeStruct(irStructType, fieldVals);
            return payloadVal;
        }
        else if( auto vecType = as<IRVectorType>(payloadType) )
        {
            auto elementType = vecType->getElementType();

            // We expect that by the time we are desugaring union types
            // all vector types have literal constant values for their
            // element count.
            //
            auto elementCountVal = vecType->getElementCount();
            auto elementCountConst = as<IRIntLit>(elementCountVal);
            SLANG_ASSERT(elementCountConst);
            UInt elementCount = UInt(elementCountConst->getValue());

            // HACK: There is currently no `VectorTypeLayout` and thus
            // no way to query the layout of the elements of a vector
            // type. Until that gets added we will kludge things here.
            //
            IRTypeLayout* elementTypeLayout = nullptr;
            size_t elementSize = 0;
            if(auto resInfo = payloadTypeLayout->findSizeAttr(LayoutResourceKind::Uniform))
                elementSize = resInfo->getSize().getFiniteValue() / elementCount;

            // Similar to the `struct` case above, we will extract a
            // value for each element of the vector, and then use
            // `makeVector` to construct the result value.
            //
            List<IRInst*> elementVals;
            for(UInt ii = 0; ii < elementCount; ++ii)
            {
                auto elementVal = extractPayload(
                    taggedUnionInfo,
                    taggedUnionVal,
                    elementType,
                    elementTypeLayout,
                    payloadOffset + ii*elementSize);
                elementVals.add(elementVal);
            }
            return builder->emitMakeVector(vecType, elementVals);
        }
        else if( auto matType = as<IRMatrixType>(payloadType) )
        {
            SLANG_UNIMPLEMENTED_X("matrix in union type");
        }
        else if( auto arrayType = as<IRArrayType>(payloadType) )
        {
            SLANG_UNIMPLEMENTED_X("array in union type");
        }
        else
        {
            // If none of the above cases match, then we assume that
            // we have an individual scalar field that we need to fetch.
            //
            UInt64 payloadSize = 0;
            if( auto resInfo = payloadTypeLayout->findSizeAttr(LayoutResourceKind::Uniform) )
            {
                // TODO: somebody before this point should generate an error if
                // we have a `union` type that contains a potentially unbounded
                // amount of data.
                //
                payloadSize = resInfo->getSize().getFiniteValue();
            }

            if( payloadSize != 4 )
            {
                // TODO: We should handle the case of 64-bit fields by fetching
                // two `uint` values to form a `uint2`, and then using an
                // appropriate bit-cast to get from `uint2` to, e.g., `double`.
                //
                // The case of 16-bit and smaller fields is more troublesome, but
                // in the worst case we can load a `uint` and then use bitwise
                // ops to extract what we need before bitcasting.
                //
                // The right long-term solution is for downstream languages to have
                // better support for raw memory addressing.

                SLANG_UNIMPLEMENTED_X("leaf union field with size other than 4 bytes");
            }

            // We know that we want to fetch a value of size `payloadSize`, and
            // we have a known base value and an initial offset into it.
            //
            IRInst* baseVal = taggedUnionVal;
            UInt64 offset = payloadOffset;

            // We are going to refine our `baseVal` and `offset` as we go, by
            // trying to narrow down the data we will access in the `struct`
            // type that will provide storage for the union.
            //
            // The first thing we want to check is if the value sits in the
            // "bulk" part of the storage, or the "rest."
            //
            UInt64 bulkSize = taggedUnionInfo->bulkSize;
            if( offset < bulkSize )
            {
                // If the value starts in the bulk area, then the whole
                // thing had better fit in the bulk area. The 16-byte
                // granularity rules for constant buffers should ensure
                // this property for us on current targets.
                //
                SLANG_ASSERT(offset + payloadSize <= bulkSize);

                // Since we know we'll be accessing the bulk storage,
                // we will extract it here. The extracted field will
                // be our new base value, but the `offset` doesn't need
                // to be updated since the bulk field sits at offset 0.
                //
                baseVal = builder->emitFieldExtract(
                    taggedUnionInfo->bulkFieldType,
                    baseVal,
                    taggedUnionInfo->bulkFieldKey);

                // The bulk storage could be an array, if there are 32
                // or more bytes of bulk storage.
                //
                if( auto baseArrayType = as<IRArrayType>(baseVal->getDataType()) )
                {
                    // If an array was allocated for bulk storage then
                    // our leaf value resides entirely within a single
                    // element (due to constant buffer layout rules),
                    // and so we will fetch the appropriate element here.
                    //
                    // We will change our `baseVal` to the extracted element,
                    // and then also adjust our `offset` to be relative
                    // to that element.
                    //
                    size_t bulkElementSize = 16;
                    auto index = offset / bulkElementSize;
                    baseVal = builder->emitElementExtract(
                        baseArrayType->getElementType(),
                        baseVal,
                        builder->getIntValue(builder->getIntType(), index));
                    offset -= index*bulkElementSize;
                }
            }
            else
            {
                // If the offset of the field we want is past the end of
                // the bulk field then it must sit inside of the rest field,
                // and we'll extract it here. This establishes a new
                // base value, and we adjust the `offset` to be relative
                // to the rest field (which starts at an offset equal to `bulkSize`).
                //
                baseVal = builder->emitFieldExtract(
                    taggedUnionInfo->restFieldType,
                    baseVal,
                    taggedUnionInfo->restFieldKey);
                offset -= bulkSize;
            }

            // We've now extracted a field that could be either a scalar or
            // a vector, and we have an offset into it. In the case where
            // the base value is a vector, we will extract out the appropriate
            // element.
            //
            if( auto baseVecType = as<IRVectorType>(baseVal->getDataType()) )
            {
                size_t vecElementSize = 4;
                auto index = offset / vecElementSize;
                baseVal = builder->emitElementExtract(
                    baseVecType->getElementType(),
                    baseVal,
                    builder->getIntValue(builder->getIntType(), index));
                offset -= index*vecElementSize;
            }

            // At this point, our `baseVal` should be a single `uint`, and
            // it should provide the storage for the exact thing we wanted
            // to access (under the assumption that we always fetch 4 bytes
            // on 4-byte alignment).
            //
            IRInst* payloadVal = baseVal;
            SLANG_ASSERT(offset == 0);

            // TODO: we could imagine adding logic here to handle types less
            // than 4 bytes in size by shifting and masking the value we
            // just loaded.

            // The payload field we were trying to extract might have a type
            // other than `uint`, and to handle that case we need to employ
            // a bit-cast to get to the desired type.
            //
            if( payloadVal->getDataType() != payloadType )
            {
                payloadVal = builder->emitBitCast(
                    payloadType,
                    payloadVal);
            }
            return payloadVal;
        }
    }

    // All of the logic so far as assumed we can just call `getTaggedUnionInfo`
    // and have easy access to all the required information and the
    // synthesized replacement type.
    //
    TaggedUnionInfo* getTaggedUnionInfo(IRType* type)
    {
        // The big picture is fairly simple: we will lazily build and
        // memoize the information about tagged unions.
        //
        {
            TaggedUnionInfo* info = nullptr;
            if(mapIRTypeToTaggedUnionInfo.TryGetValue(type, info))
                return info;
        }

        // When we don't find information in our memo-cache, we
        // will construct it and add it to both the memo-cache
        // *and* a global list of all tagged unions encountered,
        // so that we can replacement them later.
        //
        auto info = createTaggedUnionInfo(type);
        mapIRTypeToTaggedUnionInfo.Add(type, info.Ptr());
        taggedUnionInfos.add(info);

        return info;
    }

    // The actual logic for creating a `TaggedUnionInfo` is relatively
    // straightforward once we've decided what information we need.
    //
    RefPtr<TaggedUnionInfo> createTaggedUnionInfo(IRType* type)
    {
        // We expect that any type used as an operation to one of the
        // `extractTaggedUnion*` operations must be an IR tagged union.
        //
        // Note: If/when we ever expose `union`s to user and allow
        // then to create *generic* tagged union types it might appear
        // that this needs to be changed to account for a `specialize`
        // instruction in place of a concrete tagged union, but in
        // practice this pass needs to be performed late enough that
        // any such generic should be fully specialized.
        //
        auto taggedUnionType = as<IRTaggedUnionType>(type);
        SLANG_ASSERT(taggedUnionType);

        RefPtr<TaggedUnionInfo> info = new TaggedUnionInfo();
        info->taggedUnionType = taggedUnionType;

        // We are going to create an instruction to replace `type`,
        // and thus will be placing it into the same parent.
        //
        auto builder = getBuilder();
        builder->setInsertBefore(type);

        // A tagged union type will be replaced with an ordinary
        // `struct` type with fields to store all the relevant
        // data from any of the cases, plus a tag field.
        //
        auto structType = builder->createStructType();
        info->replacementInst = structType;

        // We require/expect the earlier code generation steps to have
        // associated a layout with every tagged union that appears in
        // the code.
        //
        auto layoutDecoration = type->findDecoration<IRLayoutDecoration>();
        SLANG_ASSERT(layoutDecoration);
        auto layout = layoutDecoration->getLayout();
        SLANG_ASSERT(layout);
        auto taggedUnionTypeLayout = as<IRTaggedUnionTypeLayout>(layout);
        SLANG_ASSERT(taggedUnionTypeLayout);

        info->taggedUnionTypeLayout = taggedUnionTypeLayout;

        // The size of the "payload" for the different cases (everything but
        // the tag) is taken to be the offset of the tag itself.
        //
        // TODO: this might be inaccurate if the payload size isn't a multiple
        // of the tag's alignment. We should deal with that when/if we support
        // types smaller than 4 bytes in unions.
        //
        auto payloadSize = taggedUnionTypeLayout->getTagOffset().getFiniteValue();

        // We are going to be construction IR code that makes use of the `int`
        // and `uint` types in several cases, so we go ahead and get a pointer
        // to those types here.
        //
        auto intType = getBuilder()->getIntType();
        auto uintType = getBuilder()->getBasicType(BaseType::UInt);

        // For now we will use a simple stragegy for how we encode a union,
        // which depends only on the total number of bytes needed, and not
        // on the makeup of the values being stored.
        //
        // We will start by allocating one or more `uint4` values (in an
        // array for the "or more" case) to hold the bulk of any large
        // payload value.
        //
        size_t bulkVectorSize = 16; // Note: assuming `sizeof(uint4) == 16` on all targets
        auto bulkVectorCount = payloadSize / bulkVectorSize;
        auto bulkFieldSize = bulkVectorCount * bulkVectorSize;
        if( bulkVectorCount )
        {
            IRType* bulkFieldType = builder->getVectorType(
                uintType,
                builder->getIntValue(intType, 4));

            if( bulkVectorCount > 1 )
            {
                bulkFieldType = builder->getArrayType(
                    bulkFieldType,
                    builder->getIntValue(intType, bulkVectorCount));
            }

            auto bulkFieldKey = builder->createStructKey();
            builder->createStructField(structType, bulkFieldKey, bulkFieldType);

            info->bulkFieldKey = bulkFieldKey;
            info->bulkFieldType = bulkFieldType;
        }
        info->bulkSize = bulkFieldSize;

        // The rest of the data (anything that doesn't fit in the bulk field),
        // will get allocated into a single scalar or vector of `uint`.
        //
        auto restSize = payloadSize - bulkFieldSize;
        if( restSize )
        {
            size_t restElementSize = 4; // assuming `sizeof(uint) == 4` on all targets
            auto restElementCount = restSize / restElementSize;
            auto restFieldSize = restElementSize * restElementCount;
            SLANG_ASSERT(restFieldSize == restSize); // Note: all our current targets have minimum 4-byte storage granularity

            IRType* restFieldType = uintType;
            if( restElementCount > 1 )
            {
                restFieldType = builder->getVectorType(
                    restFieldType,
                    builder->getIntValue(intType, restElementCount));
            }

            auto restFieldKey = builder->createStructKey();
            builder->createStructField(structType, restFieldKey, restFieldType);

            info->restFieldKey = restFieldKey;
            info->restFieldType = restFieldType;
            info->restSize = restFieldSize;
        }

        // Finally, we add a field to represent the tag.
        //
        auto tagFieldType = uintType;
        auto tagFieldKey = builder->createStructKey();
        builder->createStructField(structType, tagFieldKey, tagFieldType);

        info->tagFieldKey = tagFieldKey;

        return info;
    }
};

void desugarUnionTypes(
    IRModule*       module)
{
    DesugarUnionTypesContext context;
    context.module = module;

    context.processModule();
}

} // namespace Slang
