// Copyright (c) 2018 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "pass_fixture.h"
#include "pass_utils.h"

namespace {

using namespace spvtools;

using VectorDCETest = PassTest<::testing::Test>;

TEST_F(VectorDCETest, InsertAfterInsertElim) {
  // With two insertions to the same offset, the first is dead.
  //
  // Note: The SPIR-V assembly has had store/load elimination
  // performed to allow the inserts and extracts to directly
  // reference each other.
  //
  // #version 450
  //
  // layout (location=0) in float In0;
  // layout (location=1) in float In1;
  // layout (location=2) in vec2 In2;
  // layout (location=0) out vec4 OutColor;
  //
  // void main()
  // {
  //     vec2 v = In2;
  //     v.x = In0 + In1; // dead
  //     v.x = 0.0;
  //     OutColor = v.xyxy;
  // }

  const std::string before_predefs =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %In2 %In0 %In1 %OutColor
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 450
OpName %main "main"
OpName %In2 "In2"
OpName %In0 "In0"
OpName %In1 "In1"
OpName %OutColor "OutColor"
OpName %_Globals_ "_Globals_"
OpMemberName %_Globals_ 0 "g_b"
OpMemberName %_Globals_ 1 "g_n"
OpName %_ ""
OpDecorate %In2 Location 2
OpDecorate %In0 Location 0
OpDecorate %In1 Location 1
OpDecorate %OutColor Location 0
OpMemberDecorate %_Globals_ 0 Offset 0
OpMemberDecorate %_Globals_ 1 Offset 4
OpDecorate %_Globals_ Block
OpDecorate %_ DescriptorSet 0
OpDecorate %_ Binding 0
%void = OpTypeVoid
%11 = OpTypeFunction %void
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Input_v2float = OpTypePointer Input %v2float
%In2 = OpVariable %_ptr_Input_v2float Input
%_ptr_Input_float = OpTypePointer Input %float
%In0 = OpVariable %_ptr_Input_float Input
%In1 = OpVariable %_ptr_Input_float Input
%uint = OpTypeInt 32 0
%_ptr_Function_float = OpTypePointer Function %float
%float_0 = OpConstant %float 0
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%OutColor = OpVariable %_ptr_Output_v4float Output
%int = OpTypeInt 32 1
%_Globals_ = OpTypeStruct %uint %int
%_ptr_Uniform__Globals_ = OpTypePointer Uniform %_Globals_
%_ = OpVariable %_ptr_Uniform__Globals_ Uniform
)";

  const std::string after_predefs =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %In2 %In0 %In1 %OutColor
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 450
OpName %main "main"
OpName %In2 "In2"
OpName %In0 "In0"
OpName %In1 "In1"
OpName %OutColor "OutColor"
OpName %_Globals_ "_Globals_"
OpMemberName %_Globals_ 0 "g_b"
OpMemberName %_Globals_ 1 "g_n"
OpName %_ ""
OpDecorate %In2 Location 2
OpDecorate %In0 Location 0
OpDecorate %In1 Location 1
OpDecorate %OutColor Location 0
OpMemberDecorate %_Globals_ 0 Offset 0
OpMemberDecorate %_Globals_ 1 Offset 4
OpDecorate %_Globals_ Block
OpDecorate %_ DescriptorSet 0
OpDecorate %_ Binding 0
%void = OpTypeVoid
%10 = OpTypeFunction %void
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Input_v2float = OpTypePointer Input %v2float
%In2 = OpVariable %_ptr_Input_v2float Input
%_ptr_Input_float = OpTypePointer Input %float
%In0 = OpVariable %_ptr_Input_float Input
%In1 = OpVariable %_ptr_Input_float Input
%uint = OpTypeInt 32 0
%_ptr_Function_float = OpTypePointer Function %float
%float_0 = OpConstant %float 0
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%OutColor = OpVariable %_ptr_Output_v4float Output
%int = OpTypeInt 32 1
%_Globals_ = OpTypeStruct %uint %int
%_ptr_Uniform__Globals_ = OpTypePointer Uniform %_Globals_
%_ = OpVariable %_ptr_Uniform__Globals_ Uniform
)";

  const std::string before =
      R"(%main = OpFunction %void None %11
%25 = OpLabel
%26 = OpLoad %v2float %In2
%27 = OpLoad %float %In0
%28 = OpLoad %float %In1
%29 = OpFAdd %float %27 %28
%35 = OpCompositeInsert %v2float %29 %26 0
%37 = OpCompositeInsert %v2float %float_0 %35 0
%33 = OpVectorShuffle %v4float %37 %37 0 1 0 1
OpStore %OutColor %33
OpReturn
OpFunctionEnd
)";

  const std::string after =
      R"(%main = OpFunction %void None %10
%23 = OpLabel
%24 = OpLoad %v2float %In2
%25 = OpLoad %float %In0
%26 = OpLoad %float %In1
%27 = OpFAdd %float %25 %26
%28 = OpCompositeInsert %v2float %27 %24 0
%29 = OpCompositeInsert %v2float %float_0 %24 0
%30 = OpVectorShuffle %v4float %29 %29 0 1 0 1
OpStore %OutColor %30
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<opt::VectorDCE>(before_predefs + before,
                                        after_predefs + after, true, true);
}

TEST_F(VectorDCETest, DeadInsertInChainWithPhi) {
  // Dead insert eliminated with phi in insertion chain.
  //
  // Note: The SPIR-V assembly has had store/load elimination
  // performed to allow the inserts and extracts to directly
  // reference each other.
  //
  // #version 450
  //
  // layout (location=0) in vec4 In0;
  // layout (location=1) in float In1;
  // layout (location=2) in float In2;
  // layout (location=0) out vec4 OutColor;
  //
  // layout(std140, binding = 0 ) uniform _Globals_
  // {
  //     bool g_b;
  // };
  //
  // void main()
  // {
  //     vec4 v = In0;
  //     v.z = In1 + In2;
  //     if (g_b) v.w = 1.0;
  //     OutColor = vec4(v.x,v.y,0.0,v.w);
  // }

  const std::string before_predefs =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %In0 %In1 %In2 %OutColor
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 450
OpName %main "main"
OpName %In0 "In0"
OpName %In1 "In1"
OpName %In2 "In2"
OpName %_Globals_ "_Globals_"
OpMemberName %_Globals_ 0 "g_b"
OpName %_ ""
OpName %OutColor "OutColor"
OpDecorate %In0 Location 0
OpDecorate %In1 Location 1
OpDecorate %In2 Location 2
OpMemberDecorate %_Globals_ 0 Offset 0
OpDecorate %_Globals_ Block
OpDecorate %_ DescriptorSet 0
OpDecorate %_ Binding 0
OpDecorate %OutColor Location 0
%void = OpTypeVoid
%11 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Input_v4float = OpTypePointer Input %v4float
%In0 = OpVariable %_ptr_Input_v4float Input
%_ptr_Input_float = OpTypePointer Input %float
%In1 = OpVariable %_ptr_Input_float Input
%In2 = OpVariable %_ptr_Input_float Input
%uint = OpTypeInt 32 0
%_ptr_Function_float = OpTypePointer Function %float
%_Globals_ = OpTypeStruct %uint
%_ptr_Uniform__Globals_ = OpTypePointer Uniform %_Globals_
%_ = OpVariable %_ptr_Uniform__Globals_ Uniform
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
%bool = OpTypeBool
%uint_0 = OpConstant %uint 0
%float_1 = OpConstant %float 1
%_ptr_Output_v4float = OpTypePointer Output %v4float
%OutColor = OpVariable %_ptr_Output_v4float Output
%float_0 = OpConstant %float 0
)";

  const std::string after_predefs =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %In0 %In1 %In2 %OutColor
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 450
OpName %main "main"
OpName %In0 "In0"
OpName %In1 "In1"
OpName %In2 "In2"
OpName %_Globals_ "_Globals_"
OpMemberName %_Globals_ 0 "g_b"
OpName %_ ""
OpName %OutColor "OutColor"
OpDecorate %In0 Location 0
OpDecorate %In1 Location 1
OpDecorate %In2 Location 2
OpMemberDecorate %_Globals_ 0 Offset 0
OpDecorate %_Globals_ Block
OpDecorate %_ DescriptorSet 0
OpDecorate %_ Binding 0
OpDecorate %OutColor Location 0
%void = OpTypeVoid
%10 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Input_v4float = OpTypePointer Input %v4float
%In0 = OpVariable %_ptr_Input_v4float Input
%_ptr_Input_float = OpTypePointer Input %float
%In1 = OpVariable %_ptr_Input_float Input
%In2 = OpVariable %_ptr_Input_float Input
%uint = OpTypeInt 32 0
%_ptr_Function_float = OpTypePointer Function %float
%_Globals_ = OpTypeStruct %uint
%_ptr_Uniform__Globals_ = OpTypePointer Uniform %_Globals_
%_ = OpVariable %_ptr_Uniform__Globals_ Uniform
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
%bool = OpTypeBool
%uint_0 = OpConstant %uint 0
%float_1 = OpConstant %float 1
%_ptr_Output_v4float = OpTypePointer Output %v4float
%OutColor = OpVariable %_ptr_Output_v4float Output
%float_0 = OpConstant %float 0
)";

  const std::string before =
      R"(%main = OpFunction %void None %11
%31 = OpLabel
%32 = OpLoad %v4float %In0
%33 = OpLoad %float %In1
%34 = OpLoad %float %In2
%35 = OpFAdd %float %33 %34
%51 = OpCompositeInsert %v4float %35 %32 2
%37 = OpAccessChain %_ptr_Uniform_uint %_ %int_0
%38 = OpLoad %uint %37
%39 = OpINotEqual %bool %38 %uint_0
OpSelectionMerge %40 None
OpBranchConditional %39 %41 %40
%41 = OpLabel
%53 = OpCompositeInsert %v4float %float_1 %51 3
OpBranch %40
%40 = OpLabel
%60 = OpPhi %v4float %51 %31 %53 %41
%55 = OpCompositeExtract %float %60 0
%57 = OpCompositeExtract %float %60 1
%59 = OpCompositeExtract %float %60 3
%49 = OpCompositeConstruct %v4float %55 %57 %float_0 %59
OpStore %OutColor %49
OpReturn
OpFunctionEnd
)";

  const std::string after =
      R"(%main = OpFunction %void None %10
%27 = OpLabel
%28 = OpLoad %v4float %In0
%29 = OpLoad %float %In1
%30 = OpLoad %float %In2
%31 = OpFAdd %float %29 %30
%32 = OpCompositeInsert %v4float %31 %28 2
%33 = OpAccessChain %_ptr_Uniform_uint %_ %int_0
%34 = OpLoad %uint %33
%35 = OpINotEqual %bool %34 %uint_0
OpSelectionMerge %36 None
OpBranchConditional %35 %37 %36
%37 = OpLabel
%38 = OpCompositeInsert %v4float %float_1 %28 3
OpBranch %36
%36 = OpLabel
%39 = OpPhi %v4float %28 %27 %38 %37
%40 = OpCompositeExtract %float %39 0
%41 = OpCompositeExtract %float %39 1
%42 = OpCompositeExtract %float %39 3
%43 = OpCompositeConstruct %v4float %40 %41 %float_0 %42
OpStore %OutColor %43
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<opt::VectorDCE>(before_predefs + before,
                                        after_predefs + after, true, true);
}

TEST_F(VectorDCETest, DeadInsertInCycleToDo) {
  // Dead insert in chain with cycle. Demonstrates analysis can handle
  // cycles in chains going through scalars intermediate values.
  //
  // TODO: Improve algorithm to remove dead insert into v.y.  Need to treat
  //       scalars at if they are vectors with a single element.
  //
  // Note: The SPIR-V assembly has had store/load elimination
  // performed to allow the inserts and extracts to directly
  // reference each other.
  //
  // #version 450
  //
  // layout (location=0) in vec4 In0;
  // layout (location=1) in float In1;
  // layout (location=2) in float In2;
  // layout (location=0) out vec4 OutColor;
  //
  // layout(std140, binding = 0 ) uniform _Globals_
  // {
  //     int g_n  ;
  // };
  //
  // void main()
  // {
  //     vec2 v = vec2(0.0, 1.0);
  //     for (int i = 0; i < g_n; i++) {
  //       v.x = v.x + 1;
  //       v.y = v.y * 0.9; // dead
  //     }
  //     OutColor = vec4(v.x);
  // }

  const std::string assembly =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %OutColor %In0 %In1 %In2
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 450
OpName %main "main"
OpName %_Globals_ "_Globals_"
OpMemberName %_Globals_ 0 "g_n"
OpName %_ ""
OpName %OutColor "OutColor"
OpName %In0 "In0"
OpName %In1 "In1"
OpName %In2 "In2"
OpMemberDecorate %_Globals_ 0 Offset 0
OpDecorate %_Globals_ Block
OpDecorate %_ DescriptorSet 0
OpDecorate %_ Binding 0
OpDecorate %OutColor Location 0
OpDecorate %In0 Location 0
OpDecorate %In1 Location 1
OpDecorate %In2 Location 2
%void = OpTypeVoid
%10 = OpTypeFunction %void
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%16 = OpConstantComposite %v2float %float_0 %float_1
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%_Globals_ = OpTypeStruct %int
%_ptr_Uniform__Globals_ = OpTypePointer Uniform %_Globals_
%_ = OpVariable %_ptr_Uniform__Globals_ Uniform
%_ptr_Uniform_int = OpTypePointer Uniform %int
%bool = OpTypeBool
%float_0_75 = OpConstant %float 0.75
%int_1 = OpConstant %int 1
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%OutColor = OpVariable %_ptr_Output_v4float Output
%_ptr_Input_v4float = OpTypePointer Input %v4float
%In0 = OpVariable %_ptr_Input_v4float Input
%_ptr_Input_float = OpTypePointer Input %float
%In1 = OpVariable %_ptr_Input_float Input
%In2 = OpVariable %_ptr_Input_float Input
%main = OpFunction %void None %10
%29 = OpLabel
OpBranch %30
%30 = OpLabel
%31 = OpPhi %v2float %16 %29 %32 %33
%34 = OpPhi %int %int_0 %29 %35 %33
OpLoopMerge %36 %33 None
OpBranch %37
%37 = OpLabel
%38 = OpAccessChain %_ptr_Uniform_int %_ %int_0
%39 = OpLoad %int %38
%40 = OpSLessThan %bool %34 %39
OpBranchConditional %40 %41 %36
%41 = OpLabel
%42 = OpCompositeExtract %float %31 0
%43 = OpFAdd %float %42 %float_1
%44 = OpCompositeInsert %v2float %43 %31 0
%45 = OpCompositeExtract %float %44 1
%46 = OpFMul %float %45 %float_0_75
%32 = OpCompositeInsert %v2float %46 %44 1
OpBranch %33
%33 = OpLabel
%35 = OpIAdd %int %34 %int_1
OpBranch %30
%36 = OpLabel
%47 = OpCompositeExtract %float %31 0
%48 = OpCompositeConstruct %v4float %47 %47 %47 %47
OpStore %OutColor %48
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<opt::VectorDCE>(assembly, assembly, true, true);
}

TEST_F(VectorDCETest, DeadLoadFeedingCompositeConstruct) {
  // Detach the loads feeding the CompositeConstruct for the unused elements.
  // TODO: Implement the rewrite for CompositeConstruct.

  const std::string assembly =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %In0 %OutColor
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 450
OpSourceExtension "GL_GOOGLE_cpp_style_line_directive"
OpSourceExtension "GL_GOOGLE_include_directive"
OpName %main "main"
OpName %In0 "In0"
OpName %OutColor "OutColor"
OpDecorate %In0 Location 0
OpDecorate %OutColor Location 0
%void = OpTypeVoid
%6 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%In0 = OpVariable %_ptr_Input_v4float Input
%uint = OpTypeInt 32 0
%uint_0 = OpConstant %uint 0
%_ptr_Input_float = OpTypePointer Input %float
%uint_1 = OpConstant %uint 1
%uint_2 = OpConstant %uint 2
%v3float = OpTypeVector %float 3
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_20 = OpConstant %int 20
%bool = OpTypeBool
%float_1 = OpConstant %float 1
%int_1 = OpConstant %int 1
%_ptr_Output_v4float = OpTypePointer Output %v4float
%OutColor = OpVariable %_ptr_Output_v4float Output
%23 = OpUndef %v3float
%main = OpFunction %void None %6
%24 = OpLabel
%25 = OpAccessChain %_ptr_Input_float %In0 %uint_0
%26 = OpLoad %float %25
%27 = OpAccessChain %_ptr_Input_float %In0 %uint_1
%28 = OpLoad %float %27
%29 = OpAccessChain %_ptr_Input_float %In0 %uint_2
%30 = OpLoad %float %29
%31 = OpCompositeConstruct %v3float %30 %28 %26
OpBranch %32
%32 = OpLabel
%33 = OpPhi %v3float %31 %24 %34 %35
%36 = OpPhi %int %int_0 %24 %37 %35
OpLoopMerge %38 %35 None
OpBranch %39
%39 = OpLabel
%40 = OpSLessThan %bool %36 %int_20
OpBranchConditional %40 %41 %38
%41 = OpLabel
%42 = OpCompositeExtract %float %33 0
%43 = OpFAdd %float %42 %float_1
%34 = OpCompositeInsert %v3float %43 %33 0
OpBranch %35
%35 = OpLabel
%37 = OpIAdd %int %36 %int_1
OpBranch %32
%38 = OpLabel
%44 = OpCompositeExtract %float %33 0
%45 = OpCompositeConstruct %v4float %44 %44 %44 %44
OpStore %OutColor %45
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<opt::VectorDCE>(assembly, assembly, true, true);
}

#ifdef SPIRV_EFFCEE
TEST_F(VectorDCETest, DeadLoadFeedingVectorShuffle) {
  // Detach the loads feeding the CompositeConstruct for the unused elements.
  // TODO: Implement the rewrite for CompositeConstruct.

  const std::string assembly =
      R"(
; MemPass Type2Undef does not reuse and already existing undef.
; CHECK: {{%\w+}} = OpUndef %v3float
; CHECK: [[undef:%\w+]] = OpUndef %v3float
; CHECK: OpFunction
; CHECK: OpVectorShuffle %v3float {{%\w+}} [[undef]] 0 4 5
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %In0 %OutColor
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpSourceExtension "GL_GOOGLE_cpp_style_line_directive"
               OpSourceExtension "GL_GOOGLE_include_directive"
               OpName %main "main"
               OpName %In0 "In0"
               OpName %OutColor "OutColor"
               OpDecorate %In0 Location 0
               OpDecorate %OutColor Location 0
       %void = OpTypeVoid
          %6 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
        %In0 = OpVariable %_ptr_Input_v4float Input
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
%_ptr_Input_float = OpTypePointer Input %float
     %uint_1 = OpConstant %uint 1
     %uint_2 = OpConstant %uint 2
    %v3float = OpTypeVector %float 3
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
     %int_20 = OpConstant %int 20
       %bool = OpTypeBool
    %float_1 = OpConstant %float 1
    %vec_const = OpConstantComposite %v3float %float_1 %float_1 %float_1
      %int_1 = OpConstant %int 1
%_ptr_Output_v4float = OpTypePointer Output %v4float
   %OutColor = OpVariable %_ptr_Output_v4float Output
         %23 = OpUndef %v3float
       %main = OpFunction %void None %6
         %24 = OpLabel
         %25 = OpAccessChain %_ptr_Input_float %In0 %uint_0
         %26 = OpLoad %float %25
         %27 = OpAccessChain %_ptr_Input_float %In0 %uint_1
         %28 = OpLoad %float %27
         %29 = OpAccessChain %_ptr_Input_float %In0 %uint_2
         %30 = OpLoad %float %29
         %31 = OpCompositeConstruct %v3float %30 %28 %26
         %sh = OpVectorShuffle %v3float %vec_const %31 0 4 5
               OpBranch %32
         %32 = OpLabel
         %33 = OpPhi %v3float %sh %24 %34 %35
         %36 = OpPhi %int %int_0 %24 %37 %35
               OpLoopMerge %38 %35 None
               OpBranch %39
         %39 = OpLabel
         %40 = OpSLessThan %bool %36 %int_20
               OpBranchConditional %40 %41 %38
         %41 = OpLabel
         %42 = OpCompositeExtract %float %33 0
         %43 = OpFAdd %float %42 %float_1
         %34 = OpCompositeInsert %v3float %43 %33 0
               OpBranch %35
         %35 = OpLabel
         %37 = OpIAdd %int %36 %int_1
               OpBranch %32
         %38 = OpLabel
         %44 = OpCompositeExtract %float %33 0
         %45 = OpCompositeConstruct %v4float %44 %44 %44 %44
               OpStore %OutColor %45
               OpReturn
               OpFunctionEnd
)";

  SinglePassRunAndMatch<opt::VectorDCE>(assembly, true);
}

TEST_F(VectorDCETest, DeadInstThroughShuffle) {
  // Dead insert in chain with cycle. Demonstrates analysis can handle
  // cycles in chains.
  //
  // Note: The SPIR-V assembly has had store/load elimination
  // performed to allow the inserts and extracts to directly
  // reference each other.
  //
  // #version 450
  //
  // layout (location=0) out vec4 OutColor;
  //
  // void main()
  // {
  //     vec2 v;
  //     v.x = 0.0;
  //     v.y = 0.1; // dead
  //     for (int i = 0; i < 20; i++) {
  //       v.x = v.x + 1;
  //       v = v * 0.9;
  //     }
  //     OutColor = vec4(v.x);
  // }

  const std::string assembly =
      R"(
; CHECK: OpFunction
; CHECK-NOT: OpCompositeInsert %v2float {{%\w+}} 1
; CHECK: OpFunctionEnd
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %OutColor
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpSourceExtension "GL_GOOGLE_cpp_style_line_directive"
               OpSourceExtension "GL_GOOGLE_include_directive"
               OpName %main "main"
               OpName %OutColor "OutColor"
               OpDecorate %OutColor Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
    %float_0 = OpConstant %float 0
%float_0_100000001 = OpConstant %float 0.100000001
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
     %int_20 = OpConstant %int 20
       %bool = OpTypeBool
    %float_1 = OpConstant %float 1
%float_0_899999976 = OpConstant %float 0.899999976
      %int_1 = OpConstant %int 1
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
   %OutColor = OpVariable %_ptr_Output_v4float Output
         %58 = OpUndef %v2float
       %main = OpFunction %void None %3
          %5 = OpLabel
         %49 = OpCompositeInsert %v2float %float_0 %58 0
         %51 = OpCompositeInsert %v2float %float_0_100000001 %49 1
               OpBranch %22
         %22 = OpLabel
         %60 = OpPhi %v2float %51 %5 %38 %25
         %59 = OpPhi %int %int_0 %5 %41 %25
               OpLoopMerge %24 %25 None
               OpBranch %26
         %26 = OpLabel
         %30 = OpSLessThan %bool %59 %int_20
               OpBranchConditional %30 %23 %24
         %23 = OpLabel
         %53 = OpCompositeExtract %float %60 0
         %34 = OpFAdd %float %53 %float_1
         %55 = OpCompositeInsert %v2float %34 %60 0
         %38 = OpVectorTimesScalar %v2float %55 %float_0_899999976
               OpBranch %25
         %25 = OpLabel
         %41 = OpIAdd %int %59 %int_1
               OpBranch %22
         %24 = OpLabel
         %57 = OpCompositeExtract %float %60 0
         %47 = OpCompositeConstruct %v4float %57 %57 %57 %57
               OpStore %OutColor %47
               OpReturn
               OpFunctionEnd
)";

  SinglePassRunAndMatch<opt::VectorDCE>(assembly, true);
}

TEST_F(VectorDCETest, DeadInsertThroughOtherInst) {
  // Dead insert in chain with cycle. Demonstrates analysis can handle
  // cycles in chains.
  //
  // Note: The SPIR-V assembly has had store/load elimination
  // performed to allow the inserts and extracts to directly
  // reference each other.
  //
  // #version 450
  //
  // layout (location=0) out vec4 OutColor;
  //
  // void main()
  // {
  //     vec2 v;
  //     v.x = 0.0;
  //     v.y = 0.1; // dead
  //     for (int i = 0; i < 20; i++) {
  //       v.x = v.x + 1;
  //       v = v * 0.9;
  //     }
  //     OutColor = vec4(v.x);
  // }

  const std::string assembly =
      R"(
; CHECK: OpFunction
; CHECK-NOT: OpCompositeInsert %v2float {{%\w+}} 1
; CHECK: OpFunctionEnd
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %OutColor
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpSourceExtension "GL_GOOGLE_cpp_style_line_directive"
               OpSourceExtension "GL_GOOGLE_include_directive"
               OpName %main "main"
               OpName %OutColor "OutColor"
               OpDecorate %OutColor Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
    %float_0 = OpConstant %float 0
%float_0_100000001 = OpConstant %float 0.100000001
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
     %int_20 = OpConstant %int 20
       %bool = OpTypeBool
    %float_1 = OpConstant %float 1
%float_0_899999976 = OpConstant %float 0.899999976
      %int_1 = OpConstant %int 1
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
   %OutColor = OpVariable %_ptr_Output_v4float Output
         %58 = OpUndef %v2float
       %main = OpFunction %void None %3
          %5 = OpLabel
         %49 = OpCompositeInsert %v2float %float_0 %58 0
         %51 = OpCompositeInsert %v2float %float_0_100000001 %49 1
               OpBranch %22
         %22 = OpLabel
         %60 = OpPhi %v2float %51 %5 %38 %25
         %59 = OpPhi %int %int_0 %5 %41 %25
               OpLoopMerge %24 %25 None
               OpBranch %26
         %26 = OpLabel
         %30 = OpSLessThan %bool %59 %int_20
               OpBranchConditional %30 %23 %24
         %23 = OpLabel
         %53 = OpCompositeExtract %float %60 0
         %34 = OpFAdd %float %53 %float_1
         %55 = OpCompositeInsert %v2float %34 %60 0
         %38 = OpVectorTimesScalar %v2float %55 %float_0_899999976
               OpBranch %25
         %25 = OpLabel
         %41 = OpIAdd %int %59 %int_1
               OpBranch %22
         %24 = OpLabel
         %57 = OpCompositeExtract %float %60 0
         %47 = OpCompositeConstruct %v4float %57 %57 %57 %57
               OpStore %OutColor %47
               OpReturn
               OpFunctionEnd
)";

  SinglePassRunAndMatch<opt::VectorDCE>(assembly, true);
}
#endif

}  // anonymous namespace
