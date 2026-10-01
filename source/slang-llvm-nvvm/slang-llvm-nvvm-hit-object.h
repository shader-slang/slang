// Private OptiX9 state transport. Included after the provider's common validation helpers.
// HitObject values own their snapshot: no pointer into a callee's temporary storage survives.
class HitObjectEmitter
{
public:
    enum Field : unsigned
    {
        Tag,
        SbtIndex,
        Flags,
        TransformCount,
        Gas,
        Ray,
        TraverseData,
        Transforms
    };
    static constexpr unsigned kTransformCapacity = 31;
    static constexpr unsigned kStorageSize = 392;
    static constexpr unsigned kStorageAlignment = 8;

    ModuleState* state;
    llvm::IRBuilder<>& b;
    llvm::Type* u32;
    llvm::Type* u64;
    llvm::Type* f32;
    llvm::StructType* storage;

    explicit HitObjectEmitter(ModuleState* module)
        : state(module)
        , b(module->builder)
        , u32(b.getInt32Ty())
        , u64(b.getInt64Ty())
        , f32(b.getFloatTy())
        , storage(module->hitObjectType)
    {
    }

    static llvm::StructType* getStorageType(ModuleState* state)
    {
        if (!state->hitObjectType)
        {
            auto& c = state->context;
            auto u32 = llvm::Type::getInt32Ty(c);
            auto u64 = llvm::Type::getInt64Ty(c);
            auto f32 = llvm::Type::getFloatTy(c);
            state->hitObjectType = llvm::StructType::create(
                c,
                {u32,
                 u32,
                 u32,
                 u32,
                 u64,
                 llvm::ArrayType::get(f32, 9),
                 llvm::ArrayType::get(u32, 20),
                 llvm::ArrayType::get(u64, kTransformCapacity)},
                "slang.optix9.hit.object");
            assert(
                state->module->getDataLayout().getTypeAllocSize(state->hitObjectType) ==
                kStorageSize);
        }
        return state->hitObjectType;
    }

    llvm::Value* address(llvm::Value* object, unsigned field)
    {
        return b.CreateStructGEP(storage, object, field);
    }
    llvm::Value* load(llvm::Value* object, unsigned field)
    {
        return b.CreateLoad(storage->getElementType(field), address(object, field));
    }
    void store(llvm::Value* object, unsigned field, llvm::Value* value)
    {
        b.CreateStore(value, address(object, field));
    }
    llvm::Value* elementAddress(llvm::Value* object, unsigned field, llvm::Value* index)
    {
        return b.CreateInBoundsGEP(
            storage->getElementType(field),
            address(object, field),
            {b.getInt32(0), index});
    }
    llvm::Value* element(llvm::Value* object, unsigned field, unsigned index)
    {
        auto array = llvm::cast<llvm::ArrayType>(storage->getElementType(field));
        return b.CreateLoad(
            array->getElementType(),
            elementAddress(object, field, b.getInt32(index)));
    }

    // The SDK ABI is scalar register transport. Keep its multiple outputs private as LLVM tuples.
    // All calls observe or change implicit outgoing state, including queries; all retain effects.
    llvm::Value* call(
        llvm::StringRef name,
        llvm::ArrayRef<llvm::Type*> results,
        llvm::ArrayRef<llvm::Value*> args)
    {
        llvm::SmallVector<llvm::Type*, 50> parameters;
        for (auto arg : args)
            parameters.push_back(arg->getType());
        llvm::Type* result = results.empty()       ? b.getVoidTy()
                             : results.size() == 1 ? results[0]
                                                   : llvm::StructType::get(state->context, results);
        auto type = llvm::FunctionType::get(result, parameters, false);
        std::string assembly = "call (", constraints;
        for (unsigned i = 0; i < results.size(); ++i)
        {
            if (i)
            {
                assembly += ",";
                constraints += ",";
            }
            assembly += "$" + std::to_string(i);
            constraints += results[i]->isFloatTy()       ? "=f"
                           : results[i]->isIntegerTy(64) ? "=l"
                                                         : "=r";
        }
        assembly += "), " + name.str() + ", (";
        for (unsigned i = 0; i < args.size(); ++i)
        {
            if (i)
                assembly += ",";
            if (!constraints.empty())
                constraints += ",";
            assembly += "$" + std::to_string(results.size() + i);
            constraints += args[i]->getType()->isFloatTy()       ? "f"
                           : args[i]->getType()->isIntegerTy(64) ? "l"
                                                                 : "r";
        }
        assembly += ");";
        if (!constraints.empty())
            constraints += ",";
        constraints += "~{memory}";
        return b.CreateCall(llvm::InlineAsm::get(type, assembly, constraints, true), args);
    }
    llvm::Value* scalar(
        llvm::StringRef name,
        llvm::Type* type,
        llvm::ArrayRef<llvm::Value*> args = {})
    {
        return call(name, {type}, args);
    }
    void effect(llvm::StringRef name, llvm::ArrayRef<llvm::Value*> args = {})
    {
        call(name, {}, args);
    }

    llvm::Function* getStateHelper(bool capture)
    {
        auto name = capture ? "__slang_optix9_capture" : "__slang_optix9_restore";
        auto& cached = capture ? state->hitObjectCapture : state->hitObjectRestore;
        if (cached)
            return cached;
        auto saved = b.saveIP();
        auto functionType =
            llvm::FunctionType::get(b.getVoidTy(), {storage->getPointerTo()}, false);
        auto function = llvm::Function::Create(
            functionType,
            llvm::GlobalValue::InternalLinkage,
            name,
            state->module.get());
        _nameFunctionParameters(function);
        cached = function;
        function->addFnAttr(llvm::Attribute::AlwaysInline);
        auto entry = llvm::BasicBlock::Create(state->context, "entry", function);
        auto hit = llvm::BasicBlock::Create(state->context, "hit", function);
        auto miss = llvm::BasicBlock::Create(state->context, "miss", function);
        auto nop = llvm::BasicBlock::Create(state->context, "nop", function);
        auto done = llvm::BasicBlock::Create(state->context, "done", function);
        b.SetInsertPoint(entry);
        auto object = function->getArg(0);
        llvm::Value* tag;
        if (capture)
        {
            auto isHit = scalar("_optix_hitobject_is_hit", u32);
            auto isMiss = scalar("_optix_hitobject_is_miss", u32);
            tag = b.CreateSelect(
                b.CreateICmpNE(isHit, b.getInt32(0)),
                b.getInt32(1),
                b.CreateSelect(
                    b.CreateICmpNE(isMiss, b.getInt32(0)),
                    b.getInt32(2),
                    b.getInt32(0)));
            store(object, Tag, tag);
        }
        else
            tag = load(object, Tag);
        auto select = b.CreateSwitch(tag, nop, 2);
        select->addCase(b.getInt32(1), hit);
        select->addCase(b.getInt32(2), miss);

        b.SetInsertPoint(nop);
        if (!capture)
            effect("_optix_hitobject_make_nop");
        b.CreateBr(done);

        b.SetInsertPoint(miss);
        if (capture)
            captureRay(object);
        else
            restoreMiss(object);
        b.CreateBr(done);

        b.SetInsertPoint(hit);
        if (capture)
        {
            captureRay(object);
            store(object, Gas, scalar("_optix_hitobject_get_gas_traversable_handle", u64));
            llvm::SmallVector<llvm::Type*, 20> resultTypes(20, u32);
            auto words = call("_optix_hitobject_get_traverse_data", resultTypes, {});
            for (unsigned i = 0; i < 20; ++i)
                b.CreateStore(
                    b.CreateExtractValue(words, i),
                    elementAddress(object, TraverseData, b.getInt32(i)));
            auto count = scalar("_optix_hitobject_get_transform_list_size", u32);
            store(object, TransformCount, count);
            // OptiX9 limits traversable graph depth to31. The complete ordered list is owned here.
            auto test = llvm::BasicBlock::Create(state->context, "transforms.test", function);
            auto body = llvm::BasicBlock::Create(state->context, "transforms.body", function);
            b.CreateBr(test);
            b.SetInsertPoint(test);
            auto index = b.CreatePHI(u32, 2);
            index->addIncoming(b.getInt32(0), hit);
            b.CreateCondBr(b.CreateICmpULT(index, count), body, done);
            b.SetInsertPoint(body);
            auto handle = scalar("_optix_hitobject_get_transform_list_handle", u64, {index});
            b.CreateStore(handle, elementAddress(object, Transforms, index));
            auto next = b.CreateAdd(index, b.getInt32(1));
            b.CreateBr(test);
            index->addIncoming(next, body);
        }
        else
        {
            llvm::SmallVector<llvm::Value*, 32> args;
            args.push_back(load(object, Gas));
            for (unsigned i = 0; i < 7; ++i)
                args.push_back(element(object, Ray, i));
            args.push_back(element(object, Ray, 8));
            args.push_back(load(object, Flags));
            for (unsigned i = 0; i < 20; ++i)
                args.push_back(element(object, TraverseData, i));
            args.push_back(
                b.CreatePtrToInt(elementAddress(object, Transforms, b.getInt32(0)), u64));
            args.push_back(load(object, TransformCount));
            effect("_optix_hitobject_make_with_traverse_data_v2", args);
            // Do not depend on undocumented SBT bits in the opaque traversal words.
            effect("_optix_hitobject_set_sbt_record_index", {load(object, SbtIndex)});
            b.CreateBr(done);
        }
        b.SetInsertPoint(done);
        b.CreateRetVoid();
        b.restoreIP(saved);
        return function;
    }

    void captureRay(llvm::Value* object)
    {
        const char* names[] = {
            "_optix_hitobject_get_world_ray_origin_x",
            "_optix_hitobject_get_world_ray_origin_y",
            "_optix_hitobject_get_world_ray_origin_z",
            "_optix_hitobject_get_world_ray_direction_x",
            "_optix_hitobject_get_world_ray_direction_y",
            "_optix_hitobject_get_world_ray_direction_z",
            "_optix_hitobject_get_ray_tmin",
            "_optix_hitobject_get_ray_tmax",
            "_optix_hitobject_get_ray_time"};
        for (unsigned i = 0; i < 9; ++i)
            b.CreateStore(scalar(names[i], f32), elementAddress(object, Ray, b.getInt32(i)));
        store(object, Flags, scalar("_optix_hitobject_get_ray_flags", u32));
        store(object, SbtIndex, scalar("_optix_hitobject_get_sbt_record_index", u32));
    }
    void restoreMiss(llvm::Value* object)
    {
        llvm::SmallVector<llvm::Value*, 11> args;
        args.push_back(load(object, SbtIndex));
        for (unsigned i = 0; i < 9; ++i)
            args.push_back(element(object, Ray, i));
        args.push_back(load(object, Flags));
        effect("_optix_hitobject_make_miss_v2", args);
    }
    void capture(llvm::Value* object) { b.CreateCall(getStateHelper(true), {object}); }
    void restore(llvm::Value* object) { b.CreateCall(getStateHelper(false), {object}); }

    llvm::Value* payloadCall(bool traverse, uint32_t count, llvm::ArrayRef<llvm::Value*> operands)
    {
        llvm::SmallVector<llvm::Value*, 49> args;
        args.push_back(b.getInt32(0));
        if (traverse)
            args.append(operands.begin(), operands.begin() + 15);
        args.push_back(b.getInt32(count));
        auto offset = traverse ? 15 : 0;
        args.append(operands.begin() + offset, operands.end());
        while (args.size() < (traverse ? 49u : 34u))
            args.push_back(b.getInt32(0));
        llvm::SmallVector<llvm::Type*, 32> types(32, u32);
        auto result =
            call(traverse ? "_optix_hitobject_traverse" : "_optix_hitobject_invoke", types, args);
        if (!count)
            return nullptr;
        llvm::Value* array = llvm::UndefValue::get(llvm::ArrayType::get(u32, count));
        for (unsigned i = 0; i < count; ++i)
            array = b.CreateInsertValue(array, b.CreateExtractValue(result, i), i);
        return array;
    }

    llvm::Value* vectorQuery(llvm::StringRef name, unsigned count, unsigned first = 0)
    {
        llvm::SmallVector<llvm::Type*, 8> types(count, f32);
        auto tuple = call(name, types, {});
        unsigned width = count == 8 ? 4 : count;
        llvm::Value* result = llvm::UndefValue::get(llvm::FixedVectorType::get(f32, width));
        for (unsigned i = 0; i < width; ++i)
            result =
                b.CreateInsertElement(result, b.CreateExtractValue(tuple, first + i), uint64_t(i));
        return result;
    }
    struct Matrix
    {
        llvm::Value* values[3][4];
    };
    llvm::Value* real(float value) { return llvm::ConstantFP::get(f32, value); }
    Matrix identityMatrix()
    {
        Matrix result;
        for (unsigned r = 0; r < 3; ++r)
            for (unsigned c = 0; c < 4; ++c)
                result.values[r][c] = real(r == c ? 1 : 0);
        return result;
    }
    llvm::ArrayType* matrixType()
    {
        return llvm::ArrayType::get(llvm::FixedVectorType::get(f32, 4), 3);
    }
    llvm::Value* packMatrix(const Matrix& matrix)
    {
        llvm::Value* result = llvm::UndefValue::get(matrixType());
        for (unsigned r = 0; r < 3; ++r)
        {
            llvm::Value* row = llvm::UndefValue::get(matrixType()->getElementType());
            for (unsigned c = 0; c < 4; ++c)
                row = b.CreateInsertElement(row, matrix.values[r][c], uint64_t(c));
            result = b.CreateInsertValue(result, row, r);
        }
        return result;
    }
    Matrix unpackMatrix(llvm::Value* value)
    {
        Matrix result;
        for (unsigned r = 0; r < 3; ++r)
        {
            auto row = b.CreateExtractValue(value, r);
            for (unsigned c = 0; c < 4; ++c)
                result.values[r][c] = b.CreateExtractElement(row, uint64_t(c));
        }
        return result;
    }
    Matrix multiplyMatrix(const Matrix& left, const Matrix& right)
    {
        Matrix result;
        for (unsigned r = 0; r < 3; ++r)
            for (unsigned c = 0; c < 4; ++c)
            {
                llvm::Value* value = b.CreateFMul(left.values[r][0], right.values[0][c]);
                for (unsigned k = 1; k < 3; ++k)
                    value =
                        b.CreateFAdd(value, b.CreateFMul(left.values[r][k], right.values[k][c]));
                if (c == 3)
                    value = b.CreateFAdd(value, left.values[r][3]);
                result.values[r][c] = value;
            }
        return result;
    }
    Matrix inverseMatrix(const Matrix& input)
    {
        Matrix result;
        for (unsigned r = 0; r < 3; ++r)
            for (unsigned c = 0; c < 3; ++c)
                result.values[r][c] = b.CreateFSub(
                    b.CreateFMul(
                        input.values[(c + 1) % 3][(r + 1) % 3],
                        input.values[(c + 2) % 3][(r + 2) % 3]),
                    b.CreateFMul(
                        input.values[(c + 1) % 3][(r + 2) % 3],
                        input.values[(c + 2) % 3][(r + 1) % 3]));
        llvm::Value* determinant = b.CreateFMul(input.values[0][0], result.values[0][0]);
        for (unsigned k = 1; k < 3; ++k)
            determinant =
                b.CreateFAdd(determinant, b.CreateFMul(input.values[0][k], result.values[k][0]));
        auto reciprocal = b.CreateFDiv(real(1), determinant);
        for (unsigned r = 0; r < 3; ++r)
        {
            for (unsigned c = 0; c < 3; ++c)
                result.values[r][c] = b.CreateFMul(result.values[r][c], reciprocal);
            llvm::Value* translation = b.CreateFMul(result.values[r][0], input.values[0][3]);
            for (unsigned k = 1; k < 3; ++k)
                translation = b.CreateFAdd(
                    translation,
                    b.CreateFMul(result.values[r][k], input.values[k][3]));
            result.values[r][3] = b.CreateFNeg(translation);
        }
        return result;
    }
    llvm::Value* readWord(llvm::Value* base, llvm::Value* offset)
    {
        auto address = b.CreateAdd(base, offset);
        auto type = llvm::FunctionType::get(u32, {u64}, false);
        auto primitive = llvm::InlineAsm::get(
            type,
            "{ .reg .b64 p; cvta.to.global.u64 p, $1; ld.global.u32 $0, [p]; }",
            "=r,l,~{memory}",
            true);
        return b.CreateCall(primitive, {address});
    }
    llvm::Value* readWord(llvm::Value* base, unsigned offset)
    {
        return readWord(base, b.getInt64(offset));
    }
    llvm::Value* readFloat(llvm::Value* base, unsigned offset)
    {
        return b.CreateBitCast(readWord(base, offset), f32);
    }
    Matrix readMatrix(llvm::Value* pointer, unsigned offset)
    {
        Matrix result;
        for (unsigned r = 0; r < 3; ++r)
            for (unsigned c = 0; c < 4; ++c)
                result.values[r][c] = readFloat(pointer, offset + 4 * (r * 4 + c));
        return result;
    }

    // OptiX9 motion keys interpolate stored S/R/T components. Normalize the interpolated
    // quaternion, then form the public affine transform T*R*S; pivot translation is part of S.
    Matrix srtMatrix(llvm::Value* data[16], llvm::Value* weight)
    {
        llvm::Value* q[4];
        llvm::Value* norm = b.CreateFMul(data[9], data[9]);
        for (unsigned i = 1; i < 4; ++i)
            norm = b.CreateFAdd(norm, b.CreateFMul(data[9 + i], data[9 + i]));
        auto sqrt =
            llvm::Intrinsic::getDeclaration(state->module.get(), llvm::Intrinsic::sqrt, {f32});
        auto scale = b.CreateFDiv(real(1), b.CreateCall(sqrt, {norm}));
        auto interpolate = b.CreateFCmpOGT(weight, real(0));
        for (unsigned i = 0; i < 4; ++i)
            q[i] = b.CreateSelect(interpolate, b.CreateFMul(data[9 + i], scale), data[9 + i]);
        Matrix rotation = identityMatrix();
        auto w2 = b.CreateFMul(q[3], q[3]);
        for (unsigned r = 0; r < 3; ++r)
        {
            auto diagonal = b.CreateFAdd(w2, b.CreateFMul(q[r], q[r]));
            for (unsigned c = 0; c < 3; ++c)
                if (c != r)
                    diagonal = b.CreateFSub(diagonal, b.CreateFMul(q[c], q[c]));
            rotation.values[r][r] = diagonal;
            for (unsigned c = 0; c < 3; ++c)
                if (c != r)
                {
                    unsigned k = 3 - r - c;
                    auto product = b.CreateFMul(q[r], q[c]);
                    auto cross = b.CreateFMul(q[k], q[3]);
                    auto sum = c == (r + 1) % 3 ? b.CreateFSub(product, cross)
                                                : b.CreateFAdd(product, cross);
                    rotation.values[r][c] = b.CreateFMul(real(2), sum);
                }
        }
        Matrix scalePivot = identityMatrix();
        unsigned fields[3][4] = {{0, 1, 2, 3}, {0, 4, 5, 6}, {0, 0, 7, 8}};
        for (unsigned r = 0; r < 3; ++r)
            for (unsigned c = r; c < 4; ++c)
                scalePivot.values[r][c] = data[fields[r][c]];
        Matrix result = multiplyMatrix(rotation, scalePivot);
        for (unsigned r = 0; r < 3; ++r)
            result.values[r][3] = b.CreateFAdd(result.values[r][3], data[13 + r]);
        return result;
    }
    Matrix motionMatrix(llvm::Value* pointer, llvm::Value* time, bool srt)
    {
        // Public OptiX9 layout: child at0, numKeys/flags at8, begin/end at12/16, keys at32.
        auto keys = b.CreateAnd(readWord(pointer, 8), b.getInt32(0xffff));
        auto intervals = b.CreateUIToFP(b.CreateSub(keys, b.getInt32(1)), f32);
        auto begin = readFloat(pointer, 12);
        auto end = readFloat(pointer, 16);
        auto position = b.CreateFMul(
            intervals,
            b.CreateFDiv(b.CreateFSub(time, begin), b.CreateFSub(end, begin)));
        auto bounded = b.CreateSelect(b.CreateFCmpOLT(position, real(0)), real(0), position);
        bounded = b.CreateSelect(b.CreateFCmpOGT(bounded, intervals), intervals, bounded);
        bounded = b.CreateSelect(b.CreateFCmpUNO(bounded, bounded), real(0), bounded);
        // Motion-key positions are finite and nonnegative after clamping, so truncation selects
        // the lower key without introducing another floating-point intrinsic contract.
        auto key = b.CreateUIToFP(b.CreateFPToUI(bounded, u32), f32);
        auto last = b.CreateFSub(intervals, real(1));
        auto selected = b.CreateSelect(b.CreateFCmpOGT(key, last), last, key);
        auto weight = b.CreateFSub(bounded, selected);
        auto keyIndex = b.CreateFPToUI(selected, u64);
        unsigned width = srt ? 16 : 12;
        auto base = b.CreateAdd(
            pointer,
            b.CreateAdd(b.getInt64(32), b.CreateMul(keyIndex, b.getInt64(width * 4))));
        llvm::Value* data[16];
        for (unsigned i = 0; i < width; ++i)
        {
            auto left = readFloat(base, i * 4);
            auto right = readFloat(base, (i + width) * 4);
            auto blended = b.CreateFAdd(
                b.CreateFMul(left, b.CreateFSub(real(1), weight)),
                b.CreateFMul(right, weight));
            data[i] = b.CreateSelect(b.CreateFCmpOGT(weight, real(0)), blended, left);
        }
        if (srt)
            return srtMatrix(data, weight);
        Matrix result;
        for (unsigned r = 0; r < 3; ++r)
            for (unsigned c = 0; c < 4; ++c)
                result.values[r][c] = data[r * 4 + c];
        return result;
    }

    llvm::Function* getTransformHelper(bool inverse)
    {
        auto& cached = state->hitObjectTransform[inverse];
        if (cached)
            return cached;
        auto saved = b.saveIP();
        auto type = llvm::FunctionType::get(matrixType(), {u64, f32}, false);
        auto function = llvm::Function::Create(
            type,
            llvm::GlobalValue::InternalLinkage,
            inverse ? "__slang_optix9_inverse_transform" : "__slang_optix9_forward_transform",
            state->module.get());
        _nameFunctionParameters(function);
        cached = function;
        function->addFnAttr(llvm::Attribute::AlwaysInline);
        auto entry = llvm::BasicBlock::Create(state->context, "entry", function);
        auto instance = llvm::BasicBlock::Create(state->context, "instance", function);
        auto fixed = llvm::BasicBlock::Create(state->context, "static", function);
        auto matrixMotion = llvm::BasicBlock::Create(state->context, "matrix.motion", function);
        auto srtMotion = llvm::BasicBlock::Create(state->context, "srt.motion", function);
        auto none = llvm::BasicBlock::Create(state->context, "none", function);
        b.SetInsertPoint(entry);
        auto handle = function->getArg(0);
        auto time = function->getArg(1);
        auto kind = scalar("_optix_get_transform_type_from_handle", u32, {handle});
        auto branch = b.CreateSwitch(kind, none, 4);
        branch->addCase(b.getInt32(1), fixed);
        branch->addCase(b.getInt32(2), matrixMotion);
        branch->addCase(b.getInt32(3), srtMotion);
        branch->addCase(b.getInt32(4), instance);
        b.SetInsertPoint(instance);
        auto pointer = scalar(
            inverse ? "_optix_get_instance_inverse_transform_from_handle"
                    : "_optix_get_instance_transform_from_handle",
            u64,
            {handle});
        b.CreateRet(packMatrix(readMatrix(pointer, 0)));
        b.SetInsertPoint(fixed);
        pointer = scalar("_optix_get_static_transform_from_handle", u64, {handle});
        b.CreateRet(packMatrix(readMatrix(pointer, inverse ? 64 : 16)));
        b.SetInsertPoint(matrixMotion);
        pointer = scalar("_optix_get_matrix_motion_transform_from_handle", u64, {handle});
        auto matrix = motionMatrix(pointer, time, false);
        b.CreateRet(packMatrix(inverse ? inverseMatrix(matrix) : matrix));
        b.SetInsertPoint(srtMotion);
        pointer = scalar("_optix_get_srt_motion_transform_from_handle", u64, {handle});
        matrix = motionMatrix(pointer, time, true);
        b.CreateRet(packMatrix(inverse ? inverseMatrix(matrix) : matrix));
        b.SetInsertPoint(none);
        b.CreateRet(packMatrix(identityMatrix()));
        b.restoreIP(saved);
        return function;
    }
    llvm::Function* getMatrixHelper(bool inverse)
    {
        auto& cached = state->hitObjectMatrix[inverse];
        if (cached)
            return cached;
        auto leaf = getTransformHelper(inverse);
        auto saved = b.saveIP();
        auto type = llvm::FunctionType::get(matrixType(), {storage->getPointerTo()}, false);
        auto function = llvm::Function::Create(
            type,
            llvm::GlobalValue::InternalLinkage,
            inverse ? "__slang_optix9_world_to_object" : "__slang_optix9_object_to_world",
            state->module.get());
        _nameFunctionParameters(function);
        cached = function;
        function->addFnAttr(llvm::Attribute::AlwaysInline);
        auto entry = llvm::BasicBlock::Create(state->context, "entry", function);
        auto test = llvm::BasicBlock::Create(state->context, "test", function);
        auto body = llvm::BasicBlock::Create(state->context, "body", function);
        auto done = llvm::BasicBlock::Create(state->context, "done", function);
        b.SetInsertPoint(entry);
        auto object = function->getArg(0);
        auto isHit = b.CreateICmpEQ(load(object, Tag), b.getInt32(1));
        // Miss/NOP snapshots intentionally need no transform storage contents.
        auto count = b.CreateSelect(isHit, load(object, TransformCount), b.getInt32(0));
        auto time = element(object, Ray, 8);
        auto initial = packMatrix(identityMatrix());
        b.CreateBr(test);
        b.SetInsertPoint(test);
        auto index = b.CreatePHI(u32, 2);
        auto composed = b.CreatePHI(matrixType(), 2);
        index->addIncoming(b.getInt32(0), entry);
        composed->addIncoming(initial, entry);
        b.CreateCondBr(b.CreateICmpULT(index, count), body, done);
        b.SetInsertPoint(body);
        auto selected = inverse ? index : b.CreateSub(b.CreateSub(count, b.getInt32(1)), index);
        auto handle = b.CreateLoad(u64, elementAddress(object, Transforms, selected));
        auto local = b.CreateCall(leaf, {handle, time});
        auto nextMatrix = packMatrix(multiplyMatrix(unpackMatrix(local), unpackMatrix(composed)));
        auto nextIndex = b.CreateAdd(index, b.getInt32(1));
        b.CreateBr(test);
        index->addIncoming(nextIndex, body);
        composed->addIncoming(nextMatrix, body);
        b.SetInsertPoint(done);
        b.CreateRet(composed);
        b.restoreIP(saved);
        return function;
    }
    llvm::Value* matrixRow(llvm::Value* object, bool inverse, unsigned row)
    {
        return b.CreateExtractValue(b.CreateCall(getMatrixHelper(inverse), {object}), row);
    }

    llvm::Value* query(llvm::Value* object, const SlangNVVMHitObjectOperationDesc& desc)
    {
        const char* integerNames[] = {
            "_optix_hitobject_is_hit",
            "_optix_hitobject_is_miss",
            "_optix_hitobject_is_nop",
            "_optix_hitobject_get_instance_id",
            "_optix_hitobject_get_instance_idx",
            "_optix_hitobject_get_sbt_gas_idx",
            "_optix_hitobject_get_primitive_idx",
            "_optix_hitobject_get_hitkind",
            "_optix_hitobject_get_sbt_record_index",
            "_optix_hitobject_get_ray_flags"};
        if (desc.query <= SLANG_NVVM_HIT_OBJECT_QUERY_RAY_FLAGS)
            return scalar(integerNames[desc.query], u32);
        if (desc.query == SLANG_NVVM_HIT_OBJECT_QUERY_WORLD_ORIGIN ||
            desc.query == SLANG_NVVM_HIT_OBJECT_QUERY_WORLD_DIRECTION)
        {
            llvm::Value* result = llvm::UndefValue::get(llvm::FixedVectorType::get(f32, 3));
            const char* suffix[] = {"_x", "_y", "_z"};
            const char* name = desc.query == SLANG_NVVM_HIT_OBJECT_QUERY_WORLD_ORIGIN
                                   ? "_optix_hitobject_get_world_ray_origin"
                                   : "_optix_hitobject_get_world_ray_direction";
            for (unsigned i = 0; i < 3; ++i)
                result = b.CreateInsertElement(
                    result,
                    scalar(std::string(name) + suffix[i], f32),
                    uint64_t(i));
            return result;
        }
        switch (desc.query)
        {
        case SLANG_NVVM_HIT_OBJECT_QUERY_IS_SPHERE:
        case SLANG_NVVM_HIT_OBJECT_QUERY_IS_LSS:
            {
                auto kind = scalar("_optix_hitobject_get_hitkind", u32);
                auto type = scalar("_optix_get_primitive_type_from_hit_kind", u32, {kind});
                return b.CreateZExt(
                    b.CreateICmpEQ(
                        type,
                        b.getInt32(
                            desc.query == SLANG_NVVM_HIT_OBJECT_QUERY_IS_SPHERE ? 0x2506 : 0x2503)),
                    u32);
            }
        case SLANG_NVVM_HIT_OBJECT_QUERY_TMIN:
            return scalar("_optix_hitobject_get_ray_tmin", f32);
        case SLANG_NVVM_HIT_OBJECT_QUERY_TMAX:
            return scalar("_optix_hitobject_get_ray_tmax", f32);
        case SLANG_NVVM_HIT_OBJECT_QUERY_TIME:
            return scalar("_optix_hitobject_get_ray_time", f32);
        case SLANG_NVVM_HIT_OBJECT_QUERY_ATTRIBUTE:
            return scalar("_optix_hitobject_get_attribute", u32, {b.getInt32(desc.index)});
        case SLANG_NVVM_HIT_OBJECT_QUERY_CLUSTER_ID:
            return scalar("_optix_hitobject_get_cluster_id", u32);
        case SLANG_NVVM_HIT_OBJECT_QUERY_SPHERE:
            return vectorQuery("_optix_hitobject_get_sphere_data", 4);
        case SLANG_NVVM_HIT_OBJECT_QUERY_LSS:
            return vectorQuery("_optix_hitobject_get_linear_curve_vertex_data", 8, desc.index * 4);
        case SLANG_NVVM_HIT_OBJECT_QUERY_MATRIX_OBJECT_TO_WORLD:
        case SLANG_NVVM_HIT_OBJECT_QUERY_MATRIX_WORLD_TO_OBJECT:
            return matrixRow(
                object,
                desc.query == SLANG_NVVM_HIT_OBJECT_QUERY_MATRIX_WORLD_TO_OBJECT,
                desc.index);
        default:
            llvm_unreachable("validated hit object query");
        }
    }
};

static SlangResult SLANG_NVVM_CALL _getHitObjectStorageLayout(uint32_t* size, uint32_t* alignment)
{
    if (size)
        *size = 0;
    if (alignment)
        *alignment = 0;
    if (!size || !alignment)
        return SLANG_E_INVALID_ARG;
    *size = HitObjectEmitter::kStorageSize;
    *alignment = HitObjectEmitter::kStorageAlignment;
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL
_getHitObjectType(SlangNVVMModuleHandle module, SlangNVVMTypeHandle* outType)
{
    if (outType)
        *outType = nullptr;
    auto state = _getModule(module);
    if (!state || !outType)
        return SLANG_E_INVALID_ARG;
    *outType = reinterpret_cast<SlangNVVMTypeHandle>(HitObjectEmitter::getStorageType(state));
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL
_isHitObjectOperationSupported(const SlangNVVMHitObjectOperationDesc* desc, uint32_t* outSupported)
{
    if (outSupported)
        *outSupported = 0;
    if (!desc || !outSupported)
        return SLANG_E_INVALID_ARG;
    if (desc->operation < SLANG_NVVM_HIT_OBJECT_OP_MAKE_NOP ||
        desc->operation > SLANG_NVVM_HIT_OBJECT_OP_REPORT_INTERSECTION)
        return SLANG_OK;
    if (desc->operation == SLANG_NVVM_HIT_OBJECT_OP_QUERY)
    {
        if (desc->payloadCount || desc->query > SLANG_NVVM_HIT_OBJECT_QUERY_IS_LSS)
            return SLANG_OK;
        unsigned bound = desc->query == SLANG_NVVM_HIT_OBJECT_QUERY_ATTRIBUTE ? 8
                         : desc->query == SLANG_NVVM_HIT_OBJECT_QUERY_LSS     ? 2
                         : desc->query == SLANG_NVVM_HIT_OBJECT_QUERY_MATRIX_OBJECT_TO_WORLD ||
                                 desc->query == SLANG_NVVM_HIT_OBJECT_QUERY_MATRIX_WORLD_TO_OBJECT
                             ? 3
                             : 1;
        *outSupported = desc->index < bound;
    }
    else if (!desc->query && !desc->index)
    {
        unsigned bound = desc->operation == SLANG_NVVM_HIT_OBJECT_OP_TRAVERSE ||
                                 desc->operation == SLANG_NVVM_HIT_OBJECT_OP_INVOKE
                             ? 32
                         : desc->operation == SLANG_NVVM_HIT_OBJECT_OP_REPORT_INTERSECTION ? 8
                                                                                           : 0;
        *outSupported = desc->payloadCount <= bound;
    }
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _emitHitObjectOperation(
    SlangNVVMModuleHandle module,
    const SlangNVVMHitObjectOperationDesc* desc,
    const SlangNVVMValueHandle* operands,
    size_t operandCount,
    SlangNVVMValueHandle* outValue)
{
    if (outValue)
        *outValue = nullptr;
    auto state = _getModule(module);
    auto block = _getValidInsertionBlock(state);
    uint32_t supported = 0;
    if (SLANG_FAILED(_isHitObjectOperationSupported(desc, &supported)) || !supported || !outValue ||
        !block || (!operands && operandCount))
        return SLANG_E_INVALID_ARG;
    bool hasObject = desc->operation != SLANG_NVVM_HIT_OBJECT_OP_REORDER_HINT &&
                     desc->operation != SLANG_NVVM_HIT_OBJECT_OP_REPORT_INTERSECTION;
    unsigned scalarCount = 0;
    switch (desc->operation)
    {
    case SLANG_NVVM_HIT_OBJECT_OP_MAKE_MISS:
        scalarCount = 11;
        break;
    case SLANG_NVVM_HIT_OBJECT_OP_TRAVERSE:
        scalarCount = 15 + desc->payloadCount;
        break;
    case SLANG_NVVM_HIT_OBJECT_OP_INVOKE:
        scalarCount = desc->payloadCount;
        break;
    case SLANG_NVVM_HIT_OBJECT_OP_SET_SBT_INDEX:
    case SLANG_NVVM_HIT_OBJECT_OP_LOAD_SBT_U32:
        scalarCount = 1;
        break;
    case SLANG_NVVM_HIT_OBJECT_OP_REORDER:
    case SLANG_NVVM_HIT_OBJECT_OP_REORDER_HINT:
        scalarCount = 2;
        break;
    case SLANG_NVVM_HIT_OBJECT_OP_REPORT_INTERSECTION:
        scalarCount = 2 + desc->payloadCount;
        break;
    }
    if (operandCount != scalarCount + unsigned(hasObject))
        return SLANG_E_INVALID_ARG;
    auto u32 = llvm::Type::getInt32Ty(state->context);
    auto u64 = llvm::Type::getInt64Ty(state->context);
    auto f32 = llvm::Type::getFloatTy(state->context);
    llvm::Value* object = nullptr;
    llvm::SmallVector<llvm::Value*, 47> args;
    for (unsigned i = 0; i < operandCount; ++i)
    {
        auto value = _getValue(operands[i]);
        if (!_isValueUsableAtInsertionPoint(state, block, value))
            return SLANG_E_INVALID_ARG;
        if (hasObject && i == 0)
        {
            if (!state->hitObjectType || value->getType() != state->hitObjectType->getPointerTo())
                return SLANG_E_INVALID_ARG;
            object = value;
            continue;
        }
        unsigned index = i - unsigned(hasObject);
        llvm::Type* expected = u32;
        if (desc->operation == SLANG_NVVM_HIT_OBJECT_OP_TRAVERSE)
            expected = index == 0 ? u64 : index < 10 ? f32 : u32;
        else if (desc->operation == SLANG_NVVM_HIT_OBJECT_OP_MAKE_MISS)
            expected = index > 0 && index < 10 ? f32 : u32;
        else if (desc->operation == SLANG_NVVM_HIT_OBJECT_OP_REPORT_INTERSECTION && index == 0)
            expected = f32;
        if (value->getType() != expected)
            return SLANG_E_INVALID_ARG;
        args.push_back(value);
    }
    // Invalid descriptors, values, ownership and insertion points leave the whole module unchanged.
    HitObjectEmitter::getStorageType(state);
    HitObjectEmitter emitter(state);
    auto& b = state->builder;
    llvm::Value* result = nullptr;
    switch (desc->operation)
    {
    case SLANG_NVVM_HIT_OBJECT_OP_MAKE_NOP:
        b.CreateStore(llvm::ConstantAggregateZero::get(emitter.storage), object);
        break;
    case SLANG_NVVM_HIT_OBJECT_OP_MAKE_MISS:
        emitter.effect("_optix_hitobject_make_miss_v2", args);
        emitter.capture(object);
        break;
    case SLANG_NVVM_HIT_OBJECT_OP_TRAVERSE:
        result = emitter.payloadCall(true, desc->payloadCount, args);
        emitter.capture(object);
        break;
    case SLANG_NVVM_HIT_OBJECT_OP_INVOKE:
        emitter.restore(object);
        result = emitter.payloadCall(false, desc->payloadCount, args);
        break;
    case SLANG_NVVM_HIT_OBJECT_OP_QUERY:
        emitter.restore(object);
        result = emitter.query(object, *desc);
        break;
    case SLANG_NVVM_HIT_OBJECT_OP_SET_SBT_INDEX:
        // Restoration applies the SDK setter. Preserve every other object's saved record index.
        emitter.store(object, HitObjectEmitter::SbtIndex, args[0]);
        break;
    case SLANG_NVVM_HIT_OBJECT_OP_LOAD_SBT_U32:
        {
            emitter.restore(object);
            auto base = emitter.scalar("_optix_hitobject_get_sbt_data_pointer", u64);
            auto address = b.CreateAdd(base, b.CreateZExt(args[0], u64));
            auto type = llvm::FunctionType::get(u32, {u64}, false);
            auto load = llvm::InlineAsm::get(
                type,
                "{ .reg .b64 p; cvta.to.global.u64 p, $1; ld.global.u32 $0, [p]; }",
                "=r,l,~{memory}",
                true);
            result = b.CreateCall(load, {address});
        }
        break;
    case SLANG_NVVM_HIT_OBJECT_OP_REORDER:
    case SLANG_NVVM_HIT_OBJECT_OP_REORDER_HINT:
        if (object)
            emitter.restore(object);
        else
            emitter.effect("_optix_hitobject_make_nop");
        emitter.effect("_optix_hitobject_reorder", args);
        break;
    case SLANG_NVVM_HIT_OBJECT_OP_REPORT_INTERSECTION:
        result = emitter.scalar(
            "_optix_report_intersection_" + std::to_string(desc->payloadCount),
            u32,
            args);
        break;
    default:
        llvm_unreachable("validated hit object operation");
    }
    *outValue = reinterpret_cast<SlangNVVMValueHandle>(result);
    return SLANG_OK;
}
