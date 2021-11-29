/****************************************************************************
 * Copyright (c) 2021 Xiamen Yaji Software Co., Ltd.
 *
 * http://www.cocos.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 ****************************************************************************/

#include "ARBackground.h"
#include "../forward/ForwardPipeline.h"
#include "GLES2/gl2.h"
#include "gfx-base/GFXDef-common.h"
#include "gfx-base/GFXTexture.h"
#include "pipeline/Define.h"
#include "pipeline/PipelineStateManager.h"
#include "gfx-base/GFXCommandBuffer.h"
#include "gfx-base/GFXDef.h"

#include "ar/ARModule.h"
#include "gfx-base/GFXDevice.h"

namespace cc {
namespace pipeline {

ARBackground::~ARBackground(){
    CC_SAFE_DESTROY(_shader)
    CC_SAFE_DESTROY(_vertexBuffer)
    CC_SAFE_DESTROY(_uniformBuffer)
    CC_SAFE_DESTROY(_uniformBufferMVP)
    CC_SAFE_DESTROY(_inputAssembler)
    CC_SAFE_DESTROY(_descriptorSetLayout)
    CC_SAFE_DESTROY(_descriptorSet)
    CC_SAFE_DESTROY(_pipelineLayout)
    CC_SAFE_DESTROY(_pipelineState)
    //CC_SAFE_DESTROY(_pipeline)
    //CC_SAFE_DESTROY(_device)
}

void ARBackground::activate(RenderPipeline *pipeline, gfx::Device* dev) {
    _pipeline = pipeline;

    _device = dev;

    #pragma region _shader

    ShaderSources<StandardShaderSource> sources;

    sources.glsl4 = {
        R"(
            precision highp float;
            layout(location = 0) in vec2 a_position;
            layout(location = 1) in vec2 a_texCoord;
            layout(set = 1, binding = 0) uniform Mats {
                mat4 u_MVP;
                mat4 u_CoordMatrix;
            };

            layout(location = 0) out vec2 v_texCoord;
            void main() {
                v_texCoord = (u_CoordMatrix * vec4(a_texCoord, 0, 1)).xy;
                gl_Position = u_MVP * vec4(a_position, 0, 1);
            }
        )",
        R"(
            precision highp float;
            layout(location = 0) in vec2 v_texCoord;
            layout(set = 1, binding = 1) uniform sampler2D u_texture;

            layout(location = 0) out vec4 o_color;
            void main() {
                o_color = texture(u_texture, v_texCoord);
            }
        )",
    };

    sources.glsl3 = {
        R"(
            in vec2 a_position;
            in vec2 a_texCoord;
            layout(std140) uniform Mats {
                mat4 u_MVP;
                mat4 u_CoordMatrix;
            };

            out vec2 v_texCoord;
            void main() {
                v_texCoord = (u_CoordMatrix * vec4(a_texCoord, 0, 1)).xy;
                gl_Position = u_MVP * vec4(a_position, 0, 1);
            }
        )",
        R"(
            #extension GL_OES_EGL_image_external_essl3 : require
            precision mediump float;

            in vec2 v_texCoord;
            uniform samplerExternalOES u_texture;

            out vec4 o_color;
            void main() {
                o_color = texture(u_texture, v_texCoord);
            }
        )",
    };

    sources.glsl1 = {
        R"(
            attribute vec2 a_position;
            attribute vec2 a_texCoord;
            uniform mat4 u_MVP;
            uniform mat4 u_CoordMatrix;

            varying vec2 v_texCoord;
            void main() {
                v_texCoord = (u_CoordMatrix * vec4(a_texCoord, 0, 1)).xy;
                gl_Position = u_MVP * vec4(a_position, 0, 1);
            }
        )",
        R"(
            #extension GL_OES_EGL_image_external : require
            precision mediump float;

            varying vec2 v_texCoord;
            uniform samplerExternalOES u_texture;

            void main() {
                gl_FragColor = texture2D(u_texture, v_texCoord);
            }
        )",
    };

    StandardShaderSource &source = getAppropriateShaderSource(sources);

    gfx::ShaderStageList shaderStageList;

    gfx::ShaderStage     vertexShaderStage;
    vertexShaderStage.stage  = gfx::ShaderStageFlagBit::VERTEX;
    vertexShaderStage.source = source.vert;
    shaderStageList.emplace_back(std::move(vertexShaderStage));

    gfx::ShaderStage fragmentShaderStage;
    fragmentShaderStage.stage  = gfx::ShaderStageFlagBit::FRAGMENT;
    fragmentShaderStage.source = source.frag;
    shaderStageList.emplace_back(std::move(fragmentShaderStage));

    gfx::AttributeList attributeList = {
        {"a_position", gfx::Format::RG32F, false, 0, false, 0},
        {"a_texCoord", gfx::Format::RG32F, false, 0, false, 1},
    };

    gfx::UniformBlockList blockList = {{materialSet, 0, "Mats", {{"u_MVP", gfx::Type::MAT4, 1}, {"u_CoordMatrix", gfx::Type::MAT4, 1}}, 2}};
    gfx::UniformSamplerTextureList samplerTextures = {{materialSet, 1, "u_texture", gfx::Type::SAMPLER2D, 1}};

    gfx::ShaderInfo shaderInfo;
    shaderInfo.name            = "ARBackGround";
    shaderInfo.stages          = std::move(shaderStageList);
    shaderInfo.attributes      = attributeList;
    shaderInfo.blocks          = std::move(blockList);
    shaderInfo.samplerTextures = std::move(samplerTextures);
    _shader                    = _device->createShader(shaderInfo);

    #pragma endregion

    #pragma region _inputAssembler

    float vertices[] = {-1, -1, 0, 0,
                        -1, 1, 0, 1,
                        1, -1, 1, 0,
                        1, 1, 1, 1};

    _vertexBuffer = _device->createBuffer({
        gfx::BufferUsage::VERTEX,
        gfx::MemoryUsage::DEVICE,
        sizeof(vertices),
        4 * sizeof(float),
    });
    _vertexBuffer->update(vertices, sizeof(vertices));

    uint16_t        indices[]       = {0, 2, 1, 1, 2, 3};
    gfx::BufferInfo indexBufferInfo = {
        gfx::BufferUsageBit::INDEX,
        gfx::MemoryUsage::DEVICE,
        sizeof(indices),
        sizeof(uint16_t),
    };
    auto* indexBuffer = _device->createBuffer(indexBufferInfo);
    indexBuffer->update(indices, sizeof(indices));

    gfx::DrawInfo drawInfo;
    drawInfo.indexCount = 6;
    gfx::BufferInfo indirectBufferInfo = {
        gfx::BufferUsageBit::INDIRECT,
        gfx::MemoryUsage::DEVICE,
        sizeof(gfx::DrawInfo),
        sizeof(gfx::DrawInfo),
    };
    auto* indirectBuffer = _device->createBuffer(indirectBufferInfo);
    indirectBuffer->update(&drawInfo, sizeof(gfx::DrawInfo));

    gfx::InputAssemblerInfo inputAssemblerInfo;
    inputAssemblerInfo.attributes = std::move(attributeList);
    inputAssemblerInfo.vertexBuffers.emplace_back(_vertexBuffer);
    inputAssemblerInfo.indexBuffer = indexBuffer;
    inputAssemblerInfo.indirectBuffer = indirectBuffer;
    _inputAssembler = _device->createInputAssembler(inputAssemblerInfo);

    #pragma endregion

    gfx::BufferInfo uniformBufferInfo = {
        gfx::BufferUsage::UNIFORM,
        gfx::MemoryUsage::DEVICE,
        2 * sizeof(Mat4),
    };
    _uniformBuffer = _device->createBuffer(uniformBufferInfo);
    float mats[] = {
        1, 0, 0, 0, 
        0, 1, 0, 0, 
        0, 0, -1, 0, 
        0, 0, -1, 1,
        1, 0, 0, 0, 
        0, 1, 0, 0, 
        0, 0, 1, 0, 
        0, 0, 0, 1
    };
    _uniformBuffer->update(mats, 2 * sizeof(Mat4));

    gfx::DescriptorSetLayoutInfo dslInfo;
    dslInfo.bindings.push_back({0, gfx::DescriptorType::UNIFORM_BUFFER, 1, gfx::ShaderStageFlagBit::VERTEX});
    dslInfo.bindings.push_back({1, gfx::DescriptorType::SAMPLER_TEXTURE, 1, gfx::ShaderStageFlagBit::FRAGMENT});
    _descriptorSetLayout = _device->createDescriptorSetLayout(dslInfo);
    //*
    _descriptorSet = _device->createDescriptorSet({_descriptorSetLayout});
    _descriptorSet->bindBuffer(0, _uniformBuffer);
    //_descriptorSet->bindSampler(1, sampler);
    //_descriptorSet->bindTexture(1, backgroundTex);
    _descriptorSet->update();
    //*/
    _pipelineLayout = _device->createPipelineLayout({{pipeline->getDescriptorSetLayout(), _descriptorSetLayout}});

    // background id
    auto *deviceAgent = gfx::DeviceAgent::getInstance();
    deviceAgent->setMultithreaded(false);
    // GL-Specific invocations
    glGenTextures(1, &_glTex);
    deviceAgent->setMultithreaded(true);
}

void ARBackground::render(scene::Camera *camera, gfx::RenderPass *renderPass, gfx::CommandBuffer *cmdBuffer) {
    //auto *_device = gfx::DeviceAgent::getInstance();
    //if (!_device) { return; }
    /*
    const scene::Node *camNode = camera->node;
    const int flag = (static_cast<int>(camNode->getLayer())) & 0x00800000;
    if(flag == 0) return;
    //*/

    //*
    auto *const armodule = cc::ar::ARModule::get();
    if(!armodule) return;
    if(!armodule->checkStart()) return;
    if(!_setTexFlag) {
        armodule->setCameraTextureName(static_cast<int>(_glTex));

        gfx::SamplerInfo samplerInfo;
        auto *           sampler = _device->getSampler(samplerInfo);
        gfx::TextureInfo textureInfo;
        textureInfo.usage  = gfx::TextureUsage::SAMPLED | gfx::TextureUsage::TRANSFER_SRC;
        textureInfo.format = gfx::Format::RGBA16F;
        textureInfo.width = camera->width;
        textureInfo.height = camera->height;
        textureInfo.externalRes = reinterpret_cast<void *>(_glTex);
        gfx::Texture *backgroundTex = _device->createTexture(textureInfo);
        //_descriptorSet = _device->createDescriptorSet({_descriptorSetLayout});
        //_descriptorSet->bindBuffer(0, _uniformBuffer);
        _descriptorSet->bindSampler(1, sampler);
        _descriptorSet->bindTexture(1, backgroundTex);
        _descriptorSet->update();

        _setTexFlag = true;
    }
    auto *const data = armodule->getCameraTexCoords();
    float vertices[] = {-1, -1, data[0], data[1],
                        -1, 1, data[2], data[3],
                        1, -1, data[4], data[5],
                        1, 1, data[6], data[7]};
    _vertexBuffer->update(vertices, sizeof(vertices));
    
    gfx::PipelineStateInfo pipelineInfo;
    pipelineInfo.shader         = _shader;
    pipelineInfo.pipelineLayout = _pipelineLayout;
    pipelineInfo.renderPass     = renderPass;
    pipelineInfo.inputState     = {_inputAssembler->getAttributes()};

    _pipelineState = _device->createPipelineState(pipelineInfo);

    cmdBuffer->bindInputAssembler(_inputAssembler);
    cmdBuffer->bindPipelineState(_pipelineState);
    cmdBuffer->bindDescriptorSet(materialSet, _descriptorSet);
    cmdBuffer->draw(_inputAssembler);
}

template <typename T>
T &ARBackground::getAppropriateShaderSource(ShaderSources<T> &sources) {
    switch (_device->getGfxAPI()) {
        case gfx::API::GLES2:
            return sources.glsl1;
        case gfx::API::GLES3:
            return sources.glsl3;
        case gfx::API::METAL:
        case gfx::API::VULKAN:
            return sources.glsl4;
        default: break;
    }
    return sources.glsl4;
}

}  // namespace pipeline
}  // namespace cc