#include "shader-input-layout.h"
#include "core/token-reader.h"

namespace renderer_test
{
    using namespace Slang;
    void ShaderInputLayout::Parse(const char * source)
    {
        entries.Clear();
        auto lines = Split(source, '\n');
        for (auto & line : lines)
        {
            if (line.StartsWith("//TEST_INPUT:"))
            {
                auto lineContent = line.SubString(13, line.Length() - 13);
                TokenReader parser(lineContent);
                try
                {
                    ShaderInputLayoutEntry entry;

                    if (parser.LookAhead("cbuffer"))
                    {
                        entry.type = ShaderInputType::Buffer;
                        entry.bufferDesc.type = InputBufferType::ConstantBuffer;
                    }
                    else if (parser.LookAhead("ubuffer"))
                    {
                        entry.type = ShaderInputType::Buffer;
                        entry.bufferDesc.type = InputBufferType::StorageBuffer;
                    }
                    else if (parser.LookAhead("Texture1D"))
                    {
                        entry.type = ShaderInputType::Texture;
                        entry.textureDesc.dimension = 1;
                    }
                    else if (parser.LookAhead("Texture2D"))
                    {
                        entry.type = ShaderInputType::Texture;
                        entry.textureDesc.dimension = 2;
                    }
                    else if (parser.LookAhead("Texture3D"))
                    {
                        entry.type = ShaderInputType::Texture;
                        entry.textureDesc.dimension = 3;
                    }
                    else if (parser.LookAhead("TextureCube"))
                    {
                        entry.type = ShaderInputType::Texture;
                        entry.textureDesc.dimension = 2;
                        entry.textureDesc.isCube = true;
                    }
                    else if (parser.LookAhead("Sampler"))
                    {
                        entry.type = ShaderInputType::Sampler;
                    }
                    else if (parser.LookAhead("Sampler1D"))
                    {
                        entry.type = ShaderInputType::CombinedTextureSampler;
                        entry.textureDesc.dimension = 1;
                    }
                    else if (parser.LookAhead("Sampler2D"))
                    {
                        entry.type = ShaderInputType::CombinedTextureSampler;
                        entry.textureDesc.dimension = 2;
                    }
                    else if (parser.LookAhead("Sampler3D"))
                    {
                        entry.type = ShaderInputType::CombinedTextureSampler;
                        entry.textureDesc.dimension = 3;
                    }
                    else if (parser.LookAhead("SamplerCube"))
                    {
                        entry.type = ShaderInputType::CombinedTextureSampler;
                        entry.textureDesc.dimension = 2;
                        entry.textureDesc.isCube = true;
                    }
                    parser.ReadToken();
                    // parse options
                    if (parser.LookAhead("("))
                    {
                        parser.Read("(");
                        while (!parser.IsEnd() && !parser.LookAhead(")"))
                        {
                            auto word = parser.ReadWord();
                            if (word == "depth")
                            {
                                entry.textureDesc.isDepthTexture = true;
                            }
                            else if (word == "depthCompare")
                            {
                                entry.samplerDesc.isCompareSampler = true;
                            }
                            else if (word == "arrayLength")
                            {
                                parser.Read("=");
                                entry.textureDesc.arrayLength = parser.ReadInt();
                            }
                            else if (word == "stride")
                            {
                                parser.Read("=");
                                entry.bufferDesc.stride = parser.ReadInt();
                            }
                            else if (word == "data")
                            {
                                parser.Read("=");
                                parser.Read("[");
                                while (!parser.IsEnd() && !parser.LookAhead("]"))
                                {
                                    if (parser.NextToken().Type == TokenType::IntLiteral)
                                    {
                                        entry.bufferData.Add(parser.ReadUInt());
                                    }
                                    else
                                    {
                                        auto floatNum = parser.ReadFloat();
                                        entry.bufferData.Add(*(unsigned int*)&floatNum);
                                    }
                                }
                                parser.Read("]");
                            }
                            if (parser.LookAhead(","))
                                parser.Read(",");
                            else
                                break;
                        }
                        parser.Read(")");
                        // parse bindings
                        if (parser.LookAhead(":"))
                        {
                            parser.Read(":");
                            while (!parser.IsEnd())
                            {
                                if (parser.LookAhead("register"))
                                {
                                    parser.ReadToken();
                                    parser.Read("(");
                                    auto reg = parser.ReadWord();
                                    entry.hlslRegister = parser.ReadInt();
                                    parser.Read(")");
                                }
                                else if (parser.LookAhead("layout"))
                                {
                                    parser.ReadToken();
                                    parser.Read("(");
                                    while (!parser.IsEnd() && !parser.LookAhead(")"))
                                    {
                                        auto word = parser.ReadWord();
                                        if (word == "binding")
                                        {
                                            parser.Read("=");
                                            entry.glslBinding = parser.ReadInt();
                                        }
                                        else if (word == "location")
                                        {
                                            parser.Read("=");
                                            entry.glslLocation = parser.ReadInt();
                                        }
                                        if (parser.LookAhead(","))
                                            parser.Read(",");
                                        else
                                            break;
                                    }
                                    parser.Read(")");
                                }
                                if (parser.LookAhead(","))
                                    parser.Read(",");
                            }
                        }
                    }
                }
                catch (TextFormatException)
                {
                    throw TextFormatException("Invalid input syntax at line " + parser.NextToken().Position.Line);
                }
            }
        }
        
        
    }
}