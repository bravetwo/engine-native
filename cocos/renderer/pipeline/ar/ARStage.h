/****************************************************************************
 Copyright (c) 2021 Xiamen Yaji Software Co., Ltd.

 http://www.cocos.com

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated engine source code (the "Software"), a limited,
 worldwide, royalty-free, non-assignable, revocable and non-exclusive license
 to use Cocos Creator solely to develop games on your target platforms. You shall
 not use Cocos Creator software for developing other software or tools that's
 used for developing games. You are not granted to publish, distribute,
 sublicense, and/or sell copies of Cocos Creator.

 The software or tools in this License Agreement are licensed, not sold.
 Xiamen Yaji Software Co., Ltd. reserves all rights not expressly granted to you.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
****************************************************************************/

#pragma once

#define USE_BLIT 0
#include "../RenderStage.h"
#include "gfx-gles3/GLES3Wrangler.h"
#if !USE_BLIT
#include "../ar/ARBackground.h"
#endif

namespace cc {
namespace pipeline {

class RenderFlow;
class RenderBatchedQueue;
class RenderInstancedQueue;
class RenderAdditiveLightQueue;
class PlanarShadowQueue;
class ForwardPipeline;
class UIPhase;
// ARModule ADD, need remove after modify
class ARBackground;

class CC_DLL ARStage : public RenderStage {
public:
    static const RenderStageInfo &getInitializeInfo();

    ARStage();
    ~ARStage() override;

    bool initialize(const RenderStageInfo &info) override;
    void activate(RenderPipeline *pipeline, RenderFlow *flow) override;
    void destroy() override;
    void render(scene::Camera *camera) override;

private:
    void                      dispenseRenderObject2Queues();
    static RenderStageInfo    initInfo;

    #if !USE_BLIT
    ARBackground *            _arBackground       = nullptr;
    #endif
    gfx::Rect                 _renderArea;
    uint                      _phaseID = 0;
    gfx::Texture *_backgroundTex{nullptr};
    GLuint _glTex{0U};
    bool _setTexFlag = false;
};

} // namespace pipeline
} // namespace cc
