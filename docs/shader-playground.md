Using Slang on Shader Playground
================================

A fast and simple way to try out Slang is by using the [Shader Playground](http://shader-playground.timjones.io/) website. This site allows easy and interactive testing of shader code across several compilers including Slang without having to install anything on your local machine.

Using the Slang compiler is as simple as selecting 'Slang' from the list of compilers in box underneath 'Compiler #1'. The output of the Slang compilation is shown in the right hand panel with the default 'Output format' of HLSL. To see the input source compiled to GLSL, select 'GLSL' from the 'Output format' box. 

To compile using the Slang language (as opposed to the default input language of HLSL) select 'Slang' from the combo box underneath 'Shader Playground'. It may be necessary to change the Entry Point name from 'PSMain' to 'computeMain' for successful compilation of the sample. 

For compute based shaders Slang can compile to C++, CUDA and PTX. Seeing this output is as simple as selecting the option via 'Output Format'. Note that C++ and CUDA output include a 'prelude'. The prelude remains the same across compilations, with the code generated for the input Slang source placed at the very end of the output. 
