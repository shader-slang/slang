// SPDX-FileCopyrightText: The Khronos Group, Inc.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

//TEST:SIMPLE(filecheck=IR): -target ptx -stage compute -entry computeMain -O0 -capability cuda_sm_8_0 -emit-cuda-via-nvvm -dump-ir-after checkUnsupportedInst -validate-ir -o -

// The source APIs must produce ordinary typed casts before NVVM legalization. Match each
// helper by its conversion signature and returned operand, without depending on generated IDs.
// The neighboring runtime fixture checks bit patterns and all vector conversion widths.
// IR: ### AFTER checkUnsupportedInst:
// IR-LABEL: func {{.*}} : Func(Vec(Half, 2 : Int), Vec(Float, 2 : Int))
// IR: param [[INPUT0:%[a-zA-Z0-9_]+]] : Vec(Float, 2 : Int)):
// IR: let [[RESULT0:%[a-zA-Z0-9_]+]] : Vec(Half, 2 : Int) = floatCast([[INPUT0]])
// IR-NEXT: return_val([[RESULT0]])

// IR-LABEL: func {{.*}} : Func(Float, Int)
// IR: param [[INPUT1:%[a-zA-Z0-9_]+]] : Int):
// IR: let [[RESULT1:%[a-zA-Z0-9_]+]] : Float = bitCast([[INPUT1]])
// IR-NEXT: return_val([[RESULT1]])

// IR-LABEL: func {{.*}} : Func(Float, UInt)
// IR: param [[INPUT2:%[a-zA-Z0-9_]+]] : UInt):
// IR: let [[RESULT2:%[a-zA-Z0-9_]+]] : Float = bitCast([[INPUT2]])
// IR-NEXT: return_val([[RESULT2]])

// IR-LABEL: func {{.*}} : Func(Int, Float)
// IR: param [[INPUT3:%[a-zA-Z0-9_]+]] : Float):
// IR: let [[RESULT3:%[a-zA-Z0-9_]+]] : Int = bitCast([[INPUT3]])
// IR-NEXT: return_val([[RESULT3]])

// IR-LABEL: func {{.*}} : Func(Int, UInt)
// IR: param [[INPUT4:%[a-zA-Z0-9_]+]] : UInt):
// IR: let [[RESULT4:%[a-zA-Z0-9_]+]] : Int = bitCast([[INPUT4]])
// IR-NEXT: return_val([[RESULT4]])

// IR-LABEL: func {{.*}} : Func(UInt, Float)
// IR: param [[INPUT5:%[a-zA-Z0-9_]+]] : Float):
// IR: let [[RESULT5:%[a-zA-Z0-9_]+]] : UInt = bitCast([[INPUT5]])
// IR-NEXT: return_val([[RESULT5]])

// IR-LABEL: func {{.*}} : Func(UInt, Int)
// IR: param [[INPUT6:%[a-zA-Z0-9_]+]] : Int):
// IR: let [[RESULT6:%[a-zA-Z0-9_]+]] : UInt = bitCast([[INPUT6]])
// IR-NEXT: return_val([[RESULT6]])

// IR-LABEL: func {{.*}} : Func(Half, Float)
// IR: param [[INPUT7:%[a-zA-Z0-9_]+]] : Float):
// IR: let [[RESULT7:%[a-zA-Z0-9_]+]] : Half = floatCast([[INPUT7]])
// IR-NEXT: return_val([[RESULT7]])

// IR-LABEL: func {{.*}} : Func(Float, Half)
// IR: param [[INPUT8:%[a-zA-Z0-9_]+]] : Half):
// IR: let [[RESULT8:%[a-zA-Z0-9_]+]] : Float = floatCast([[INPUT8]])
// IR-NEXT: return_val([[RESULT8]])

[CUDAKernel]
void computeMain(
    uniform Ptr<float, Access::ReadWrite, AddressSpace::Device> floats,
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> signedInts,
    uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> unsignedInts,
    uniform float realValue,
    uniform int signedValue,
    uniform uint unsignedValue,
    uniform float vectorX,
    uniform float vectorY)
{
    floats[0] = asfloat(signedValue);
    floats[1] = asfloat(unsignedValue);
    signedInts[0] = asint(realValue);
    signedInts[1] = asint(unsignedValue);
    unsignedInts[0] = asuint(realValue);
    unsignedInts[1] = asuint(signedValue);
    floats[2] = f16tof32(f32tof16_(realValue));
    half2 halves = f32tof16_(float2(vectorX, vectorY));
    floats[3] = float(halves.x);
    floats[4] = float(halves.y);
}
