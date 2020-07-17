#include "VKStd.h"

#include "VKBindingLayout.h"
#include "VKBuffer.h"
#include "VKCommandBuffer.h"
#include "VKCommands.h"
#include "VKDevice.h"
#include "VKFramebuffer.h"
#include "VKInputAssembler.h"
#include "VKPipelineState.h"
#include "VKQueue.h"
#include "VKRenderPass.h"
#include "VKTexture.h"

namespace cc {
namespace gfx {

CCVKCommandBuffer::CCVKCommandBuffer(Device *device)
: CommandBuffer(device) {
}

CCVKCommandBuffer::~CCVKCommandBuffer() {
}

bool CCVKCommandBuffer::initialize(const CommandBufferInfo &info) {
    _type = info.type;
    _queue = info.queue;

    _gpuCommandBuffer = CC_NEW(CCVKGPUCommandBuffer);
    _gpuCommandBuffer->level = MapVkCommandBufferLevel(_type);
    _gpuCommandBuffer->queueFamilyIndex = ((CCVKQueue *)_queue)->gpuQueue()->queueFamilyIndex;
    ((CCVKDevice *)_device)->gpuCommandBufferPool()->request(_gpuCommandBuffer);

    _status = Status::SUCCESS;
    return true;
}

void CCVKCommandBuffer::destroy() {
    if (_gpuCommandBuffer) {
        ((CCVKDevice *)_device)->gpuCommandBufferPool()->yield(_gpuCommandBuffer);
        CC_DELETE(_gpuCommandBuffer);
        _gpuCommandBuffer = nullptr;
    }

    _status = Status::UNREADY;
}

void CCVKCommandBuffer::begin(RenderPass *renderPass, uint subpass, Framebuffer *frameBuffer) {
    _curGPUPipelineState = nullptr;
    _curGPUBindingLayout = nullptr;
    _curGPUInputAssember = nullptr;
    _numDrawCalls = 0;
    _numInstances = 0;
    _numTriangles = 0;

    VkCommandBufferBeginInfo beginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (_type == CommandBufferType::SECONDARY) {
        if (!renderPass) {
            CC_LOG_ERROR("RenderPass has to be specified when beginning secondary command buffers.");
            return;
        }
        VkCommandBufferInheritanceInfo inheritanceInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO};
        inheritanceInfo.renderPass = ((CCVKRenderPass *)renderPass)->gpuRenderPass()->vkRenderPass;
        inheritanceInfo.subpass = subpass;
        if (frameBuffer) inheritanceInfo.framebuffer = ((CCVKFramebuffer *)frameBuffer)->gpuFBO()->vkFramebuffer;
        beginInfo.pInheritanceInfo = &inheritanceInfo;
    }
    VK_CHECK(vkBeginCommandBuffer(_gpuCommandBuffer->vkCommandBuffer, &beginInfo));
}

void CCVKCommandBuffer::end() {
    _curGPUFBO = nullptr;
    VK_CHECK(vkEndCommandBuffer(_gpuCommandBuffer->vkCommandBuffer));
}

void CCVKCommandBuffer::beginRenderPass(RenderPass *renderPass, Framebuffer *fbo, const Rect &renderArea,
                                        const vector<Color> &colors, float depth, int stencil) {
    _curGPUFBO = ((CCVKFramebuffer *)fbo)->gpuFBO();
    CCVKGPURenderPass *gpuRenderPass = ((CCVKRenderPass*)renderPass)->gpuRenderPass();
    VkFramebuffer framebuffer = _curGPUFBO->vkFramebuffer;
    if (!_curGPUFBO->isOffscreen) {
        framebuffer = _curGPUFBO->swapchain->vkSwapchainFramebufferListMap[_curGPUFBO][_curGPUFBO->swapchain->curImageIndex];
    }

    vector<VkClearValue> &clearValues = gpuRenderPass->clearValues;
    size_t attachmentCount = clearValues.size();

    if (attachmentCount) {
        for (size_t i = 0u; i < attachmentCount - 1; i++) {
            clearValues[i].color = {colors[i].r, colors[i].g, colors[i].b, colors[i].a};
        }
        clearValues[attachmentCount - 1].depthStencil = {depth, (uint)stencil};
    }

    VkRenderPassBeginInfo passBeginInfo{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
    passBeginInfo.renderPass = gpuRenderPass->vkRenderPass;
    passBeginInfo.framebuffer = framebuffer;
    passBeginInfo.renderArea.offset.x = renderArea.x;
    passBeginInfo.renderArea.offset.y = renderArea.y;
    passBeginInfo.renderArea.extent.width = renderArea.width;
    passBeginInfo.renderArea.extent.height = renderArea.height;
    passBeginInfo.clearValueCount = clearValues.size();
    passBeginInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(_gpuCommandBuffer->vkCommandBuffer, &passBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{(float)renderArea.x, (float)renderArea.y, (float)renderArea.width, (float)renderArea.height, 0, 1};
    vkCmdSetViewport(_gpuCommandBuffer->vkCommandBuffer, 0, 1, &viewport);

    VkRect2D scissor{{renderArea.x, renderArea.y}, {renderArea.width, renderArea.height}};
    vkCmdSetScissor(_gpuCommandBuffer->vkCommandBuffer, 0, 1, &scissor);
}

void CCVKCommandBuffer::endRenderPass() {
    vkCmdEndRenderPass(_gpuCommandBuffer->vkCommandBuffer);
    _curGPUFBO = nullptr;
}

void CCVKCommandBuffer::bindPipelineState(PipelineState *pso) {
    CCVKGPUPipelineState *gpuPipelineState = ((CCVKPipelineState *)pso)->gpuPipelineState();

    if (_curGPUPipelineState != gpuPipelineState) {
        vkCmdBindPipeline(_gpuCommandBuffer->vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gpuPipelineState->vkPipeline);
        _curGPUPipelineState = gpuPipelineState;
    }
}

void CCVKCommandBuffer::bindBindingLayout(BindingLayout *layout) {
    if (!_curGPUPipelineState) {
        CC_LOG_ERROR("Command 'bindBindingLayout' must be recorded after 'bindPipelineState'.");
        return;
    }

    CCVKGPUBindingLayout *gpuBindingLayout = ((CCVKBindingLayout *)layout)->gpuBindingLayout();

    if (_curGPUBindingLayout != gpuBindingLayout) {

        // Update refs here in case resources updated, e.g. resize events
        // TODO: maybe there is a better way?
        vector<vector<CCVKDescriptorInfo>> &descriptorInfos = gpuBindingLayout->descriptorInfos;
        vector<vector<CCVKGPUBinding>> &gpuBindings = gpuBindingLayout->gpuBindings;
        for (uint i = 0u; i < descriptorInfos.size(); i++) {
            vector<CCVKDescriptorInfo> &descriptors = descriptorInfos[i];
            vector<CCVKGPUBinding> &bindings = gpuBindings[i];
            for (uint j = 0u; j < descriptors.size(); j++) {
                CCVKDescriptorInfo &descriptor = descriptors[j];
                CCVKGPUBinding &binding = bindings[j];
                if (binding.buffer) {
                    descriptor.buffer.buffer = binding.buffer->vkBuffer;
                    descriptor.buffer.offset = binding.buffer->startOffset;
                    descriptor.buffer.range = binding.buffer->size;
                } else {
                    if (binding.texView) {
                        descriptor.image.imageView = binding.texView->vkImageView;
                    }
                    if (binding.sampler) {
                        descriptor.image.sampler = binding.sampler->vkSampler;
                    }
                }
            }
        }

        CCVKDevice *device = (CCVKDevice *)_device;
        VkCommandBuffer cmdBuff = _gpuCommandBuffer->vkCommandBuffer;
        VkPipelineLayout pipelineLayout = _curGPUPipelineState->gpuShader->vkPipelineLayout;

        if (device->isPushDescriptorSetSupported()) {
            vector<VkDescriptorUpdateTemplate> &templates = _curGPUPipelineState->gpuShader->vkDescriptorUpdateTemplates;

            for (uint i = 0u; i < templates.size(); i++) {
                vkCmdPushDescriptorSetWithTemplateKHR(cmdBuff, templates[i], pipelineLayout, i, descriptorInfos[i].data());
            }
        } else {
            vector<VkDescriptorSetLayout> &layouts = _curGPUPipelineState->gpuShader->vkDescriptorSetLayouts;
            vector<VkDescriptorSet> &sets = gpuBindingLayout->descriptorSets;

            device->gpuDescriptorSetPool()->alloc(layouts.data(), sets.data(), layouts.size());

            if (device->isDescriptorUpdateTemplateSupported()) {
                vector<VkDescriptorUpdateTemplate> &templates = _curGPUPipelineState->gpuShader->vkDescriptorUpdateTemplates;

                for (uint i = 0u; i < templates.size(); i++) {
                    vkUpdateDescriptorSetWithTemplateKHR(device->gpuDevice()->vkDevice, sets[i], templates[i], descriptorInfos[i].data());
                }
            } else {
                vector<vector<VkWriteDescriptorSet>> &templates = _curGPUPipelineState->gpuShader->manualDescriptorUpdateTemplates;

                for (uint i = 0u; i < templates.size(); i++) {
                    vector<VkWriteDescriptorSet> &entries = templates[i];
                    vector<CCVKDescriptorInfo> &descriptors = descriptorInfos[i];
                    for (uint j = 0u; j < entries.size(); j++) {
                        entries[j].dstSet = sets[i];
                        switch (entries[j].descriptorType) {
                            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                            case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
                            case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
                                entries[j].pBufferInfo = &descriptors[j].buffer;
                                break;
                            case VK_DESCRIPTOR_TYPE_SAMPLER:
                            case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                            case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                            case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                            case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
                                entries[j].pImageInfo = &descriptors[j].image;
                                break;
                            case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
                            case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
                                entries[j].pTexelBufferView = &descriptors[j].texelBufferView;
                                break;
                        }
                    }
                    vkUpdateDescriptorSets(device->gpuDevice()->vkDevice, entries.size(), entries.data(), 0, nullptr);
                }
            }
            vkCmdBindDescriptorSets(cmdBuff, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, layouts.size(), sets.data(), 0, nullptr);
        }
        _curGPUBindingLayout = gpuBindingLayout;
    }
}

void CCVKCommandBuffer::bindInputAssembler(InputAssembler *ia) {
    CCVKGPUInputAssembler *gpuInputAssembler = ((CCVKInputAssembler *)ia)->gpuInputAssembler();

    if (_curGPUInputAssember != gpuInputAssembler) {
        // buffers may be rebuilt(e.g. resize event) without IA's acknowledge
        size_t vbCount = gpuInputAssembler->gpuVertexBuffers.size();
        if (gpuInputAssembler->vertexBuffers.size() < vbCount) {
            gpuInputAssembler->vertexBuffers.resize(vbCount);
            gpuInputAssembler->vertexBufferOffsets.resize(vbCount);
        }

        for (size_t i = 0u; i < vbCount; i++) {
            gpuInputAssembler->vertexBuffers[i] = gpuInputAssembler->gpuVertexBuffers[i]->vkBuffer;
            gpuInputAssembler->vertexBufferOffsets[i] = gpuInputAssembler->gpuVertexBuffers[i]->startOffset;
        }

        vkCmdBindVertexBuffers(_gpuCommandBuffer->vkCommandBuffer, 0, vbCount,
                               gpuInputAssembler->vertexBuffers.data(), gpuInputAssembler->vertexBufferOffsets.data());

        if (gpuInputAssembler->gpuIndexBuffer) {
            vkCmdBindIndexBuffer(_gpuCommandBuffer->vkCommandBuffer, gpuInputAssembler->gpuIndexBuffer->vkBuffer, 0,
                                 gpuInputAssembler->gpuIndexBuffer->stride == 4 ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16);
        }
        _curGPUInputAssember = gpuInputAssembler;
    }
}

void CCVKCommandBuffer::setViewport(const Viewport &vp) {
    if (_curViewport != vp) {
        _curViewport = vp;
        VkViewport viewport{(float)vp.left, (float)vp.top, (float)vp.width, (float)vp.height, vp.minDepth, vp.maxDepth};
        vkCmdSetViewport(_gpuCommandBuffer->vkCommandBuffer, 0, 1, &viewport);
    }
}

void CCVKCommandBuffer::setScissor(const Rect &rect) {
    if (_curScissor != rect) {
        _curScissor = rect;
        VkRect2D scissor{{rect.x, rect.y}, {rect.width, rect.height}};
        vkCmdSetScissor(_gpuCommandBuffer->vkCommandBuffer, 0, 1, &scissor);
    }
}

void CCVKCommandBuffer::setLineWidth(const float width) {
    if (math::IsNotEqualF(_curLineWidth, width)) {
        _curLineWidth = width;
        vkCmdSetLineWidth(_gpuCommandBuffer->vkCommandBuffer, width);
    }
}

void CCVKCommandBuffer::setDepthBias(float constant, float clamp, float slope) {
    if (math::IsNotEqualF(_curDepthBias.constant, constant) ||
        math::IsNotEqualF(_curDepthBias.clamp, clamp) ||
        math::IsNotEqualF(_curDepthBias.slope, slope)) {
        _curDepthBias.constant = constant;
        _curDepthBias.clamp = clamp;
        _curDepthBias.slope = slope;
        vkCmdSetDepthBias(_gpuCommandBuffer->vkCommandBuffer, constant, clamp, slope);
    }
}

void CCVKCommandBuffer::setBlendConstants(const Color &constants) {
    if (math::IsNotEqualF(_curBlendConstants.r, constants.r) ||
        math::IsNotEqualF(_curBlendConstants.g, constants.g) ||
        math::IsNotEqualF(_curBlendConstants.b, constants.b) ||
        math::IsNotEqualF(_curBlendConstants.a, constants.a)) {
        _curBlendConstants.r = constants.r;
        _curBlendConstants.g = constants.g;
        _curBlendConstants.b = constants.b;
        _curBlendConstants.a = constants.a;
        vkCmdSetBlendConstants(_gpuCommandBuffer->vkCommandBuffer, reinterpret_cast<const float *>(&constants));
    }
}

void CCVKCommandBuffer::setDepthBound(float minBounds, float maxBounds) {
    if (math::IsNotEqualF(_curDepthBounds.minBounds, minBounds) ||
        math::IsNotEqualF(_curDepthBounds.maxBounds, maxBounds)) {
        _curDepthBounds.minBounds = minBounds;
        _curDepthBounds.maxBounds = maxBounds;
        vkCmdSetDepthBounds(_gpuCommandBuffer->vkCommandBuffer, minBounds, maxBounds);
    }
}

void CCVKCommandBuffer::setStencilWriteMask(StencilFace face, uint mask) {
    if ((_curStencilWriteMask.face != face) ||
        (_curStencilWriteMask.write_mask != mask)) {
        _curStencilWriteMask.face = face;
        _curStencilWriteMask.write_mask = mask;
        vkCmdSetStencilWriteMask(_gpuCommandBuffer->vkCommandBuffer,
                                 face == StencilFace::FRONT ? VK_STENCIL_FACE_FRONT_BIT : VK_STENCIL_FACE_BACK_BIT, mask);
    }
}

void CCVKCommandBuffer::setStencilCompareMask(StencilFace face, int reference, uint mask) {
    if ((_curStencilCompareMask.face != face) ||
        (_curStencilCompareMask.reference != reference) ||
        (_curStencilCompareMask.compareMask != mask)) {
        _curStencilCompareMask.face = face;
        _curStencilCompareMask.reference = reference;
        _curStencilCompareMask.compareMask = mask;

        VkStencilFaceFlagBits vkFace = (face == StencilFace::FRONT ? VK_STENCIL_FACE_FRONT_BIT : VK_STENCIL_FACE_BACK_BIT);
        vkCmdSetStencilReference(_gpuCommandBuffer->vkCommandBuffer, vkFace, reference);
        vkCmdSetStencilCompareMask(_gpuCommandBuffer->vkCommandBuffer, vkFace, mask);
    }
}

void CCVKCommandBuffer::draw(InputAssembler *ia) {
    if ((_type == CommandBufferType::PRIMARY && _curGPUFBO) ||
        (_type == CommandBufferType::SECONDARY)) {
        CCVKGPUInputAssembler *gpuInputAssembler = ((CCVKInputAssembler *)ia)->gpuInputAssembler();
        DrawInfo drawInfo;

        if (gpuInputAssembler->gpuIndirectBuffer) {
            uint drawInfoCount = gpuInputAssembler->gpuIndirectBuffer->count;
            if (static_cast<CCVKDevice *>(_device)->isMultiDrawIndirectSupported()) {
                if (gpuInputAssembler->gpuIndirectBuffer->isDrawIndirectByIndex) {
                    vkCmdDrawIndexedIndirect(_gpuCommandBuffer->vkCommandBuffer,
                                             gpuInputAssembler->gpuIndirectBuffer->vkBuffer,
                                             0,
                                             drawInfoCount,
                                             sizeof(VkDrawIndexedIndirectCommand));
                } else {
                    vkCmdDrawIndirect(_gpuCommandBuffer->vkCommandBuffer,
                                      gpuInputAssembler->gpuIndirectBuffer->vkBuffer,
                                      0,
                                      drawInfoCount,
                                      sizeof(VkDrawIndirectCommand));
                }
            } else {
                // If multi draw is not available, we must issue separate draw commands
                if (gpuInputAssembler->gpuIndirectBuffer->isDrawIndirectByIndex) {
                    for (uint j = 0u; j < drawInfoCount; j++) {
                        vkCmdDrawIndexedIndirect(_gpuCommandBuffer->vkCommandBuffer,
                                                 gpuInputAssembler->gpuIndirectBuffer->vkBuffer,
                                                 j * sizeof(VkDrawIndexedIndirectCommand),
                                                 1,
                                                 sizeof(VkDrawIndexedIndirectCommand));
                    }
                } else {
                    for (uint j = 0u; j < drawInfoCount; j++) {
                        vkCmdDrawIndirect(_gpuCommandBuffer->vkCommandBuffer,
                                          gpuInputAssembler->gpuIndirectBuffer->vkBuffer,
                                          j * sizeof(VkDrawIndirectCommand),
                                          1,
                                          sizeof(VkDrawIndirectCommand));
                    }
                }
            }
        } else {
            ((CCVKInputAssembler *)ia)->extractDrawInfo(drawInfo);
            uint instanceCount = std::max(drawInfo.instanceCount, 1u);
            bool hasIndexBuffer = gpuInputAssembler->gpuIndexBuffer && drawInfo.indexCount >= 0;

            if (hasIndexBuffer) {
                vkCmdDrawIndexed(_gpuCommandBuffer->vkCommandBuffer, drawInfo.indexCount, instanceCount,
                                 drawInfo.firstIndex, drawInfo.vertexOffset, drawInfo.firstInstance);
            } else {
                vkCmdDraw(_gpuCommandBuffer->vkCommandBuffer, drawInfo.vertexCount, instanceCount,
                          drawInfo.firstVertex, drawInfo.firstInstance);
            }

            ++_numDrawCalls;
            _numInstances += drawInfo.instanceCount;
            if (_curGPUPipelineState) {
                uint indexCount = hasIndexBuffer ? drawInfo.indexCount : drawInfo.vertexCount;
                switch (_curGPUPipelineState->primitive) {
                    case PrimitiveMode::TRIANGLE_LIST:
                        _numTriangles += indexCount / 3 * instanceCount;
                        break;
                    case PrimitiveMode::TRIANGLE_STRIP:
                    case PrimitiveMode::TRIANGLE_FAN:
                        _numTriangles += (indexCount - 2) * instanceCount;
                        break;
                }
            }
        }
    } else {
        CC_LOG_ERROR("Command 'draw' must be recorded inside a render pass.");
    }
}

void CCVKCommandBuffer::execute(const CommandBufferList &cmdBuffs, uint count) {
    if (!count) {
        return;
    }

    vector<VkCommandBuffer> vkCmdBuffs(count);

    for (uint i = 0u; i < count; ++i) {
        CCVKCommandBuffer *cmdBuff = (CCVKCommandBuffer *)cmdBuffs[i];
        vkCmdBuffs[i] = cmdBuff->_gpuCommandBuffer->vkCommandBuffer;

        _numDrawCalls += cmdBuff->getNumDrawCalls();
        _numInstances += cmdBuff->getNumInstances();
        _numTriangles += cmdBuff->getNumTris();
    }

    vkCmdExecuteCommands(_gpuCommandBuffer->vkCommandBuffer, count, vkCmdBuffs.data());
}

void CCVKCommandBuffer::updateBuffer(Buffer *buff, void *data, uint size, uint offset) {
    if ((_type == CommandBufferType::PRIMARY && !_curGPUFBO) ||
        (_type == CommandBufferType::SECONDARY)) {
        CCVKCmdFuncUpdateBuffer((CCVKDevice *)_device, ((CCVKBuffer *)buff)->gpuBuffer(), data, offset, size);
    } else {
        CC_LOG_ERROR("Command 'updateBuffer' must be recorded outside a render pass.");
    }
}

void CCVKCommandBuffer::copyBuffersToTexture(const BufferDataList &buffers, Texture *texture, const BufferTextureCopyList &regions) {
    if ((_type == CommandBufferType::PRIMARY && !_curGPUFBO) ||
        (_type == CommandBufferType::SECONDARY)) {
        //const CCVKGPUBuffer* gpuBuffer = ((CCVKBuffer*)src)->gpuBuffer();
        //const CCVKGPUTexture* gpuTexture = ((CCVKTexture*)dst)->gpuTexture();
        //vkCmdCopyBufferToImage(_gpuCommandBuffer->vkCommandBuffer, gpuBuffer->vkBuffer, gpuTexture->vkImage, MapVkImageLayout(layout),
        //    regions.size(), regions.data());
    } else {
        CC_LOG_ERROR("Command 'copyBuffersToTexture' must be recorded outside a render pass.");
    }
}

} // namespace gfx
} // namespace cc