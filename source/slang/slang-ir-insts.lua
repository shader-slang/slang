--
-- This file contains the canonical definitions for the instions to the Slang IR.
--
-- Add new instructions here.
--
-- !!
-- !! Please make sure to update the supported module versions in
-- !! Slang::IRModule accordingly when modifying this file.
-- !!
--
-- For a detailed description of the schema, please see docs/design/ir-instruction-definition.md
--
local insts = {
	{ nop = {} },
	-- This opcode is used as a placeholder if we were ever to deserialize a
	-- module which contains an instruction we don't have defined in this file,
	-- it should never appear except immediately after deserialization.
	{ Unrecognized = {} },
	{
		Type = {
			{
				BasicType = {
					hoistable = true,
					{ Void = { struct_name = "VoidType" } },
					{ Bool = { struct_name = "BoolType" } },
					{ Int8 = { struct_name = "Int8Type" } },
					{ Int16 = { struct_name = "Int16Type" } },
					{ Int = { struct_name = "IntType" } },
					{ Int64 = { struct_name = "Int64Type" } },
					{ UInt8 = { struct_name = "UInt8Type" } },
					{ UInt16 = { struct_name = "UInt16Type" } },
					{ UInt = { struct_name = "UIntType" } },
					{ UInt64 = { struct_name = "UInt64Type" } },
					{ Half = { struct_name = "HalfType" } },
					{ Float = { struct_name = "FloatType" } },
					{ Double = { struct_name = "DoubleType" } },
					{ Char = { struct_name = "CharType" } },
					{ IntPtr = { struct_name = "IntPtrType" } },
					{ UIntPtr = { struct_name = "UIntPtrType" } },
				},
			},
			{ AfterBaseType = {} },
			{
				StringTypeBase = {
					hoistable = true,
					{ String = { struct_name = "StringType" } },
					{ NativeString = { struct_name = "NativeStringType" } },
				},
			},
			{ CapabilitySet = { struct_name = "CapabilitySetType", hoistable = true } },
			{ DynamicType = { hoistable = true } },
			{ AnyValueType = { operands = { { "size" } }, hoistable = true } },
			{
				RawPointerTypeBase = {
					hoistable = true,
					{ RawPointerType = {} },
					{ RTTIPointerType = { operands = { { "rTTIOperand" } } } },
					{ AfterRawPointerTypeBase = {} },
				},
			},
			{
				ArrayTypeBase = {
					hoistable = true,
					{ Array = { struct_name = "ArrayType", min_operands = 2 } },
					{ UnsizedArray = { struct_name = "UnsizedArrayType", min_operands = 1 } },
				},
			},
			{ Func = { struct_name = "FuncType", hoistable = true } },
			{ BasicBlock = { struct_name = "BasicBlockType", hoistable = true } },
			{ Vec = { struct_name = "VectorType", operands = { { "elementType", "IRType" }, { "elementCount" } }, hoistable = true } },
			{ Mat = { struct_name = "MatrixType", operands = { { "elementType", "IRType" }, { "rowCount" }, { "columnCount" }, { "layout" } }, hoistable = true } },
			{ Conjunction = { struct_name = "ConjunctionType", hoistable = true } },
			{ Attributed = { struct_name = "AttributedType", operands =  { { "baseType", "IRType" }, { "attr" } }, hoistable = true } },
			{ Result = { struct_name = "ResultType", operands = { { "valueType", "IRType" }, { "errorType", "IRType" } }, hoistable = true } },
			{ Optional = { struct_name = "OptionalType", operands = { { "valueType", "IRType" } }, hoistable = true } },
			{ Enum = { struct_name = "EnumType", operands = { { "tagType", "IRType" } }, parent = true } },
			{
				DifferentialPairTypeBase = {
					hoistable = true,
					{ DiffPair = { struct_name = "DifferentialPairType", min_operands = 1 } },
					{ DiffPairUserCode = { struct_name = "DifferentialPairUserCodeType", min_operands = 1 } },
					{ DiffRefPair = { struct_name = "DifferentialPtrPairType", min_operands = 1 } },
				},
			},
			{
				BwdDiffIntermediateCtxType = {
					struct_name = "BackwardDiffIntermediateContextType",
					operands = { { "func" } },
					hoistable = true,
				},
			},
			{ TensorView = { struct_name = "TensorViewType", operands = { { "elementType", "IRType" } }, hoistable = true } },
			{ TorchTensor = { struct_name = "TorchTensorType", hoistable = true } },
			{ ArrayListVector = { struct_name = "ArrayListType", operands = { { "elementType", "IRType" } }, hoistable = true } },
			{ Atomic = { struct_name = "AtomicType", operands = { { "elementType", "IRType" } }, hoistable = true } },
			{
				BindExistentialsTypeBase = {
					hoistable = true,
					{
						BindExistentials = {
							-- A `BindExistentials<B, T0,w0, T1,w1, ...>` represents
							-- taking type `B` and binding each of its existential type
							-- parameters, recursively, with the specified arguments,
							-- where each `Ti, wi` pair represents the concrete type
							-- and witness table to plug in for parameter `i`.
							struct_name = "BindExistentialsType",
							min_operands = 1,
						},
					},
					{
						BoundInterface = {
							-- An `BindInterface<B, T0, w0>` represents the special case
							-- of a `BindExistentials` where the type `B` is known to be
							-- an interface type.
							struct_name = "BoundInterfaceType",
							min_operands = 3,
						},
					},
				},
			},
			{
				Rate = {
					hoistable = true,
					{ ConstExpr = { struct_name = "ConstExprRate" } },
					{ SpecConst = { struct_name = "SpecConstRate" } },
					{ GroupShared = { struct_name = "GroupSharedRate" } },
					{ ActualGlobalRate = {} },
				},
			},
			{ RateQualified = { struct_name = "RateQualifiedType", operands = { { "rate", "IRRate" }, { "valueType", "IRType" } }, hoistable = true } },
			{
				Kind = {
					-- Kinds represent the "types of types."
					-- They should not really be nested under `IRType`
					-- in the overall hierarchy, but we can fix that later.
					hoistable = true,
					{ Type = { struct_name = "TypeKind" } },
					{ TypeParameterPack = { struct_name = "TypeParameterPackKind" } },
					{ Rate = { struct_name = "RateKind" } },
					{ Generic = { struct_name = "GenericKind" } },
				},
			},
			{
				PtrTypeBase = {
					hoistable = true,
					{ Ptr = { struct_name = "PtrType", min_operands = 1 } },
					{ Ref = { struct_name = "RefType", min_operands = 1 } },
					{ ConstRef = { struct_name = "ConstRefType", min_operands = 1 } },
					{
						PseudoPtr = {
							-- A `PsuedoPtr<T>` logically represents a pointer to a value of type
							-- `T` on a platform that cannot support pointers. The expectation
							-- is that the "pointer" will be legalized away by storing a value
							-- of type `T` somewhere out-of-line.
							struct_name = "PseudoPtrType",
							min_operands = 1,
						},
					},
					{
						OutTypeBase = {
							{ Out = { struct_name = "OutType", min_operands = 1 } },
							{ InOut = { struct_name = "InOutType", min_operands = 1 } },
						},
					},
				},
			},
			{
				ComPtr = {
					-- A ComPtr<T> type is treated as a opaque type that represents a reference-counted handle to a COM object.
					struct_name = "ComPtrType",
					operands = { { "valueType", "IRType" } },
					hoistable = true,
				},
			},
			{
				NativePtr = {
					-- A NativePtr<T> type represents a native pointer to a managed resource.
					struct_name = "NativePtrType",
					operands = { { "valueType", "IRType" } },
					hoistable = true,
				},
			},
			{
				DescriptorHandle = {
					-- A DescriptorHandle<T> type represents a bindless handle to an opaue resource type.
					struct_name = "DescriptorHandleType",
					operands = { { "resourceType", "IRType" } },
					hoistable = true,
				},
			},
			{
				GLSLAtomicUint = {
					-- An AtomicUint is a placeholder type for a storage buffer, and will be mangled during compiling.
					struct_name = "GLSLAtomicUintType",
					hoistable = true,
				},
			},
			{
				SamplerStateTypeBase = {
					hoistable = true,
					{ SamplerState = { struct_name = "SamplerStateType" } },
					{ SamplerComparisonState = { struct_name = "SamplerComparisonStateType" } },
				},
			},
			{ DefaultLayout = { struct_name = "DefaultBufferLayoutType", hoistable = true } },
			{ Std140Layout = { struct_name = "Std140BufferLayoutType", hoistable = true } },
			{ Std430Layout = { struct_name = "Std430BufferLayoutType", hoistable = true } },
			{ ScalarLayout = { struct_name = "ScalarBufferLayoutType", hoistable = true } },
			{ SubpassInputType = { operands = { { "elementType", "IRType" }, { "isMultisampleInst" } }, hoistable = true } },
			{ TextureFootprintType = { min_operands = 1, hoistable = true } },
			{ TextureShape1DType = { hoistable = true } },
			{ TextureShape2DType = { struct_name = "TextureShape2DType", hoistable = true } },
			{ TextureShape3DType = { struct_name = "TextureShape3DType", hoistable = true } },
			{ TextureShapeCubeDType = { struct_name = "TextureShapeCubeType", hoistable = true } },
			{ TextureShapeBufferType = { hoistable = true } },
			{
				ResourceTypeBase = {
					-- TODO: Why do we have all this hierarchy here, when everything
					-- that actually matters is currently nested under `TextureTypeBase`?
					{
						ResourceType = {
							{
								TextureTypeBase = {
									{
										TextureType = { min_operands = 8, hoistable = true },
									},
									{ GLSLImageType = { use_other = true, hoistable = true } },
								},
							},
						},
					},
				},
			},
			{
				UntypedBufferResourceType = {
					hoistable = true,
					{
						ByteAddressBufferTypeBase = {
							{ ByteAddressBuffer = { struct_name = "HLSLByteAddressBufferType" } },
							{ RWByteAddressBuffer = { struct_name = "HLSLRWByteAddressBufferType" } },
							{
								RasterizerOrderedByteAddressBuffer = {
									struct_name = "HLSLRasterizerOrderedByteAddressBufferType",
								},
							},
						},
					},
					{ RaytracingAccelerationStructure = { struct_name = "RaytracingAccelerationStructureType" } },
				},
			},
			{
				HLSLPatchType = {
					hoistable = true,
					{ InputPatch = { struct_name = "HLSLInputPatchType", min_operands = 2 } },
					{ OutputPatch = { struct_name = "HLSLOutputPatchType", min_operands = 2 } },
				},
			},
			{ GLSLInputAttachment = { struct_name = "GLSLInputAttachmentType", hoistable = true } },
			{
				BuiltinGenericType = {
					hoistable = true,
					{
						HLSLStreamOutputType = {
							{ PointStream = { struct_name = "HLSLPointStreamType", min_operands = 1 } },
							{ LineStream = { struct_name = "HLSLLineStreamType", min_operands = 1 } },
							{ TriangleStream = { struct_name = "HLSLTriangleStreamType", min_operands = 1 } },
						},
					},
					{
						MeshOutputType = {
							{ Vertices = { struct_name = "VerticesType", min_operands = 2 } },
							{ Indices = { struct_name = "IndicesType", min_operands = 2 } },
							{ Primitives = { struct_name = "PrimitivesType", min_operands = 2 } },
						},
					},
					{ ["metal::mesh"] = { struct_name = "MetalMeshType", operands = { { "verticesType", "IRType" }, { "primitivesType", "IRType" }, { "numVertices" }, { "numPrimitives" }, { "topology", "IRIntLit" } } } },
					{ mesh_grid_properties = { struct_name = "MetalMeshGridPropertiesType" } },
					{
						HLSLStructuredBufferTypeBase = {
							{ StructuredBuffer = { struct_name = "HLSLStructuredBufferType" } },
							{ RWStructuredBuffer = { struct_name = "HLSLRWStructuredBufferType" } },
							{
								RasterizerOrderedStructuredBuffer = {
									struct_name = "HLSLRasterizerOrderedStructuredBufferType",
								},
							},
							{ AppendStructuredBuffer = { struct_name = "HLSLAppendStructuredBufferType" } },
							{ ConsumeStructuredBuffer = { struct_name = "HLSLConsumeStructuredBufferType" } },
						},
					},
					{
						PointerLikeType = {
							{
								ParameterGroupType = {
									{
										UniformParameterGroupType = {
											{
												ConstantBuffer = {
													struct_name = "ConstantBufferType",
													min_operands = 1,
												},
											},
											{ TextureBuffer = { struct_name = "TextureBufferType", min_operands = 1 } },
											{
												ParameterBlock = {
													struct_name = "ParameterBlockType",
													min_operands = 1,
												},
											},
										},
									},
									{
										VaryingParameterGroupType = {
											{
												GLSLInputParameterGroup = {
													struct_name = "GLSLInputParameterGroupType",
												},
											},
											{
												GLSLOutputParameterGroup = {
													struct_name = "GLSLOutputParameterGroupType",
												},
											},
										},
									},
									{
										GLSLShaderStorageBuffer = {
											struct_name = "GLSLShaderStorageBufferType",
											min_operands = 1,
										},
									},
								},
							},
						},
					},
				},
			},
			{
				RayQuery = {
					-- Types
					struct_name = "RayQueryType",
					min_operands = 1,
					hoistable = true,
				},
			},
			{
				HitObject = {
					struct_name = "HitObjectType",
					hoistable = true,
				},
			},
			{ CoopVectorType = { operands = { { "elementType", "IRType"}, { "elementCount" } }, hoistable = true } },
			{ CoopMatrixType = { operands = { { "elementType", "IRType"}, { "scope" }, { "rowCount" }, { "columnCount" }, { "matrixUse" } }, hoistable = true } },
			{
				TensorAddressingTensorLayoutType = { operands = { { "dimension"}, { "clampMode" } }, hoistable = true },
			},
			{
				TensorAddressingTensorViewType = {
					min_operands = 3,
					hoistable = true,
				},
			},
			{ MakeTensorAddressingTensorLayout = {} },
			{ MakeTensorAddressingTensorView = {} },
			{
				DynamicResource = {
					-- Opaque type that can be dynamically cast to other resource types.
					struct_name = "DynamicResourceType",
					hoistable = true,
				},
			},
			{
				struct = {
					-- A user-defined structure declaration at the IR level.
					-- Unlike in the AST where there is a distinction between
					-- a `StructDecl` and a `DeclRefType` that refers to it,
					-- at the IR level the struct declaration and the type
					-- are the same IR instruction.
					--
					-- This is a parent instruction that holds zero or more
					-- `field` instructions.
					struct_name = "StructType",
					parent = true,
				},
			},
			{
				class = { struct_name = "ClassType", parent = true },
			},
			{ interface = { struct_name = "InterfaceType", global = true } },
			{ associated_type = { hoistable = true } },
			{ this_type = { hoistable = true } },
			{ rtti_type = { struct_name = "RTTIType", hoistable = true } },
			{
				rtti_handle_type = {
					struct_name = "RTTIHandleType",
					hoistable = true,
				},
			},
			{
				TupleTypeBase = {
					hoistable = true,
					{ tuple_type = {} },
					{ TypePack = {} },
				},
			},
			{ TargetTuple = { struct_name = "TargetTupleType", hoistable = true } },
			{ ExpandTypeOrVal = { min_operands = 1, hoistable = true } },
			{
				spirvLiteralType = {
					-- A type that identifies it's contained type as being emittable as `spirv_literal.
					struct_name = "SPIRVLiteralType",
					operands = { { "valueType", "IRType" } },
					hoistable = true,
				},
			},
			{
				type_t = {
					-- A TypeType-typed IRValue represents a IRType.
					-- It is used to represent a type parameter/argument in a generics.
					struct_name = "TypeType",
					hoistable = true,
				},
			},
			{
				WitnessTableTypeBase = {
					-- IRWitnessTableTypeBase
					hoistable = true,
					{
						witness_table_t = {
							-- An `IRWitnessTable` has type `WitnessTableType`.
							struct_name = "WitnessTableType",
							min_operands = 1,
						},
					},
					{
						witness_table_id_t = {
							-- An integer type representing a witness table for targets where
							-- witness tables are represented as integer IDs. This type is used
							-- during the lower-generics pass while generating dynamic dispatch
							-- code and will eventually lower into an uint type.
							struct_name = "WitnessTableIDType",
							min_operands = 1,
						},
					},
				},
			},
		},
	},
	-- IRGlobalValueWithCode
	{
		GlobalValueWithCode = {
			{
				GlobalValueWithParams = {
					-- IRGlobalValueWithParams
					parent = true,
					{ func = {} },
					{ generic = {} },
				},
			},
			{ global_var = { global = true } },
		},
	},
	{ global_param = { global = true } },
	{
		globalConstant = { global = true },
	},
	{ key = { struct_name = "StructKey", global = true } },
	{ global_generic_param = { global = true } },
	{ witness_table = { hoistable = true } },
	{ indexedFieldKey = { min_operands = 2, hoistable = true } },
	-- A placeholder witness that ThisType implements the enclosing interface.
	-- Used only in interface definitions.
	{ thisTypeWitness = { min_operands = 1 } },
	-- A placeholder witness for the fact that two types are equal.
	{ TypeEqualityWitness = { min_operands = 2, hoistable = true } },
	{ global_hashed_string_literals = {} },
	{
		module = { struct_name = "ModuleInst", parent = true },
	},
	{ block = { parent = true } },
	-- IRConstant
	{
		Constant = {
			{ boolConst = { struct_name = "BoolLit" } },
			{
				integer_constant = { struct_name = "IntLit" },
			},
			{ float_constant = { struct_name = "FloatLit" } },
			{
				ptr_constant = { struct_name = "PtrLit" },
			},
			{ void_constant = { struct_name = "VoidLit" } },
			{ string_constant = { struct_name = "StringLit" } },
			{
				blob_constant = { struct_name = "BlobLit" },
			},
		},
	},
	{ CapabilitySet = { hoistable = true, { capabilityConjunction = {} }, { capabilityDisjunction = {} } } },
	{ undefined = {} },
	-- A `defaultConstruct` operation creates an initialized
	-- value of the result type, and can only be used for types
	-- where default construction is a meaningful thing to do.
	{ defaultConstruct = {} },
	{
		MakeDifferentialPairBase = {
			{ MakeDiffPair = { struct_name = "MakeDifferentialPair", min_operands = 2 } },
			{ MakeDiffPairUserCode = { struct_name = "MakeDifferentialPairUserCode", min_operands = 2 } },
			{
				MakeDiffRefPair = { struct_name = "MakeDifferentialPtrPair", min_operands = 2 },
			},
		},
	},
	{
		DifferentialPairGetDifferentialBase = {
			{ GetDifferential = { struct_name = "DifferentialPairGetDifferential", min_operands = 1 } },
			{
				GetDifferentialUserCode = { struct_name = "DifferentialPairGetDifferentialUserCode", min_operands = 1 },
			},
			{
				GetDifferentialPtr = {
					struct_name = "DifferentialPtrPairGetDifferential",
					min_operands = 1,
				},
			},
		},
	},
	{
		DifferentialPairGetPrimalBase = {
			{
				GetPrimal = { struct_name = "DifferentialPairGetPrimal", min_operands = 1 },
			},
			{
				GetPrimalUserCode = {
					struct_name = "DifferentialPairGetPrimalUserCode",
					min_operands = 1,
				},
			},
			{ GetPrimalRef = { struct_name = "DifferentialPtrPairGetPrimal", min_operands = 1 } },
		},
	},
	{ specialize = { min_operands = 2, hoistable = true } },
	{ lookupWitness = { struct_name = "LookupWitnessMethod", min_operands = 2, hoistable = true } },
	{ GetSequentialID = { min_operands = 1, hoistable = true } },
	{
		bind_global_generic_param = {
			min_operands = 2,
		},
	},
	{ allocObj = {} },
	{ globalValueRef = { min_operands = 1 } },
	{ makeUInt64 = { min_operands = 2 } },
	{ makeVector = {} },
	{ makeMatrix = {} },
	{
		makeMatrixFromScalar = {
			min_operands = 1,
		},
	},
	{ matrixReshape = { min_operands = 1 } },
	{
		vectorReshape = {
			min_operands = 1,
		},
	},
	{ makeArray = {} },
	{ makeArrayFromElement = { min_operands = 1 } },
	{ makeCoopVector = {} },
	{ makeCoopVectorFromValuePack = { min_operands = 1 } },
	{ makeStruct = {} },
	{ makeTuple = {} },
	{ makeTargetTuple = { struct_name = "MakeTargetTuple" } },
	{ makeValuePack = {} },
	{ getTargetTupleElement = {} },
	{
		getTupleElement = {
			min_operands = 2,
		},
	},
	{ LoadResourceDescriptorFromHeap = { min_operands = 1 } },
	{
		LoadSamplerDescriptorFromHeap = {
			min_operands = 1,
		},
	},
	{ MakeCombinedTextureSamplerFromHandle = { min_operands = 1 } },
	{
		MakeWitnessPack = {
			hoistable = true,
		},
	},
	{ Expand = { min_operands = 1 } },
	{
		Each = {
			min_operands = 1,
			hoistable = true,
		},
	},
	{ makeResultValue = { min_operands = 1 } },
	{ makeResultError = { min_operands = 1 } },
	{ isResultError = { min_operands = 1 } },
	{ getResultError = { min_operands = 1 } },
	{ getResultValue = { min_operands = 1 } },
	{ getOptionalValue = { min_operands = 1 } },
	{ optionalHasValue = { min_operands = 1 } },
	{ makeOptionalValue = { min_operands = 1 } },
	{ makeOptionalNone = { min_operands = 1 } },
	{ CombinedTextureSamplerGetTexture = { min_operands = 1 } },
	{ CombinedTextureSamplerGetSampler = { min_operands = 1 } },
	{ call = { min_operands = 1 } },
	{ rtti_object = { struct_name = "RTTIObject" } },
	{ alloca = { min_operands = 1 } },
	{ updateElement = { min_operands = 2 } },
	{ detachDerivative = { min_operands = 1 } },
	{ bitfieldExtract = { min_operands = 3 } },
	{ bitfieldInsert = { min_operands = 4 } },
	{ packAnyValue = { min_operands = 1 } },
	{ unpackAnyValue = { min_operands = 1 } },
	{ witness_table_entry = { min_operands = 2 } },
	{ interface_req_entry = { struct_name = "InterfaceRequirementEntry", operands = { { "requirementKey" }, { "requirementVal" } }, global = true } },
	-- An inst to represent the workgroup size of the calling entry point.
	-- We will materialize this inst during `translateGlobalVaryingVar`.
	{ GetWorkGroupSize = { hoistable = true } },
	-- An inst that returns the current stage of the calling entry point.
	{ GetCurrentStage = {} },
	{ param = {} },
	{ field = { struct_name = "StructField", min_operands = 2 } },
	{ var = {} },
	{ load = { min_operands = 1 } },
	{ store = { min_operands = 2 } },
	-- Atomic Operations
	{
		AtomicOperation = {
			{ atomicLoad = { min_operands = 1 } },
			{
				atomicStore = { min_operands = 2 },
			},
			{ atomicExchange = { min_operands = 2 } },
			{
				atomicCompareExchange = { min_operands = 3 },
			},
			{ atomicAdd = { min_operands = 2 } },
			{
				atomicSub = { min_operands = 2 },
			},
			{ atomicAnd = { min_operands = 2 } },
			{
				atomicOr = { min_operands = 2 },
			},
			{ atomicXor = { min_operands = 2 } },
			{
				atomicMin = { min_operands = 2 },
			},
			{ atomicMax = { min_operands = 2 } },
			{
				atomicInc = { min_operands = 1 },
			},
			{ atomicDec = { min_operands = 1 } },
		},
	},
	-- Produced and removed during backward auto-diff pass as a temporary placeholder representing the
	-- currently accumulated derivative to pass to some dOut argument in a nested call.
	{ LoadReverseGradient = { min_operands = 1 } },
	-- Produced and removed during backward auto-diff pass as a temporary placeholder containing the
	-- primal and accumulated derivative values to pass to an inout argument in a nested call.
	{ ReverseGradientDiffPairRef = { min_operands = 2 } },
	-- Produced and removed during backward auto-diff pass. This inst is generated by the splitting step
	-- to represent a reference to an inout parameter for use in the primal part of the computation.
	{ PrimalParamRef = { min_operands = 1 } },
	-- Produced and removed during backward auto-diff pass. This inst is generated by the splitting step
	-- to represent a reference to an inout parameter for use in the back-prop part of the computation.
	{ DiffParamRef = { min_operands = 1 } },
	-- Check that the value is a differential null value.
	{ IsDifferentialNull = { min_operands = 1 } },
	{
		get_field = {
			struct_name = "FieldExtract",
			min_operands = 2,
		},
	},
	{ get_field_addr = { struct_name = "FieldAddress", min_operands = 2 } },
	{ getElement = { min_operands = 2 } },
	{ getElementPtr = { min_operands = 2 } },
	-- Pointer offset: computes pBase + offset_in_elements
	{ getOffsetPtr = { min_operands = 2 } },
	{ getAddr = { struct_name = "GetAddress", min_operands = 1 } },
	{ castDynamicResource = { min_operands = 1 } },
	-- Get an unowned NativeString from a String.
	{ getNativeStr = { min_operands = 1 } },
	-- Make String from a NativeString.
	{ makeString = { min_operands = 1 } },
	-- Get a native ptr from a ComPtr or RefPtr
	{ getNativePtr = { min_operands = 1 } },
	-- Get a write reference to a managed ptr var (operand must be Ptr<ComPtr<T>> or Ptr<RefPtr<T>>).
	{ getManagedPtrWriteRef = { min_operands = 1 } },
	-- Attach a managedPtr var to a NativePtr without changing its ref count.
	{ ManagedPtrAttach = { min_operands = 1 } },
	-- Attach a managedPtr var to a NativePtr without changing its ref count.
	{ ManagedPtrDetach = { min_operands = 1 } },
	-- "Subscript" an image at a pixel coordinate to get pointer
	{ imageSubscript = { min_operands = 2 } },
	-- Load from an Image.
	{ imageLoad = { min_operands = 2 } },
	-- Store into an Image.
	{ imageStore = { min_operands = 3 } },
	-- Load (almost) arbitrary-type data from a byte-address buffer
	-- %dst = byteAddressBufferLoad(%buffer, %offset, %alignment)
	-- where
	-- - `buffer` is a value of some `ByteAddressBufferTypeBase` type
	-- - `offset` is an `int`
	-- - `alignment` is an `int`
	-- - `dst` is a value of some type containing only ordinary data
	{ byteAddressBufferLoad = { min_operands = 3 } },
	-- Store (almost) arbitrary-type data to a byte-address buffer
	-- byteAddressBufferLoad(%buffer, %offset, %alignment, %src)
	-- where
	-- - `buffer` is a value of some `ByteAddressBufferTypeBase` type
	-- - `offset` is an `int`
	-- - `alignment` is an `int`
	-- - `src` is a value of some type containing only ordinary data
	{ byteAddressBufferStore = { min_operands = 4 } },
	-- Load data from a structured buffer
	-- %dst = structuredBufferLoad(%buffer, %index)
	-- where
	-- - `buffer` is a value of some `StructuredBufferTypeBase` type with element type T
	-- - `offset` is an `int`
	-- - `dst` is a value of type T
	{ structuredBufferLoad = { min_operands = 2 } },
	{ structuredBufferLoadStatus = { min_operands = 3 } },
	{ rwstructuredBufferLoad = { struct_name = "RWStructuredBufferLoad", min_operands = 2 } },
	{
		rwstructuredBufferLoadStatus = {
			struct_name = "RWStructuredBufferLoadStatus",
			min_operands = 3,
		},
	},
	-- Store data to a structured buffer
	-- structuredBufferLoad(%buffer, %offset, %src)
	-- where
	-- - `buffer` is a value of some `StructuredBufferTypeBase` type with element type T
	-- - `offset` is an `int`
	-- - `src` is a value of type T
	{ rwstructuredBufferStore = { struct_name = "RWStructuredBufferStore", min_operands = 3 } },
	{
		rwstructuredBufferGetElementPtr = {
			struct_name = "RWStructuredBufferGetElementPtr",
			min_operands = 2,
		},
	},
	-- Append/Consume-StructuredBuffer operations
	{ StructuredBufferAppend = { min_operands = 1 } },
	{ StructuredBufferConsume = { min_operands = 1 } },
	{ StructuredBufferGetDimensions = { min_operands = 1 } },
	-- Resource qualifiers for dynamically varying index
	{ nonUniformResourceIndex = { min_operands = 1 } },
	{ getNaturalStride = { min_operands = 1 } },
	{ meshOutputRef = { min_operands = 2 } },
	{ meshOutputSet = { min_operands = 3 } },
	-- only two parameters as they are effectively static
	-- TODO: make them reference the _slang_mesh object directly
	{ metalSetVertex = { min_operands = 2 } },
	{ metalSetPrimitive = { min_operands = 2 } },
	{ metalSetIndices = { min_operands = 2 } },
	{ MetalCastToDepthTexture = { min_operands = 1 } },
	-- Construct a vector from a scalar
	-- %dst = MakeVectorFromScalar %T %N %val
	-- where
	-- - `T` is a `Type`
	-- - `N` is a (compile-time) `Int`
	-- - `val` is a `T`
	-- - dst is a `Vec<T,N>`
	{ MakeVectorFromScalar = { min_operands = 3 } },
	-- A swizzle of a vector:
	-- %dst = swizzle %src %idx0 %idx1 ...
	-- where:
	-- - `src` is a vector<T,N>
	-- - `dst` is a vector<T,M>
	-- - `idx0` through `idx[M-1]` are literal integers
	{ swizzle = { min_operands = 1 } },
	-- Setting a vector via swizzle
	--
	-- %dst = swizzle %base %src %idx0 %idx1 ...
	--
	-- where:
	-- - `base` is a vector<T,N>
	-- - `dst` is a vector<T,N>
	-- - `src` is a vector<T,M>
	-- - `idx0` through `idx[M-1]` are literal integers
	--
	-- The semantics of the op is:
	--
	--   dst = base;
	--   for(ii : 0 ... M-1 )
	--     dst[ii] = src[idx[ii]];
	{ swizzleSet = { min_operands = 2 } },
	-- Store to memory with a swizzle
	--
	-- TODO: eventually this should be reduced to just
	-- a write mask by moving the actual swizzle to the RHS.
	--
	-- swizzleStore %dst %src %idx0 %idx1 ...
	--
	-- where:
	-- - `dst` is a vector<T,N>
	-- - `src` is a vector<T,M>
	-- - `idx0` through `idx[M-1]` are literal integers
	--
	-- The semantics of the op is:
	--
	--   for(ii : 0 ... M-1 )
	--     dst[ii] = src[idx[ii]];
	{ swizzledStore = { min_operands = 2 } },
	{
		TerminatorInst = {
			{ return_val = { struct_name = "Return", min_operands = 1 } },
			{ yield = { min_operands = 1 } },
			{
				UnconditionalBranch = {
					-- IRUnconditionalBranch
					{
						unconditionalBranch = {
							-- unconditionalBranch <target>
							min_operands = 1,
						},
					},
					{
						loop = {
							-- loop <target> <breakLabel> <continueLabel>
							min_operands = 3,
						},
					},
				},
			},
			{
				ConditionalBranch = {
					-- IRTerminatorInst
					{
						conditionalBranch = {
							-- conditionalBranch <condition> <trueBlock> <falseBlock>
							struct_name = "ConditionalBranch",
							min_operands = 3,
						},
					},
					{
						ifElse = {
							-- ifElse <condition> <trueBlock> <falseBlock> <mergeBlock>
							struct_name = "IfElse",
							min_operands = 4,
						},
					},
				},
			},
			{
				throw = {
					-- IRConditionalbranch
					min_operands = 1,
				},
			},
			{
				tryCall = {
					-- tryCall <successBlock> <failBlock> <callee> <args>...
					min_operands = 3,
				},
			},
			{
				switch = {
					-- switch <val> <break> <default> <caseVal1> <caseBlock1> ...
					min_operands = 3,
				},
			},
			{
				targetSwitch = {
					-- target_switch <break> <targetName1> <block1> ...
					min_operands = 1,
				},
			},
			{
				GenericAsm = {
					-- A generic asm inst has an return semantics that terminates the control flow.
					min_operands = 1,
				},
			},
			{
				Unreachable = {
					{
						missingReturn = {
							-- IRUnreachable
						},
					},
					{ unreachable = {} },
				},
			},
			{ defer = { min_operands = 3 } },
		},
	},
	{ discard = {} },
	{
		RequirePrelude = { min_operands = 1 },
	},
	{ RequireTargetExtension = { min_operands = 1 } },
	{ RequireComputeDerivative = {} },
	{ StaticAssert = { min_operands = 2 } },
	{ Printf = { min_operands = 1 } },
	-- Quad control execution modes.
	{ RequireMaximallyReconverges = {} },
	{ RequireQuadDerivatives = {} },
	-- TODO: We should consider splitting the basic arithmetic/comparison
	-- ops into cases for signed integers, unsigned integers, and floating-point
	-- values, to better match downstream targets that want to treat them
	-- all differently ().
	{ add = { min_operands = 2 } },
	{ sub = { min_operands = 2 } },
	{ mul = { min_operands = 2 } },
	{ div = { min_operands = 2 } },
	-- Remainder of division.
	-- Note: this is distinct from modulus, and we should have a separate
	-- opcode for `mod` if we ever need to support it.
	{ irem = { struct_name = "IRem", min_operands = 2 } },
	{
		frem = {
			struct_name = "FRem",
			min_operands = 2,
		},
	},
	{
		shl = { struct_name = "Lsh", min_operands = 2 },
	},
	{ shr = { struct_name = "Rsh", min_operands = 2 } },
	{ cmpEQ = { struct_name = "Eql", min_operands = 2 } },
	{
		cmpNE = {
			struct_name = "Neq",
			min_operands = 2,
		},
	},
	{
		cmpGT = { struct_name = "Greater", min_operands = 2 },
	},
	{ cmpLT = { struct_name = "Less", min_operands = 2 } },
	{ cmpGE = { struct_name = "Geq", min_operands = 2 } },
	{
		cmpLE = {
			struct_name = "Leq",
			min_operands = 2,
		},
	},
	{
		["and"] = { struct_name = "BitAnd", min_operands = 2 },
	},
	{ xor = { struct_name = "BitXor", min_operands = 2 } },
	{ ["or"] = { struct_name = "BitOr", min_operands = 2 } },
	{
		logicalAnd = {
			struct_name = "And",
			min_operands = 2,
		},
	},
	{
		logicalOr = { struct_name = "Or", min_operands = 2 },
	},
	{ neg = { min_operands = 1 } },
	{
		["not"] = { min_operands = 1 },
	},
	{ bitnot = { struct_name = "BitNot", min_operands = 1 } },
	{ select = { min_operands = 3 } },
	{
		checkpointObj = {
			struct_name = "CheckpointObject",
			min_operands = 1,
		},
	},
	{ loopExitValue = { min_operands = 1 } },
	{
		getStringHash = {
			operands = { { "stringLit", "IRStringLit" } },
		},
	},
	{ waveGetActiveMask = {} },
	-- trueMask = waveMaskBallot(mask, condition)
	{ waveMaskBallot = { min_operands = 2 } },
	-- matchMask = waveMaskBallot(mask, value)
	{ waveMaskMatch = { min_operands = 2 } },
	-- Texture sampling operation of the form `t.Sample(s,u)`
	{ sample = { min_operands = 3 } },
	{ sampleGrad = { min_operands = 4 } },
	{ GroupMemoryBarrierWithGroupSync = {} },
	{ ControlBarrier = {} },
	-- GPU_FOREACH loop of the form
	{ gpuForeach = { min_operands = 3 } },
	-- Wrapper for OptiX intrinsics used to load and store ray payload data using
	-- a pointer represented by two payload registers.
	{ getOptiXRayPayloadPtr = { hoistable = true } },
	-- Wrapper for OptiX intrinsics used to load a single hit attribute
	-- Takes two arguments: the type (either float or int), and the hit
	-- attribute index
	{ getOptiXHitAttribute = { min_operands = 2 } },
	-- Wrapper for OptiX intrinsics used to load shader binding table record data
	-- using a pointer.
	{ getOptiXSbtDataPointer = { struct_name = "GetOptiXSbtDataPtr" } },
	{ GetVulkanRayTracingPayloadLocation = { min_operands = 1 } },
	{ GetLegalizedSPIRVGlobalParamAddr = { min_operands = 1 } },
	{
		GetPerVertexInputArray = {
			min_operands = 1,
			hoistable = true,
		},
	},
	{ ResolveVaryingInputRef = { min_operands = 1, hoistable = true } },
	{
		ForceVarIntoStructTemporarilyBase = {
			{ ForceVarIntoStructTemporarily = { min_operands = 1 } },
			{
				ForceVarIntoRayPayloadStructTemporarily = {
					min_operands = 1,
				},
			},
		},
	},
	{ MetalAtomicCast = { min_operands = 1 } },
	{ IsTextureAccess = { min_operands = 1 } },
	{ IsTextureScalarAccess = { min_operands = 1 } },
	{ IsTextureArrayAccess = { min_operands = 1 } },
	{ ExtractTextureFromTextureAccess = { min_operands = 1 } },
	{ ExtractCoordFromTextureAccess = { min_operands = 1 } },
	{ ExtractArrayCoordFromTextureAccess = { min_operands = 1 } },
	{ makeArrayList = {} },
	{ makeTensorView = {} },
	{ allocTorchTensor = { struct_name = "AllocateTorchTensor" } },
	{ TorchGetCudaStream = {} },
	{ TorchTensorGetView = {} },
	{ CoopMatMapElementIFunc = { min_operands = 2 } },
	{ allocateOpaqueHandle = {} },
	{
		BindingQuery = {
			{
				getRegisterIndex = {
					-- Return the register index thtat a resource is bound to.
					min_operands = 1,
				},
			},
			{
				getRegisterSpace = {
					-- Return the registe space that a resource is bound to.
					min_operands = 1,
				},
			},
		},
	},
	{
		Decoration = {
			{
				highLevelDecl = { struct_name = "HighLevelDeclDecoration", min_operands = 1 },
			},
			{
				layout = {
					struct_name = "LayoutDecoration",
					min_operands = 1,
				},
			},
			{ branch = { struct_name = "BranchDecoration" } },
			{
				flatten = {
					struct_name = "FlattenDecoration",
				},
			},
			{ loopControl = { struct_name = "LoopControlDecoration", min_operands = 1 } },
			{ loopMaxIters = { struct_name = "LoopMaxItersDecoration", min_operands = 1 } },
			{
				loopExitPrimalValue = { struct_name = "LoopExitPrimalValueDecoration", min_operands = 2 },
			},
			{
				intrinsicOp = {
					struct_name = "IntrinsicOpDecoration",
					min_operands = 1,
				},
			},
			{
				TargetSpecificDecoration = {
					{
						TargetSpecificDefinitionDecoration = {
							{
								target = { struct_name = "TargetDecoration", min_operands = 1 },
							},
							{
								targetIntrinsic = {
									struct_name = "TargetIntrinsicDecoration",
									min_operands = 2,
								},
							},
						},
					},
					{
						requirePrelude = {
							struct_name = "RequirePreludeDecoration",
							min_operands = 2,
						},
					},
				},
			},
			{
				glslOuterArray = {
					struct_name = "GLSLOuterArrayDecoration",
					min_operands = 1,
				},
			},
			{ TargetSystemValue = { struct_name = "TargetSystemValueDecoration", min_operands = 2 } },
			{ interpolationMode = { struct_name = "InterpolationModeDecoration", min_operands = 1 } },
			{
				nameHint = { struct_name = "NameHintDecoration", min_operands = 1 },
			},
			{
				PhysicalType = {
					struct_name = "PhysicalTypeDecoration",
					min_operands = 1,
				},
			},
			{
				AlignedAddressDecoration = {
					-- Mark an address instruction as aligned to a specific byte boundary.
					min_operands = 1,
				},
			},
			{
				BinaryInterfaceType = {
					-- Marks a type as being used as binary interface (e.g. shader parameters).
					-- This prevents the legalizeEmptyType() pass from eliminating it on C++/CUDA targets.
					struct_name = "BinaryInterfaceTypeDecoration",
				},
			},
			{
				transitory = {
					-- *  The decorated _instruction_ is transitory. Such a decoration should NEVER be found on an output instruction a module.
					--     Typically used mark an instruction so can be specially handled - say when creating a IRConstant literal, and the payload of
					--     needs to be special cased for lookup.
					struct_name = "TransitoryDecoration",
				},
			},
			{
				ResultWitness = {
					-- The result witness table that the functon's return type is a subtype of an interface.
					-- This is used to keep track of the original witness table in a function that used to
					-- return an existential value but now returns a concrete type after specialization.
					struct_name = "ResultWitnessDecoration",
					min_operands = 1,
				},
			},
			-- A decoration that indicates that a variable represents
			-- a vulkan ray payload, and should have a location assigned
			-- to it.
			{ vulkanRayPayload = { struct_name = "VulkanRayPayloadDecoration" } },
			{ vulkanRayPayloadIn = { struct_name = "VulkanRayPayloadInDecoration" } },
			-- A decoration that indicates that a variable represents
			-- vulkan hit attributes, and should have a location assigned
			-- to it.
			{ vulkanHitAttributes = { struct_name = "VulkanHitAttributesDecoration" } },
			-- A decoration that indicates that a variable represents
			-- vulkan hit object attributes, and should have a location assigned
			-- to it.
			{ vulkanHitObjectAttributes = { struct_name = "VulkanHitObjectAttributesDecoration" } },
			{ GlobalVariableShadowingGlobalParameterDecoration = { min_operands = 2 } },
			{ requireSPIRVVersion = { struct_name = "RequireSPIRVVersionDecoration", min_operands = 1 } },
			{
				requireGLSLVersion = {
					struct_name = "RequireGLSLVersionDecoration",
					min_operands = 1,
				},
			},
			{
				requireGLSLExtension = { struct_name = "RequireGLSLExtensionDecoration", min_operands = 1 },
			},
			{ requireWGSLExtension = { struct_name = "RequireWGSLExtensionDecoration", min_operands = 1 } },
			{ requireCUDASMVersion = { struct_name = "RequireCUDASMVersionDecoration", min_operands = 1 } },
			{
				requireCapabilityAtom = {
					struct_name = "RequireCapabilityAtomDecoration",
					min_operands = 1,
				},
			},
			{ HasExplicitHLSLBinding = { struct_name = "HasExplicitHLSLBindingDecoration" } },
			{ DefaultValue = { struct_name = "DefaultValueDecoration", min_operands = 1 } },
			{
				readNone = { struct_name = "ReadNoneDecoration" },
			},
			-- A decoration that indicates that a variable represents
			-- a vulkan callable shader payload, and should have a location assigned
			-- to it.
			{ vulkanCallablePayload = { struct_name = "VulkanCallablePayloadDecoration" } },
			{ vulkanCallablePayloadIn = { struct_name = "VulkanCallablePayloadInDecoration" } },
			{ earlyDepthStencil = { struct_name = "EarlyDepthStencilDecoration" } },
			{ precise = { struct_name = "PreciseDecoration" } },
			{ public = { struct_name = "PublicDecoration" } },
			{ hlslExport = { struct_name = "HLSLExportDecoration" } },
			{ downstreamModuleExport = { struct_name = "DownstreamModuleExportDecoration" } },
			{ downstreamModuleImport = { struct_name = "DownstreamModuleImportDecoration" } },
			{ patchConstantFunc = { struct_name = "PatchConstantFuncDecoration", min_operands = 1 } },
			{
				maxTessFactor = {
					struct_name = "MaxTessFactorDecoration",
					min_operands = 1,
				},
			},
			{
				outputControlPoints = { struct_name = "OutputControlPointsDecoration", min_operands = 1 },
			},
			{ outputTopology = { struct_name = "OutputTopologyDecoration", min_operands = 2 } },
			{ partioning = { struct_name = "PartitioningDecoration", min_operands = 1 } },
			{
				domain = {
					struct_name = "DomainDecoration",
					min_operands = 1,
				},
			},
			{
				maxVertexCount = { struct_name = "MaxVertexCountDecoration", min_operands = 1 },
			},
			{ instance = { struct_name = "InstanceDecoration", min_operands = 1 } },
			{ numThreads = { struct_name = "NumThreadsDecoration", min_operands = 3 } },
			{ fpDenormalPreserve = { struct_name = "FpDenormalPreserveDecoration", min_operands = 1 } },
			{ fpDenormalFlushToZero = { struct_name = "FpDenormalFlushToZeroDecoration", min_operands = 1 } },
			{
				waveSize = {
					struct_name = "WaveSizeDecoration",
					min_operands = 1,
				},
			},
			{
				availableInDownstreamIR = { struct_name = "AvailableInDownstreamIRDecoration", min_operands = 1 },
			},
			{
				GeometryInputPrimitiveTypeDecoration = {
					{
						pointPrimitiveType = {
							-- Added to IRParam parameters to an entry point
							struct_name = "PointInputPrimitiveTypeDecoration",
						},
					},
					{ linePrimitiveType = { struct_name = "LineInputPrimitiveTypeDecoration" } },
					{
						trianglePrimitiveType = {
							struct_name = "TriangleInputPrimitiveTypeDecoration",
						},
					},
					{ lineAdjPrimitiveType = { struct_name = "LineAdjInputPrimitiveTypeDecoration" } },
					{
						triangleAdjPrimitiveType = {
							struct_name = "TriangleAdjInputPrimitiveTypeDecoration",
						},
					},
				},
			},
			{ streamOutputTypeDecoration = { min_operands = 1 } },
			{
				entryPoint = {
					-- An `[entryPoint]` decoration marks a function that represents a shader entry point
					struct_name = "EntryPointDecoration",
					min_operands = 2,
				},
			},
			{ CudaKernel = { struct_name = "CudaKernelDecoration" } },
			{ CudaHost = { struct_name = "CudaHostDecoration" } },
			{ TorchEntryPoint = { struct_name = "TorchEntryPointDecoration" } },
			{ AutoPyBindCUDA = { struct_name = "AutoPyBindCudaDecoration" } },
			{ CudaKernelFwdDiffRef = { struct_name = "CudaKernelForwardDerivativeDecoration" } },
			{ CudaKernelBwdDiffRef = { struct_name = "CudaKernelBackwardDerivativeDecoration" } },
			{ PyBindExportFuncInfo = { struct_name = "AutoPyBindExportInfoDecoration" } },
			{ PyExportDecoration = {} },
			{
				entryPointParam = {
					-- Used to mark parameters that are moved from entry point parameters to global params as coming from the entry
					-- point.
					struct_name = "EntryPointParamDecoration",
				},
			},
			{
				dependsOn = {
					-- A `[dependsOn(x)]` decoration indicates that the parent instruction depends on `x`
					-- even if it does not otherwise reference it.
					struct_name = "DependsOnDecoration",
					min_operands = 1,
				},
			},
			{
				keepAlive = {
					-- A `[keepAlive]` decoration marks an instruction that should not be eliminated.
					struct_name = "KeepAliveDecoration",
				},
			},
			{
				noSideEffect = {
					-- A `[NoSideEffect]` decoration marks a callee to be side-effect free.
					struct_name = "NoSideEffectDecoration",
				},
			},
			{ bindExistentialSlots = { struct_name = "BindExistentialSlotsDecoration" } },
			{
				format = {
					-- A `[format(f)]` decoration specifies that the format of an image should be `f`
					struct_name = "FormatDecoration",
					min_operands = 1,
				},
			},
			{
				unsafeForceInlineEarly = {
					-- An `[unsafeForceInlineEarly]` decoration specifies that calls to this function should be inline after initial
					-- codegen
					struct_name = "UnsafeForceInlineEarlyDecoration",
				},
			},
			{
				ForceInline = {
					-- A `[ForceInline]` decoration indicates the callee should be inlined by the Slang compiler.
					struct_name = "ForceInlineDecoration",
				},
			},
			{
				ForceUnroll = {
					-- A `[ForceUnroll]` decoration indicates the loop should be unrolled by the Slang compiler.
					struct_name = "ForceUnrollDecoration",
				},
			},
			{
				SizeAndAlignment = {
					-- A `[SizeAndAlignment(l,s,a)]` decoration is attached to a type to indicate that is has size `s` and alignment
					-- `a` under layout rules `l`.
					struct_name = "SizeAndAlignmentDecoration",
					min_operands = 3,
				},
			},
			{
				Offset = {
					-- A `[Offset(l, o)]` decoration is attached to a field to indicate that it has offset `o` in the parent type
					-- under layout rules `l`.
					struct_name = "OffsetDecoration",
					min_operands = 2,
				},
			},
			{
				LinkageDecoration = {
					{
						import = {
							struct_name = "ImportDecoration",
							min_operands = 1,
						},
					},
					{
						export = { struct_name = "ExportDecoration", min_operands = 1 },
					},
				},
			},
			{
				TargetBuiltinVar = {
					-- Mark a global variable as a target builtin variable.
					struct_name = "TargetBuiltinVarDecoration",
					min_operands = 1,
				},
			},
			{
				UserExtern = {
					-- Marks an inst as coming from an `extern` symbol defined in the user code.
					struct_name = "UserExternDecoration",
				},
			},
			{
				externCpp = {
					-- An extern_cpp decoration marks the inst to emit its name without mangling for C++ interop.
					struct_name = "ExternCppDecoration",
					min_operands = 1,
				},
			},
			{
				externC = {
					-- An externC decoration marks a function should be emitted inside an extern "C" block.
					struct_name = "ExternCDecoration",
				},
			},
			{
				dllImport = {
					-- An dllImport decoration marks a function as imported from a DLL. Slang will generate dynamic function loading
					-- logic to use this function at runtime.
					struct_name = "DllImportDecoration",
					min_operands = 2,
				},
			},
			{
				dllExport = {
					-- An dllExport decoration marks a function as an export symbol. Slang will generate a native wrapper function
					-- that is exported to DLL.
					struct_name = "DllExportDecoration",
					min_operands = 1,
				},
			},
			{
				cudaDeviceExport = {
					-- An cudaDeviceExport decoration marks a function to be exported as a cuda __device__ function.
					struct_name = "CudaDeviceExportDecoration",
					min_operands = 1,
				},
			},
			{
				COMInterface = {
					-- Marks an interface as a COM interface declaration.
					struct_name = "ComInterfaceDecoration",
				},
			},
			{
				KnownBuiltinDecoration = {
					-- Attaches a name to this instruction so that it can be identified
					-- later in the compiler reliably
					min_operands = 1,
				},
			},
			{
				RTTI_typeSize = {
					-- Decorations for RTTI objects
					struct_name = "RTTITypeSizeDecoration",
					min_operands = 1,
				},
			},
			{
				AnyValueSize = { struct_name = "AnyValueSizeDecoration", min_operands = 1 },
			},
			{ SpecializeDecoration = {} },
			{ SequentialIDDecoration = { min_operands = 1 } },
			{ DynamicDispatchWitnessDecoration = {} },
			{ StaticRequirementDecoration = {} },
			{ DispatchFuncDecoration = { min_operands = 1 } },
			{ TypeConstraintDecoration = { min_operands = 1 } },
			{ BuiltinDecoration = {} },
			{
				requiresNVAPI = {
					-- The decorated instruction requires NVAPI to be included via prelude when compiling for D3D.
					struct_name = "RequiresNVAPIDecoration",
				},
			},
			{
				nvapiMagic = {
					-- The decorated instruction is part of the NVAPI "magic" and should always use its original name
					struct_name = "NVAPIMagicDecoration",
					min_operands = 1,
				},
			},
			{
				nvapiSlot = {
					-- A decoration that applies to an entire IR module, and indicates the register/space binding
					-- that the NVAPI shader parameter intends to use.
					struct_name = "NVAPISlotDecoration",
					min_operands = 2,
				},
			},
			{
				noInline = {
					-- Applie to an IR function and signals that inlining should not be performed unless unavoidable.
					struct_name = "NoInlineDecoration",
				},
			},
			{ noRefInline = { struct_name = "NoRefInlineDecoration" } },
			{
				DerivativeGroupQuad = {
					struct_name = "DerivativeGroupQuadDecoration",
				},
			},
			{ DerivativeGroupLinear = { struct_name = "DerivativeGroupLinearDecoration" } },
			{
				MaximallyReconverges = {
					struct_name = "MaximallyReconvergesDecoration",
				},
			},
			{ QuadDerivatives = { struct_name = "QuadDerivativesDecoration" } },
			{
				RequireFullQuads = {
					struct_name = "RequireFullQuadsDecoration",
				},
			},
			{ TempCallArgVar = { struct_name = "TempCallArgVarDecoration" } },
			{
				nonCopyable = {
					-- Marks a type to be non copyable, causing SSA pass to skip turning variables of the the type into SSA values.
					struct_name = "NonCopyableTypeDecoration",
				},
			},
			{
				DynamicUniform = {
					-- Marks a value to be dynamically uniform.
					struct_name = "DynamicUniformDecoration",
				},
			},
			{
				alwaysFold = {
					-- A call to the decorated function should always be folded into its use site.
					struct_name = "AlwaysFoldIntoUseSiteDecoration",
				},
			},
			{ output = { struct_name = "GlobalOutputDecoration" } },
			{
				input = {
					struct_name = "GlobalInputDecoration",
				},
			},
			{ glslLocation = { struct_name = "GLSLLocationDecoration", min_operands = 1 } },
			{ glslOffset = { struct_name = "GLSLOffsetDecoration", min_operands = 1 } },
			{
				vkStructOffset = { struct_name = "VkStructOffsetDecoration", min_operands = 1 },
			},
			{ raypayload = { struct_name = "RayPayloadDecoration" } },
			{
				MeshOutputDecoration = {
					-- Mesh Shader outputs
					{ vertices = { struct_name = "VerticesDecoration", min_operands = 1 } },
					{
						indices = {
							struct_name = "IndicesDecoration",
							min_operands = 1,
						},
					},
					{
						primitives = { struct_name = "PrimitivesDecoration", min_operands = 1 },
					},
				},
			},
			{ HLSLMeshPayloadDecoration = { struct_name = "HLSLMeshPayloadDecoration" } },
			{ perprimitive = { struct_name = "GLSLPrimitivesRateDecoration" } },
			{
				PositionOutput = {
					-- Marks an inst that represents the gl_Position output.
					struct_name = "GLPositionOutputDecoration",
				},
			},
			{
				PositionInput = {
					-- Marks an inst that represents the gl_Position input.
					struct_name = "GLPositionInputDecoration",
				},
			},
			{
				PerVertex = {
					-- Marks a fragment shader input as per-vertex.
					struct_name = "PerVertexDecoration",
				},
			},
			{
				StageAccessDecoration = {
					{ stageReadAccess = { struct_name = "StageReadAccessDecoration" } },
					{ stageWriteAccess = { struct_name = "StageWriteAccessDecoration" } },
				},
			},
			{
				semantic = { struct_name = "SemanticDecoration", min_operands = 2 },
			},
			{
				constructor = {
					struct_name = "ConstructorDecoration",
					min_operands = 1,
				},
			},
			{ method = { struct_name = "MethodDecoration" } },
			{
				packoffset = {
					struct_name = "PackOffsetDecoration",
					min_operands = 2,
				},
			},
			{ SpecializationConstantDecoration = { min_operands = 1 } },
			{
				UserTypeName = {
					-- Reflection metadata for a shader parameter that provides the original type name.
					struct_name = "UserTypeNameDecoration",
					min_operands = 1,
				},
			},
			{
				CounterBuffer = {
					-- Reflection metadata for a shader parameter that refers to the associated counter buffer of a UAV.
					struct_name = "CounterBufferDecoration",
					min_operands = 1,
				},
			},
			{ RequireSPIRVDescriptorIndexingExtensionDecoration = {} },
			{
				spirvOpDecoration = {
					struct_name = "SPIRVOpDecoration",
					min_operands = 1,
				},
			},
			{
				forwardDifferentiable = {
					-- Decorated function is marked for the forward-mode differentiation pass.
					struct_name = "ForwardDifferentiableDecoration",
				},
			},
			{
				AutoDiffOriginalValueDecoration = {
					-- Decorates a auto-diff transcribed value with the original value that the inst is transcribed from.
					min_operands = 1,
				},
			},
			{
				AutoDiffBuiltinDecoration = {
					-- Decorates a type as auto-diff builtin type.
				},
			},
			{
				fwdDerivative = {
					-- Used by the auto-diff pass to hold a reference to the
					-- generated derivative function.
					struct_name = "ForwardDerivativeDecoration",
					min_operands = 1,
				},
			},
			{
				backwardDifferentiable = {
					-- Used by the auto-diff pass to hold a reference to the
					-- generated derivative function.
					struct_name = "BackwardDifferentiableDecoration",
					min_operands = 1,
				},
			},
			{
				primalSubstFunc = {
					-- Used by the auto-diff pass to hold a reference to the
					-- primal substitute function.
					struct_name = "PrimalSubstituteDecoration",
					min_operands = 1,
				},
			},
			{
				backwardDiffPrimalReference = {
					-- Decorations to associate an original function with compiler generated backward derivative functions.
					struct_name = "BackwardDerivativePrimalDecoration",
					min_operands = 1,
				},
			},
			{
				backwardDiffPropagateReference = {
					struct_name = "BackwardDerivativePropagateDecoration",
					min_operands = 1,
				},
			},
			{
				backwardDiffIntermediateTypeReference = {
					struct_name = "BackwardDerivativeIntermediateTypeDecoration",
					min_operands = 1,
				},
			},
			{ backwardDiffReference = { struct_name = "BackwardDerivativeDecoration", min_operands = 1 } },
			{
				userDefinedBackwardDiffReference = {
					struct_name = "UserDefinedBackwardDerivativeDecoration",
					min_operands = 1,
				},
			},
			{ BackwardDerivativePrimalContextDecoration = { min_operands = 1 } },
			{ BackwardDerivativePrimalReturnDecoration = { min_operands = 1 } },
			{
				PrimalContextDecoration = {
					-- Mark a parameter as autodiff primal context.
				},
			},
			{ loopCounterDecoration = {} },
			{ loopCounterUpdateDecoration = {} },
			{
				AutodiffInstDecoration = {
					-- Auto-diff inst decorations
					{
						primalInstDecoration = {
							-- Used by the auto-diff pass to mark insts that compute
							-- a primal value.
						},
					},
					{
						diffInstDecoration = {
							-- Used by the auto-diff pass to mark insts that compute
							-- a differential value.
							struct_name = "DifferentialInstDecoration",
							min_operands = 1,
						},
					},
					{
						mixedDiffInstDecoration = {
							-- Used by the auto-diff pass to mark insts that compute
							-- BOTH a differential and a primal value.
							struct_name = "MixedDifferentialInstDecoration",
							min_operands = 1,
						},
					},
					{ RecomputeBlockDecoration = {} },
				},
			},
			{
				primalValueKey = {
					-- Used by the auto-diff pass to mark insts whose result is stored
					-- in an intermediary struct for reuse in backward propagation phase.
					struct_name = "PrimalValueStructKeyDecoration",
					min_operands = 1,
				},
			},
			{
				primalElementType = {
					-- Used by the auto-diff pass to mark the primal element type of an
					-- forward-differentiated updateElement inst.
					struct_name = "PrimalElementTypeDecoration",
					min_operands = 1,
				},
			},
			{
				IntermediateContextFieldDifferentialTypeDecoration = {
					-- Used by the auto-diff pass to mark the differential type of an intermediate context field.
					min_operands = 1,
				},
			},
			{
				derivativeMemberDecoration = {
					-- Used by the auto-diff pass to hold a reference to a
					-- differential member of a type in its associated differential type.
					min_operands = 1,
				},
			},
			{
				treatAsDifferentiableDecoration = {
					-- Treat a function as differentiable function
				},
			},
			{
				treatCallAsDifferentiableDecoration = {
					-- Treat a call to arbitrary function as a differentiable call.
				},
			},
			{
				differentiableCallDecoration = {
					-- Mark a call as explicitly calling a differentiable function.
				},
			},
			{
				optimizableTypeDecoration = {
					-- Mark a type as being eligible for trimming if necessary. If
					-- any fields don't have any effective loads from them, they can be
					-- removed.
				},
			},
			{
				ignoreSideEffectsDecoration = {
					-- Informs the DCE pass to ignore side-effects on this call for
					-- the purposes of dead code elimination, even if the call does have
					-- side-effects.
				},
			},
			{
				CheckpointHintDecoration = {
					{
						PreferCheckpointDecoration = {
							-- Hint that the result from a call to the decorated function should be stored in backward prop function.
						},
					},
					{
						PreferRecomputeDecoration = {
							-- Hint that the result from a call to the decorated function should be recomputed in backward prop function.
						},
					},
					{
						CheckpointIntermediateDecoration = {
							-- Hint that a struct is used for reverse mode checkpointing
							min_operands = 1,
						},
					},
				},
			},
			{
				NonDynamicUniformReturnDecoration = {
					-- Marks a function whose return value is never dynamic uniform.
				},
			},
			{
				COMWitnessDecoration = {
					-- Marks a class type as a COM interface implementation, which enables
					-- the witness table to be easily picked up by emit.
					min_operands = 1,
				},
			},
			{
				DifferentiableTypeDictionaryDecoration = {
					-- Differentiable Type Dictionary
					parent = true,
				},
			},
			{
				FloatingPointModeOverride = {
					struct_name = "FloatingPointModeOverrideDecoration",
					-- Overrides the floating mode for the target function
					min_operands = 1,
				},
			},
			{
				spvBufferBlock = {
					-- Recognized by SPIRV-emit pass so we can emit a SPIRV `BufferBlock` decoration.
					struct_name = "SPIRVBufferBlockDecoration",
				},
			},
			{
				DebugLocation = {
					-- Decorates an inst with a debug source location (IRDebugSource, IRIntLit(line), IRIntLit(col)).
					struct_name = "DebugLocationDecoration",
					min_operands = 3,
				},
			},
			{
				DebugFunction = {
					-- Decorates a function with a link to its debug function representation
					struct_name = "DebugFuncDecoration",
					min_operands = 1,
				},
			},
			{
				spvBlock = {
					-- Recognized by SPIRV-emit pass so we can emit a SPIRV `Block` decoration.
					struct_name = "SPIRVBlockDecoration",
				},
			},
			{
				NonUniformResource = {
					-- Decorates a SPIRV-inst as `NonUniformResource` to guarantee non-uniform index lookup of
					-- - a resource within an array of resources via IRGetElement.
					-- - an IRLoad that takes a pointer within a memory buffer via IRGetElementPtr.
					-- - an IRIntCast to a resource that is casted from signed to unsigned or viceversa.
					-- - an IRGetElementPtr itself when using the pointer on an intrinsic operation.
					struct_name = "SPIRVNonUniformResourceDecoration",
				},
			},
			{
				MemoryQualifierSetDecoration = {
					-- Stores flag bits of which memory qualifiers an object has
					min_operands = 1,
				},
			},
			{
				BitFieldAccessorDecoration = {
					-- Marks a function as one which access a bitfield with the specified
					-- backing value key, width and offset
					min_operands = 3,
				},
			},
		},
	},
	-- Decoration
	-- A `makeExistential(v : C, w) : I` instruction takes a value `v` of type `C`
	-- and produces a value of interface type `I` by using the witness `w` which
	-- shows that `C` conforms to `I`.
	{ makeExistential = { min_operands = 2 } },
	-- A `MakeExistentialWithRTTI(v, w, t)` is the same with `MakeExistential`,
	-- but with the type of `v` being an explict operand.
	{ makeExistentialWithRTTI = { min_operands = 3 } },
	-- A 'CreateExistentialObject<I>(typeID, T)` packs user-provided `typeID` and a
	-- value of any type, and constructs an existential value of type `I`.
	{ createExistentialObject = { min_operands = 2 } },
	-- A `wrapExistential(v, T0,w0, T1,w0) : T` instruction is similar to `makeExistential`.
	-- but applies to a value `v` that is of type `BindExistentials(T, T0,w0, ...)`. The
	-- result of the `wrapExistentials` operation is a value of type `T`, allowing us to
	-- "smuggle" a value of specialized type into computations that expect an unspecialized type.
	{ wrapExistential = { min_operands = 1 } },
	-- A `GetValueFromBoundInterface` takes a `BindInterface<I, T, w0>` value and returns the
	-- value of concrete type `T` value that is being stored.
	{ getValueFromBoundInterface = { min_operands = 1 } },
	{ extractExistentialValue = { min_operands = 1 } },
	{ extractExistentialType = { min_operands = 1, hoistable = true } },
	{
		extractExistentialWitnessTable = {
			min_operands = 1,
			hoistable = true,
		},
	},
	{ isNullExistential = { min_operands = 1 } },
	{ extractTaggedUnionTag = { min_operands = 1 } },
	{ extractTaggedUnionPayload = { min_operands = 1 } },
	{ BuiltinCast = { min_operands = 1 } },
	{ bitCast = { min_operands = 1 } },
	{ reinterpret = { min_operands = 1 } },
	{ unmodified = { min_operands = 1 } },
	{ outImplicitCast = { min_operands = 1 } },
	{ inOutImplicitCast = { min_operands = 1 } },
	{ intCast = { min_operands = 1 } },
	{ floatCast = { min_operands = 1 } },
	{ castIntToFloat = { min_operands = 1 } },
	{ castFloatToInt = { min_operands = 1 } },
	{ CastPtrToBool = { min_operands = 1 } },
	{ CastPtrToInt = { min_operands = 1 } },
	{ CastIntToPtr = { min_operands = 1 } },
	{ castToVoid = { min_operands = 1 } },
	{ PtrCast = { min_operands = 1 } },
	{ CastEnumToInt = { min_operands = 1 } },
	{ CastIntToEnum = { min_operands = 1 } },
	{ EnumCast = { min_operands = 1 } },
	{ CastUInt2ToDescriptorHandle = { min_operands = 1 } },
	{ CastDescriptorHandleToUInt2 = { min_operands = 1 } },
	-- Represents a no-op cast to convert a resource pointer to a resource on targets where the resource handles are
	-- already concrete types.
	{ CastDescriptorHandleToResource = { min_operands = 1 } },
	{ TreatAsDynamicUniform = { min_operands = 1 } },
	{ sizeOf = { min_operands = 1 } },
	{ alignOf = { min_operands = 1 } },
	{ countOf = { min_operands = 1 } },
	{ GetArrayLength = { min_operands = 1 } },
	{ IsType = { min_operands = 3 } },
	{ TypeEquals = { min_operands = 2 } },
	{ IsInt = { min_operands = 1 } },
	{ IsBool = { min_operands = 1 } },
	{ IsFloat = { min_operands = 1 } },
	{ IsHalf = { min_operands = 1 } },
	{ IsUnsignedInt = { min_operands = 1 } },
	{ IsSignedInt = { min_operands = 1 } },
	{ IsVector = { min_operands = 1 } },
	{ GetDynamicResourceHeap = { hoistable = true } },
	{ ForwardDifferentiate = { min_operands = 1 } },
	-- Produces the primal computation of backward derivatives, will return an intermediate context for
	-- backward derivative func.
	{ BackwardDifferentiatePrimal = { min_operands = 1 } },
	-- Produces the actual backward derivative propagate function, using the intermediate context returned by the
	-- primal func produced from `BackwardDifferentiatePrimal`.
	{ BackwardDifferentiatePropagate = { min_operands = 1 } },
	-- Represents the conceptual backward derivative function. Only produced by lower-to-ir and will be
	-- replaced with `BackwardDifferentiatePrimal` and `BackwardDifferentiatePropagate`.
	{ BackwardDifferentiate = { min_operands = 1 } },
	{ PrimalSubstitute = { min_operands = 1 } },
	{ DispatchKernel = { min_operands = 3 } },
	{ CudaKernelLaunch = { min_operands = 6 } },
	-- Converts other resources (such as ByteAddressBuffer) to the equivalent StructuredBuffer
	{ getEquivalentStructuredBuffer = { min_operands = 1 } },
	-- Gets a T[] pointer to the underlying data of a StructuredBuffer etc...
	{ getStructuredBufferPtr = { min_operands = 1 } },
	-- Gets a uint[] pointer to the underlying data of a ByteAddressBuffer etc...
	{ getUntypedBufferPtr = { min_operands = 1 } },
	{
		Layout = {
			hoistable = true,
			{ varLayout = { min_operands = 1 } },
			{
				TypeLayout = {
					{
						typeLayout = { struct_name = "TypeLayoutBase" },
					},
					{ parameterGroupTypeLayout = { min_operands = 2 } },
					{
						arrayTypeLayout = { min_operands = 1 },
					},
					{ streamOutputTypeLayout = { min_operands = 1 } },
					{
						matrixTypeLayout = { min_operands = 1 },
					},
					{ existentialTypeLayout = {} },
					{ structTypeLayout = {} },
					{ tupleTypeLayout = {} },
					{ structuredBufferTypeLayout = { min_operands = 1 } },
					{
						ptrTypeLayout = {
							-- "TODO(JS): Ideally we'd have the layout to the pointed to value type (ie 1 instead of 0 here). But to
							-- avoid infinite recursion we don't."
							struct_name = "PointerTypeLayout",
						},
					},
				},
			},
			{ EntryPointLayout = { min_operands = 1 } },
		},
	},
	{
		Attr = {
			hoistable = true,
			{
				pendingLayout = {
					struct_name = "PendingLayoutAttr",
					min_operands = 1,
				},
			},
			{ stage = { struct_name = "StageAttr", min_operands = 1 } },
			{ structFieldLayout = { struct_name = "StructFieldLayoutAttr", min_operands = 2 } },
			{
				tupleFieldLayout = { struct_name = "TupleFieldLayoutAttr", min_operands = 1 },
			},
			{
				caseLayout = {
					struct_name = "CaseTypeLayoutAttr",
					min_operands = 1,
				},
			},
			{ unorm = { struct_name = "UNormAttr" } },
			{
				snorm = {
					struct_name = "SNormAttr",
				},
			},
			{ no_diff = { struct_name = "NoDiffAttr" } },
			{
				nonuniform = {
					struct_name = "NonUniformAttr",
				},
			},
			{ Aligned = { struct_name = "AlignedAttr", min_operands = 1 } },
			{
				SemanticAttr = {
					{ userSemantic = { struct_name = "UserSemanticAttr", min_operands = 2 } },
					{ systemValueSemantic = { struct_name = "SystemValueSemanticAttr", min_operands = 2 } },
				},
			},
			{
				LayoutResourceInfoAttr = {
					{ size = { struct_name = "TypeSizeAttr", min_operands = 2 } },
					{ offset = { struct_name = "VarOffsetAttr", min_operands = 2 } },
				},
			},
			{ FuncThrowType = { struct_name = "FuncThrowTypeAttr", min_operands = 1 } },
		},
	},
	-- Liveness
	{
		LiveRangeMarker = { { liveRangeStart = { min_operands = 2 } }, { liveRangeEnd = {} } },
	},
	-- IRSpecialization
	{ SpecializationDictionaryItem = {} },
	{ GenericSpecializationDictionary = { parent = true } },
	{ ExistentialFuncSpecializationDictionary = { parent = true } },
	{ ExistentialTypeSpecializationDictionary = { parent = true } },
	-- Differentiable Type Dictionary
	{ DifferentiableTypeDictionaryItem = {} },
	-- Differentiable Type Annotation (for run-time types)
	{ DifferentiableTypeAnnotation = { min_operands = 2, hoistable = true } },
	{ BeginFragmentShaderInterlock = {} },
	{
		EndFragmentShaderInterlock = { struct_name = "EndFragmentShaderInterlock" },
	},
	-- DebugInfo
	{ DebugSource = { min_operands = 2, hoistable = true } },
	{
		DebugLine = {
			min_operands = 5,
		},
	},
	{ DebugVar = { min_operands = 4 } },
	{
		DebugValue = {
			min_operands = 2,
		},
	},
	{ DebugInlinedAt = { min_operands = 5 } },
	{
		DebugFunction = {
			min_operands = 5,
		},
	},
	{ DebugInlinedVariable = { min_operands = 2 } },
	{
		DebugScope = {
			min_operands = 2,
		},
	},
	{ DebugNoScope = { min_operands = 1 } },
	{
		DebugBuildIdentifier = {
			min_operands = 2,
		},
	},
	-- Embedded Precompiled Libraries
	{ EmbeddedDownstreamIR = { min_operands = 2 } },
	-- Inline assembly
	{ SPIRVAsm = { parent = true } },
	{ SPIRVAsmInst = { min_operands = 1 } },
	{
		SPIRVAsmOperand = {
			{
				SPIRVAsmOperandLiteral = {
					-- These instruction serve to inform the backend precisely how to emit each
					-- instruction, consider the difference between emitting a literal integer
					-- and a reference to a literal integer instruction
					-- A literal string or 32-bit integer to be passed as operands
					min_operands = 1,
					hoistable = true,
				},
			},
			{
				SPIRVAsmOperandInst = {
					-- A reference to a slang IRInst, either a value or a type
					-- This isn't hoistable, as we sometimes need to change the used value and
					-- instructions around the specific asm block
					min_operands = 1,
				},
			},
			{ SPIRVAsmOperandConvertTexel = { min_operands = 1 } },
			{
				SPIRVAsmOperandRayPayloadFromLocation = {
					-- a late resolving type to handle the case of ray objects (resolving late due to constexpr data requirment)
					min_operands = 1,
				},
			},
			{ SPIRVAsmOperandRayAttributeFromLocation = { min_operands = 1 } },
			{ SPIRVAsmOperandRayCallableFromLocation = { min_operands = 1 } },
			{
				SPIRVAsmOperandEnum = {
					-- A named enumerator, the value is stored as a constant operand
					-- It may have a second operand, which if present is a type with which to
					-- construct a constant id to pass, instead of a literal constant
					min_operands = 1,
					hoistable = true,
				},
			},
			{
				SPIRVAsmOperandBuiltinVar = {
					-- A reference to a builtin variable.
					min_operands = 1,
					hoistable = true,
				},
			},
			{
				SPIRVAsmOperandGLSL450Set = {
					-- A reference to the glsl450 instruction set.
					hoistable = true,
				},
			},
			{ SPIRVAsmOperandDebugPrintfSet = { hoistable = true } },
			{
				SPIRVAsmOperandId = {
					-- A string which is given a unique ID in the backend, used to refer to
					-- results of other instrucions in the same asm block
					min_operands = 1,
					hoistable = true,
				},
			},
			{
				SPIRVAsmOperandResult = {
					-- A special instruction which marks the place to insert the generated
					-- result operand
					hoistable = true,
				},
			},
			{
				["__truncate"] = {
					-- A special instruction which represents a type directed truncation
					-- operation where extra components are dropped
					struct_name = "SPIRVAsmOperandTruncate",
					hoistable = true,
				},
			},
			{
				["__entryPoint"] = {
					-- A special instruction which represents an ID of an entry point that references the current function.
					struct_name = "SPIRVAsmOperandEntryPoint",
					hoistable = true,
				},
			},
			{
				["__sampledType"] = {
					-- A type function which returns the result type of sampling an image of
					-- this component type
					struct_name = "SPIRVAsmOperandSampledType",
					min_operands = 1,
					hoistable = true,
				},
			},
			{
				["__imageType"] = {
					-- A type function which returns the equivalent OpTypeImage type of sampled image value
					struct_name = "SPIRVAsmOperandImageType",
					min_operands = 1,
					hoistable = true,
				},
			},
			{
				["__sampledImageType"] = {
					-- A type function which returns the equivalent OpTypeImage type of sampled image value
					struct_name = "SPIRVAsmOperandSampledImageType",
					min_operands = 1,
					hoistable = true,
				},
			},
		},
	},
}

-- A function to calculate some useful properties and put it in the table,
--
-- Annotates instructions with whether they are a leaf or not
-- Calculates flags from the parent flags
local function process(insts)
	local stable_names_file = "source/slang/slang-ir-insts-stable-names.lua"

	local function to_pascal_case(str)
		local result = str:gsub("_(.)", function(c)
			return c:upper()
		end)
		return result:sub(1, 1):upper() .. result:sub(2)
	end

	-- Check if an instruction is a leaf
	local function is_leaf(inst)
		for k, v in ipairs(inst) do
			return false
		end
		return true
	end

	-- Load stable names if file is provided
	local name_to_stable_name = loadfile(stable_names_file)()
	local stable_name_to_inst = {}
	local max_stable_name = 0

	-- Build full path for stable name lookup
	local function build_path(inst, name)
		local path = { name }
		local current = inst.parent_inst
		while current and current.parent_inst do
			-- Find the name of current in its parent
			for _, entry in ipairs(current.parent_inst) do
				local k, v = next(entry)
				if v == current then
					table.insert(path, 1, k)
					break
				end
			end
			current = current.parent_inst
		end
		return table.concat(path, ".")
	end

	-- Recursively process instructions
	local function process_inst(tbl, inherited_flags)
		inherited_flags = inherited_flags or {}

		-- Collect flags from current level
		local current_flags = {}
		for k, v in pairs(inherited_flags) do
			current_flags[k] = v
		end

		-- Add flags from this table if it has any
		if tbl.hoistable then
			current_flags.hoistable = true
		end
		if tbl.parent then
			current_flags.parent = true
		end
		if tbl.useOther then
			current_flags.useOther = true
		end
		if tbl.global then
			current_flags.global = true
		end

		-- If we have any children, this will be set to false
		tbl.is_leaf = true

		for _, i in ipairs(tbl) do
			local key, value = next(i)
			tbl.is_leaf = false

			if not value.mnemonic then
				value.mnemonic = key
			end

			-- Add struct_name if missing
			if not value.struct_name then
				value.struct_name = to_pascal_case(key)
			end

			value.parent_inst = tbl

			-- Apply inherited flags
			for flag, flag_value in pairs(current_flags) do
				if value[flag] == nil then
					value[flag] = flag_value
				end
			end

			-- If it's a leaf and doesn't have min_operands and operands, add min_operands = 0
			if is_leaf(value) and value.min_operands == nil and value.operands == nil then
				value.min_operands = 0
			end

			-- Recursively process children
			process_inst(value, current_flags)
		end
	end

	-- Process the entire tree
	process_inst(insts)

	-- Now add stable names after parent_inst is set
	local function add_stable_names(tbl)
		for _, i in ipairs(tbl) do
			local key, value = next(i)

			-- Build the full path for this instruction
			local full_path = build_path(value, key)

			-- Look up stable name
			local stable_id = name_to_stable_name[full_path]
			if stable_id then
				value.stable_name = stable_id
				stable_name_to_inst[stable_id] = value
				if stable_id > max_stable_name then
					max_stable_name = stable_id
				end
			end

			-- Recursively process children
			add_stable_names(value)
		end
	end

	-- Add stable names to all instructions
	add_stable_names(insts)

	-- Helper function to traverse the instruction tree
	local function traverse(callback)
		local function walk_insts(tbl)
			for _, i in ipairs(tbl) do
				local _, value = next(i)
				callback(value)
				-- Recursively process nested instructions
				walk_insts(value)
			end
		end

		-- Start walking from the top-level insts
		walk_insts(insts)
	end
	return {
		insts = insts,
		stable_name_to_inst = stable_name_to_inst,
		max_stable_name = max_stable_name,
		traverse = traverse,
	}
end

return process(insts)
