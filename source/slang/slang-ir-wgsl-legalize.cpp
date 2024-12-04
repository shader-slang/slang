#include "slang-ir-wgsl-legalize.h"

#include "slang-ir-insts.h"
#include "slang-ir-legalize-varying-params.h"
#include "slang-ir-legalize-global-values.h"
#include "slang-ir-util.h"
#include "slang-ir.h"
#include "slang-parameter-binding.h"

#include <set>

namespace Slang
{

struct EntryPointInfo
{
    IRFunc* entryPointFunc;
    IREntryPointDecoration* entryPointDecor;
};

struct LegalizeWGSLEntryPointContext
{
    HashSet<IRStructField*> semanticInfoToRemove;
    UnownedStringSlice userSemanticName = toSlice("user_semantic");

    DiagnosticSink* m_sink;
    IRModule* m_module;

    LegalizeWGSLEntryPointContext(DiagnosticSink* sink, IRModule* module)
        : m_sink(sink), m_module(module)
    {
    }

    void removeSemanticLayoutsFromLegalizedStructs()
    {
        // WGSL does not allow duplicate attributes to appear in the same shader.
        // If we emit our own struct with `[[color(0)]`, all existing uses of `[[color(0)]]`
        // must be removed.
        for (auto field : semanticInfoToRemove)
        {
            auto key = field->getKey();
            // Some decorations appear twice, destroy all found
            for (;;)
            {
                if (auto semanticDecor = key->findDecoration<IRSemanticDecoration>())
                {
                    semanticDecor->removeAndDeallocate();
                    continue;
                }
                else if (auto layoutDecor = key->findDecoration<IRLayoutDecoration>())
                {
                    layoutDecor->removeAndDeallocate();
                    continue;
                }
                break;
            }
        }
    }

    // Flattens all struct parameters of an entryPoint to ensure parameters are a flat struct
    void flattenInputParameters(EntryPointInfo entryPoint)
    {
        // Goal is to ensure we have a flattened IRParam (0 nested IRStructType members).
        /*
            // Assume the following code
            struct NestedFragment
            {
                float2 p3;
            };
            struct Fragment
            {
                float4 p1;
                float3 p2;
                NestedFragment p3_nested;
            };

            // Fragment flattens into
            struct Fragment
            {
                float4 p1;
                float3 p2;
                float2 p3;
            };
        */

        // This is important since WGSL does not allow semantic's on a struct
        /*
            // Assume the following code
            struct NestedFragment1
            {
                float2 p3;
            };
            struct Fragment1
            {
                float4 p1 : SV_TARGET0;
                float3 p2 : SV_TARGET1;
                NestedFragment p3_nested : SV_TARGET2; // error, semantic on struct
            };

        */

        // Unlike Metal, WGSL does NOT allow semantics on members of a nested struct.
        /*
            // Assume the following code
            struct NestedFragment
            {
                float2 p3;
            };
            struct Fragment
            {
                float4 p1 : SV_TARGET0;
                NestedFragment p2 : SV_TARGET1;
                NestedFragment p3 : SV_TARGET2;
            };

            // Legalized with flattening
            struct Fragment
            {
                float4 p1 : SV_TARGET0;
                float2 p2 : SV_TARGET1;
                float2 p3 : SV_TARGET2;
            };
        */

        auto func = entryPoint.entryPointFunc;
        bool modified = false;
        for (auto param : func->getParams())
        {
            auto layout = findVarLayout(param);
            if (!layout)
                continue;
            if (!layout->findOffsetAttr(LayoutResourceKind::VaryingInput))
                continue;
            if (param->findDecorationImpl(kIROp_HLSLMeshPayloadDecoration))
                continue;
            // If we find a IRParam with a IRStructType member, we need to flatten the entire
            // IRParam
            if (auto structType = as<IRStructType>(param->getDataType()))
            {
                IRBuilder builder(func);
                MapStructToFlatStruct mapOldFieldToNewField;

                // Flatten struct if we have nested IRStructType
                auto flattenedStruct = maybeFlattenNestedStructs(
                    builder,
                    structType,
                    mapOldFieldToNewField,
                    semanticInfoToRemove);
                // Validate/rearange all semantics which overlap in our flat struct.
                fixFieldSemanticsOfFlatStruct(flattenedStruct);
                ensureStructHasUserSemantic<LayoutResourceKind::VaryingInput>(
                    flattenedStruct,
                    layout);
                if (flattenedStruct != structType)
                {
                    // Replace the 'old IRParam type' with a 'new IRParam type'
                    param->setFullType(flattenedStruct);

                    // Emit a new variable at EntryPoint of 'old IRParam type'
                    builder.setInsertBefore(func->getFirstBlock()->getFirstOrdinaryInst());
                    auto dstVal = builder.emitVar(structType);
                    auto dstLoad = builder.emitLoad(dstVal);
                    param->replaceUsesWith(dstLoad);
                    builder.setInsertBefore(dstLoad);
                    // Copy the 'new IRParam type' to our 'old IRParam type'
                    mapOldFieldToNewField
                        .emitCopy<(int)MapStructToFlatStruct::CopyOptions::FlatStructIntoStruct>(
                            builder,
                            dstVal,
                            param);

                    modified = true;
                }
            }
        }
        if (modified)
            fixUpFuncType(func);
    }

    struct WGSLSystemValueInfo
    {
        String wgslSystemValueName;
        SystemValueSemanticName wgslSystemValueNameEnum;
        ShortList<IRType*> permittedTypes;
        bool isUnsupported = false;
        WGSLSystemValueInfo()
        {
            // most commonly need 2
            permittedTypes.reserveOverflowBuffer(2);
        }
    };

    WGSLSystemValueInfo getSystemValueInfo(
        String inSemanticName,
        String* optionalSemanticIndex,
        IRInst* parentVar)
    {
        IRBuilder builder(m_module);
        WGSLSystemValueInfo result = {};
        UnownedStringSlice semanticName;
        UnownedStringSlice semanticIndex;

        auto hasExplicitIndex =
            splitNameAndIndex(inSemanticName.getUnownedSlice(), semanticName, semanticIndex);
        if (!hasExplicitIndex && optionalSemanticIndex)
            semanticIndex = optionalSemanticIndex->getUnownedSlice();

        result.wgslSystemValueNameEnum = convertSystemValueSemanticNameToEnum(semanticName);

        switch (result.wgslSystemValueNameEnum)
        {

        case SystemValueSemanticName::CullDistance:
            {
                result.isUnsupported = true;
            }
            break;

        case SystemValueSemanticName::ClipDistance:
            {
                // TODO: Implement this based on the 'clip-distances' feature in WGSL
                // https: // www.w3.org/TR/webgpu/#dom-gpufeaturename-clip-distances
                result.isUnsupported = true;
            }
            break;

        case SystemValueSemanticName::Coverage:
            {
                result.wgslSystemValueName = toSlice("sample_mask");
                result.permittedTypes.add(builder.getUIntType());
            }
            break;

        case SystemValueSemanticName::Depth:
            {
                result.wgslSystemValueName = toSlice("frag_depth");
                result.permittedTypes.add(builder.getBasicType(BaseType::Float));
            }
            break;

        case SystemValueSemanticName::DepthGreaterEqual:
        case SystemValueSemanticName::DepthLessEqual:
            {
                result.isUnsupported = true;
            }
            break;

        case SystemValueSemanticName::DispatchThreadID:
            {
                result.wgslSystemValueName = toSlice("global_invocation_id");
                result.permittedTypes.add(builder.getVectorType(
                    builder.getBasicType(BaseType::UInt),
                    builder.getIntValue(builder.getIntType(), 3)));
            }
            break;

        case SystemValueSemanticName::DomainLocation:
            {
                result.isUnsupported = true;
            }
            break;

        case SystemValueSemanticName::GroupID:
            {
                result.wgslSystemValueName = toSlice("workgroup_id");
                result.permittedTypes.add(builder.getVectorType(
                    builder.getBasicType(BaseType::UInt),
                    builder.getIntValue(builder.getIntType(), 3)));
            }
            break;

        case SystemValueSemanticName::GroupIndex:
            {
                result.wgslSystemValueName = toSlice("local_invocation_index");
                result.permittedTypes.add(builder.getUIntType());
            }
            break;

        case SystemValueSemanticName::GroupThreadID:
            {
                result.wgslSystemValueName = toSlice("local_invocation_id");
                result.permittedTypes.add(builder.getVectorType(
                    builder.getBasicType(BaseType::UInt),
                    builder.getIntValue(builder.getIntType(), 3)));
            }
            break;

        case SystemValueSemanticName::GSInstanceID:
            {
                // No Geometry shaders in WGSL
                result.isUnsupported = true;
            }
            break;

        case SystemValueSemanticName::InnerCoverage:
            {
                result.isUnsupported = true;
            }
            break;

        case SystemValueSemanticName::InstanceID:
            {
                result.wgslSystemValueName = toSlice("instance_index");
                result.permittedTypes.add(builder.getUIntType());
            }
            break;

        case SystemValueSemanticName::IsFrontFace:
            {
                result.wgslSystemValueName = toSlice("front_facing");
                result.permittedTypes.add(builder.getBoolType());
            }
            break;

        case SystemValueSemanticName::OutputControlPointID:
        case SystemValueSemanticName::PointSize:
            {
                result.isUnsupported = true;
            }
            break;

        case SystemValueSemanticName::Position:
            {
                result.wgslSystemValueName = toSlice("position");
                result.permittedTypes.add(builder.getVectorType(
                    builder.getBasicType(BaseType::Float),
                    builder.getIntValue(builder.getIntType(), 4)));
                break;
            }

        case SystemValueSemanticName::PrimitiveID:
        case SystemValueSemanticName::RenderTargetArrayIndex:
            {
                result.isUnsupported = true;
                break;
            }

        case SystemValueSemanticName::SampleIndex:
            {
                result.wgslSystemValueName = toSlice("sample_index");
                result.permittedTypes.add(builder.getUIntType());
                break;
            }

        case SystemValueSemanticName::StencilRef:
        case SystemValueSemanticName::Target:
        case SystemValueSemanticName::TessFactor:
            {
                result.isUnsupported = true;
                break;
            }

        case SystemValueSemanticName::VertexID:
            {
                result.wgslSystemValueName = toSlice("vertex_index");
                result.permittedTypes.add(builder.getUIntType());
                break;
            }

        case SystemValueSemanticName::ViewID:
        case SystemValueSemanticName::ViewportArrayIndex:
            {
                result.isUnsupported = true;
                break;
            }

        default:
            {
                m_sink->diagnose(
                    parentVar,
                    Diagnostics::unimplementedSystemValueSemantic,
                    semanticName);
                return result;
            }
        }

        return result;
    }

    void reportUnsupportedSystemAttribute(IRInst* param, String semanticName)
    {
        m_sink->diagnose(
            param->sourceLoc,
            Diagnostics::systemValueAttributeNotSupported,
            semanticName);
    }

    template<LayoutResourceKind K>
    void ensureStructHasUserSemantic(IRStructType* structType, IRVarLayout* varLayout)
    {
        // Ensure each field in an output struct type has either a system semantic or a user
        // semantic, so that signature matching can happen correctly.
        auto typeLayout = as<IRStructTypeLayout>(varLayout->getTypeLayout());
        Index index = 0;
        IRBuilder builder(structType);
        for (auto field : structType->getFields())
        {
            auto key = field->getKey();
            if (auto semanticDecor = key->findDecoration<IRSemanticDecoration>())
            {
                if (semanticDecor->getSemanticName().startsWithCaseInsensitive(toSlice("sv_")))
                {
                    auto indexAsString = String(UInt(semanticDecor->getSemanticIndex()));
                    auto sysValInfo =
                        getSystemValueInfo(semanticDecor->getSemanticName(), &indexAsString, field);
                    if (sysValInfo.isUnsupported)
                    {
                        reportUnsupportedSystemAttribute(field, semanticDecor->getSemanticName());
                    }
                    else
                    {
                        builder.addTargetSystemValueDecoration(
                            key,
                            sysValInfo.wgslSystemValueName.getUnownedSlice());
                        semanticDecor->removeAndDeallocate();
                    }
                }
                index++;
                continue;
            }
            typeLayout->getFieldLayout(index);
            auto fieldLayout = typeLayout->getFieldLayout(index);
            if (auto offsetAttr = fieldLayout->findOffsetAttr(K))
            {
                UInt varOffset = 0;
                if (auto varOffsetAttr = varLayout->findOffsetAttr(K))
                    varOffset = varOffsetAttr->getOffset();
                varOffset += offsetAttr->getOffset();
                builder.addSemanticDecoration(key, toSlice("_slang_attr"), (int)varOffset);
            }
            index++;
        }
    }

    // Stores a hicharchy of members and children which map 'oldStruct->member' to
    // 'flatStruct->member' Note: this map assumes we map to FlatStruct since it is easier/faster to
    // process
    struct MapStructToFlatStruct
    {
        /*
        We need a hicharchy map to resolve dependencies for mapping
        oldStruct to newStruct efficently. Example:

        MyStruct
            |
          / | \
         /  |  \
        /   |   \
    M0<A> M1<A> M2<B>
       |    |    |
      A_0  A_0  B_0

      Without storing hicharchy information, there will be no way to tell apart
      `myStruct.M0.A0` from `myStruct.M1.A0` since IRStructKey/IRStructField
      only has 1 instance of `A::A0`
      */

        enum CopyOptions : int
        {
            // Copy a flattened-struct into a struct
            FlatStructIntoStruct = 0,

            // Copy a struct into a flattened-struct
            StructIntoFlatStruct = 1,
        };

    private:
        // Children of member if applicable.
        Dictionary<IRStructField*, MapStructToFlatStruct> members;

        // Field correlating to MapStructToFlatStruct Node.
        IRInst* node;
        IRStructKey* getKey()
        {
            SLANG_ASSERT(as<IRStructField>(node));
            return as<IRStructField>(node)->getKey();
        }
        IRInst* getNode() { return node; }
        IRType* getFieldType()
        {
            SLANG_ASSERT(as<IRStructField>(node));
            return as<IRStructField>(node)->getFieldType();
        }

        // Whom node maps to inside target flatStruct
        IRStructField* targetMapping;

        auto begin() { return members.begin(); }
        auto end() { return members.end(); }

        // Copies members of oldStruct to/from newFlatStruct. Assumes members of val1 maps to
        // members in val2 using `MapStructToFlatStruct`
        template<int copyOptions>
        static void _emitCopy(
            IRBuilder& builder,
            IRInst* val1,
            IRStructType* type1,
            IRInst* val2,
            IRStructType* type2,
            MapStructToFlatStruct& node)
        {
            for (auto& field1Pair : node)
            {
                auto& field1 = field1Pair.second;

                // Get member of val1
                IRInst* fieldAddr1 = nullptr;
                if constexpr (copyOptions == (int)CopyOptions::FlatStructIntoStruct)
                {
                    fieldAddr1 = builder.emitFieldAddress(type1, val1, field1.getKey());
                }
                else
                {
                    if (as<IRPtrTypeBase>(val1))
                        val1 = builder.emitLoad(val1);
                    fieldAddr1 = builder.emitFieldExtract(type1, val1, field1.getKey());
                }

                // If val1 is a struct, recurse
                if (auto fieldAsStruct1 = as<IRStructType>(field1.getFieldType()))
                {
                    _emitCopy<copyOptions>(
                        builder,
                        fieldAddr1,
                        fieldAsStruct1,
                        val2,
                        type2,
                        field1);
                    continue;
                }

                // Get member of val2 which maps to val1.member
                auto field2 = field1.getMapping();
                SLANG_ASSERT(field2);
                IRInst* fieldAddr2 = nullptr;
                if constexpr (copyOptions == (int)CopyOptions::FlatStructIntoStruct)
                {
                    if (as<IRPtrTypeBase>(val2))
                        val2 = builder.emitLoad(val1);
                    fieldAddr2 = builder.emitFieldExtract(type2, val2, field2->getKey());
                }
                else
                {
                    fieldAddr2 = builder.emitFieldAddress(type2, val2, field2->getKey());
                }

                // Copy val2/val1 member into val1/val2 member
                if constexpr (copyOptions == (int)CopyOptions::FlatStructIntoStruct)
                {
                    builder.emitStore(fieldAddr1, fieldAddr2);
                }
                else
                {
                    builder.emitStore(fieldAddr2, fieldAddr1);
                }
            }
        }

    public:
        void setNode(IRInst* newNode) { node = newNode; }
        // Get 'MapStructToFlatStruct' that is a child of 'parent'.
        // Make 'MapStructToFlatStruct' if no 'member' is currently mapped to 'parent'.
        MapStructToFlatStruct& getMember(IRStructField* member) { return members[member]; }
        MapStructToFlatStruct& operator[](IRStructField* member) { return getMember(member); }

        void setMapping(IRStructField* newTargetMapping) { targetMapping = newTargetMapping; }
        // Get 'MapStructToFlatStruct' that is a child of 'parent'.
        // Return nullptr if no member is mapped to 'parent'
        IRStructField* getMapping() { return targetMapping; }

        // Copies srcVal into dstVal using hicharchy map.
        template<int copyOptions>
        void emitCopy(IRBuilder& builder, IRInst* dstVal, IRInst* srcVal)
        {
            auto dstType = dstVal->getDataType();
            if (auto dstPtrType = as<IRPtrTypeBase>(dstType))
                dstType = dstPtrType->getValueType();
            auto dstStructType = as<IRStructType>(dstType);
            SLANG_ASSERT(dstStructType);

            auto srcType = srcVal->getDataType();
            if (auto srcPtrType = as<IRPtrTypeBase>(srcType))
                srcType = srcPtrType->getValueType();
            auto srcStructType = as<IRStructType>(srcType);
            SLANG_ASSERT(srcStructType);

            if constexpr (copyOptions == (int)CopyOptions::FlatStructIntoStruct)
            {
                // CopyOptions::FlatStructIntoStruct copy a flattened-struct (mapped member) into a
                // struct
                SLANG_ASSERT(node == dstStructType);
                _emitCopy<copyOptions>(
                    builder,
                    dstVal,
                    dstStructType,
                    srcVal,
                    srcStructType,
                    *this);
            }
            else
            {
                // CopyOptions::StructIntoFlatStruct copy a struct into a flattened-struct
                SLANG_ASSERT(node == srcStructType);
                _emitCopy<copyOptions>(
                    builder,
                    srcVal,
                    srcStructType,
                    dstVal,
                    dstStructType,
                    *this);
            }
        }
    };

    IRStructType* _flattenNestedStructs(
        IRBuilder& builder,
        IRStructType* dst,
        IRStructType* src,
        IRSemanticDecoration* parentSemanticDecoration,
        IRLayoutDecoration* parentLayout,
        MapStructToFlatStruct& mapFieldToField,
        HashSet<IRStructField*>& varsWithSemanticInfo)
    {
        // For all fields ('oldField') of a struct do the following:
        // 1. Check for 'decorations which carry semantic info' (IRSemanticDecoration,
        // IRLayoutDecoration), store these if found.
        //  * Do not propagate semantic info if the current node has *any* form of semantic
        //  information.
        // Update varsWithSemanticInfo.
        // 2. If IRStructType:
        //  2a. Recurse this function with 'decorations that carry semantic info' from parent.
        // 3. If not IRStructType:
        //  3a. Emit 'newField' with 'newKey' equal to 'oldField' and 'oldKey', respectively,
        //      where 'oldKey' is the key corresponding to 'oldField'.
        //      Add 'decorations which carry semantic info' to 'newField', and move all decorations
        //      of 'oldKey' to 'newKey'.
        //  3b. Store a mapping from 'oldField' to 'newField' in 'mapFieldToField'. This info is
        //  needed to copy between types.
        for (auto oldField : src->getFields())
        {
            auto& fieldMappingNode = mapFieldToField[oldField];
            fieldMappingNode.setNode(oldField);

            // step 1
            bool foundSemanticDecor = false;
            auto oldKey = oldField->getKey();
            IRSemanticDecoration* fieldSemanticDecoration = parentSemanticDecoration;
            if (auto oldSemanticDecoration = oldKey->findDecoration<IRSemanticDecoration>())
            {
                foundSemanticDecor = true;
                fieldSemanticDecoration = oldSemanticDecoration;
                parentLayout = nullptr;
            }

            IRLayoutDecoration* fieldLayout = parentLayout;
            if (auto oldLayout = oldKey->findDecoration<IRLayoutDecoration>())
            {
                fieldLayout = oldLayout;
                if (!foundSemanticDecor)
                    fieldSemanticDecoration = nullptr;
            }
            if (fieldSemanticDecoration != parentSemanticDecoration || parentLayout != fieldLayout)
                varsWithSemanticInfo.add(oldField);

            // step 2a
            if (auto structFieldType = as<IRStructType>(oldField->getFieldType()))
            {
                _flattenNestedStructs(
                    builder,
                    dst,
                    structFieldType,
                    fieldSemanticDecoration,
                    fieldLayout,
                    fieldMappingNode,
                    varsWithSemanticInfo);
                continue;
            }

            // step 3a
            auto newKey = builder.createStructKey();
            oldKey->transferDecorationsTo(newKey);

            auto newField = builder.createStructField(dst, newKey, oldField->getFieldType());
            copyNameHintAndDebugDecorations(newField, oldField);

            if (fieldSemanticDecoration)
                builder.addSemanticDecoration(
                    newKey,
                    fieldSemanticDecoration->getSemanticName(),
                    fieldSemanticDecoration->getSemanticIndex());

            if (fieldLayout)
            {
                IRLayout* oldLayout = fieldLayout->getLayout();
                List<IRInst*> instToCopy;
                // Only copy certain decorations needed for resolving system semantics
                for (UInt i = 0; i < oldLayout->getOperandCount(); i++)
                {
                    auto operand = oldLayout->getOperand(i);
                    if (as<IRVarOffsetAttr>(operand) || as<IRUserSemanticAttr>(operand) ||
                        as<IRSystemValueSemanticAttr>(operand) || as<IRStageAttr>(operand))
                        instToCopy.add(operand);
                }
                IRVarLayout* newLayout = builder.getVarLayout(instToCopy);
                builder.addLayoutDecoration(newKey, newLayout);
            }
            // step 3b
            fieldMappingNode.setMapping(newField);
        }

        return dst;
    }

    // Returns a `IRStructType*` without any `IRStructType*` members. `src` may be returned if there
    // was no struct flattening.
    // @param mapFieldToField Behavior maps all `IRStructField` of `src` to the new struct
    // `IRStructFields`s
    IRStructType* maybeFlattenNestedStructs(
        IRBuilder& builder,
        IRStructType* src,
        MapStructToFlatStruct& mapFieldToField,
        HashSet<IRStructField*>& varsWithSemanticInfo)
    {
        // Find all values inside struct that need flattening and legalization.
        bool hasStructTypeMembers = false;
        for (auto field : src->getFields())
        {
            if (as<IRStructType>(field->getFieldType()))
            {
                hasStructTypeMembers = true;
                break;
            }
        }
        if (!hasStructTypeMembers)
            return src;

        // We need to:
        // 1. Make new struct 1:1 with old struct but without nestested structs (flatten)
        // 2. Ensure semantic attributes propegate. This will create overlapping semantics (can be
        // handled later).
        // 3. Store the mapping from old to new struct fields to allow copying a old-struct to
        // new-struct.
        builder.setInsertAfter(src);
        auto newStruct = builder.createStructType();
        copyNameHintAndDebugDecorations(newStruct, src);
        mapFieldToField.setNode(src);
        return _flattenNestedStructs(
            builder,
            newStruct,
            src,
            nullptr,
            nullptr,
            mapFieldToField,
            varsWithSemanticInfo);
    }

    // Replaces all 'IRReturn' by copying the current 'IRReturn' to a new var of type 'newType'.
    // Copying logic from 'IRReturn' to 'newType' is controlled by 'copyLogicFunc' function.
    template<typename CopyLogicFunc>
    void _replaceAllReturnInst(
        IRBuilder& builder,
        IRFunc* targetFunc,
        IRStructType* newType,
        CopyLogicFunc copyLogicFunc)
    {
        for (auto block : targetFunc->getBlocks())
        {
            if (auto returnInst = as<IRReturn>(block->getTerminator()))
            {
                builder.setInsertBefore(returnInst);
                auto returnVal = returnInst->getVal();
                returnInst->setOperand(0, copyLogicFunc(builder, newType, returnVal));
            }
        }
    }

    UInt _returnNonOverlappingAttributeIndex(std::set<UInt>& usedSemanticIndex)
    {
        // Find first unused semantic index of equal semantic type
        // to fill any gaps in user set semantic bindings
        UInt prev = 0;
        for (auto i : usedSemanticIndex)
        {
            if (i > prev + 1)
            {
                break;
            }
            prev = i;
        }
        usedSemanticIndex.insert(prev + 1);
        return prev + 1;
    }

    template<typename T>
    struct AttributeParentPair
    {
        IRLayoutDecoration* layoutDecor;
        T* attr;
    };

    IRLayoutDecoration* _replaceAttributeOfLayout(
        IRBuilder& builder,
        IRLayoutDecoration* parentLayoutDecor,
        IRInst* instToReplace,
        IRInst* instToReplaceWith)
    {
        // Replace `instToReplace` with a `instToReplaceWith`

        auto layout = parentLayoutDecor->getLayout();
        // Find the exact same decoration `instToReplace` in-case multiple of the same type exist
        List<IRInst*> opList;
        opList.add(instToReplaceWith);
        for (UInt i = 0; i < layout->getOperandCount(); i++)
        {
            if (layout->getOperand(i) != instToReplace)
                opList.add(layout->getOperand(i));
        }
        auto newLayoutDecor = builder.addLayoutDecoration(
            parentLayoutDecor->getParent(),
            builder.getVarLayout(opList));
        parentLayoutDecor->removeAndDeallocate();
        return newLayoutDecor;
    }

    IRLayoutDecoration* _simplifyUserSemanticNames(
        IRBuilder& builder,
        IRLayoutDecoration* layoutDecor)
    {
        // Ensure all 'ExplicitIndex' semantics such as "SV_TARGET0" are simplified into
        // ("SV_TARGET", 0) using 'IRUserSemanticAttr' This is done to ensure we can check semantic
        // groups using 'IRUserSemanticAttr1->getName() == IRUserSemanticAttr2->getName()'
        SLANG_ASSERT(layoutDecor);
        auto layout = layoutDecor->getLayout();
        List<IRInst*> layoutOps;
        layoutOps.reserve(3);
        bool changed = false;
        for (auto attr : layout->getAllAttrs())
        {
            if (auto userSemantic = as<IRUserSemanticAttr>(attr))
            {
                UnownedStringSlice outName;
                UnownedStringSlice outIndex;
                bool hasStringIndex = splitNameAndIndex(userSemantic->getName(), outName, outIndex);

                changed = true;
                auto newDecoration = builder.getUserSemanticAttr(
                    userSemanticName,
                    hasStringIndex ? stringToInt(outIndex) : 0);
                userSemantic->replaceUsesWith(newDecoration);
                userSemantic->removeAndDeallocate();
                userSemantic = newDecoration;

                layoutOps.add(userSemantic);
                continue;
            }
            layoutOps.add(attr);
        }
        if (changed)
        {
            auto parent = layoutDecor->parent;
            layoutDecor->removeAndDeallocate();
            builder.addLayoutDecoration(parent, builder.getVarLayout(layoutOps));
        }
        return layoutDecor;
    }

    // Find overlapping field semantics and legalize them
    void fixFieldSemanticsOfFlatStruct(IRStructType* structType)
    {
        // Goal is to ensure we do not have overlapping semantics for the user defined semantics:
        // Note that in WGSL, the semantics can be either `builtin` without index or `location` with
        // index.
        /*
            // Assume the following code
            struct Fragment
            {
                float4 p0 : SV_POSITION;
                float2 p1 : TEXCOORD0;
                float2 p2 : TEXCOORD1;
                float3 p3 : COLOR0;
                float3 p4 : COLOR1;
            };

            // Translates into
            struct Fragment
            {
                float4 p0 : BUILTIN_POSITION;
                float2 p1 : LOCATION_0;
                float2 p2 : LOCATION_1;
                float3 p3 : LOCATION_2;
                float3 p4 : LOCATION_3;
            };
        */

        // For Multi-Render-Target, the semantic index must be translated to `location` with
        // the same index. Assume the following code
        /*
            struct Fragment
            {
                float4 p0 : SV_TARGET1;
                float4 p1 : SV_TARGET0;
            };

            // Translates into
            struct Fragment
            {
                float4 p0 : LOCATION_1;
                float4 p1 : LOCATION_0;
            };
        */

        IRBuilder builder(this->m_module);

        List<IRSemanticDecoration*> overlappingSemanticsDecor;
        Dictionary<UnownedStringSlice, std::set<UInt, std::less<UInt>>>
            usedSemanticIndexSemanticDecor;

        List<AttributeParentPair<IRVarOffsetAttr>> overlappingVarOffset;
        Dictionary<UInt, std::set<UInt, std::less<UInt>>> usedSemanticIndexVarOffset;

        List<AttributeParentPair<IRUserSemanticAttr>> overlappingUserSemantic;
        Dictionary<UnownedStringSlice, std::set<UInt, std::less<UInt>>>
            usedSemanticIndexUserSemantic;

        // We store a map from old `IRLayoutDecoration*` to new `IRLayoutDecoration*` since when
        // legalizing we may destroy and remake a `IRLayoutDecoration*`
        Dictionary<IRLayoutDecoration*, IRLayoutDecoration*> oldLayoutDecorToNew;

        // Collect all "semantic info carrying decorations". Any collected decoration will
        // fill up their respective 'Dictionary<SEMANTIC_TYPE, OrderedHashSet<UInt>>'
        // to keep track of in-use offsets for a semantic type.
        // Example: IRSemanticDecoration with name of "SV_TARGET1".
        // * This will have SEMANTIC_TYPE of "sv_target".
        // * This will use up index '1'
        //
        // Now if a second equal semantic "SV_TARGET1" is found, we add this decoration to
        // a list of 'overlapping semantic info decorations' so we can legalize this
        // 'semantic info decoration' later.
        //
        // NOTE: this is a flat struct, all members are children of the initial
        // IRStructType.
        for (auto field : structType->getFields())
        {
            auto key = field->getKey();
            if (auto semanticDecoration = key->findDecoration<IRSemanticDecoration>())
            {
                auto semanticName = semanticDecoration->getSemanticName();

                // sv_target is treated as a user-semantic because it should be emitted with
                // @location like how the user semantics are emitted.
                // For fragment shader, only sv_target will user @location, and for non-fragment
                // shaders, sv_target is not valid.
                bool isUserSemantic =
                    (semanticName.startsWithCaseInsensitive(toSlice("sv_target")) ||
                     !semanticName.startsWithCaseInsensitive(toSlice("sv_")));

                // Ensure names are in a uniform lowercase format so we can bunch together simmilar
                // semantics.
                UnownedStringSlice outName;
                UnownedStringSlice outIndex;
                bool hasStringIndex = splitNameAndIndex(semanticName, outName, outIndex);

                // user semantics gets all same semantic-name.
                auto loweredName = String(outName).toLower();
                auto loweredNameSlice =
                    isUserSemantic ? userSemanticName : loweredName.getUnownedSlice();
                auto newDecoration = builder.addSemanticDecoration(
                    key,
                    loweredNameSlice,
                    hasStringIndex ? stringToInt(outIndex) : 0);
                semanticDecoration->replaceUsesWith(newDecoration);
                semanticDecoration->removeAndDeallocate();
                semanticDecoration = newDecoration;

                auto& semanticUse =
                    usedSemanticIndexSemanticDecor[semanticDecoration->getSemanticName()];
                if (semanticUse.find(semanticDecoration->getSemanticIndex()) != semanticUse.end())
                    overlappingSemanticsDecor.add(semanticDecoration);
                else
                    semanticUse.insert(semanticDecoration->getSemanticIndex());
            }
            if (auto layoutDecor = key->findDecoration<IRLayoutDecoration>())
            {
                // Ensure names are in a uniform lowercase format so we can bunch together simmilar
                // semantics
                layoutDecor = _simplifyUserSemanticNames(builder, layoutDecor);
                oldLayoutDecorToNew[layoutDecor] = layoutDecor;
                auto layout = layoutDecor->getLayout();
                for (auto attr : layout->getAllAttrs())
                {
                    if (auto offset = as<IRVarOffsetAttr>(attr))
                    {
                        auto& semanticUse = usedSemanticIndexVarOffset[offset->getResourceKind()];
                        if (semanticUse.find(offset->getOffset()) != semanticUse.end())
                            overlappingVarOffset.add({layoutDecor, offset});
                        else
                            semanticUse.insert(offset->getOffset());
                    }
                    else if (auto userSemantic = as<IRUserSemanticAttr>(attr))
                    {
                        auto& semanticUse = usedSemanticIndexUserSemantic[userSemantic->getName()];
                        if (semanticUse.find(userSemantic->getIndex()) != semanticUse.end())
                            overlappingUserSemantic.add({layoutDecor, userSemantic});
                        else
                            semanticUse.insert(userSemantic->getIndex());
                    }
                }
            }
        }

        // Legalize all overlapping 'semantic info decorations'
        for (auto decor : overlappingSemanticsDecor)
        {
            auto newOffset = _returnNonOverlappingAttributeIndex(
                usedSemanticIndexSemanticDecor[decor->getSemanticName()]);
            builder.addSemanticDecoration(
                decor->getParent(),
                decor->getSemanticName(),
                (int)newOffset);
            decor->removeAndDeallocate();
        }
        for (auto& varOffset : overlappingVarOffset)
        {
            auto newOffset = _returnNonOverlappingAttributeIndex(
                usedSemanticIndexVarOffset[varOffset.attr->getResourceKind()]);
            auto newVarOffset = builder.getVarOffsetAttr(
                varOffset.attr->getResourceKind(),
                newOffset,
                varOffset.attr->getSpace());
            oldLayoutDecorToNew[varOffset.layoutDecor] = _replaceAttributeOfLayout(
                builder,
                oldLayoutDecorToNew[varOffset.layoutDecor],
                varOffset.attr,
                newVarOffset);
        }
        for (auto& userSemantic : overlappingUserSemantic)
        {
            auto newOffset = _returnNonOverlappingAttributeIndex(
                usedSemanticIndexUserSemantic[userSemantic.attr->getName()]);
            auto newUserSemantic =
                builder.getUserSemanticAttr(userSemantic.attr->getName(), newOffset);
            oldLayoutDecorToNew[userSemantic.layoutDecor] = _replaceAttributeOfLayout(
                builder,
                oldLayoutDecorToNew[userSemantic.layoutDecor],
                userSemantic.attr,
                newUserSemantic);
        }
    }

    void wrapReturnValueInStruct(EntryPointInfo entryPoint)
    {
        // Wrap return value into a struct if it is not already a struct.
        // For example, given this entry point:
        // ```
        // float4 main() : SV_Target { return float3(1,2,3); }
        // ```
        // We are going to transform it into:
        // ```
        // struct Output {
        //     float4 value : SV_Target;
        // };
        // Output main() { return {float3(1,2,3)}; }

        auto func = entryPoint.entryPointFunc;

        auto returnType = func->getResultType();
        if (as<IRVoidType>(returnType))
            return;
        auto entryPointLayoutDecor = func->findDecoration<IRLayoutDecoration>();
        if (!entryPointLayoutDecor)
            return;
        auto entryPointLayout = as<IREntryPointLayout>(entryPointLayoutDecor->getLayout());
        if (!entryPointLayout)
            return;
        auto resultLayout = entryPointLayout->getResultLayout();

        // If return type is already a struct, just make sure every field has a semantic.
        if (auto returnStructType = as<IRStructType>(returnType))
        {
            IRBuilder builder(func);
            MapStructToFlatStruct mapOldFieldToNewField;
            // Flatten result struct type to ensure we do not have nested semantics
            auto flattenedStruct = maybeFlattenNestedStructs(
                builder,
                returnStructType,
                mapOldFieldToNewField,
                semanticInfoToRemove);
            if (returnStructType != flattenedStruct)
            {
                // Replace all return-values with the flattenedStruct we made.
                _replaceAllReturnInst(
                    builder,
                    func,
                    flattenedStruct,
                    [&](IRBuilder& copyBuilder, IRStructType* dstType, IRInst* srcVal) -> IRInst*
                    {
                        auto srcStructType = as<IRStructType>(srcVal->getDataType());
                        SLANG_ASSERT(srcStructType);
                        auto dstVal = copyBuilder.emitVar(dstType);
                        mapOldFieldToNewField.emitCopy<(
                            int)MapStructToFlatStruct::CopyOptions::StructIntoFlatStruct>(
                            copyBuilder,
                            dstVal,
                            srcVal);
                        return builder.emitLoad(dstVal);
                    });
                fixUpFuncType(func, flattenedStruct);
            }
            // Ensure non-overlapping semantics
            fixFieldSemanticsOfFlatStruct(flattenedStruct);
            ensureStructHasUserSemantic<LayoutResourceKind::VaryingOutput>(
                flattenedStruct,
                resultLayout);
            return;
        }

        IRBuilder builder(func);
        builder.setInsertBefore(func);
        IRStructType* structType = builder.createStructType();
        auto stageText = getStageText(entryPoint.entryPointDecor->getProfile().getStage());
        builder.addNameHintDecoration(
            structType,
            (String(stageText) + toSlice("Output")).getUnownedSlice());
        auto key = builder.createStructKey();
        builder.addNameHintDecoration(key, toSlice("output"));
        builder.addLayoutDecoration(key, resultLayout);
        builder.createStructField(structType, key, returnType);
        IRStructTypeLayout::Builder structTypeLayoutBuilder(&builder);
        structTypeLayoutBuilder.addField(key, resultLayout);
        auto typeLayout = structTypeLayoutBuilder.build();
        IRVarLayout::Builder varLayoutBuilder(&builder, typeLayout);
        auto varLayout = varLayoutBuilder.build();
        ensureStructHasUserSemantic<LayoutResourceKind::VaryingOutput>(structType, varLayout);

        _replaceAllReturnInst(
            builder,
            func,
            structType,
            [](IRBuilder& copyBuilder, IRStructType* dstType, IRInst* srcVal) -> IRInst*
            { return copyBuilder.emitMakeStruct(dstType, 1, &srcVal); });

        // Assign an appropriate system value semantic for stage output
        auto stage = entryPoint.entryPointDecor->getProfile().getStage();
        switch (stage)
        {
        case Stage::Compute:
        case Stage::Fragment:
            {
                IRInst* operands[] = {
                    builder.getStringValue(userSemanticName),
                    builder.getIntValue(builder.getIntType(), 0)};
                builder.addDecoration(
                    key,
                    kIROp_SemanticDecoration,
                    operands,
                    SLANG_COUNT_OF(operands));
                break;
            }
        case Stage::Vertex:
            {
                builder.addTargetSystemValueDecoration(key, toSlice("position"));
                break;
            }
        default:
            SLANG_ASSERT(false);
            return;
        }

        fixUpFuncType(func, structType);
    }

    IRInst* tryConvertValue(IRBuilder& builder, IRInst* val, IRType* toType)
    {
        auto fromType = val->getFullType();
        if (auto fromVector = as<IRVectorType>(fromType))
        {
            if (auto toVector = as<IRVectorType>(toType))
            {
                if (fromVector->getElementCount() != toVector->getElementCount())
                {
                    fromType = builder.getVectorType(
                        fromVector->getElementType(),
                        toVector->getElementCount());
                    val = builder.emitVectorReshape(fromType, val);
                }
            }
            else if (as<IRBasicType>(toType))
            {
                UInt index = 0;
                val = builder.emitSwizzle(fromVector->getElementType(), val, 1, &index);
                if (toType->getOp() == kIROp_VoidType)
                    return nullptr;
            }
        }
        else if (auto fromBasicType = as<IRBasicType>(fromType))
        {
            if (fromBasicType->getOp() == kIROp_VoidType)
                return nullptr;
            if (!as<IRBasicType>(toType))
                return nullptr;
            if (toType->getOp() == kIROp_VoidType)
                return nullptr;
        }
        else
        {
            return nullptr;
        }
        return builder.emitCast(toType, val);
    }

    struct SystemValLegalizationWorkItem
    {
        IRInst* var;
        IRType* varType;
        String attrName;
        UInt attrIndex;
    };

    std::optional<SystemValLegalizationWorkItem> tryToMakeSystemValWorkItem(
        IRInst* var,
        IRType* varType)
    {
        if (auto semanticDecoration = var->findDecoration<IRSemanticDecoration>())
        {
            if (semanticDecoration->getSemanticName().startsWithCaseInsensitive(toSlice("sv_")))
            {
                return {
                    {var,
                     varType,
                     String(semanticDecoration->getSemanticName()).toLower(),
                     (UInt)semanticDecoration->getSemanticIndex()}};
            }
        }

        auto layoutDecor = var->findDecoration<IRLayoutDecoration>();
        if (!layoutDecor)
            return {};
        auto sysValAttr = layoutDecor->findAttr<IRSystemValueSemanticAttr>();
        if (!sysValAttr)
            return {};
        auto semanticName = String(sysValAttr->getName());
        auto sysAttrIndex = sysValAttr->getIndex();

        return {{var, varType, semanticName, sysAttrIndex}};
    }

    List<SystemValLegalizationWorkItem> collectSystemValFromEntryPoint(EntryPointInfo entryPoint)
    {
        List<SystemValLegalizationWorkItem> systemValWorkItems;
        for (auto param : entryPoint.entryPointFunc->getParams())
        {
            if (auto structType = as<IRStructType>(param->getDataType()))
            {
                for (auto field : structType->getFields())
                {
                    // Nested struct-s are flattened already by flattenInputParameters().
                    SLANG_ASSERT(!as<IRStructType>(field->getFieldType()));

                    auto key = field->getKey();
                    auto fieldType = field->getFieldType();
                    auto maybeWorkItem = tryToMakeSystemValWorkItem(key, fieldType);
                    if (maybeWorkItem.has_value())
                        systemValWorkItems.add(std::move(maybeWorkItem.value()));
                }
                continue;
            }

            auto maybeWorkItem = tryToMakeSystemValWorkItem(param, param->getFullType());
            if (maybeWorkItem.has_value())
                systemValWorkItems.add(std::move(maybeWorkItem.value()));
        }
        return systemValWorkItems;
    }

    void legalizeSystemValue(EntryPointInfo entryPoint, SystemValLegalizationWorkItem& workItem)
    {
        IRBuilder builder(entryPoint.entryPointFunc);

        auto var = workItem.var;
        auto varType = workItem.varType;
        auto semanticName = workItem.attrName;

        auto indexAsString = String(workItem.attrIndex);
        auto info = getSystemValueInfo(semanticName, &indexAsString, var);

        if (info.isUnsupported)
        {
            reportUnsupportedSystemAttribute(var, semanticName);
            return;
        }
        if (!info.permittedTypes.getCount())
            return;

        builder.addTargetSystemValueDecoration(var, info.wgslSystemValueName.getUnownedSlice());

        bool varTypeIsPermitted = false;
        for (auto& permittedType : info.permittedTypes)
        {
            varTypeIsPermitted = varTypeIsPermitted || permittedType == varType;
        }

        if (!varTypeIsPermitted)
        {
            // Note: we do not currently prefer any conversion
            // example:
            // * allowed types for semantic: `float4`, `uint4`, `int4`
            // * user used, `float2`
            // * Slang will equally prefer `float4` to `uint4` to `int4`.
            //   This means the type may lose data if slang selects `uint4` or `int4`.
            bool foundAConversion = false;
            for (auto permittedType : info.permittedTypes)
            {
                var->setFullType(permittedType);
                builder.setInsertBefore(
                    entryPoint.entryPointFunc->getFirstBlock()->getFirstOrdinaryInst());

                // get uses before we `tryConvertValue` since this creates a new use
                List<IRUse*> uses;
                for (auto use = var->firstUse; use; use = use->nextUse)
                    uses.add(use);

                auto convertedValue = tryConvertValue(builder, var, varType);
                if (convertedValue == nullptr)
                    continue;

                foundAConversion = true;
                copyNameHintAndDebugDecorations(convertedValue, var);

                for (auto use : uses)
                    builder.replaceOperand(use, convertedValue);
            }
            if (!foundAConversion)
            {
                // If we can't convert the value, report an error.
                for (auto permittedType : info.permittedTypes)
                {
                    StringBuilder typeNameSB;
                    getTypeNameHint(typeNameSB, permittedType);
                    m_sink->diagnose(
                        var->sourceLoc,
                        Diagnostics::systemValueTypeIncompatible,
                        semanticName,
                        typeNameSB.produceString());
                }
            }
        }
    }

    void legalizeSystemValueParameters(EntryPointInfo entryPoint)
    {
        List<SystemValLegalizationWorkItem> systemValWorkItems =
            collectSystemValFromEntryPoint(entryPoint);

        for (auto index = 0; index < systemValWorkItems.getCount(); index++)
        {
            legalizeSystemValue(entryPoint, systemValWorkItems[index]);
        }
        fixUpFuncType(entryPoint.entryPointFunc);
    }

    void legalizeEntryPointForWGSL(EntryPointInfo entryPoint)
    {
        // Input Parameter Legalize
        flattenInputParameters(entryPoint);

        // System Value Legalize
        legalizeSystemValueParameters(entryPoint);

        // Output Value Legalize
        wrapReturnValueInStruct(entryPoint);
    }

    void legalizeCall(IRCall* call)
    {
        // WGSL does not allow forming a pointer to a sub part of a composite value.
        // For example, if we have
        // ```
        // struct S { float x; float y; };
        // void foo(inout float v) { v = 1.0f; }
        // void main() { S s; foo(s.x); }
        // ```
        // The call to `foo(s.x)` is illegal in WGSL because `s.x` is a sub part of `s`.
        // And trying to form `&s.x` in WGSL is illegal.
        // To work around this, we will create a local variable to hold the sub part of
        // the composite value.
        // And then pass the local variable to the function.
        // After the call, we will write back the local variable to the sub part of the
        // composite value.
        //
        IRBuilder builder(call);
        builder.setInsertBefore(call);
        struct WritebackPair
        {
            IRInst* dest;
            IRInst* value;
        };
        ShortList<WritebackPair> pendingWritebacks;

        for (UInt i = 0; i < call->getArgCount(); i++)
        {
            auto arg = call->getArg(i);
            auto ptrType = as<IRPtrTypeBase>(arg->getDataType());
            if (!ptrType)
                continue;
            switch (arg->getOp())
            {
            case kIROp_Var:
            case kIROp_Param:
            case kIROp_GlobalParam:
            case kIROp_GlobalVar:
                continue;
            default:
                break;
            }

            // Create a local variable to hold the input argument.
            auto var = builder.emitVar(ptrType->getValueType(), AddressSpace::Function);

            // Store the input argument into the local variable.
            builder.emitStore(var, builder.emitLoad(arg));
            builder.replaceOperand(call->getArgs() + i, var);
            pendingWritebacks.add({arg, var});
        }

        // Perform writebacks after the call.
        builder.setInsertAfter(call);
        for (auto& pair : pendingWritebacks)
        {
            builder.emitStore(pair.dest, builder.emitLoad(pair.value));
        }
    }

    void legalizeFunc(IRFunc* func)
    {
        // Insert casts to convert integer return types
        auto funcReturnType = func->getResultType();
        if (isIntegralType(funcReturnType))
        {
            for (auto block : func->getBlocks())
            {
                if (auto returnInst = as<IRReturn>(block->getTerminator()))
                {
                    auto returnedValue = returnInst->getOperand(0);
                    auto returnedValueType = returnedValue->getDataType();
                    if (isIntegralType(returnedValueType))
                    {
                        IRBuilder builder(returnInst);
                        builder.setInsertBefore(returnInst);
                        auto newOp = builder.emitCast(funcReturnType, returnedValue);
                        builder.replaceOperand(returnInst->getOperands(), newOp);
                    }
                }
            }
        }
    }

    void legalizeSwitch(IRSwitch* switchInst)
    {
        // WGSL Requires all switch statements to contain a default case.
        // If the switch statement does not contain a default case, we will add one.
        if (switchInst->getDefaultLabel() != switchInst->getBreakLabel())
            return;
        IRBuilder builder(switchInst);
        auto defaultBlock = builder.createBlock();
        builder.setInsertInto(defaultBlock);
        builder.emitBranch(switchInst->getBreakLabel());
        defaultBlock->insertBefore(switchInst->getBreakLabel());
        List<IRInst*> cases;
        for (UInt i = 0; i < switchInst->getCaseCount(); i++)
        {
            cases.add(switchInst->getCaseValue(i));
            cases.add(switchInst->getCaseLabel(i));
        }
        builder.setInsertBefore(switchInst);
        auto newSwitch = builder.emitSwitch(
            switchInst->getCondition(),
            switchInst->getBreakLabel(),
            defaultBlock,
            (UInt)cases.getCount(),
            cases.getBuffer());
        switchInst->transferDecorationsTo(newSwitch);
        switchInst->removeAndDeallocate();
    }

    void legalizeBinaryOp(IRInst* inst)
    {
        auto isVectorOrMatrix = [](IRType* type)
        {
            switch (type->getOp())
            {
            case kIROp_VectorType:
            case kIROp_MatrixType:
                return true;
            default:
                return false;
            }
        };
        if (isVectorOrMatrix(inst->getOperand(0)->getDataType()) &&
            as<IRBasicType>(inst->getOperand(1)->getDataType()))
        {
            IRBuilder builder(inst);
            builder.setInsertBefore(inst);
            auto newRhs = builder.emitMakeCompositeFromScalar(
                inst->getOperand(0)->getDataType(),
                inst->getOperand(1));
            builder.replaceOperand(inst->getOperands() + 1, newRhs);
        }
        else if (
            as<IRBasicType>(inst->getOperand(0)->getDataType()) &&
            isVectorOrMatrix(inst->getOperand(1)->getDataType()))
        {
            IRBuilder builder(inst);
            builder.setInsertBefore(inst);
            auto newLhs = builder.emitMakeCompositeFromScalar(
                inst->getOperand(1)->getDataType(),
                inst->getOperand(0));
            builder.replaceOperand(inst->getOperands(), newLhs);
        }
        else if (
            isIntegralType(inst->getOperand(0)->getDataType()) &&
            isIntegralType(inst->getOperand(1)->getDataType()))
        {
            // If integer operands differ in signedness, convert the signed one to unsigned.
            // We're assuming that the cases where this is bad have already been caught by
            // common validation checks.
            IntInfo opIntInfo[2] = {
                getIntTypeInfo(inst->getOperand(0)->getDataType()),
                getIntTypeInfo(inst->getOperand(1)->getDataType())};
            if (opIntInfo[0].isSigned != opIntInfo[1].isSigned)
            {
                int signedOpIndex = (int)opIntInfo[1].isSigned;
                opIntInfo[signedOpIndex].isSigned = false;
                IRBuilder builder(inst);
                builder.setInsertBefore(inst);
                auto newOp = builder.emitCast(
                    builder.getType(getIntTypeOpFromInfo(opIntInfo[signedOpIndex])),
                    inst->getOperand(signedOpIndex));
                builder.replaceOperand(inst->getOperands() + signedOpIndex, newOp);
            }
        }
    }

    void processInst(IRInst* inst)
    {
        switch (inst->getOp())
        {
        case kIROp_Call:
            legalizeCall(static_cast<IRCall*>(inst));
            break;

        case kIROp_Switch:
            legalizeSwitch(as<IRSwitch>(inst));
            break;

        // For all binary operators, make sure both side of the operator have the same type
        // (vector-ness and matrix-ness).
        case kIROp_Add:
        case kIROp_Sub:
        case kIROp_Mul:
        case kIROp_Div:
        case kIROp_FRem:
        case kIROp_IRem:
        case kIROp_And:
        case kIROp_Or:
        case kIROp_BitAnd:
        case kIROp_BitOr:
        case kIROp_BitXor:
        case kIROp_Lsh:
        case kIROp_Rsh:
        case kIROp_Eql:
        case kIROp_Neq:
        case kIROp_Greater:
        case kIROp_Less:
        case kIROp_Geq:
        case kIROp_Leq:
            legalizeBinaryOp(inst);
            break;

        case kIROp_Func:
            legalizeFunc(static_cast<IRFunc*>(inst));
            [[fallthrough]];
        default:
            for (auto child : inst->getModifiableChildren())
            {
                processInst(child);
            }
        }
    }
};

struct GlobalInstInliningContext: public GlobalInstInliningContextGeneric
{
    bool isLegalGlobalInstForTarget(IRInst* /* inst */) override
    {
        // The global instructions that are generically considered legal are fine for
        // WGSL.
        return false;
    }

    bool isInlinableGlobalInstForTarget(IRInst* /* inst */) override
    {
        // The global instructions that are generically considered inlineable are fine
        // for WGSL.
        return false;
    }

    bool shouldBeInlinedForTarget(IRInst* /* user */) override
    {
        // WGSL doesn't do any extra inlining beyond what is generically done by default.
        return false;
    }
};

void legalizeIRForWGSL(IRModule* module, DiagnosticSink* sink)
{
    List<EntryPointInfo> entryPoints;
    for (auto inst : module->getGlobalInsts())
    {
        IRFunc* const func{as<IRFunc>(inst)};
        if (!func)
            continue;
        IREntryPointDecoration* const entryPointDecor =
            func->findDecoration<IREntryPointDecoration>();
        if (!entryPointDecor)
            continue;
        EntryPointInfo info;
        info.entryPointDecor = entryPointDecor;
        info.entryPointFunc = func;
        entryPoints.add(info);
    }

    LegalizeWGSLEntryPointContext context(sink, module);
    for (auto entryPoint : entryPoints)
        context.legalizeEntryPointForWGSL(entryPoint);
    context.removeSemanticLayoutsFromLegalizedStructs();

    // Go through every instruction in the module and legalize them as needed.
    context.processInst(module->getModuleInst());

    GlobalInstInliningContext().inlineGlobalValues(module);
}

} // namespace Slang
