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
					{
						Array = {
							struct_name = "ArrayType",
							operands = {
								{ "elementType", "IRType" },
								{ "elementCount" },
								{ "stride", optional = true },
							},
						},
					},
					{
						UnsizedArray = {
							struct_name = "UnsizedArrayType",
							operands = { { "elementType", "IRType" }, { "stride", optional = true } },
						},
					},
				},
			},
			{
				Func = {
					struct_name = "FuncType",
					hoistable = true,
					operands = {
						{ "resultType", "IRType" },
						{ "paramTypes", "IRType", variadic = true },
					},
				},
			},
			{ BasicBlock = { struct_name = "BasicBlockType", hoistable = true } },
			{
				Vec = {
					struct_name = "VectorType",
					operands = { { "elementType", "IRType" }, { "elementCount" } },
					hoistable = true,
				},
			},
			{
				Mat = {
					struct_name = "MatrixType",
					operands = { { "elementType", "IRType" }, { "rowCount" }, { "columnCount" }, { "layout" } },
					hoistable = true,
				},
			},
			{ Conjunction = { struct_name = "ConjunctionType", hoistable = true } },
			{
				Attributed = {
					struct_name = "AttributedType",
					operands = { { "baseType", "IRType" }, { "attr" } },
					hoistable = true,
				},
			},
			{
				-- Represents an `Result<T,E>`, used by functions that throws error codes.
				Result = {
					struct_name = "ResultType",
					operands = { { "valueType", "IRType" }, { "errorType", "IRType" } },
					hoistable = true,
				},
			},
			-- Represents an `Optional<T>`.
			{ Optional = { struct_name = "OptionalType", operands = { { "valueType", "IRType" } }, hoistable = true } },
			-- Represents an enum type
			{ Enum = { struct_name = "EnumType", operands = { { "tagType", "IRType" } }, parent = true } },
			{
				DifferentialPairTypeBase = {
					hoistable = true,
					{
						DiffPair = {
							struct_name = "DifferentialPairType",
							operands = { { "valueType", "IRType" }, { "witnessTable" } },
						},
					},
					{
						DiffPairUserCode = {
							struct_name = "DifferentialPairUserCodeType",
							operands = { { "valueType", "IRType" }, { "witnessTable" } },
						},
					},
					{
						DiffRefPair = {
							struct_name = "DifferentialPtrPairType",
							operands = { { "valueType", "IRType" }, { "witnessTable" } },
						},
					},
				},
			},
			{
				BwdDiffIntermediateCtxType = {
					struct_name = "BackwardDiffIntermediateContextType",
					operands = { { "func" } },
					hoistable = true,
				},
			},
			{
				TensorView = {
					struct_name = "TensorViewType",
					operands = { { "elementType", "IRType" } },
					hoistable = true,
				},
			},
			{ TorchTensor = { struct_name = "TorchTensorType", hoistable = true } },
			{
				ArrayListVector = {
					struct_name = "ArrayListType",
					operands = { { "elementType", "IRType" } },
					hoistable = true,
				},
			},
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
							operands = { { "baseType", "IRType" } },
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
			{
				RateQualified = {
					struct_name = "RateQualifiedType",
					operands = { { "rate", "IRRate" }, { "valueType", "IRType" } },
					hoistable = true,
				},
			},
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
					{
						Ptr = {
							struct_name = "PtrType",
							operands = {
								{ "valueType", "IRType" },
								{ "accessQualifierOperand", "IRIntLit", optional = true },
								{ "addressSpaceOperand", "IRIntLit", optional = true },
							},
						},
					},
					{
						RefParam = {
							struct_name = "RefParamType",
							operands = {
								{ "valueType", "IRType" },
								{ "accessQualifierOperand", "IRIntLit", optional = true },
								{ "addressSpaceOperand", "IRIntLit", optional = true },
							},
						},
					},
					{
						BorrowInParam = {
							struct_name = "BorrowInParamType",
							operands = {
								{ "valueType", "IRType" },
								{ "accessQualifierOperand", "IRIntLit", optional = true },
								{ "addressSpaceOperand", "IRIntLit", optional = true },
							},
						},
					},
					{
						PseudoPtr = {
							-- A `PsuedoPtr<T>` logically represents a pointer to a value of type
							-- `T` on a platform that cannot support pointers. The expectation
							-- is that the "pointer" will be legalized away by storing a value
							-- of type `T` somewhere out-of-line.
							struct_name = "PseudoPtrType",
							operands = {
								{ "valueType", "IRType" },
								{ "accessQualifierOperand", "IRIntLit", optional = true },
								{ "addressSpaceOperand", "IRIntLit", optional = true },
							},
						},
					},
					{
						OutParamTypeBase = {
							{ OutParam = { struct_name = "OutParamType", operands = { { "valueType", "IRType" } } } },
							{
								BorrowInOutParam = {
									struct_name = "BorrowInOutParamType",
									operands = { { "valueType", "IRType" } },
								},
							},
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
			{ CLayout = { struct_name = "CBufferLayoutType", hoistable = true } },
			{
				SubpassInputType = {
					operands = { { "elementType", "IRType" }, { "isMultisampleInst" } },
					hoistable = true,
				},
			},
			{ TextureFootprintType = { operands = { { "elementType" } }, hoistable = true } },
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
										TextureType = {
											operands = {
												{ "elementType", "IRType" },
												{ "shape", "IRInst" },
												{ "isArray", "IRInst" },
												{ "isMS", "IRInst" },
												{ "sampleCount", "IRInst" },
												{ "accessOperand", "IRInst" },
												{ "isShadow", "IRInst" },
												{ "isCombined", "IRInst" },
												{ "format", "IRInst" },
											},
											hoistable = true,
										},
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
					{
						InputPatch = {
							struct_name = "HLSLInputPatchType",
							operands = { { "elementType", "IRType" }, { "elementCount" } },
						},
					},
					{
						OutputPatch = {
							struct_name = "HLSLOutputPatchType",
							operands = { { "elementType", "IRType" }, { "elementCount" } },
						},
					},
				},
			},
			{ GLSLInputAttachment = { struct_name = "GLSLInputAttachmentType", hoistable = true } },
			{
				BuiltinGenericType = {
					hoistable = true,
					{
						HLSLStreamOutputType = {
							{
								PointStream = {
									struct_name = "HLSLPointStreamType",
									operands = { { "elementType", "IRType" } },
								},
							},
							{
								LineStream = {
									struct_name = "HLSLLineStreamType",
									operands = { { "elementType", "IRType" } },
								},
							},
							{
								TriangleStream = {
									struct_name = "HLSLTriangleStreamType",
									operands = { { "elementType", "IRType" } },
								},
							},
						},
					},
					{
						MeshOutputType = {
							{
								Vertices = {
									struct_name = "VerticesType",
									operands = { { "elementType", "IRType" }, { "maxVertices" } },
								},
							},
							{
								Indices = {
									struct_name = "IndicesType",
									operands = { { "elementType", "IRType" }, { "maxIndices" } },
								},
							},
							{
								Primitives = {
									struct_name = "PrimitivesType",
									operands = { { "elementType", "IRType" }, { "maxPrimitives" } },
								},
							},
						},
					},
					{
						["metal::mesh"] = {
							struct_name = "MetalMeshType",
							operands = {
								{ "verticesType", "IRType" },
								{ "primitivesType", "IRType" },
								{ "numVertices" },
								{ "numPrimitives" },
								{ "topology", "IRIntLit" },
							},
						},
					},
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
													operands = {
														{ "elementType", "IRType" },
														{ "layoutType", "IRType" },
													},
												},
											},
											{
												TextureBuffer = {
													struct_name = "TextureBufferType",
													operands = { { "elementType" } },
												},
											},
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
											operands = {
												{ "valueType", "IRType" },
												{ "dataLayout", "IRType", optional = true },
											},
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
			{ CoopVectorType = { operands = { { "elementType", "IRType" }, { "elementCount" } }, hoistable = true } },
			{
				CoopMatrixType = {
					operands = {
						{ "elementType", "IRType" },
						{ "scope" },
						{ "rowCount" },
						{ "columnCount" },
						{ "matrixUse" },
					},
					hoistable = true,
				},
			},
			{
				TensorAddressingTensorLayoutType = { operands = { { "dimension" }, { "clampMode" } }, hoistable = true },
			},
			{
				TensorAddressingTensorViewType = {
					operands = { { "dimension" }, { "hasDimension" } },
					min_operands = 2,
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
			{
				associated_type = {
					struct_name = "AssociatedType",
					operands = { "constraintTypes", "IRInterfaceType", variadic = true },
					hoistable = true,
				},
			},
			{ this_type = { operands = { { "interfaceType", "IRType" } }, hoistable = true } },
			-- Represents the IR type for an `IRRTTIObject`.
			{ rtti_type = { struct_name = "RTTIType", hoistable = true } },
			-- Represents a handle to an RTTI object.
			-- This is lowered as an integer number identifying a type.
			{
				rtti_handle_type = {
					struct_name = "RTTIHandleType",
					hoistable = true,
				},
			},
			{
				TupleTypeBase = {
					hoistable = true,
					--  Represents a tuple. Tuples are created by `IRMakeTuple` and its elements
					--  are accessed via `GetTupleElement(tupleValue, IRIntLit)`.
					{ tuple_type = { operands = { "types", "IRType", variadic = true } } },
					-- Represents a type pack. Type packs behave like tuples, but they have a
					-- "flattening" semantics, so that MakeTypePack(MakeTypePack(T1,T2), T3) is
					-- MakeTypePack(T1,T2,T3).
					{ TypePack = { operands = { "types", "IRType", variadic = true } } },
				},
			},
			-- Represents a tuple in target language. TargetTupleType will not be lowered to structs.
			{
				TargetTuple = {
					struct_name = "TargetTupleType",
					operands = { "types", "IRType", variadic = true },
					hoistable = true,
				},
			},
			{ ExpandTypeOrVal = { operands = { { "type" } }, hoistable = true } },
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
							operands = { { "baseType", "IRType" } },
						},
					},
					{
						witness_table_id_t = {
							-- An integer type representing a witness table for targets where
							-- witness tables are represented as integer IDs. This type is used
							-- during the lower-generics pass while generating dynamic dispatch
							-- code and will eventually lower into an uint type.
							struct_name = "WitnessTableIDType",
							operands = { { "baseType", "IRType" } },
						},
					},
				},
			},
			{ UntaggedUnionType = {
				hoistable = true,
				-- A type that represents that the value's _type_ is one of types in the set operand.
			} },
			{ ElementOfSetType = {
				hoistable = true,
				-- A type that represents that the value must be an element of the set operand.
			} },
			{ SetTagType = {
				hoistable = true,
				-- Represents a tag-type for a set.
				--
				-- An inst whose type is SetTagType(set) is semantically carrying a 
				-- run-time value that "picks" one of the elements of the set operand.
				--
				-- Only operand is a SetBase
			} }, 
			{ TaggedUnionType = {
				hoistable = true,
				-- Represents a tagged union type.
				--
				-- An inst whose type is a TaggedUnionType(typeSet, witnessTableSet) is semantically carrying a tuple of
				-- two values: a value of SetTagType(witnessTableSet) to represent the tag, and a payload value of type
				-- UntaggedUnionType(typeSet), which conceptually represents a union/"anyvalue" type.
				--
				-- This is most commonly used to specialize the type of existential insts once the possibilities can be statically determined.
				-- 
				-- Operands are a TypeSet and a WitnessTableSet that represent the possibilities of the existential
			} }
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

					-- A generic is akin to a function, but is conceptually executed
					-- before runtime, to specialize the code nested within--.

					-- In practice, a generic always holds only a single block, and ends
					-- with a `return` instruction for the value that the generic yields.
					{ generic = {} },
				},
			},
			{ global_var = { global = true } },
		},
	},
	{ global_param = { global = true } },
	{ globalConstant = { global = true } },
	-- A structure type is represented as a parent instruction,
	-- where the child instructions represent the fields of the
	-- struct.
	--
	-- The space of fields that a given struct type supports
	-- are defined as its "keys", which are global values
	-- (that is, they have mangled names that can be used
	-- for linkage).
	--
	{ key = { struct_name = "StructKey", global = true } },
	{ global_generic_param = { global = true } },
	{ witness_table = { hoistable = true } },
	{ indexedFieldKey = { operands = { { "baseType" }, { "index" } }, hoistable = true } },
	-- A placeholder witness that ThisType implements the enclosing interface.
	-- Used only in interface definitions.
	{ thisTypeWitness = { operands = { { "type" } } } },
	-- A placeholder witness for the fact that two types are equal.
	{ TypeEqualityWitness = { operands = { { "subType" }, { "superType" } }, hoistable = true } },
	{ global_hashed_string_literals = {} },
	{
		module = { struct_name = "ModuleInst", parent = true },
	},
	{ block = { parent = true } },

	-- A global inst representing an alias of another symbol, under a different mangled name.
	-- This inst should be completely eliminated after linking, with its references replaced
	-- to use the canonical symbol being aliased.
	{ SymbolAlias = { operands = { { "symbol" } } } },

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

	-- Instructions that represent something with an undefined value.
	{ Undefined = {

		-- A load from a memory location that is known to be uninitialized.
		--
		-- Primarily used so that the compiler front-end can diagnose an error on such cases.
		--
		-- A given `LoadFromUninitializedMemory` might evaluate to an arbitrary value of its type,
		-- and an optimization pass may freely decide on a particular value to use and replace
		-- all uses of the instruction with that value.
		--
		-- If there are multiple distinct `LoadFromUninitializedMemory` instructions, then they
		-- might each yield a different value, even if they all reference the same memory
		-- location.
		--
		-- Akin to `freeze(undefined)` in LLVM.
		--
		{ LoadFromUninitializedMemory = {} },

		-- An undefined value that is infectious.
		--
		-- Semantically, a poison value of some type T can be thought of as a
		-- hypothetical out-of-band instance of type T, akin to a T-specific NaN
		-- value (although a poison `float` is distinct from a `float` NaN value...).
		-- The motivation for this interpretation is that it allows most optimizations
		-- to ignore the possibility of poison/undefined values, while still being
		-- semantically correct.
		--
		-- In most cases, an instruction that is executed with a poison value as one
		-- of its operands yields a poison value as its result. The main exception
		-- is instructions that only conditionally use an operand, such as `select`,
		-- and block/function parameters (just because one branch passes a poison
		-- argument for a parmeter, that doesn't mean the parameter would be poison
		-- every time the block executes).
		--
		-- Corresponds to the LLVM `poison` instruction.
		--
		{ Poison = {} },
	}},

	-- A `defaultConstruct` operation creates an initialized
	-- value of the result type, and can only be used for types
	-- where default construction is a meaningful thing to do.
	{ defaultConstruct = {} },
	{
		MakeDifferentialPairBase = {
			{
				MakeDiffPair = { struct_name = "MakeDifferentialPair", operands = { { "primal" }, { "differential" } } },
			},
			{
				MakeDiffPairUserCode = {
					struct_name = "MakeDifferentialPairUserCode",
					operands = { { "primal" }, { "differential" } },
				},
			},
			{
				MakeDiffRefPair = {
					struct_name = "MakeDifferentialPtrPair",
					operands = { { "primal" }, { "differential" } },
				},
			},
		},
	},
	{
		DifferentialPairGetDifferentialBase = {
			{ GetDifferential = { struct_name = "DifferentialPairGetDifferential", operands = { { "pair" } } } },
			{
				GetDifferentialUserCode = {
					struct_name = "DifferentialPairGetDifferentialUserCode",
					operands = { { "pair" } },
				},
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
				GetPrimal = { struct_name = "DifferentialPairGetPrimal", operands = { { "pair" } } },
			},
			{
				GetPrimalUserCode = {
					struct_name = "DifferentialPairGetPrimalUserCode",
					min_operands = 1,
				},
			},
			{ GetPrimalRef = { struct_name = "DifferentialPtrPairGetPrimal", operands = { { "ptrPair" } } } },
		},
	},
	{ specialize = { operands = { { "base" }, { "arg" } }, hoistable = true } },
	{ lookupWitness = { struct_name = "LookupWitnessMethod", min_operands = 2, hoistable = true } },
	{ GetSequentialID = { operands = { { "RTTIOperand" } }, hoistable = true } },
	{
		bind_global_generic_param = {
			operands = { { "param", "IRGlobalGenericParam" }, { "val", "IRInst" } },
		},
	},
	{ allocObj = {} },
	{ globalValueRef = { operands = { { "value" } } } },
	{ makeUInt64 = { operands = { { "low" }, { "high" } } } },
	{ makeVector = {} },
	{ makeMatrix = {} },
	{
		makeMatrixFromScalar = {
			operands = { { "scalarVal" } },
		},
	},
	{ matrixReshape = { operands = { { "matrix" } } } },
	{
		vectorReshape = {
			operands = { { "vector" } },
		},
	},
	{ makeArray = {} },
	{ makeArrayFromElement = { operands = { { "element" } } } },
	{ makeCoopVector = {} },
	{ makeCoopVectorFromValuePack = { operands = { { "valuePack" } } } },
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
	{ LoadResourceDescriptorFromHeap = { operands = { { "index" } } } },
	{
		LoadSamplerDescriptorFromHeap = {
			operands = { { "index" } },
		},
	},
	{ MakeCombinedTextureSamplerFromHandle = { operands = { { "handle" } } } },
	{
		MakeWitnessPack = {
			hoistable = true,
		},
	},
	{ Expand = { operands = { { "value" } } } },
	{
		Each = {
			operands = { { "value" } },
			hoistable = true,
		},
	},
	{ makeResultValue = { operands = { { "value" } } } },
	{ makeResultError = { operands = { { "errorValue" } } } },
	{ isResultError = { operands = { { "resultOperand" } } } },
	{ getResultError = { operands = { { "resultOperand" } } } },
	{ getResultValue = { operands = { { "resultOperand" } } } },
	{ getOptionalValue = { operands = { { "optionalOperand" } } } },
	{ optionalHasValue = { operands = { { "optionalOperand" } } } },
	{ makeOptionalValue = { operands = { { "value" } } } },
	{ makeOptionalNone = { operands = { { "defaultValue" } } } },
	{ CombinedTextureSamplerGetTexture = { operands = { { "sampler" } } } },
	{ CombinedTextureSamplerGetSampler = { operands = { { "sampler" } } } },
	{ call = { operands = { { "callee" } } } },
	{ rtti_object = { struct_name = "RTTIObject" } },
	{ alloca = { operands = { { "allocSize" } } } },
	{ updateElement = { operands = { { "oldValue" }, { "elementValue" } } } },
	{ detachDerivative = { operands = { { "value" } } } },
	{ bitfieldExtract = { operands = { { "value" }, { "offset" }, { "count" } } } },
	{ bitfieldInsert = { operands = { { "base" }, { "insert" }, { "offset" }, { "count" } } } },
	{ packAnyValue = { operands = { { "value" } } } },
	{ unpackAnyValue = { operands = { { "value" } } } },
	{ witness_table_entry = { operands = { { "requirementKey" }, { "satisfyingVal" } } } },
	{
		interface_req_entry = {
			struct_name = "InterfaceRequirementEntry",
			operands = { { "requirementKey" }, { "requirementVal" } },
			global = true,
		},
	},
	-- An inst to represent the workgroup size of the calling entry point.
	-- We will materialize this inst during `translateGlobalVaryingVar`.
	{ GetWorkGroupSize = { hoistable = true } },
	-- An inst that returns the current stage of the calling entry point.
	{ GetCurrentStage = {} },
	{ param = {} },
	{ field = { struct_name = "StructField", min_operands = 2 } },
	{ var = {} },
	{ load = { min_operands = 1 } },
	{
		StoreBase =
		{
			operands = {{"ptr"}, {"val"}},
			{ store = {} },
			{ copyLogical = {} },
		},
	},
	{ CUDA_LDG = {min_operands = 1 } },

	-- Atomic Operations
	{
		AtomicOperation = {
			{ atomicLoad = { min_operands = 1 } },
			{
				atomicStore = { min_operands = 2 },
			},
			{ atomicExchange = { min_operands = 2 } },
			{
				atomicCompareExchange = { operands = { { "ptr" }, { "expected" }, { "desired" } } },
			},
			{ atomicAdd = { operands = { { "ptr" }, { "val" } } } },
			{
				atomicSub = { operands = { { "ptr" }, { "val" } } },
			},
			{ atomicAnd = { operands = { { "ptr" }, { "val" } } } },
			{
				atomicOr = { operands = { { "ptr" }, { "val" } } },
			},
			{ atomicXor = { operands = { { "ptr" }, { "val" } } } },
			{
				atomicMin = { operands = { { "ptr" }, { "val" } } },
			},
			{ atomicMax = { operands = { { "ptr" }, { "val" } } } },
			{
				atomicInc = { operands = { { "ptr" } } },
			},
			{ atomicDec = { operands = { { "ptr" } } } },
		},
	},
	-- Produced and removed during backward auto-diff pass as a temporary placeholder representing the
	-- currently accumulated derivative to pass to some dOut argument in a nested call.
	{ LoadReverseGradient = { operands = { { "value" } } } },
	-- Produced and removed during backward auto-diff pass as a temporary placeholder containing the
	-- primal and accumulated derivative values to pass to an inout argument in a nested call.
	{ ReverseGradientDiffPairRef = { operands = { { "primal" }, { "diff" } } } },
	-- Produced and removed during backward auto-diff pass. This inst is generated by the splitting step
	-- to represent a reference to an inout parameter for use in the primal part of the computation.
	{ PrimalParamRef = { operands = { { "referencedParam" } } } },
	-- Produced and removed during backward auto-diff pass. This inst is generated by the splitting step
	-- to represent a reference to an inout parameter for use in the back-prop part of the computation.
	{ DiffParamRef = { operands = { { "referencedParam" } } } },
	-- Check that the value is a differential null value.
	{ IsDifferentialNull = { operands = { { "base" } } } },
	{
		get_field = {
			struct_name = "FieldExtract",
			min_operands = 2,
		},
	},
	{ get_field_addr = { struct_name = "FieldAddress", min_operands = 2 } },
	{ getElement = { operands = { { "base" }, { "index" } } } },
	{ getElementPtr = { operands = { { "base" }, { "index" } } } },
	-- Pointer offset: computes pBase + offset_in_elements
	{ getOffsetPtr = { operands = { { "base" }, { "offset" } } } },
	{ getAddr = { struct_name = "GetAddress", operands = { { "ptr" } } } },
	{ castDynamicResource = { operands = { { "resource" } } } },
	-- Get an unowned NativeString from a String.
	{ getNativeStr = { operands = { { "stringValue" } } } },
	-- Make String from a NativeString.
	{ makeString = { operands = { { "nativeStringValue" } } } },
	-- Get a native ptr from a ComPtr or RefPtr
	{ getNativePtr = { operands = { { "elementType" } } } },
	-- Get a write reference to a managed ptr var (operand must be Ptr<ComPtr<T>> or Ptr<RefPtr<T>>).
	{ getManagedPtrWriteRef = { operands = { { "ptrToManagedPtr" } } } },
	-- Attach a managedPtr var to a NativePtr without changing its ref count.
	{ ManagedPtrAttach = { operands = { { "ptrValue" } } } },
	-- Attach a managedPtr var to a NativePtr without changing its ref count.
	{ ManagedPtrDetach = { operands = { { "ptrValue" } } } },
	-- "Subscript" an image at a pixel coordinate to get pointer
	{ imageSubscript = { operands = { { "image" }, { "coord" }, { "sampleCoord", optional = true } } } },
	-- Load from an Image.
	{
		imageLoad = {
			operands = { { "image" }, { "coord" }, { "auxCoord1", optional = true }, { "auxCoord2", optional = true } },
		},
	},
	-- Store into an Image.
	{ imageStore = { operands = { { "image" }, { "coord" }, { "value" } } } },
	-- Load (almost) arbitrary-type data from a byte-address buffer
	-- %dst = byteAddressBufferLoad(%buffer, %offset, %alignment)
	-- where
	-- - `buffer` is a value of some `ByteAddressBufferTypeBase` type
	-- - `offset` is an `int`
	-- - `alignment` is an `int`
	-- - `dst` is a value of some type containing only ordinary data
	{ byteAddressBufferLoad = { operands = { { "buffer" }, { "offset" }, { "alignment" } } } },
	-- Store (almost) arbitrary-type data to a byte-address buffer
	-- byteAddressBufferLoad(%buffer, %offset, %alignment, %src)
	-- where
	-- - `buffer` is a value of some `ByteAddressBufferTypeBase` type
	-- - `offset` is an `int`
	-- - `alignment` is an `int`
	-- - `src` is a value of some type containing only ordinary data
	{ byteAddressBufferStore = { operands = { { "buffer" }, { "offset" }, { "value" }, { "alignment" } } } },
	-- Load data from a structured buffer
	-- %dst = structuredBufferLoad(%buffer, %index)
	-- where
	-- - `buffer` is a value of some `StructuredBufferTypeBase` type with element type T
	-- - `offset` is an `int`
	-- - `dst` is a value of type T
	{ structuredBufferLoad = { min_operands = 2 } },
	{ structuredBufferLoadStatus = { operands = { { "buffer" }, { "index" }, { "status" } } } },
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
	{
		rwstructuredBufferStore = {
			struct_name = "RWStructuredBufferStore",
			operands = { { "structuredBuffer" }, { "index" }, { "val" } },
		},
	},
	{
		rwstructuredBufferGetElementPtr = {
			struct_name = "RWStructuredBufferGetElementPtr",
			operands = { { "base" }, { "index" } },
		},
	},
	-- Append/Consume-StructuredBuffer operations
	{ StructuredBufferAppend = { operands = { { "buffer" }, { "element", optional = true } } } },
	{ StructuredBufferConsume = { operands = { { "buffer" } } } },
	{ StructuredBufferGetDimensions = { operands = { { "buffer" } } } },
	-- Resource qualifiers for dynamically varying index
	{ nonUniformResourceIndex = { operands = { { "index" } } } },
	{ getNaturalStride = { operands = { { "type" } } } },
	{ meshOutputRef = { operands = { { "base" }, { "index" } } } },
	{ meshOutputSet = { operands = { { "base" }, { "index" }, { "elementValue" } } } },
	-- only two parameters as they are effectively static
	-- TODO: make them reference the _slang_mesh object directly
	{ metalSetVertex = { operands = { { "index" }, { "elementValue" } } } },
	{ metalSetPrimitive = { operands = { { "index" }, { "elementValue" } } } },
	{ metalSetIndices = { operands = { { "index" }, { "elementValue" } } } },
	{ MetalCastToDepthTexture = { operands = { { "texture" } } } },
	-- Construct a vector from a scalar
	-- %dst = MakeVectorFromScalar %T %N %val
	-- where
	-- - `T` is a `Type`
	-- - `N` is a (compile-time) `Int`
	-- - `val` is a `T`
	-- - dst is a `Vec<T,N>`
	{ MakeVectorFromScalar = { operands = { { "elementType" }, { "elementCount" }, { "scalarValue" } } } },
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
	{ swizzledStore = { operands = { { "dest" }, { "source" } }, min_operands = 2 } },
	{
		TerminatorInst = {
			{ return_val = { struct_name = "Return", operands = { { "val" } } } },
			{ yield = { operands = { { "val" } } } },
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
					operands = { { "value" } },
				},
			},
			{
				tryCall = {
					-- tryCall <successBlock> <failBlock> <callee> <args>...
					operands = { { "successBlock", "IRBlock" }, { "failureBlock", "IRBlock" }, { "callee" } },
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
				UnreachableBase = {
					{
						missingReturn = {},
					},
					{ unreachable = {} },
				},
			},
			{
				defer = {
					operands = { { "deferBlock", "IRBlock" }, { "mergeBlock", "IRBlock" }, { "scopeBlock", "IRBlock" } },
				},
			},
		},
	},
	{ discard = {} },
	{
		RequirePrelude = { min_operands = 1 },
	},
	{ RequireTargetExtension = { operands = { { "extension" } } } },
	{ RequireComputeDerivative = {} },
	{ StaticAssert = { operands = { { "condition" }, { "message" } } } },
	{ Printf = { operands = { { "format" } } } },
	-- Quad control execution modes.
	{ RequireMaximallyReconverges = {} },
	{ RequireQuadDerivatives = {} },
	-- TODO: We should consider splitting the basic arithmetic/comparison
	-- ops into cases for signed integers, unsigned integers, and floating-point
	-- values, to better match downstream targets that want to treat them
	-- all differently ().
	{ add = { operands = { { "left" }, { "right" } } } },
	{ sub = { operands = { { "left" }, { "right" } } } },
	{ mul = { operands = { { "left" }, { "right" } } } },
	{ div = { operands = { { "left" }, { "right" } } } },
	-- Remainder of division.
	-- Note: this is distinct from modulus, and we should have a separate
	-- opcode for `mod` if we ever need to support it.
	{ irem = { struct_name = "IRem", operands = { { "left" }, { "right" } } } },
	{
		frem = {
			struct_name = "FRem",
			operands = { { "left" }, { "right" } },
		},
	},
	{
		shl = { struct_name = "Lsh", operands = { { "value" }, { "amount" } } },
	},
	{ shr = { struct_name = "Rsh", operands = { { "value" }, { "amount" } } } },
	{ cmpEQ = { struct_name = "Eql", operands = { { "left" }, { "right" } } } },
	{
		cmpNE = {
			struct_name = "Neq",
			operands = { { "left" }, { "right" } },
		},
	},
	{
		cmpGT = { struct_name = "Greater", operands = { { "left" }, { "right" } } },
	},
	{ cmpLT = { struct_name = "Less", operands = { { "left" }, { "right" } } } },
	{ cmpGE = { struct_name = "Geq", operands = { { "left" }, { "right" } } } },
	{
		cmpLE = {
			struct_name = "Leq",
			operands = { { "left" }, { "right" } },
		},
	},
	{
		["and"] = { struct_name = "BitAnd", operands = { { "left" }, { "right" } } },
	},
	{ xor = { struct_name = "BitXor", operands = { { "left" }, { "right" } } } },
	{ ["or"] = { struct_name = "BitOr", operands = { { "left" }, { "right" } } } },
	{
		logicalAnd = {
			struct_name = "And",
			operands = { { "left" }, { "right" } },
		},
	},
	{
		logicalOr = { struct_name = "Or", operands = { { "left" }, { "right" } } },
	},
	{ neg = { operands = { { "value" } } } },
	{
		["not"] = { operands = { { "value" } } },
	},
	{ bitnot = { struct_name = "BitNot", operands = { { "value" } } } },
	{ select = { operands = { { "condition" }, { "trueResult" }, { "falseResult" } } } },
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
	{ waveMaskBallot = { operands = { { "mask" }, { "condition" } } } },
	-- matchMask = waveMaskBallot(mask, value)
	{ waveMaskMatch = { operands = { { "mask" }, { "value" } } } },
	-- Texture sampling operation of the form `t.Sample(s,u)`
	{ sample = { operands = { { "texture" }, { "sampler" }, { "coord" } } } },
	{ sampleGrad = { operands = { { "texture" }, { "sampler" }, { "coord" }, { "gradX" } } } },
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
				highLevelDecl = {
					struct_name = "HighLevelDeclDecoration",
					operands = { { "declOperand", "IRPtrLit" } },
				},
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
			{ loopControl = { struct_name = "LoopControlDecoration", operands = { { "modeOperand", "IRConstant" } } } },
			{ loopMaxIters = { struct_name = "LoopMaxItersDecoration", min_operands = 1 } },
			{
				loopExitPrimalValue = {
					struct_name = "LoopExitPrimalValueDecoration",
					operands = { { "targetInst" }, { "loopExitValInst" } },
				},
			},
			{
				intrinsicOp = {
					struct_name = "IntrinsicOpDecoration",
					operands = { { "intrinsicOpOperand", "IRIntLit" } },
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
									operands = { { "target" }, { "definitionOperand", "IRStringLit" } },
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
					operands = { { "outerArrayNameOperand", "IRStringLit" } },
				},
			},
			{
				TargetSystemValue = {
					struct_name = "TargetSystemValueDecoration",
					operands = { { "semanticOperand", "IRStringLit" }, { "index", "IRIntLit" } },
				},
			},
			{
				interpolationMode = {
					struct_name = "InterpolationModeDecoration",
					operands = { { "modeOperand", "IRConstant" } },
				},
			},
			{
				nameHint = { struct_name = "NameHintDecoration", operands = { { "nameOperand", "IRStringLit" } } },
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
					operands = { { "alignment" } },
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
					operands = { { "witness" } },
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
			{
				requireSPIRVVersion = {
					struct_name = "RequireSPIRVVersionDecoration",
					operands = { { "SPIRVVersionOperand", "IRConstant" } },
				},
			},
			{
				requireGLSLVersion = {
					struct_name = "RequireGLSLVersionDecoration",
					operands = { { "languageVersionOperand", "IRConstant" } },
				},
			},
			{
				requireGLSLExtension = {
					struct_name = "RequireGLSLExtensionDecoration",
					operands = { { "extensionNameOperand", "IRStringLit" } },
				},
			},
			{
				requireWGSLExtension = {
					struct_name = "RequireWGSLExtensionDecoration",
					operands = { { "extensionNameOperand", "IRStringLit" } },
				},
			},
			{
				requireCUDASMVersion = {
					struct_name = "RequireCUDASMVersionDecoration",
					operands = { { "CUDASMVersionOperand", "IRConstant" } },
				},
			},
			{
				requireCapabilityAtom = {
					struct_name = "RequireCapabilityAtomDecoration",
					operands = { { "capabilityAtomOperand", "IRConstant" } },
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
			{
				patchConstantFunc = { struct_name = "PatchConstantFuncDecoration", operands = { { "func", "IRInst" } } },
			},
			{
				maxTessFactor = {
					struct_name = "MaxTessFactorDecoration",
					operands = { { "maxTessFactor", "IRFloatLit" } },
				},
			},
			{
				outputControlPoints = {
					struct_name = "OutputControlPointsDecoration",
					operands = { { "controlPointCount", "IRIntLit" } },
				},
			},
			{
				outputTopology = {
					struct_name = "OutputTopologyDecoration",
					operands = { { "topology", "IRStringLit" }, { "topologyTypeOperand", "IRIntLit" } },
				},
			},
			{
				partitioning = {
					struct_name = "PartitioningDecoration",
					operands = { { "partitioning", "IRStringLit" } },
				},
			},
			{
				domain = {
					struct_name = "DomainDecoration",
					operands = { { "domain", "IRStringLit" } },
				},
			},
			{
				maxVertexCount = { struct_name = "MaxVertexCountDecoration", operands = { { "count", "IRIntLit" } } },
			},
			{ instance = { struct_name = "InstanceDecoration", operands = { { "count", "IRIntLit" } } } },
			{ numThreads = { struct_name = "NumThreadsDecoration", min_operands = 3 } },
			{
				fpDenormalPreserve = {
					struct_name = "FpDenormalPreserveDecoration",
					operands = { { "width", "IRIntLit" } },
				},
			},
			{
				fpDenormalFlushToZero = {
					struct_name = "FpDenormalFlushToZeroDecoration",
					operands = { { "width", "IRIntLit" } },
				},
			},
			{
				waveSize = {
					struct_name = "WaveSizeDecoration",
					operands = { { "numLanes", "IRIntLit" } },
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
			{
				streamOutputTypeDecoration = {
					struct_name = "StreamOutputTypeDecoration",
					operands = { { "streamType", "IRHLSLStreamOutputType" } },
				},
			},
			{
				entryPoint = {
					-- An `[entryPoint]` decoration marks a function that represents a shader entry point
					struct_name = "EntryPointDecoration",
					operands = {
						{ "profileInst", "IRIntLit" },
						{ "name", "IRStringLit" },
						{ "moduleName", "IRStringLit", optional = true },
					},
				},
			},
			{ CudaKernel = { struct_name = "CudaKernelDecoration" } },
			{ CudaHost = { struct_name = "CudaHostDecoration" } },
			{
				TorchEntryPoint = {
					struct_name = "TorchEntryPointDecoration",
					operands = { { "functionNameOperand", "IRStringLit" } },
				},
			},
			{
				AutoPyBindCUDA = {
					struct_name = "AutoPyBindCudaDecoration",
					operands = { { "functionNameOperand", "IRStringLit" } },
				},
			},
			{
				CudaKernelFwdDiffRef = {
					struct_name = "CudaKernelForwardDerivativeDecoration",
					operands = { { "forwardDerivativeFunc", optional = true } },
				},
			},
			{
				CudaKernelBwdDiffRef = {
					struct_name = "CudaKernelBackwardDerivativeDecoration",
					operands = { { "backwardDerivativeFunc", optional = true } },
				},
			},
			{ PyBindExportFuncInfo = { struct_name = "AutoPyBindExportInfoDecoration" } },
			{
				PyExportDecoration = {
					struct_name = "PyExportDecoration",
					operands = { { "exportNameOperand", "IRStringLit" } },
				},
			},
			{
				entryPointParam = {
					-- Used to mark parameters that are moved from entry point parameters to global params as coming from the entry
					-- point.
					struct_name = "EntryPointParamDecoration",
					operands = { { "entryPoint", "IRFunc" } },
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
					operands = { { "formatOperand", "IRConstant" } },
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
					operands = {
						{ "layoutNameOperand" },
						{ "sizeOperand", "IRIntLit" },
						{ "alignmentOperand", "IRIntLit" },
					},
				},
			},
			{
				Offset = {
					-- A `[Offset(l, o)]` decoration is attached to a field to indicate that it has offset `o` in the parent type
					-- under layout rules `l`.
					struct_name = "OffsetDecoration",
					operands = { { "layoutNameOperand" }, { "offsetOperand", "IRIntLit" } },
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
					operands = { { "builtinVarOperand", "IRIntLit" } },
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
					operands = { { "nameOperand", "IRStringLit" } },
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
					operands = { { "libraryNameOperand", "IRStringLit" }, { "functionNameOperand", "IRStringLit" } },
				},
			},
			{
				dllExport = {
					-- An dllExport decoration marks a function as an export symbol. Slang will generate a native wrapper function
					-- that is exported to DLL.
					struct_name = "DllExportDecoration",
					operands = { { "functionNameOperand", "IRStringLit" } },
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
					operands = { { "nameOperand", "IRIntLit" } },
				},
			},
			{
				RTTI_typeSize = {
					-- Decorations for RTTI objects
					struct_name = "RTTITypeSizeDecoration",
					operands = { { "typeSizeOperand", "IRIntLit" } },
				},
			},
			{
				AnyValueSize = { struct_name = "AnyValueSizeDecoration", operands = { { "sizeOperand", "IRIntLit" } } },
			},
			{ SpecializeDecoration = {} },
			{ SequentialIDDecoration = { operands = { { "sequentialIdOperand", "IRIntLit" } } } },
			{ DynamicDispatchWitnessDecoration = {} },
			{ StaticRequirementDecoration = {} },
			{ DispatchFuncDecoration = { operands = { { "func" } } } },
			{
				TypeConstraintDecoration = {
					-- A decoration on `IRParam`s that represent generic parameters,
					-- marking the interface type that the generic parameter conforms to.
					-- A generic parameter can have more than one `IRTypeConstraintDecoration`s
					operands = { { "constraintType" } },
				},
			},
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
					operands = { { "nameOperand", "IRStringLit" } },
				},
			},
			{
				nvapiSlot = {
					-- A decoration that applies to an entire IR module, and indicates the register/space binding
					-- that the NVAPI shader parameter intends to use.
					struct_name = "NVAPISlotDecoration",
					operands = { { "registerNameOperand", "IRStringLit" }, { "spaceNameOperand", "IRStringLit" } },
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
			-- Marks a var as a temporary local variable to replace references to a `in` parameter from the function body
			-- This is to support legacy code that modifies an `in` parameter as if it is copied to a local variable.
			{ InParamProxyVar = { struct_name = "InParamProxyVarDecoration", min_operands = 1 } },
			{ TempCallArgImmutableVar = { struct_name = "TempCallArgImmutableVarDecoration" } },
			{ TempCallArgVar = { struct_name = "TempCallArgVarDecoration" } },
			{
				nonCopyable = {
					-- Marks a type to be non copyable, causing SSA pass to skip turning variables of the the type into SSA values.
					struct_name = "NonCopyableTypeDecoration",
				},
			},
			{ DisableCopyEliminationDecoration = {} },
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
			{ glslLocation = { struct_name = "GLSLLocationDecoration", operands = { { "location", "IRIntLit" } } } },
			{ glslOffset = { struct_name = "GLSLOffsetDecoration", operands = { { "offset", "IRIntLit" } } } },
			{ vkStructOffset = { struct_name = "VkStructOffsetDecoration", operands = { { "offset", "IRIntLit" } } } },
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
				semantic = {
					struct_name = "SemanticDecoration",
					operands = { { "semanticNameOperand", "IRStringLit" }, { "semanticIndexOperand", "IRIntLit" } },
				},
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
					operands = { { "registerOffset", "IRIntLit" }, { "componentOffset", "IRIntLit" } },
				},
			},
			{ SpecializationConstantDecoration = { min_operands = 1 } },
			{
				UserTypeName = {
					-- Reflection metadata for a shader parameter that provides the original type name.
					struct_name = "UserTypeNameDecoration",
					operands = { { "userTypeName", "IRStringLit" } },
				},
			},
			{
				CounterBuffer = {
					-- Reflection metadata for a shader parameter that refers to the associated counter buffer of a UAV.
					struct_name = "CounterBufferDecoration",
					operands = { { "counterBuffer" } },
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
					operands = { { "originalValue" } },
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
					operands = { { "forwardDerivativeFunc" } },
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
					operands = { { "primalSubstituteFunc" } },
				},
			},
			{
				backwardDiffPrimalReference = {
					-- Decorations to associate an original function with compiler generated backward derivative functions.
					struct_name = "BackwardDerivativePrimalDecoration",
					operands = { { "backwardDerivativePrimalFunc" } },
				},
			},
			{
				backwardDiffPropagateReference = {
					struct_name = "BackwardDerivativePropagateDecoration",
					operands = { { "backwardDerivativePropagateFunc" } },
				},
			},
			{
				backwardDiffIntermediateTypeReference = {
					struct_name = "BackwardDerivativeIntermediateTypeDecoration",
					operands = { { "backwardDerivativeIntermediateType" } },
				},
			},
			{
				backwardDiffReference = {
					struct_name = "BackwardDerivativeDecoration",
					operands = { { "backwardDerivativeFunc" } },
				},
			},
			{
				userDefinedBackwardDiffReference = {
					struct_name = "UserDefinedBackwardDerivativeDecoration",
					operands = { { "backwardDerivativeFunc" } },
				},
			},
			{ BackwardDerivativePrimalContextDecoration = { operands = { { "backwardDerivativePrimalContextVar" } } } },
			{ BackwardDerivativePrimalReturnDecoration = { operands = { { "backwardDerivativePrimalReturnValue" } } } },
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
							operands = {
								{ "primalType", "IRType" },
								{ "primalInst", optional = true },
								{ "witness", optional = true },
							},
						},
					},
					{
						mixedDiffInstDecoration = {
							-- Used by the auto-diff pass to mark insts that compute
							-- BOTH a differential and a primal value.
							struct_name = "MixedDifferentialInstDecoration",
							operands = { { "pairType", "IRType" } },
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
					operands = { { "structKey", "IRStructKey" } },
				},
			},
			{
				primalElementType = {
					-- Used by the auto-diff pass to mark the primal element type of an
					-- forward-differentiated updateElement inst.
					struct_name = "PrimalElementTypeDecoration",
					operands = { { "primalElementType" } },
				},
			},
			{
				IntermediateContextFieldDifferentialTypeDecoration = {
					-- Used by the auto-diff pass to mark the differential type of an intermediate context field.
					operands = { { "differentialWitness" } },
				},
			},
			{
				derivativeMemberDecoration = {
					-- Used by the auto-diff pass to hold a reference to a
					-- differential member of a type in its associated differential type.
					operands = { { "derivativeMemberStructKey" } },
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
							operands = { { "sourceFunction" } },
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
					operands = { { "witnessTable" } },
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
					operands = { { "source" }, { "line" }, { "col" } },
				},
			},
			{
				DebugFunction = {
					-- Decorates a function with a link to its debug function representation
					struct_name = "DebugFuncDecoration",
					operands = { { "debugFunc" } },
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
					operands = { { "SPIRVNonUniformResourceOperand", "IRConstant" } },
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
			{
				experimentalModule = {
					-- Marks a module as an experimental module
					struct_name = "ExperimentalModuleDecoration"
				},
			},
			{
				DisallowSpecializationWithExistentialsDecoration = { },
			}
		},
	},
	-- Decoration
	-- A `makeExistential(v : C, w) : I` instruction takes a value `v` of type `C`
	-- and produces a value of interface type `I` by using the witness `w` which
	-- shows that `C` conforms to `I`.
	{ makeExistential = { operands = { { "value" }, { "witness" } } } },
	-- A `MakeExistentialWithRTTI(v, w, t)` is the same with `MakeExistential`,
	-- but with the type of `v` being an explict operand.
	{ makeExistentialWithRTTI = { operands = { { "value" }, { "witness" }, { "typeRTTI" } } } },
	-- A 'CreateExistentialObject<I>(typeID, T)` packs user-provided `typeID` and a
	-- value of any type, and constructs an existential value of type `I`.
	{ createExistentialObject = { operands = { { "typeID" }, { "value" } } } },
	-- A `wrapExistential(v, T0,w0, T1,w0) : T` instruction is similar to `makeExistential`.
	-- but applies to a value `v` that is of type `BindExistentials(T, T0,w0, ...)`. The
	-- result of the `wrapExistentials` operation is a value of type `T`, allowing us to
	-- "smuggle" a value of specialized type into computations that expect an unspecialized type.
	{ wrapExistential = { operands = { { "wrappedValue" } } } },
	-- A `GetValueFromBoundInterface` takes a `BindInterface<I, T, w0>` value and returns the
	-- value of concrete type `T` value that is being stored.
	{ getValueFromBoundInterface = { operands = { { "value" } } } },
	{ extractExistentialValue = { operands = { { "existential" } } } },
	{ extractExistentialType = { operands = { { "existential" } }, hoistable = true } },
	{
		extractExistentialWitnessTable = {
			operands = { { "existential" } },
			hoistable = true,
		},
	},
	{ isNullExistential = { operands = { { "val" } } } },
	{ extractTaggedUnionTag = { operands = { { "val" } } } },
	{ extractTaggedUnionPayload = { operands = { { "unionVal" } } } },
	{ BuiltinCast = { operands = { { "val" } } } },
	{ bitCast = { operands = { { "val" } } } },
	{ reinterpret = { operands = { { "val" } } } },
	{ unmodified = { operands = { { "val" } } } },
	{ outImplicitCast = { operands = { { "value" } } } },
	{ inOutImplicitCast = { operands = { { "value" } } } },
	{ intCast = { operands = { { "value" } } } },
	{ floatCast = { operands = { { "value" } } } },
	{ castIntToFloat = { operands = { { "value" } } } },
	{ castFloatToInt = { operands = { { "value" } } } },
	{ CastPtrToBool = { operands = { { "value" } } } },
	{ CastPtrToInt = { operands = { { "value" } } } },
	{ CastIntToPtr = { operands = { { "value" } } } },
	{ castToVoid = { operands = { { "value" } } } },
	{ PtrCast = { operands = { { "value" } } } },
	{ CastEnumToInt = { operands = { { "value" } } } },
	{ CastIntToEnum = { operands = { { "value" } } } },
	{ EnumCast = { operands = { { "value" } } } },
	{ CastUInt2ToDescriptorHandle = { operands = { { "value" } } } },
	{ CastDescriptorHandleToUInt2 = { operands = { { "value" } } } },
	-- Represents a psuedo cast to convert between an original(user declared) type and a storage Type
	-- (valid in buffer locations). The operand can either be a value or an address.
	-- The first operand is a pointer to a storage type, the second operand must be a `MakeStorageTypeLoweringConfig` inst
	-- that defines how the storage type is lowered from the original type.
	{
		CastStorageToLogicalBase = {
			min_operands = 2,
			struct_name = "CastStorageToLogicalBase",
			{ CastStorageToLogical = { min_operands = 2, struct_name = "CastStorageToLogical" } },
			{ CastStorageToLogicalDeref = { min_operands = 2, struct_name = "CastStorageToLogicalDeref" } },
		},
	},
	-- IR encoding of a `TypeLoweringConfig` object that defines how a type is lowered to a storage type.
	-- This is produced/consumed only in the lower-buffer-element-to-storage-type pass.
	{ MakeStorageTypeLoweringConfig = { hoistable = true, operands = { { "addressSpace" }, { "layoutRule" }, { "lowerToPhysicalType" } } } },
	{ CastUInt64ToDescriptorHandle = { operands = { { "value" } } } },
	{ CastDescriptorHandleToUInt64 = { operands = { { "value" } } } },
	-- Represents a no-op cast to convert a resource pointer to a resource on targets where the resource handles are
	-- already concrete types.
	{ CastDescriptorHandleToResource = { operands = { { "handle" } } } },
	{ CastResourceToDescriptorHandle = { operands = { { "resource" } } } },
	{ TreatAsDynamicUniform = { operands = { { "value" } } } },
	{ sizeOf = { operands = { { "type" } } } },
	{ alignOf = { operands = { { "baseOp" } } } },
	{ countOf = { operands = { { "type" } } } },
	{ GetArrayLength = { operands = { { "array" } } } },
	{
		IsType = {
			operands = { { "value" }, { "valueWitness" }, { "typeOperand" }, { "targetWitness", optional = true } },
		},
	},
	{ TypeEquals = { operands = { { "type1" }, { "type2" } } } },
	{ IsInt = { operands = { { "value" } } } },
	{ IsBool = { operands = { { "value" } } } },
	{ IsFloat = { operands = { { "value" } } } },
	{ IsHalf = { operands = { { "value" } } } },
	{ IsUnsignedInt = { operands = { { "value" } } } },
	{ IsSignedInt = { operands = { { "value" } } } },
	{ IsVector = { operands = { { "value" } } } },
	{ GetDynamicResourceHeap = { hoistable = true } },
	{ ForwardDifferentiate = { operands = { { "baseFn" } } } },
	-- Produces the primal computation of backward derivatives, will return an intermediate context for
	-- backward derivative func.
	{ BackwardDifferentiatePrimal = { operands = { { "baseFn" } } } },
	-- Produces the actual backward derivative propagate function, using the intermediate context returned by the
	-- primal func produced from `BackwardDifferentiatePrimal`.
	{ BackwardDifferentiatePropagate = { operands = { { "baseFn" } } } },
	-- Represents the conceptual backward derivative function. Only produced by lower-to-ir and will be
	-- replaced with `BackwardDifferentiatePrimal` and `BackwardDifferentiatePropagate`.
	{ BackwardDifferentiate = { operands = { { "baseFn" } } } },
	{ PrimalSubstitute = { operands = { { "baseFn" } } } },
	{ DispatchKernel = { operands = { { "baseFn" }, { "threadGroupSize" }, { "dispatchSize" } } } },
	{
		CudaKernelLaunch = {
			operands = {
				{ "kernel" },
				{ "gridDimX" },
				{ "gridDimY" },
				{ "gridDimZ" },
				{ "blockDimX" },
				{ "blockDimY" },
			},
		},
	},
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
			{ stage = { struct_name = "StageAttr", operands = { { "stageOperand", "IRIntLit" } } } },
			{
				structFieldLayout = {
					struct_name = "StructFieldLayoutAttr",
					operands = { { "fieldKey" }, { "layout", "IRVarLayout" } },
				},
			},
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
			{ Aligned = { struct_name = "AlignedAttr", operands = { { "alignment" } } } },
			{ MemoryScope = { struct_name = "MemoryScopeAttr", min_operands = 1 } },
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
			{ FuncThrowType = { struct_name = "FuncThrowTypeAttr", operands = { { "errorType", "IRType" } } } },
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
	{ DifferentiableTypeDictionaryItem = { operands = { { "concreteType" }, { "witness" } } } },
	-- Differentiable Type Annotation (for run-time types)
	{ DifferentiableTypeAnnotation = { operands = { { "baseType" }, { "witness" } }, hoistable = true } },
	{ BeginFragmentShaderInterlock = {} },
	{
		EndFragmentShaderInterlock = { struct_name = "EndFragmentShaderInterlock" },
	},
	-- DebugInfo
	{ DebugSource = { min_operands = 3, hoistable = true } },
	{
		DebugLine = {
			min_operands = 5,
		},
	},
	{ DebugVar = { operands = { { "name" }, { "type" }, { "scope" }, { "location" } } } },
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
	{ EmbeddedDownstreamIR = { operands = { { "targetOperand", "IRIntLit" }, { "blob", "IRBlobLit" } } } },
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
	{
		SetBase = {
			-- Base class for all set representation.s
			--
			-- Semantically, `SetBase` types model sets of concrete values, and use Slang's de-duplication infrastructure
			-- to allow set-equality to be the same as inst identity.
			--
			-- - Set ops have one or more operands that represent the elements of the set
			--
			-- - Set ops must have at least one operand. A zero-operand set is illegal.
			--   The type-flow pass will represent this case using nullptr, so that uniqueness is preserved.
			--
			-- - All operands of a set _must_ be concrete, individual insts 
			--      - Operands should NOT be an interface or abstract type.
			--      - Operands should NOT be type parameters or existentail types (i.e. insts that appear in blocks)
			--      - Operands should NOT be sets (i.e. sets should be flat and never heirarchical)
			-- 
			-- - Since sets are hositable, set ops should (consequently) only appear in the global scope.
			--
			-- - Set operands must be consistently sorted. i.e. a TypeSet(A, B) and TypeSet(B, A)
			--   cannot exist at the same time, but either one is okay.
			--
			-- - To help with the implementation of sets, the IRBuilder class provides operations such as `getSet`
			--   that will ensure the above invariants are maintained, and uses a persistent unique ID map to
			--   ensure stable ordering of set elements.
			-- 
			--   Set representations should never be manually constructed to avoid breaking these invariants.
			-- 
			hoistable = true,
			{ TypeSet = {} },
			{ FuncSet = {} },
			{ WitnessTableSet = {} },
			{ GenericSet = {} }
		},
	},
	{ CastInterfaceToTaggedUnionPtr = {
		-- Cast an interface-typed pointer to a tagged-union pointer with a known set.
	} }, 
	{ GetTagForSuperSet = {
		-- Translate a tag from a set to its equivalent in a super-set
		--
		-- Operands: (the tag for the source set)
		-- The source and destination sets are implied by the type of the operand and the type of the result
	} }, 
	{ GetTagForSubSet = {
		-- Translate a tag from a set to its equivalent in a sub-set
		--
		-- Operands: (the tag for the source set)
		-- The source and destination sets are implied by the type of the operand and the type of the result
	} }, 
	{ GetTagForMappedSet = {
		-- Translate a tag from a set to its equivalent in a different set
		-- based on a mapping induced by a lookup key
		--
		-- Operands: (the tag for the witness table set, the lookup key)
	} },
	{ GetTagForSpecializedSet = { 
		-- Translate a tag from a set of generics to its equivalent in a specialized set
		-- according to the set of specialization arguments that are encoded in the 
		-- operands of this instruction.
		--
		-- Operands: (the tag for the generic set, any number of specialization arguments....)
	} },
	{ GetTagFromSequentialID = {
		-- Translate an existing sequential ID (a 'global' ID) & and interface type into a tag
	    -- the provided set (a 'local' ID)
	} }, 
	{ GetSequentialIDFromTag = {
		-- Translate a tag from the given set (a 'local' ID) to a sequential ID (a 'global' ID)
	} },
	{ GetElementFromTag = { 
	    -- Translate a tag to its corresponding element in the set. 
		-- Input's type: SetTagType(set). 
		-- Output's type: ElementOfSetType(set)
		--
		operands = {{"tag"}}
	} },
	{ GetDispatcher = {
		-- Get a dispatcher function for a given witness table set + key.
		--
		-- Inputs: set of witness tables to create a dispatched for and the key to use to identify the 
		--         entry that needs to be dispatched to. All witness tables must have an entry for the given key.
		--         or else this is a malformed inst.
		--
		-- Output: a value of 'FuncType' that can be called.
		--         This func-type will take a `TagType(witnessTableSet)` as the first parameter to 
		--         discriminate which witness table to use, and the rest of the parameters.
		--
		hoistable = true,
		operands = {{"witnessTableSet", "IRWitnessTableSet"}, {"lookupKey", "IRStructKey"}}
	} },
	{ GetSpecializedDispatcher = {
		-- Get a specialized dispatcher function for a given witness table set + key, where
		-- the key points to a generic function.
		--
		-- Operands: (set of witness tables, lookup key, specialization args...)
		--
		--
		-- Output: a value of `FuncType` that can be called.
		--         This func-type will take a `TagType(witnessTableSet)` as the first parameter to 
		--         discriminate which generic to use, and the rest of the parameters.
		--
		hoistable = true
	} },
	{ GetTagFromTaggedUnion = {
		-- Translate a tagged-union value to its corresponding tag in the tagged-union's set.
		--
		-- Input's type: TaggedUnionType(typeSet, tableSet)
		--
		-- Output's type: SetTagType(tableSet)
		--
		operands = {{"taggedUnionValue"}}
	} },
	{ GetTypeTagFromTaggedUnion = {
		-- Translate a tagged-union value to its corresponding type tag in the tagged-union's set.
		--
		-- Input's type: TaggedUnionType(typeSet, tableSet)
		--
		-- Output's type: SetTagType(typeSet)
		--
		operands = {{"taggedUnionValue"}}
	} },
	{ GetValueFromTaggedUnion = {
		-- Translate a tagged-union value to its corresponding value in the tagged-union's set.
		--
		-- Input's type: TaggedUnionType(typeSet, tableSet)
		--
		-- Output's type: UntaggedUnionType(typeSet)
		--
		operands = {{"taggedUnionValue"}}
	} },
	{ MakeTaggedUnion = {
		-- Create a tagged-union value from a tag and a value.
		--
		-- Input's type: SetTagType(tableSet), UntaggedUnionType(typeSet)
		--
		-- Output's type: TaggedUnionType(typeSet, tableSet)
		--
		operands = { { "tag" }, { "value" } },
	} },
	{ GetTagOfElementInSet = {
		-- Get the tag corresponding to an element in a set.
		--
		-- Operands: (element, set)
		--    "element" must resolve into a concrete inst before lowering,
		--    otherwise, this is an error.
		--
		-- Output's type: SetTagType(set)
		--
		hoistable = true
	} },
	{ UnboundedTypeElement = {
		-- An element of TypeSet that represents an unbounded set of types conforming to
		-- the given interface type.
		-- 
		-- Used in cases where a finite set of types cannot be determined during type-flow analysis.
		-- 
		-- Note that this is a set element, not a set in itself, so a TypeSet(A, B, UnboundedTypeElement(I))
		-- represents a set where we know two concrete types A and B, and any number of other types that conform to interface I.
		--
		hoistable = true,
		operands = { {"baseInterfaceType"} }
	} },
	{ UnboundedFuncElement = {
		-- An element of FuncSet that represents an unbounded set of functions of a certain
		-- func-type
		-- 
		-- Used in cases where a finite set of functions cannot be determined during type-flow analysis.
		--
		-- Similar to UnboundedTypeElement, this is a set element, not a set in itself.
		-- 
		hoistable = true,
		operands = { {"funcType"} }
	} },
	{ UnboundedWitnessTableElement = {
		-- An element of WitnessTableSet that represents an unbounded set of witness tables of a certain
		-- interface type
		-- 
		-- Used in cases where a finite set of witness tables cannot be determined during type-flow analysis.
		--
		-- Similar to UnboundedTypeElement, this is a set element, not a set in itself.
		-- 
		hoistable = true,
		operands = { {"baseInterfaceType"} }
	} },
	{ UnboundedGenericElement = {
		-- An element of GenericSet that represents an unbounded set of generics of a certain
		-- interface type
		-- 
		-- Used in cases where a finite set of generics cannot be determined during type-flow analysis.
		--
		-- Similar to UnboundedTypeElement, this is a set element, not a set in itself.
		-- 
		hoistable = true,
	} },
	{ UninitializedTypeElement = {
		-- An element that represents an uninitialized type of a certain interface.
		-- 
		-- Used to denote cases where the type represented may be garbage (e.g. from a `LoadFromUninitializedMemory`)
		--
		-- Similar to UnboundedXYZElement IR ops described above, this is a set element, not a set in itself.
		-- e.g. a `TypeSet(A, B, UninitializedTypeElement(I))` represents a set where we know two concrete types A and B,
		-- and an uninitialized type that conforms to interface I.
		--
		-- Note: In practice, having any uninitialized type in a TypeSet will likely force the entire set to be treated as 
		-- uninitialized, and this element is mainly so that we can provide useful errors during the type-flow specialization pass.
		--
		hoistable = true,
		operands = { {"baseInterfaceType"} }
	} },
	{ UninitializedWitnessTableElement = {
		-- An element that represents an uninitialized witness table of a certain interface.
		-- 
		-- Used to denote cases where the witness table information may be garbage (e.g. from a `LoadFromUninitializedMemory`)
		--
		-- Similar to UninitializedTypeElement, this is a set element, not a set in itself.
		-- 
		hoistable = true,
		operands = { {"baseInterfaceType"} }
	} },
	{ NoneTypeElement = {
		-- An element that represents a default 'none' case (only relevant in the context of OptionalType)
		-- 
		-- Similar to UnboundedXYZElement IR ops described above, this is a set element, not a set in itself.
		--
		hoistable = true
	} },
	{ NoneWitnessTableElement = {
		-- An element that represents a default 'none' case (only relevant in the context of OptionalType)
		--
		-- Similar to UnboundedXYZElement IR ops described above, this is a set element, not a set in itself.
		--
		hoistable = true
	} },
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
	-- Extract type instructions from the main instruction list
	local type_insts = nil
	for _, inst in ipairs(insts) do
		if inst.Type then
			type_insts = inst.Type
			break
		end
	end

	return {
		insts = insts,
		type_insts = type_insts,
		stable_name_to_inst = stable_name_to_inst,
		max_stable_name = max_stable_name,
		traverse = traverse,
	}
end

return process(insts)
