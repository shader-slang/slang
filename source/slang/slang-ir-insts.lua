--
-- This file contains the canonical definitions for the instions to the Slang IR
-- Add new instructions here
--
-- The instructions struct name, i.e. something like "IRVoidType" can be specified with struct_name, otherwise it will be a PascalCase version of the instruction key
--
-- Flags, such as hoistable, global, parent, use_other are inherited from a parent abstract type
--

local insts = {
	nop = {},
	Type = {
		BasicType = {
			hoistable = true,
			Void = {
				struct_name = "VoidType",
			},
			Bool = {
				struct_name = "BoolType",
			},
			Int8 = {
				struct_name = "Int8Type",
			},
			Int16 = {
				struct_name = "Int16Type",
			},
			Int = {
				struct_name = "IntType",
			},
			Int64 = {
				struct_name = "Int64Type",
			},
			UInt8 = {
				struct_name = "UInt8Type",
			},
			UInt16 = {
				struct_name = "UInt16Type",
			},
			UInt = {
				struct_name = "UIntType",
			},
			UInt64 = {
				struct_name = "UInt64Type",
			},
			Half = {
				struct_name = "HalfType",
			},
			Float = {
				struct_name = "FloatType",
			},
			Double = {
				struct_name = "DoubleType",
			},
			Char = {
				struct_name = "CharType",
			},
			IntPtr = {
				struct_name = "IntPtrType",
			},
			UIntPtr = {
				struct_name = "UIntPtrType",
			},
		},
		StringTypeBase = {
			hoistable = true,
			String = {
				struct_name = "StringType",
			},
			NativeString = {
				struct_name = "NativeStringType",
			},
		},
		CapabilitySet = {
			struct_name = "CapabilitySetType",
			hoistable = true,
		},
		DynamicType = {
			hoistable = true,
		},
		AnyValueType = {
			minOperands = 1,
			hoistable = true,
		},
		RawPointerTypeBase = {
			hoistable = true,
			RawPointerType = {},
			RTTIPointerType = {
				minOperands = 1,
			},
		},
		ArrayTypeBase = {
			hoistable = true,
			Array = {
				struct_name = "ArrayType",
				minOperands = 2,
			},
			UnsizedArray = {
				struct_name = "UnsizedArrayType",
				minOperands = 1,
			},
		},
		Func = {
			struct_name = "FuncType",
			hoistable = true,
		},
		BasicBlock = {
			struct_name = "BasicBlockType",
			hoistable = true,
		},
		Vec = {
			struct_name = "VectorType",
			minOperands = 2,
			hoistable = true,
		},
		Mat = {
			struct_name = "MatrixType",
			minOperands = 4,
			hoistable = true,
		},
		Conjunction = {
			struct_name = "ConjunctionType",
			hoistable = true,
		},
		Attributed = {
			struct_name = "AttributedType",
			hoistable = true,
		},
		Result = {
			struct_name = "ResultType",
			minOperands = 2,
			hoistable = true,
		},
		Optional = {
			struct_name = "OptionalType",
			minOperands = 1,
			hoistable = true,
		},
		Enum = {
			struct_name = "EnumType",
			minOperands = 1,
			parent = true,
		},
		DifferentialPairTypeBase = {
			hoistable = true,
			DiffPair = {
				struct_name = "DifferentialPairType",
				minOperands = 1,
			},
			DiffPairUserCode = {
				struct_name = "DifferentialPairUserCodeType",
				minOperands = 1,
			},
			DiffRefPair = {
				struct_name = "DifferentialPtrPairType",
				minOperands = 1,
			},
		},
		BwdDiffIntermediateCtxType = {
			struct_name = "BackwardDiffIntermediateContextType",
			minOperands = 1,
			hoistable = true,
		},
		TensorView = {
			struct_name = "TensorViewType",
			minOperands = 1,
			hoistable = true,
		},
		TorchTensor = {
			struct_name = "TorchTensorType",
			hoistable = true,
		},
		ArrayListVector = {
			struct_name = "ArrayListType",
			minOperands = 1,
			hoistable = true,
		},
		Atomic = {
			struct_name = "AtomicType",
			minOperands = 1,
			hoistable = true,
		},
		BindExistentialsTypeBase = {
			hoistable = true,
			BindExistentials = {
				-- A `BindExistentials<B, T0,w0, T1,w1, ...>` represents
				-- taking type `B` and binding each of its existential type
				-- parameters, recursively, with the specified arguments,
				-- where each `Ti, wi` pair represents the concrete type
				-- and witness table to plug in for parameter `i`.
				struct_name = "BindExistentialsType",
				minOperands = 1,
			},
			BoundInterface = {
				-- An `BindInterface<B, T0, w0>` represents the special case
				-- of a `BindExistentials` where the type `B` is known to be
				-- an interface type.
				struct_name = "BoundInterfaceType",
				minOperands = 3,
			},
		},
		Rate = {
			hoistable = true,
			ConstExpr = {
				struct_name = "ConstExprRate",
			},
			SpecConst = {
				struct_name = "SpecConstRate",
			},
			GroupShared = {
				struct_name = "GroupSharedRate",
			},
			ActualGlobalRate = {},
		},
		RateQualified = {
			struct_name = "RateQualifiedType",
			minOperands = 2,
			hoistable = true,
		},
		Kind = {
			-- Kinds represent the "types of types."
			-- They should not really be nested under `IRType`
			-- in the overall hierarchy, but we can fix that later.
			hoistable = true,
			Type = {
				struct_name = "TypeKind",
			},
			TypeParameterPack = {
				struct_name = "TypeParameterPackKind",
			},
			Rate = {
				struct_name = "RateKind",
			},
			Generic = {
				struct_name = "GenericKind",
			},
		},
		PtrTypeBase = {
			hoistable = true,
			Ptr = {
				struct_name = "PtrType",
				minOperands = 1,
			},
			Ref = {
				struct_name = "RefType",
				minOperands = 1,
			},
			ConstRef = {
				struct_name = "ConstRefType",
				minOperands = 1,
			},
			PseudoPtr = {
				-- A `PsuedoPtr<T>` logically represents a pointer to a value of type
				-- `T` on a platform that cannot support pointers. The expectation
				-- is that the "pointer" will be legalized away by storing a value
				-- of type `T` somewhere out-of-line.
				struct_name = "PseudoPtrType",
				minOperands = 1,
			},
			OutTypeBase = {
				Out = {
					struct_name = "OutType",
					minOperands = 1,
				},
				InOut = {
					struct_name = "InOutType",
					minOperands = 1,
				},
			},
		},
		ComPtr = {
			-- A ComPtr<T> type is treated as a opaque type that represents a reference-counted handle to a COM object.
			struct_name = "ComPtrType",
			minOperands = 1,
			hoistable = true,
		},
		NativePtr = {
			-- A NativePtr<T> type represents a native pointer to a managed resource.
			struct_name = "NativePtrType",
			minOperands = 1,
			hoistable = true,
		},
		DescriptorHandle = {
			-- A DescriptorHandle<T> type represents a bindless handle to an opaue resource type.
			struct_name = "DescriptorHandleType",
			minOperands = 1,
			hoistable = true,
		},
		GLSLAtomicUint = {
			-- An AtomicUint is a placeholder type for a storage buffer, and will be mangled during compiling.
			struct_name = "GLSLAtomicUintType",
			hoistable = true,
		},
		SamplerStateTypeBase = {
			hoistable = true,
			SamplerState = {
				struct_name = "SamplerStateType",
			},
			SamplerComparisonState = {
				struct_name = "SamplerComparisonStateType",
			},
		},
		DefaultLayout = {
			struct_name = "DefaultBufferLayoutType",
			hoistable = true,
		},
		Std140Layout = {
			struct_name = "Std140BufferLayoutType",
			hoistable = true,
		},
		Std430Layout = {
			struct_name = "Std430BufferLayoutType",
			hoistable = true,
		},
		ScalarLayout = {
			struct_name = "ScalarBufferLayoutType",
			hoistable = true,
		},
		SubpassInputType = {
			minOperands = 2,
			hoistable = true,
		},
		TextureFootprintType = {
			minOperands = 1,
			hoistable = true,
		},
		TextureShape1DType = {
			hoistable = true,
		},
		TextureShape2DType = {
			struct_name = "TextureShape2DType",
			hoistable = true,
		},
		TextureShape3DType = {
			struct_name = "TextureShape3DType",
			hoistable = true,
		},
		TextureShapeCubeDType = {
			struct_name = "TextureShapeCubeType",
			hoistable = true,
		},
		TextureShapeBufferType = {
			hoistable = true,
		},
		ResourceTypeBase = {
			-- TODO: Why do we have all this hierarchy here, when everything
			-- that actually matters is currently nested under `TextureTypeBase`?
			ResourceType = {
				TextureTypeBase = {
					TextureType = {
						minOperands = 8,
						hoistable = true,
					},
					GLSLImageType = {
						use_other = true,
						hoistable = true,
					},
				},
			},
		},
		UntypedBufferResourceType = {
			hoistable = true,
			ByteAddressBufferTypeBase = {
				ByteAddressBuffer = {
					struct_name = "HLSLByteAddressBufferType",
				},
				RWByteAddressBuffer = {
					struct_name = "HLSLRWByteAddressBufferType",
				},
				RasterizerOrderedByteAddressBuffer = {
					struct_name = "HLSLRasterizerOrderedByteAddressBufferType",
				},
			},
			RaytracingAccelerationStructure = {
				struct_name = "RaytracingAccelerationStructureType",
			},
		},
		HLSLPatchType = {
			hoistable = true,
			InputPatch = {
				struct_name = "HLSLInputPatchType",
				minOperands = 2,
			},
			OutputPatch = {
				struct_name = "HLSLOutputPatchType",
				minOperands = 2,
			},
		},
		GLSLInputAttachment = {
			struct_name = "GLSLInputAttachmentType",
			hoistable = true,
		},
		BuiltinGenericType = {
			hoistable = true,
			HLSLStreamOutputType = {
				PointStream = {
					struct_name = "HLSLPointStreamType",
					minOperands = 1,
				},
				LineStream = {
					struct_name = "HLSLLineStreamType",
					minOperands = 1,
				},
				TriangleStream = {
					struct_name = "HLSLTriangleStreamType",
					minOperands = 1,
				},
			},
			MeshOutputType = {
				Vertices = {
					struct_name = "VerticesType",
					minOperands = 2,
				},
				Indices = {
					struct_name = "IndicesType",
					minOperands = 2,
				},
				Primitives = {
					struct_name = "PrimitivesType",
					minOperands = 2,
				},
			},
			["metal::mesh"] = {
				struct_name = "MetalMeshType",
				minOperands = 5,
			},
			mesh_grid_properties = {
				struct_name = "MetalMeshGridPropertiesType",
			},
			HLSLStructuredBufferTypeBase = {
				StructuredBuffer = {
					struct_name = "HLSLStructuredBufferType",
				},
				RWStructuredBuffer = {
					struct_name = "HLSLRWStructuredBufferType",
				},
				RasterizerOrderedStructuredBuffer = {
					struct_name = "HLSLRasterizerOrderedStructuredBufferType",
				},
				AppendStructuredBuffer = {
					struct_name = "HLSLAppendStructuredBufferType",
				},
				ConsumeStructuredBuffer = {
					struct_name = "HLSLConsumeStructuredBufferType",
				},
			},
			PointerLikeType = {
				ParameterGroupType = {
					UniformParameterGroupType = {
						ConstantBuffer = {
							struct_name = "ConstantBufferType",
							minOperands = 1,
						},
						TextureBuffer = {
							struct_name = "TextureBufferType",
							minOperands = 1,
						},
						ParameterBlock = {
							struct_name = "ParameterBlockType",
							minOperands = 1,
						},
					},
					VaryingParameterGroupType = {
						GLSLInputParameterGroup = {
							struct_name = "GLSLInputParameterGroupType",
						},
						GLSLOutputParameterGroup = {
							struct_name = "GLSLOutputParameterGroupType",
						},
					},
					GLSLShaderStorageBuffer = {
						struct_name = "GLSLShaderStorageBufferType",
						minOperands = 1,
					},
				},
			},
		},
		RayQuery = {
			-- Types
			struct_name = "RayQueryType",
			minOperands = 1,
			hoistable = true,
		},
		HitObject = {
			struct_name = "HitObjectType",
			hoistable = true,
		},
		CoopVectorType = {
			minOperands = 2,
			hoistable = true,
		},
		CoopMatrixType = {
			minOperands = 5,
			hoistable = true,
		},
		TensorAddressingTensorLayoutType = {
			minOperands = 2,
			hoistable = true,
		},
		TensorAddressingTensorViewType = {
			minOperands = 3,
			hoistable = true,
		},
		MakeTensorAddressingTensorLayout = {},
		MakeTensorAddressingTensorView = {},
		DynamicResource = {
			-- Opaque type that can be dynamically cast to other resource types.
			struct_name = "DynamicResourceType",
			hoistable = true,
		},
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
		class = {
			struct_name = "ClassType",
			parent = true,
		},
		interface = {
			struct_name = "InterfaceType",
			global = true,
		},
		associated_type = {
			hoistable = true,
		},
		this_type = {
			hoistable = true,
		},
		rtti_type = {
			struct_name = "RTTIType",
			hoistable = true,
		},
		rtti_handle_type = {
			struct_name = "RTTIHandleType",
			hoistable = true,
		},
		TupleTypeBase = {
			hoistable = true,
			tuple_type = {},
			TypePack = {},
		},
		TargetTuple = {
			struct_name = "TargetTupleType",
			hoistable = true,
		},
		ExpandTypeOrVal = {
			minOperands = 1,
			hoistable = true,
		},
		spirvLiteralType = {
			-- A type that identifies it's contained type as being emittable as `spirv_literal.
			struct_name = "SPIRVLiteralType",
			minOperands = 1,
			hoistable = true,
		},
		type_t = {
			-- A TypeType-typed IRValue represents a IRType.
			-- It is used to represent a type parameter/argument in a generics.
			hoistable = true,
		},
		WitnessTableTypeBase = {
			-- IRWitnessTableTypeBase
			hoistable = true,
			witness_table_t = {
				-- An `IRWitnessTable` has type `WitnessTableType`.
				minOperands = 1,
			},
			witness_table_id_t = {
				-- An integer type representing a witness table for targets where
				-- witness tables are represented as integer IDs. This type is used
				-- during the lower-generics pass while generating dynamic dispatch
				-- code and will eventually lower into an uint type.
				struct_name = "WitnessTableIDType",
				minOperands = 1,
			},
		},
	},
	-- IRGlobalValueWithCode
	GlobalValueWithCode = {
		GlobalValueWithParams = {
			-- IRGlobalValueWithParams
			parent = true,
			func = {},
			generic = {},
		},
		global_var = {
			global = true,
		},
	},
	global_param = {
		global = true,
	},
	globalConstant = {
		global = true,
	},
	key = {
		struct_name = "StructKey",
		global = true,
	},
	global_generic_param = {
		global = true,
	},
	witness_table = {
		hoistable = true,
	},
	indexedFieldKey = {
		minOperands = 2,
		hoistable = true,
	},
	-- A placeholder witness that ThisType implements the enclosing interface.
	-- Used only in interface definitions.
	thisTypeWitness = {
		minOperands = 1,
	},
	-- A placeholder witness for the fact that two types are equal.
	TypeEqualityWitness = {
		minOperands = 2,
		hoistable = true,
	},
	global_hashed_string_literals = {},
	module = {
		parent = true,
	},
	block = {
		parent = true,
	},
	-- IRConstant
	Constant = {
		boolConst = {
			struct_name = "BoolLit",
		},
		integer_constant = {
			struct_name = "IntLit",
		},
		float_constant = {
			struct_name = "FloatLit",
		},
		ptr_constant = {
			struct_name = "PtrLit",
		},
		string_constant = {
			struct_name = "StringLit",
		},
		blob_constant = {
			struct_name = "BlobLit",
		},
		void_constant = {
			struct_name = "VoidLit",
		},
	},
	CapabilitySet = {
		hoistable = true,
		capabilityConjunction = {},
		capabilityDisjunction = {},
	},
	undefined = {
		struct_name = "undefined",
	},
	-- A `defaultConstruct` operation creates an initialized
	-- value of the result type, and can only be used for types
	-- where default construction is a meaningful thing to do.
	defaultConstruct = {},
	MakeDifferentialPairBase = {
		MakeDiffPair = {
			struct_name = "MakeDifferentialPair",
			minOperands = 2,
		},
		MakeDiffPairUserCode = {
			struct_name = "MakeDifferentialPairUserCode",
			minOperands = 2,
		},
		MakeDiffRefPair = {
			struct_name = "MakeDifferentialPtrPair",
			minOperands = 2,
		},
	},
	DifferentialPairGetDifferentialBase = {
		GetDifferential = {
			struct_name = "DifferentialPairGetDifferential",
			minOperands = 1,
		},
		GetDifferentialUserCode = {
			struct_name = "DifferentialPairGetDifferentialUserCode",
			minOperands = 1,
		},
		GetDifferentialPtr = {
			struct_name = "DifferentialPtrPairGetDifferential",
			minOperands = 1,
		},
	},
	DifferentialPairGetPrimalBase = {
		GetPrimal = {
			struct_name = "DifferentialPairGetPrimal",
			minOperands = 1,
		},
		GetPrimalUserCode = {
			struct_name = "DifferentialPairGetPrimalUserCode",
			minOperands = 1,
		},
		GetPrimalRef = {
			struct_name = "DifferentialPtrPairGetPrimal",
			minOperands = 1,
		},
	},
	specialize = {
		minOperands = 2,
		hoistable = true,
	},
	lookupWitness = {
		minOperands = 2,
		hoistable = true,
	},
	GetSequentialID = {
		minOperands = 1,
		hoistable = true,
	},
	bind_global_generic_param = {
		minOperands = 2,
	},
	allocObj = {},
	globalValueRef = {
		minOperands = 1,
	},
	makeUInt64 = {
		minOperands = 2,
	},
	makeVector = {},
	makeMatrix = {},
	makeMatrixFromScalar = {
		minOperands = 1,
	},
	matrixReshape = {
		minOperands = 1,
	},
	vectorReshape = {
		minOperands = 1,
	},
	makeArray = {},
	makeArrayFromElement = {
		minOperands = 1,
	},
	makeCoopVector = {},
	makeCoopVectorFromValuePack = {
		minOperands = 1,
	},
	makeStruct = {},
	makeTuple = {},
	makeTargetTuple = {
		struct_name = "MakeTargetTuple",
	},
	makeValuePack = {},
	getTargetTupleElement = {},
	getTupleElement = {
		minOperands = 2,
	},
	LoadResourceDescriptorFromHeap = {
		minOperands = 1,
	},
	LoadSamplerDescriptorFromHeap = {
		minOperands = 1,
	},
	MakeCombinedTextureSamplerFromHandle = {
		minOperands = 1,
	},
	MakeWitnessPack = {
		hoistable = true,
	},
	Expand = {
		minOperands = 1,
	},
	Each = {
		minOperands = 1,
		hoistable = true,
	},
	makeResultValue = {
		minOperands = 1,
	},
	makeResultError = {
		minOperands = 1,
	},
	isResultError = {
		minOperands = 1,
	},
	getResultError = {
		minOperands = 1,
	},
	getResultValue = {
		minOperands = 1,
	},
	getOptionalValue = {
		minOperands = 1,
	},
	optionalHasValue = {
		minOperands = 1,
	},
	makeOptionalValue = {
		minOperands = 1,
	},
	makeOptionalNone = {
		minOperands = 1,
	},
	CombinedTextureSamplerGetTexture = {
		minOperands = 1,
	},
	CombinedTextureSamplerGetSampler = {
		minOperands = 1,
	},
	call = {
		minOperands = 1,
	},
	rtti_object = {
		struct_name = "RTTIObject",
	},
	alloca = {
		minOperands = 1,
	},
	updateElement = {
		minOperands = 2,
	},
	detachDerivative = {
		minOperands = 1,
	},
	bitfieldExtract = {
		minOperands = 3,
	},
	bitfieldInsert = {
		minOperands = 4,
	},
	packAnyValue = {
		minOperands = 1,
	},
	unpackAnyValue = {
		minOperands = 1,
	},
	witness_table_entry = {
		minOperands = 2,
	},
	interface_req_entry = {
		struct_name = "InterfaceRequirementEntry",
		minOperands = 2,
		global = true,
	},
	-- An inst to represent the workgroup size of the calling entry point.
	-- We will materialize this inst during `translateGlobalVaryingVar`.
	GetWorkGroupSize = {
		hoistable = true,
	},
	-- An inst that returns the current stage of the calling entry point.
	GetCurrentStage = {},
	param = {},
	field = {
		struct_name = "StructField",
		minOperands = 2,
	},
	var = {},
	load = {
		minOperands = 1,
	},
	store = {
		minOperands = 2,
	},
	-- Atomic Operations
	AtomicOperation = {
		atomicLoad = {
			minOperands = 1,
		},
		atomicStore = {
			minOperands = 2,
		},
		atomicExchange = {
			minOperands = 2,
		},
		atomicCompareExchange = {
			minOperands = 3,
		},
		atomicAdd = {
			minOperands = 2,
		},
		atomicSub = {
			minOperands = 2,
		},
		atomicAnd = {
			minOperands = 2,
		},
		atomicOr = {
			minOperands = 2,
		},
		atomicXor = {
			minOperands = 2,
		},
		atomicMin = {
			minOperands = 2,
		},
		atomicMax = {
			minOperands = 2,
		},
		atomicInc = {
			minOperands = 1,
		},
		atomicDec = {
			minOperands = 1,
		},
	},
	-- Produced and removed during backward auto-diff pass as a temporary placeholder representing the
	-- currently accumulated derivative to pass to some dOut argument in a nested call.
	LoadReverseGradient = {
		minOperands = 1,
	},
	-- Produced and removed during backward auto-diff pass as a temporary placeholder containing the
	-- primal and accumulated derivative values to pass to an inout argument in a nested call.
	ReverseGradientDiffPairRef = {
		minOperands = 2,
	},
	-- Produced and removed during backward auto-diff pass. This inst is generated by the splitting step
	-- to represent a reference to an inout parameter for use in the primal part of the computation.
	PrimalParamRef = {
		minOperands = 1,
	},
	-- Produced and removed during backward auto-diff pass. This inst is generated by the splitting step
	-- to represent a reference to an inout parameter for use in the back-prop part of the computation.
	DiffParamRef = {
		minOperands = 1,
	},
	-- Check that the value is a differential null value.
	IsDifferentialNull = {
		minOperands = 1,
	},
	get_field = {
		struct_name = "FieldExtract",
		minOperands = 2,
	},
	get_field_addr = {
		struct_name = "FieldAddress",
		minOperands = 2,
	},
	getElement = {
		minOperands = 2,
	},
	getElementPtr = {
		minOperands = 2,
	},
	-- Pointer offset: computes pBase + offset_in_elements
	getOffsetPtr = {
		minOperands = 2,
	},
	getAddr = {
		minOperands = 1,
	},
	castDynamicResource = {
		minOperands = 1,
	},
	-- Get an unowned NativeString from a String.
	getNativeStr = {
		struct_name = "getNativeStr",
		minOperands = 1,
	},
	-- Make String from a NativeString.
	makeString = {
		minOperands = 1,
	},
	-- Get a native ptr from a ComPtr or RefPtr
	getNativePtr = {
		minOperands = 1,
	},
	-- Get a write reference to a managed ptr var (operand must be Ptr<ComPtr<T>> or Ptr<RefPtr<T>>).
	getManagedPtrWriteRef = {
		minOperands = 1,
	},
	-- Attach a managedPtr var to a NativePtr without changing its ref count.
	ManagedPtrAttach = {
		minOperands = 1,
	},
	-- Attach a managedPtr var to a NativePtr without changing its ref count.
	ManagedPtrDetach = {
		minOperands = 1,
	},
	-- "Subscript" an image at a pixel coordinate to get pointer
	imageSubscript = {
		minOperands = 2,
	},
	-- Load from an Image.
	imageLoad = {
		minOperands = 2,
	},
	-- Store into an Image.
	imageStore = {
		minOperands = 3,
	},
	-- Load (almost) arbitrary-type data from a byte-address buffer
	-- %dst = byteAddressBufferLoad(%buffer, %offset, %alignment)
	-- where
	-- - `buffer` is a value of some `ByteAddressBufferTypeBase` type
	-- - `offset` is an `int`
	-- - `alignment` is an `int`
	-- - `dst` is a value of some type containing only ordinary data
	byteAddressBufferLoad = {
		minOperands = 3,
	},
	-- Store (almost) arbitrary-type data to a byte-address buffer
	-- byteAddressBufferLoad(%buffer, %offset, %alignment, %src)
	-- where
	-- - `buffer` is a value of some `ByteAddressBufferTypeBase` type
	-- - `offset` is an `int`
	-- - `alignment` is an `int`
	-- - `src` is a value of some type containing only ordinary data
	byteAddressBufferStore = {
		minOperands = 4,
	},
	-- Load data from a structured buffer
	-- %dst = structuredBufferLoad(%buffer, %index)
	-- where
	-- - `buffer` is a value of some `StructuredBufferTypeBase` type with element type T
	-- - `offset` is an `int`
	-- - `dst` is a value of type T
	structuredBufferLoad = {
		minOperands = 2,
	},
	structuredBufferLoadStatus = {
		minOperands = 3,
	},
	rwstructuredBufferLoad = {
		struct_name = "RWStructuredBufferLoad",
		minOperands = 2,
	},
	rwstructuredBufferLoadStatus = {
		struct_name = "RWStructuredBufferLoadStatus",
		minOperands = 3,
	},
	-- Store data to a structured buffer
	-- structuredBufferLoad(%buffer, %offset, %src)
	-- where
	-- - `buffer` is a value of some `StructuredBufferTypeBase` type with element type T
	-- - `offset` is an `int`
	-- - `src` is a value of type T
	rwstructuredBufferStore = {
		struct_name = "RWStructuredBufferStore",
		minOperands = 3,
	},
	rwstructuredBufferGetElementPtr = {
		struct_name = "RWStructuredBufferGetElementPtr",
		minOperands = 2,
	},
	-- Append/Consume-StructuredBuffer operations
	StructuredBufferAppend = {
		minOperands = 1,
	},
	StructuredBufferConsume = {
		minOperands = 1,
	},
	StructuredBufferGetDimensions = {
		minOperands = 1,
	},
	-- Resource qualifiers for dynamically varying index
	nonUniformResourceIndex = {
		minOperands = 1,
	},
	getNaturalStride = {
		minOperands = 1,
	},
	meshOutputRef = {
		minOperands = 2,
	},
	meshOutputSet = {
		minOperands = 3,
	},
	-- only two parameters as they are effectively static
	-- TODO: make them reference the _slang_mesh object directly
	metalSetVertex = {
		minOperands = 2,
	},
	metalSetPrimitive = {
		minOperands = 2,
	},
	metalSetIndices = {
		minOperands = 2,
	},
	MetalCastToDepthTexture = {
		minOperands = 1,
	},
	-- Construct a vector from a scalar
	-- %dst = MakeVectorFromScalar %T %N %val
	-- where
	-- - `T` is a `Type`
	-- - `N` is a (compile-time) `Int`
	-- - `val` is a `T`
	-- - dst is a `Vec<T,N>`
	MakeVectorFromScalar = {
		minOperands = 3,
	},
	-- A swizzle of a vector:
	-- %dst = swizzle %src %idx0 %idx1 ...
	-- where:
	-- - `src` is a vector<T,N>
	-- - `dst` is a vector<T,M>
	-- - `idx0` through `idx[M-1]` are literal integers
	swizzle = {
		struct_name = "swizzle",
		minOperands = 1,
	},
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
	swizzleSet = {
		struct_name = "swizzleSet",
		minOperands = 2,
	},
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
	swizzledStore = {
		minOperands = 2,
	},
	TerminatorInst = {
		return_val = {
			struct_name = "Return",
			minOperands = 1,
		},
		yield = {
			minOperands = 1,
		},
		UnconditionalBranch = {
			-- IRUnconditionalBranch
			unconditionalBranch = {
				-- unconditionalBranch <target>
				struct_name = "unconditionalBranch",
				minOperands = 1,
			},
			loop = {
				-- loop <target> <breakLabel> <continueLabel>
				struct_name = "loop",
				minOperands = 3,
			},
		},
		ConditionalBranch = {
			-- IRTerminatorInst
			conditionalBranch = {
				-- conditionalBranch <condition> <trueBlock> <falseBlock>
				struct_name = "conditionalBranch",
				minOperands = 3,
			},
			ifElse = {
				-- ifElse <condition> <trueBlock> <falseBlock> <mergeBlock>
				struct_name = "ifElse",
				minOperands = 4,
			},
		},
		throw = {
			-- IRConditionalbranch
			minOperands = 1,
		},
		tryCall = {
			-- tryCall <successBlock> <failBlock> <callee> <args>...
			minOperands = 3,
		},
		switch = {
			-- switch <val> <break> <default> <caseVal1> <caseBlock1> ...
			minOperands = 3,
		},
		targetSwitch = {
			-- target_switch <break> <targetName1> <block1> ...
			minOperands = 1,
		},
		GenericAsm = {
			-- A generic asm inst has an return semantics that terminates the control flow.
			minOperands = 1,
		},
		Unreachable = {
			missingReturn = {
				-- IRUnreachable
			},
			unreachable = {},
		},
		defer = {
			minOperands = 3,
		},
	},
	discard = {
		struct_name = "discard",
	},
	RequirePrelude = {
		minOperands = 1,
	},
	RequireTargetExtension = {
		minOperands = 1,
	},
	RequireComputeDerivative = {},
	StaticAssert = {
		minOperands = 2,
	},
	Printf = {
		minOperands = 1,
	},
	-- Quad control execution modes.
	RequireMaximallyReconverges = {},
	RequireQuadDerivatives = {},
	-- TODO: We should consider splitting the basic arithmetic/comparison
	-- ops into cases for signed integers, unsigned integers, and floating-point
	-- values, to better match downstream targets that want to treat them
	-- all differently (e.g., SPIR-V).
	add = {
		minOperands = 2,
	},
	sub = {
		minOperands = 2,
	},
	mul = {
		minOperands = 2,
	},
	div = {
		minOperands = 2,
	},
	-- Remainder of division.
	-- Note: this is distinct from modulus, and we should have a separate
	-- opcode for `mod` if we ever need to support it.
	irem = {
		struct_name = "IRem",
		minOperands = 2,
	},
	frem = {
		struct_name = "FRem",
		minOperands = 2,
	},
	shl = {
		struct_name = "Lsh",
		minOperands = 2,
	},
	shr = {
		struct_name = "Rsh",
		minOperands = 2,
	},
	cmpEQ = {
		struct_name = "Eql",
		minOperands = 2,
	},
	cmpNE = {
		struct_name = "Neq",
		minOperands = 2,
	},
	cmpGT = {
		struct_name = "Greater",
		minOperands = 2,
	},
	cmpLT = {
		struct_name = "Less",
		minOperands = 2,
	},
	cmpGE = {
		struct_name = "Geq",
		minOperands = 2,
	},
	cmpLE = {
		struct_name = "Leq",
		minOperands = 2,
	},
	["and"] = {
		struct_name = "BitAnd",
		minOperands = 2,
	},
	xor = {
		struct_name = "BitXor",
		minOperands = 2,
	},
	["or"] = {
		struct_name = "BitOr",
		minOperands = 2,
	},
	logicalAnd = {
		struct_name = "And",
		minOperands = 2,
	},
	logicalOr = {
		struct_name = "Or",
		minOperands = 2,
	},
	neg = {
		minOperands = 1,
	},
	["not"] = {
		minOperands = 1,
	},
	bitnot = {
		struct_name = "BitNot",
		minOperands = 1,
	},
	select = {
		minOperands = 3,
	},
	checkpointObj = {
		struct_name = "CheckpointObject",
		minOperands = 1,
	},
	loopExitValue = {
		minOperands = 1,
	},
	getStringHash = {
		minOperands = 1,
	},
	waveGetActiveMask = {},
	-- trueMask = waveMaskBallot(mask, condition)
	waveMaskBallot = {
		minOperands = 2,
	},
	-- matchMask = waveMaskBallot(mask, value)
	waveMaskMatch = {
		minOperands = 2,
	},
	-- Texture sampling operation of the form `t.Sample(s,u)`
	sample = {
		minOperands = 3,
	},
	sampleGrad = {
		minOperands = 4,
	},
	GroupMemoryBarrierWithGroupSync = {},
	ControlBarrier = {},
	-- GPU_FOREACH loop of the form
	gpuForeach = {
		minOperands = 3,
	},
	-- Wrapper for OptiX intrinsics used to load and store ray payload data using
	-- a pointer represented by two payload registers.
	getOptiXRayPayloadPtr = {
		hoistable = true,
	},
	-- Wrapper for OptiX intrinsics used to load a single hit attribute
	-- Takes two arguments: the type (either float or int), and the hit
	-- attribute index
	getOptiXHitAttribute = {
		minOperands = 2,
	},
	-- Wrapper for OptiX intrinsics used to load shader binding table record data
	-- using a pointer.
	getOptiXSbtDataPointer = {
		struct_name = "GetOptiXSbtDataPtr",
	},
	GetVulkanRayTracingPayloadLocation = {
		minOperands = 1,
	},
	GetLegalizedSPIRVGlobalParamAddr = {
		minOperands = 1,
	},
	GetPerVertexInputArray = {
		minOperands = 1,
		hoistable = true,
	},
	ResolveVaryingInputRef = {
		minOperands = 1,
		hoistable = true,
	},
	ForceVarIntoStructTemporarily = {
		ForceVarIntoStructTemporarily = {
			minOperands = 1,
		},
		ForceVarIntoRayPayloadStructTemporarily = {
			minOperands = 1,
		},
	},
	MetalAtomicCast = {
		minOperands = 1,
	},
	IsTextureAccess = {
		minOperands = 1,
	},
	IsTextureScalarAccess = {
		minOperands = 1,
	},
	IsTextureArrayAccess = {
		minOperands = 1,
	},
	ExtractTextureFromTextureAccess = {
		minOperands = 1,
	},
	ExtractCoordFromTextureAccess = {
		minOperands = 1,
	},
	ExtractArrayCoordFromTextureAccess = {
		minOperands = 1,
	},
	makeArrayList = {},
	makeTensorView = {},
	allocTorchTensor = {
		struct_name = "AllocateTorchTensor",
	},
	TorchGetCudaStream = {},
	TorchTensorGetView = {},
	CoopMatMapElementIFunc = {
		minOperands = 2,
	},
	allocateOpaqueHandle = {},
	BindingQuery = {
		getRegisterIndex = {
			-- Return the register index thtat a resource is bound to.
			minOperands = 1,
		},
		getRegisterSpace = {
			-- Return the registe space that a resource is bound to.
			minOperands = 1,
		},
	},
	Decoration = {
		highLevelDecl = {
			struct_name = "HighLevelDeclDecoration",
			minOperands = 1,
		},
		layout = {
			struct_name = "LayoutDecoration",
			minOperands = 1,
		},
		branch = {
			struct_name = "BranchDecoration",
		},
		flatten = {
			struct_name = "FlattenDecoration",
		},
		loopControl = {
			struct_name = "LoopControlDecoration",
			minOperands = 1,
		},
		loopMaxIters = {
			struct_name = "LoopMaxItersDecoration",
			minOperands = 1,
		},
		loopExitPrimalValue = {
			struct_name = "LoopExitPrimalValueDecoration",
			minOperands = 2,
		},
		intrinsicOp = {
			struct_name = "IntrinsicOpDecoration",
			minOperands = 1,
		},
		TargetSpecificDecoration = {
			TargetSpecificDefinitionDecoration = {
				target = {
					struct_name = "TargetDecoration",
					minOperands = 1,
				},
				targetIntrinsic = {
					struct_name = "TargetIntrinsicDecoration",
					minOperands = 2,
				},
			},
			requirePrelude = {
				struct_name = "RequirePreludeDecoration",
				minOperands = 2,
			},
		},
		glslOuterArray = {
			struct_name = "GLSLOuterArrayDecoration",
			minOperands = 1,
		},
		TargetSystemValue = {
			struct_name = "TargetSystemValueDecoration",
			minOperands = 2,
		},
		interpolationMode = {
			struct_name = "InterpolationModeDecoration",
			minOperands = 1,
		},
		nameHint = {
			struct_name = "NameHintDecoration",
			minOperands = 1,
		},
		PhysicalType = {
			struct_name = "PhysicalTypeDecoration",
			minOperands = 1,
		},
		AlignedAddressDecoration = {
			-- Mark an address instruction as aligned to a specific byte boundary.
			minOperands = 1,
		},
		BinaryInterfaceType = {
			-- Marks a type as being used as binary interface (e.g. shader parameters).
			-- This prevents the legalizeEmptyType() pass from eliminating it on C++/CUDA targets.
			struct_name = "BinaryInterfaceTypeDecoration",
		},
		transitory = {
			-- *  The decorated _instruction_ is transitory. Such a decoration should NEVER be found on an output instruction a module.
			--     Typically used mark an instruction so can be specially handled - say when creating a IRConstant literal, and the payload of
			--     needs to be special cased for lookup.
			struct_name = "TransitoryDecoration",
		},
		ResultWitness = {
			-- The result witness table that the functon's return type is a subtype of an interface.
			-- This is used to keep track of the original witness table in a function that used to
			-- return an existential value but now returns a concrete type after specialization.
			struct_name = "ResultWitnessDecoration",
			minOperands = 1,
		},
		vulkanRayPayload = {
			struct_name = "VulkanRayPayloadDecoration",
		},
		vulkanRayPayloadIn = {
			struct_name = "VulkanRayPayloadInDecoration",
		},
		vulkanHitAttributes = {
			struct_name = "VulkanHitAttributesDecoration",
		},
		vulkanHitObjectAttributes = {
			struct_name = "VulkanHitObjectAttributesDecoration",
		},
		GlobalVariableShadowingGlobalParameterDecoration = {
			minOperands = 2,
		},
		requireSPIRVVersion = {
			struct_name = "RequireSPIRVVersionDecoration",
			minOperands = 1,
		},
		requireGLSLVersion = {
			struct_name = "RequireGLSLVersionDecoration",
			minOperands = 1,
		},
		requireGLSLExtension = {
			struct_name = "RequireGLSLExtensionDecoration",
			minOperands = 1,
		},
		requireWGSLExtension = {
			struct_name = "RequireWGSLExtensionDecoration",
			minOperands = 1,
		},
		requireCUDASMVersion = {
			struct_name = "RequireCUDASMVersionDecoration",
			minOperands = 1,
		},
		requireCapabilityAtom = {
			struct_name = "RequireCapabilityAtomDecoration",
			minOperands = 1,
		},
		HasExplicitHLSLBinding = {
			struct_name = "HasExplicitHLSLBindingDecoration",
		},
		DefaultValue = {
			struct_name = "DefaultValueDecoration",
			minOperands = 1,
		},
		readNone = {
			struct_name = "ReadNoneDecoration",
		},
		vulkanCallablePayload = {
			struct_name = "VulkanCallablePayloadDecoration",
		},
		vulkanCallablePayloadIn = {
			struct_name = "VulkanCallablePayloadInDecoration",
		},
		earlyDepthStencil = {
			struct_name = "EarlyDepthStencilDecoration",
		},
		precise = {
			struct_name = "PreciseDecoration",
		},
		public = {
			struct_name = "PublicDecoration",
		},
		hlslExport = {
			struct_name = "HLSLExportDecoration",
		},
		downstreamModuleExport = {
			struct_name = "DownstreamModuleExportDecoration",
		},
		downstreamModuleImport = {
			struct_name = "DownstreamModuleImportDecoration",
		},
		patchConstantFunc = {
			struct_name = "PatchConstantFuncDecoration",
			minOperands = 1,
		},
		maxTessFactor = {
			struct_name = "MaxTessFactorDecoration",
			minOperands = 1,
		},
		outputControlPoints = {
			struct_name = "OutputControlPointsDecoration",
			minOperands = 1,
		},
		outputTopology = {
			struct_name = "OutputTopologyDecoration",
			minOperands = 2,
		},
		partioning = {
			struct_name = "PartitioningDecoration",
			minOperands = 1,
		},
		domain = {
			struct_name = "DomainDecoration",
			minOperands = 1,
		},
		maxVertexCount = {
			struct_name = "MaxVertexCountDecoration",
			minOperands = 1,
		},
		instance = {
			struct_name = "InstanceDecoration",
			minOperands = 1,
		},
		numThreads = {
			struct_name = "NumThreadsDecoration",
			minOperands = 3,
		},
		waveSize = {
			struct_name = "WaveSizeDecoration",
			minOperands = 1,
		},
		availableInDownstreamIR = {
			struct_name = "AvailableInDownstreamIRDecoration",
			minOperands = 1,
		},
		GeometryInputPrimitiveTypeDecoration = {
			pointPrimitiveType = {
				-- Added to IRParam parameters to an entry point
				struct_name = "PointInputPrimitiveTypeDecoration",
			},
			linePrimitiveType = {
				struct_name = "LineInputPrimitiveTypeDecoration",
			},
			trianglePrimitiveType = {
				struct_name = "TriangleInputPrimitiveTypeDecoration",
			},
			lineAdjPrimitiveType = {
				struct_name = "LineAdjInputPrimitiveTypeDecoration",
			},
			triangleAdjPrimitiveType = {
				struct_name = "TriangleAdjInputPrimitiveTypeDecoration",
			},
		},
		streamOutputTypeDecoration = {
			minOperands = 1,
		},
		entryPoint = {
			-- An `[entryPoint]` decoration marks a function that represents a shader entry point
			struct_name = "EntryPointDecoration",
			minOperands = 2,
		},
		CudaKernel = {
			struct_name = "CudaKernelDecoration",
		},
		CudaHost = {
			struct_name = "CudaHostDecoration",
		},
		TorchEntryPoint = {
			struct_name = "TorchEntryPointDecoration",
		},
		AutoPyBindCUDA = {
			struct_name = "AutoPyBindCudaDecoration",
		},
		CudaKernelFwdDiffRef = {
			struct_name = "CudaKernelForwardDerivativeDecoration",
		},
		CudaKernelBwdDiffRef = {
			struct_name = "CudaKernelBackwardDerivativeDecoration",
		},
		PyBindExportFuncInfo = {
			struct_name = "AutoPyBindExportInfoDecoration",
		},
		PyExportDecoration = {},
		entryPointParam = {
			-- Used to mark parameters that are moved from entry point parameters to global params as coming from the entry
			-- point.
			struct_name = "EntryPointParamDecoration",
		},
		dependsOn = {
			-- A `[dependsOn(x)]` decoration indicates that the parent instruction depends on `x`
			-- even if it does not otherwise reference it.
			struct_name = "DependsOnDecoration",
			minOperands = 1,
		},
		keepAlive = {
			-- A `[keepAlive]` decoration marks an instruction that should not be eliminated.
			struct_name = "KeepAliveDecoration",
		},
		noSideEffect = {
			-- A `[NoSideEffect]` decoration marks a callee to be side-effect free.
			struct_name = "NoSideEffectDecoration",
		},
		bindExistentialSlots = {
			struct_name = "BindExistentialSlotsDecoration",
		},
		format = {
			-- A `[format(f)]` decoration specifies that the format of an image should be `f`
			struct_name = "FormatDecoration",
			minOperands = 1,
		},
		unsafeForceInlineEarly = {
			-- An `[unsafeForceInlineEarly]` decoration specifies that calls to this function should be inline after initial
			-- codegen
			struct_name = "UnsafeForceInlineEarlyDecoration",
		},
		ForceInline = {
			-- A `[ForceInline]` decoration indicates the callee should be inlined by the Slang compiler.
			struct_name = "ForceInlineDecoration",
		},
		ForceUnroll = {
			-- A `[ForceUnroll]` decoration indicates the loop should be unrolled by the Slang compiler.
			struct_name = "ForceUnrollDecoration",
		},
		SizeAndAlignment = {
			-- A `[SizeAndAlignment(l,s,a)]` decoration is attached to a type to indicate that is has size `s` and alignment
			-- `a` under layout rules `l`.
			struct_name = "SizeAndAlignmentDecoration",
			minOperands = 3,
		},
		Offset = {
			-- A `[Offset(l, o)]` decoration is attached to a field to indicate that it has offset `o` in the parent type
			-- under layout rules `l`.
			struct_name = "OffsetDecoration",
			minOperands = 2,
		},
		LinkageDecoration = {
			import = {
				struct_name = "ImportDecoration",
				minOperands = 1,
			},
			export = {
				struct_name = "ExportDecoration",
				minOperands = 1,
			},
		},
		TargetBuiltinVar = {
			-- Mark a global variable as a target builtin variable.
			struct_name = "TargetBuiltinVarDecoration",
			minOperands = 1,
		},
		UserExtern = {
			-- Marks an inst as coming from an `extern` symbol defined in the user code.
			struct_name = "UserExternDecoration",
		},
		externCpp = {
			-- An extern_cpp decoration marks the inst to emit its name without mangling for C++ interop.
			struct_name = "ExternCppDecoration",
			minOperands = 1,
		},
		externC = {
			-- An externC decoration marks a function should be emitted inside an extern "C" block.
			struct_name = "ExternCDecoration",
		},
		dllImport = {
			-- An dllImport decoration marks a function as imported from a DLL. Slang will generate dynamic function loading
			-- logic to use this function at runtime.
			struct_name = "DllImportDecoration",
			minOperands = 2,
		},
		dllExport = {
			-- An dllExport decoration marks a function as an export symbol. Slang will generate a native wrapper function
			-- that is exported to DLL.
			struct_name = "DllExportDecoration",
			minOperands = 1,
		},
		cudaDeviceExport = {
			-- An cudaDeviceExport decoration marks a function to be exported as a cuda __device__ function.
			struct_name = "CudaDeviceExportDecoration",
			minOperands = 1,
		},
		COMInterface = {
			-- Marks an interface as a COM interface declaration.
			struct_name = "ComInterfaceDecoration",
		},
		KnownBuiltinDecoration = {
			-- Attaches a name to this instruction so that it can be identified
			-- later in the compiler reliably
			minOperands = 1,
		},
		RTTI_typeSize = {
			-- Decorations for RTTI objects
			struct_name = "RTTITypeSizeDecoration",
			minOperands = 1,
		},
		AnyValueSize = {
			struct_name = "AnyValueSizeDecoration",
			minOperands = 1,
		},
		SpecializeDecoration = {},
		SequentialIDDecoration = {
			minOperands = 1,
		},
		DynamicDispatchWitnessDecoration = {},
		StaticRequirementDecoration = {},
		DispatchFuncDecoration = {
			minOperands = 1,
		},
		TypeConstraintDecoration = {
			minOperands = 1,
		},
		BuiltinDecoration = {},
		requiresNVAPI = {
			-- The decorated instruction requires NVAPI to be included via prelude when compiling for D3D.
			struct_name = "RequiresNVAPIDecoration",
		},
		nvapiMagic = {
			-- The decorated instruction is part of the NVAPI "magic" and should always use its original name
			struct_name = "NVAPIMagicDecoration",
			minOperands = 1,
		},
		nvapiSlot = {
			-- A decoration that applies to an entire IR module, and indicates the register/space binding
			-- that the NVAPI shader parameter intends to use.
			struct_name = "NVAPISlotDecoration",
			minOperands = 2,
		},
		noInline = {
			-- Applie to an IR function and signals that inlining should not be performed unless unavoidable.
			struct_name = "NoInlineDecoration",
		},
		noRefInline = {
			struct_name = "NoRefInlineDecoration",
		},
		DerivativeGroupQuad = {
			struct_name = "DerivativeGroupQuadDecoration",
		},
		DerivativeGroupLinear = {
			struct_name = "DerivativeGroupLinearDecoration",
		},
		MaximallyReconverges = {
			struct_name = "MaximallyReconvergesDecoration",
		},
		QuadDerivatives = {
			struct_name = "QuadDerivativesDecoration",
		},
		RequireFullQuads = {
			struct_name = "RequireFullQuadsDecoration",
		},
		TempCallArgVar = {
			struct_name = "TempCallArgVarDecoration",
		},
		nonCopyable = {
			-- Marks a type to be non copyable, causing SSA pass to skip turning variables of the the type into SSA values.
			struct_name = "NonCopyableTypeDecoration",
		},
		DynamicUniform = {
			-- Marks a value to be dynamically uniform.
			struct_name = "DynamicUniformDecoration",
		},
		alwaysFold = {
			-- A call to the decorated function should always be folded into its use site.
			struct_name = "AlwaysFoldIntoUseSiteDecoration",
		},
		output = {
			struct_name = "GlobalOutputDecoration",
		},
		input = {
			struct_name = "GlobalInputDecoration",
		},
		glslLocation = {
			struct_name = "GLSLLocationDecoration",
			minOperands = 1,
		},
		glslOffset = {
			struct_name = "GLSLOffsetDecoration",
			minOperands = 1,
		},
		vkStructOffset = {
			struct_name = "VkStructOffsetDecoration",
			minOperands = 1,
		},
		payload = {
			struct_name = "PayloadDecoration",
		},
		raypayload = {
			struct_name = "RayPayloadDecoration",
		},
		MeshOutputDecoration = {
			-- Mesh Shader outputs
			vertices = {
				struct_name = "VerticesDecoration",
				minOperands = 1,
			},
			indices = {
				struct_name = "IndicesDecoration",
				minOperands = 1,
			},
			primitives = {
				struct_name = "PrimitivesDecoration",
				minOperands = 1,
			},
		},
		HLSLMeshPayloadDecoration = {
			struct_name = "HLSLMeshPayloadDecoration",
		},
		perprimitive = {
			struct_name = "GLSLPrimitivesRateDecoration",
		},
		PositionOutput = {
			-- Marks an inst that represents the gl_Position output.
			struct_name = "GLPositionOutputDecoration",
		},
		PositionInput = {
			-- Marks an inst that represents the gl_Position input.
			struct_name = "GLPositionInputDecoration",
		},
		PerVertex = {
			-- Marks a fragment shader input as per-vertex.
			struct_name = "PerVertexDecoration",
		},
		StageAccessDecoration = {
			stageReadAccess = {
				struct_name = "StageReadAccessDecoration",
			},
			stageWriteAccess = {
				struct_name = "StageWriteAccessDecoration",
			},
		},
		semantic = {
			struct_name = "SemanticDecoration",
			minOperands = 2,
		},
		constructor = {
			struct_name = "ConstructorDecoration",
			minOperands = 1,
		},
		method = {
			struct_name = "MethodDecoration",
		},
		packoffset = {
			struct_name = "PackOffsetDecoration",
			minOperands = 2,
		},
		SpecializationConstantDecoration = {
			minOperands = 1,
		},
		UserTypeName = {
			-- Reflection metadata for a shader parameter that provides the original type name.
			struct_name = "UserTypeNameDecoration",
			minOperands = 1,
		},
		CounterBuffer = {
			-- Reflection metadata for a shader parameter that refers to the associated counter buffer of a UAV.
			struct_name = "CounterBufferDecoration",
			minOperands = 1,
		},
		RequireSPIRVDescriptorIndexingExtensionDecoration = {},
		spirvOpDecoration = {
			struct_name = "SPIRVOpDecoration",
			minOperands = 1,
		},
		forwardDifferentiable = {
			-- Decorated function is marked for the forward-mode differentiation pass.
			struct_name = "ForwardDifferentiableDecoration",
		},
		AutoDiffOriginalValueDecoration = {
			-- Decorates a auto-diff transcribed value with the original value that the inst is transcribed from.
			minOperands = 1,
		},
		AutoDiffBuiltinDecoration = {
			-- Decorates a type as auto-diff builtin type.
		},
		fwdDerivative = {
			-- Used by the auto-diff pass to hold a reference to the
			-- generated derivative function.
			struct_name = "ForwardDerivativeDecoration",
			minOperands = 1,
		},
		backwardDifferentiable = {
			-- Used by the auto-diff pass to hold a reference to the
			-- generated derivative function.
			struct_name = "BackwardDifferentiableDecoration",
			minOperands = 1,
		},
		primalSubstFunc = {
			-- Used by the auto-diff pass to hold a reference to the
			-- primal substitute function.
			struct_name = "PrimalSubstituteDecoration",
			minOperands = 1,
		},
		backwardDiffPrimalReference = {
			-- Decorations to associate an original function with compiler generated backward derivative functions.
			struct_name = "BackwardDerivativePrimalDecoration",
			minOperands = 1,
		},
		backwardDiffPropagateReference = {
			struct_name = "BackwardDerivativePropagateDecoration",
			minOperands = 1,
		},
		backwardDiffIntermediateTypeReference = {
			struct_name = "BackwardDerivativeIntermediateTypeDecoration",
			minOperands = 1,
		},
		backwardDiffReference = {
			struct_name = "BackwardDerivativeDecoration",
			minOperands = 1,
		},
		userDefinedBackwardDiffReference = {
			struct_name = "UserDefinedBackwardDerivativeDecoration",
			minOperands = 1,
		},
		BackwardDerivativePrimalContextDecoration = {
			minOperands = 1,
		},
		BackwardDerivativePrimalReturnDecoration = {
			minOperands = 1,
		},
		PrimalContextDecoration = {
			-- Mark a parameter as autodiff primal context.
		},
		loopCounterDecoration = {},
		loopCounterUpdateDecoration = {},
		AutodiffInstDecoration = {
			-- Auto-diff inst decorations
			primalInstDecoration = {
				-- Used by the auto-diff pass to mark insts that compute
				-- a primal value.
			},
			diffInstDecoration = {
				-- Used by the auto-diff pass to mark insts that compute
				-- a differential value.
				struct_name = "DifferentialInstDecoration",
				minOperands = 1,
			},
			mixedDiffInstDecoration = {
				-- Used by the auto-diff pass to mark insts that compute
				-- BOTH a differential and a primal value.
				struct_name = "MixedDifferentialInstDecoration",
				minOperands = 1,
			},
			RecomputeBlockDecoration = {},
		},
		primalValueKey = {
			-- Used by the auto-diff pass to mark insts whose result is stored
			-- in an intermediary struct for reuse in backward propagation phase.
			struct_name = "PrimalValueStructKeyDecoration",
			minOperands = 1,
		},
		primalElementType = {
			-- Used by the auto-diff pass to mark the primal element type of an
			-- forward-differentiated updateElement inst.
			struct_name = "PrimalElementTypeDecoration",
			minOperands = 1,
		},
		IntermediateContextFieldDifferentialTypeDecoration = {
			-- Used by the auto-diff pass to mark the differential type of an intermediate context field.
			minOperands = 1,
		},
		derivativeMemberDecoration = {
			-- Used by the auto-diff pass to hold a reference to a
			-- differential member of a type in its associated differential type.
			minOperands = 1,
		},
		treatAsDifferentiableDecoration = {
			-- Treat a function as differentiable function
		},
		treatCallAsDifferentiableDecoration = {
			-- Treat a call to arbitrary function as a differentiable call.
		},
		differentiableCallDecoration = {
			-- Mark a call as explicitly calling a differentiable function.
		},
		optimizableTypeDecoration = {
			-- Mark a type as being eligible for trimming if necessary. If
			-- any fields don't have any effective loads from them, they can be
			-- removed.
		},
		ignoreSideEffectsDecoration = {
			-- Informs the DCE pass to ignore side-effects on this call for
			-- the purposes of dead code elimination, even if the call does have
			-- side-effects.
		},
		CheckpointHintDecoration = {
			PreferCheckpointDecoration = {
				-- Hint that the result from a call to the decorated function should be stored in backward prop function.
			},
			PreferRecomputeDecoration = {
				-- Hint that the result from a call to the decorated function should be recomputed in backward prop function.
			},
			CheckpointIntermediateDecoration = {
				-- Hint that a struct is used for reverse mode checkpointing
				minOperands = 1,
			},
		},
		NonDynamicUniformReturnDecoration = {
			-- Marks a function whose return value is never dynamic uniform.
		},
		COMWitnessDecoration = {
			-- Marks a class type as a COM interface implementation, which enables
			-- the witness table to be easily picked up by emit.
			minOperands = 1,
		},
		DifferentiableTypeDictionaryDecoration = {
			-- Differentiable Type Dictionary
			parent = true,
		},
		FloatingPointModeOverride = {
			-- Overrides the floating mode for the target function
			struct_name = "FloatingPointModeOverrideDecoration",
			minOperands = 1,
		},
		spvBufferBlock = {
			-- Recognized by SPIRV-emit pass so we can emit a SPIRV `BufferBlock` decoration.
			struct_name = "SPIRVBufferBlockDecoration",
		},
		DebugLocation = {
			-- Decorates an inst with a debug source location (IRDebugSource, IRIntLit(line), IRIntLit(col)).
			struct_name = "DebugLocationDecoration",
			minOperands = 3,
		},
		DebugFunction = {
			-- Decorates a function with a link to its debug function representation
			struct_name = "DebugFunctionDecoration",
			minOperands = 1,
		},
		spvBlock = {
			-- Recognized by SPIRV-emit pass so we can emit a SPIRV `Block` decoration.
			struct_name = "SPIRVBlockDecoration",
		},
		NonUniformResource = {
			-- Decorates a SPIRV-inst as `NonUniformResource` to guarantee non-uniform index lookup of
			-- - a resource within an array of resources via IRGetElement.
			-- - an IRLoad that takes a pointer within a memory buffer via IRGetElementPtr.
			-- - an IRIntCast to a resource that is casted from signed to unsigned or viceversa.
			-- - an IRGetElementPtr itself when using the pointer on an intrinsic operation.
			struct_name = "SPIRVNonUniformResourceDecoration",
		},
		MemoryQualifierSetDecoration = {
			-- Stores flag bits of which memory qualifiers an object has
			minOperands = 1,
		},
		BitFieldAccessorDecoration = {
			-- Marks a function as one which access a bitfield with the specified
			-- backing value key, width and offset
			minOperands = 3,
		},
	},
	-- Decoration
	-- A `makeExistential(v : C, w) : I` instruction takes a value `v` of type `C`
	-- and produces a value of interface type `I` by using the witness `w` which
	-- shows that `C` conforms to `I`.
	makeExistential = {
		minOperands = 2,
	},
	-- A `MakeExistentialWithRTTI(v, w, t)` is the same with `MakeExistential`,
	-- but with the type of `v` being an explict operand.
	makeExistentialWithRTTI = {
		minOperands = 3,
	},
	-- A 'CreateExistentialObject<I>(typeID, T)` packs user-provided `typeID` and a
	-- value of any type, and constructs an existential value of type `I`.
	createExistentialObject = {
		minOperands = 2,
	},
	-- A `wrapExistential(v, T0,w0, T1,w0) : T` instruction is similar to `makeExistential`.
	-- but applies to a value `v` that is of type `BindExistentials(T, T0,w0, ...)`. The
	-- result of the `wrapExistentials` operation is a value of type `T`, allowing us to
	-- "smuggle" a value of specialized type into computations that expect an unspecialized type.
	wrapExistential = {
		minOperands = 1,
	},
	-- A `GetValueFromBoundInterface` takes a `BindInterface<I, T, w0>` value and returns the
	-- value of concrete type `T` value that is being stored.
	getValueFromBoundInterface = {
		minOperands = 1,
	},
	extractExistentialValue = {
		minOperands = 1,
	},
	extractExistentialType = {
		minOperands = 1,
		hoistable = true,
	},
	extractExistentialWitnessTable = {
		minOperands = 1,
		hoistable = true,
	},
	isNullExistential = {
		minOperands = 1,
	},
	extractTaggedUnionTag = {
		minOperands = 1,
	},
	extractTaggedUnionPayload = {
		minOperands = 1,
	},
	BuiltinCast = {
		minOperands = 1,
	},
	bitCast = {
		minOperands = 1,
	},
	reinterpret = {
		minOperands = 1,
	},
	unmodified = {
		minOperands = 1,
	},
	outImplicitCast = {
		minOperands = 1,
	},
	inOutImplicitCast = {
		minOperands = 1,
	},
	intCast = {
		minOperands = 1,
	},
	floatCast = {
		minOperands = 1,
	},
	castIntToFloat = {
		minOperands = 1,
	},
	castFloatToInt = {
		minOperands = 1,
	},
	CastPtrToBool = {
		minOperands = 1,
	},
	CastPtrToInt = {
		minOperands = 1,
	},
	CastIntToPtr = {
		minOperands = 1,
	},
	castToVoid = {
		minOperands = 1,
	},
	PtrCast = {
		minOperands = 1,
	},
	CastEnumToInt = {
		minOperands = 1,
	},
	CastIntToEnum = {
		minOperands = 1,
	},
	EnumCast = {
		minOperands = 1,
	},
	CastUInt2ToDescriptorHandle = {
		minOperands = 1,
	},
	CastDescriptorHandleToUInt2 = {
		minOperands = 1,
	},
	-- Represents a no-op cast to convert a resource pointer to a resource on targets where the resource handles are
	-- already concrete types.
	CastDescriptorHandleToResource = {
		minOperands = 1,
	},
	TreatAsDynamicUniform = {
		minOperands = 1,
	},
	sizeOf = {
		minOperands = 1,
	},
	alignOf = {
		minOperands = 1,
	},
	countOf = {
		minOperands = 1,
	},
	GetArrayLength = {
		minOperands = 1,
	},
	IsType = {
		minOperands = 3,
	},
	TypeEquals = {
		minOperands = 2,
	},
	IsInt = {
		minOperands = 1,
	},
	IsBool = {
		minOperands = 1,
	},
	IsFloat = {
		minOperands = 1,
	},
	IsHalf = {
		minOperands = 1,
	},
	IsUnsignedInt = {
		minOperands = 1,
	},
	IsSignedInt = {
		minOperands = 1,
	},
	IsVector = {
		minOperands = 1,
	},
	GetDynamicResourceHeap = {
		hoistable = true,
	},
	ForwardDifferentiate = {
		minOperands = 1,
	},
	-- Produces the primal computation of backward derivatives, will return an intermediate context for
	-- backward derivative func.
	BackwardDifferentiatePrimal = {
		minOperands = 1,
	},
	-- Produces the actual backward derivative propagate function, using the intermediate context returned by the
	-- primal func produced from `BackwardDifferentiatePrimal`.
	BackwardDifferentiatePropagate = {
		minOperands = 1,
	},
	-- Represents the conceptual backward derivative function. Only produced by lower-to-ir and will be
	-- replaced with `BackwardDifferentiatePrimal` and `BackwardDifferentiatePropagate`.
	BackwardDifferentiate = {
		minOperands = 1,
	},
	PrimalSubstitute = {
		minOperands = 1,
	},
	DispatchKernel = {
		minOperands = 3,
	},
	CudaKernelLaunch = {
		minOperands = 6,
	},
	-- Converts other resources (such as ByteAddressBuffer) to the equivalent StructuredBuffer
	getEquivalentStructuredBuffer = {
		minOperands = 1,
	},
	-- Gets a T[] pointer to the underlying data of a StructuredBuffer etc...
	getStructuredBufferPtr = {
		minOperands = 1,
	},
	-- Gets a uint[] pointer to the underlying data of a ByteAddressBuffer etc...
	getUntypedBufferPtr = {
		minOperands = 1,
	},
	Layout = {
		hoistable = true,
		varLayout = {
			minOperands = 1,
		},
		TypeLayout = {
			typeLayout = {
				struct_name = "TypeLayoutBase",
			},
			parameterGroupTypeLayout = {
				minOperands = 2,
			},
			arrayTypeLayout = {
				minOperands = 1,
			},
			streamOutputTypeLayout = {
				minOperands = 1,
			},
			matrixTypeLayout = {
				minOperands = 1,
			},
			existentialTypeLayout = {},
			structTypeLayout = {},
			tupleTypeLayout = {},
			structuredBufferTypeLayout = {
				minOperands = 1,
			},
			ptrTypeLayout = {
				-- "TODO(JS): Ideally we'd have the layout to the pointed to value type (ie 1 instead of 0 here). But to
				-- avoid infinite recursion we don't."
				struct_name = "PointerTypeLayout",
			},
		},
		EntryPointLayout = {
			minOperands = 1,
		},
	},
	Attr = {
		hoistable = true,
		pendingLayout = {
			struct_name = "PendingLayoutAttr",
			minOperands = 1,
		},
		stage = {
			struct_name = "StageAttr",
			minOperands = 1,
		},
		structFieldLayout = {
			struct_name = "StructFieldLayoutAttr",
			minOperands = 2,
		},
		tupleFieldLayout = {
			struct_name = "TupleFieldLayoutAttr",
			minOperands = 1,
		},
		caseLayout = {
			struct_name = "CaseTypeLayoutAttr",
			minOperands = 1,
		},
		unorm = {
			struct_name = "UNormAttr",
		},
		snorm = {
			struct_name = "SNormAttr",
		},
		no_diff = {
			struct_name = "NoDiffAttr",
		},
		nonuniform = {
			struct_name = "NonUniformAttr",
		},
		Aligned = {
			struct_name = "AlignedAttr",
			minOperands = 1,
		},
		SemanticAttr = {
			userSemantic = {
				struct_name = "UserSemanticAttr",
				minOperands = 2,
			},
			systemValueSemantic = {
				struct_name = "SystemValueSemanticAttr",
				minOperands = 2,
			},
		},
		LayoutResourceInfoAttr = {
			size = {
				struct_name = "TypeSizeAttr",
				minOperands = 2,
			},
			offset = {
				struct_name = "VarOffsetAttr",
				minOperands = 2,
			},
		},
		FuncThrowType = {
			struct_name = "FuncThrowTypeAttr",
			minOperands = 1,
		},
	},
	-- Liveness
	LiveRangeMarker = {
		liveRangeStart = {
			minOperands = 2,
		},
		liveRangeEnd = {},
	},
	-- IRSpecialization
	SpecializationDictionaryItem = {},
	GenericSpecializationDictionary = {
		parent = true,
	},
	ExistentialFuncSpecializationDictionary = {
		parent = true,
	},
	ExistentialTypeSpecializationDictionary = {
		parent = true,
	},
	-- Differentiable Type Dictionary
	DifferentiableTypeDictionaryItem = {},
	-- Differentiable Type Annotation (for run-time types)
	DifferentiableTypeAnnotation = {
		minOperands = 2,
		hoistable = true,
	},
	BeginFragmentShaderInterlock = {},
	EndFragmentShaderInterlock = {
		struct_name = "EndFragmentShaderInterlock",
	},
	-- DebugInfo
	DebugSource = {
		minOperands = 2,
		hoistable = true,
	},
	DebugLine = {
		minOperands = 5,
	},
	DebugVar = {
		minOperands = 4,
	},
	DebugValue = {
		minOperands = 2,
	},
	DebugInlinedAt = {
		minOperands = 5,
	},
	DebugFunction = {
		minOperands = 5,
	},
	DebugInlinedVariable = {
		minOperands = 2,
	},
	DebugScope = {
		minOperands = 2,
	},
	DebugNoScope = {
		minOperands = 1,
	},
	DebugBuildIdentifier = {
		minOperands = 2,
	},
	-- Embedded Precompiled Libraries
	EmbeddedDownstreamIR = {
		minOperands = 2,
	},
	-- Inline assembly
	SPIRVAsm = {
		parent = true,
	},
	SPIRVAsmInst = {
		minOperands = 1,
	},
	SPIRVAsmOperand = {
		SPIRVAsmOperandLiteral = {
			-- These instruction serve to inform the backend precisely how to emit each
			-- instruction, consider the difference between emitting a literal integer
			-- and a reference to a literal integer instruction
			-- A literal string or 32-bit integer to be passed as operands
			minOperands = 1,
			hoistable = true,
		},
		SPIRVAsmOperandInst = {
			-- A reference to a slang IRInst, either a value or a type
			-- This isn't hoistable, as we sometimes need to change the used value and
			-- instructions around the specific asm block
			minOperands = 1,
		},
		SPIRVAsmOperandConvertTexel = {
			minOperands = 1,
		},
		SPIRVAsmOperandRayPayloadFromLocation = {
			-- a late resolving type to handle the case of ray objects (resolving late due to constexpr data requirment)
			minOperands = 1,
		},
		SPIRVAsmOperandRayAttributeFromLocation = {
			minOperands = 1,
		},
		SPIRVAsmOperandRayCallableFromLocation = {
			minOperands = 1,
		},
		SPIRVAsmOperandEnum = {
			-- A named enumerator, the value is stored as a constant operand
			-- It may have a second operand, which if present is a type with which to
			-- construct a constant id to pass, instead of a literal constant
			minOperands = 1,
			hoistable = true,
		},
		SPIRVAsmOperandBuiltinVar = {
			-- A reference to a builtin variable.
			minOperands = 1,
			hoistable = true,
		},
		SPIRVAsmOperandGLSL450Set = {
			-- A reference to the glsl450 instruction set.
			hoistable = true,
		},
		SPIRVAsmOperandDebugPrintfSet = {
			hoistable = true,
		},
		SPIRVAsmOperandId = {
			-- A string which is given a unique ID in the backend, used to refer to
			-- results of other instrucions in the same asm block
			minOperands = 1,
			hoistable = true,
		},
		SPIRVAsmOperandResult = {
			-- A special instruction which marks the place to insert the generated
			-- result operand
			hoistable = true,
		},
		["__truncate"] = {
			-- A special instruction which represents a type directed truncation
			-- operation where extra components are dropped
			struct_name = "SPIRVAsmOperandTruncate",
			hoistable = true,
		},
		["__entryPoint"] = {
			-- A special instruction which represents an ID of an entry point that references the current function.
			struct_name = "SPIRVAsmOperandEntryPoint",
			hoistable = true,
		},
		["__sampledType"] = {
			-- A type function which returns the result type of sampling an image of
			-- this component type
			struct_name = "SPIRVAsmOperandSampledType",
			minOperands = 1,
			hoistable = true,
		},
		["__imageType"] = {
			-- A type function which returns the equivalent OpTypeImage type of sampled image value
			struct_name = "SPIRVAsmOperandImageType",
			minOperands = 1,
			hoistable = true,
		},
		["__sampledImageType"] = {
			-- A type function which returns the equivalent OpTypeImage type of sampled image value
			struct_name = "SPIRVAsmOperandSampledImageType",
			minOperands = 1,
			hoistable = true,
		},
	},
}

local function process(insts)
	local flat_insts = {}

	-- Convert to PascalCase
	local function to_pascal_case(str)
		-- Otherwise convert from snake_case or lowercase
		local result = str:gsub("_(.)", function(c)
			return c:upper()
		end)
		return result:sub(1, 1):upper() .. result:sub(2)
	end

	-- Check if an instruction is a leaf
	local function is_leaf(inst)
		for k, v in pairs(inst) do
			if
				type(v) == "table"
				and k ~= "struct_name"
				and k ~= "min_operands"
				and k ~= "hoistable"
				and k ~= "parent"
				and k ~= "useOther"
				and k ~= "global"
			then
				return false
			end
		end
		return true
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

		tbl.is_leaf = true

		for key, value in pairs(tbl) do
			-- Process instruction nodes only
			if
				type(value) == "table"
				and key ~= "struct_name"
				and key ~= "min_operands"
				and key ~= "hoistable"
				and key ~= "parent"
				and key ~= "useOther"
				and key ~= "global"
			then
				tbl.is_leaf = false

				-- Add struct_name if missing
				if not value.struct_name then
					value.struct_name = to_pascal_case(key)
				end

				-- Apply inherited flags
				for flag, flag_value in pairs(current_flags) do
					if value[flag] == nil then
						value[flag] = flag_value
					end
				end

				-- If it's a leaf and doesn't have min_operands, add it
				if is_leaf(value) and value.min_operands == nil then
					value.min_operands = 0
				end

				-- Add to flat list (preorder - parent before children)
				flat_insts[key] = value

				-- Recursively process children
				process_inst(value, current_flags)
			end
		end
	end

	-- Process the entire tree
	process_inst(insts)

	return {
		insts = insts,
		flat_insts = flat_insts,
	}
end

return process(insts)
