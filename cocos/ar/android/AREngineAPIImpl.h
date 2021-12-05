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

#include "ar/IARAPI.h"
#include "renderer/gfx-agent/DeviceAgent.h"
#include <array>

class _jobject;

namespace cc {
namespace ar {

class AREngineAPIImpl : public IARAPI{
public:
    AREngineAPIImpl();
    ~AREngineAPIImpl() override;
    void start() override;
    void resume() override;
    void pause() override;
    void update() override;
    bool checkStart() override;

    float* getCameraPose() override;
    float* getCameraViewMatrix() override;
    float* getCameraProjectionMatrix() override;
    float* getCameraTexCoords() override;
    void setCameraTextureName(int id) override;
    void* getCameraTextureRef() override;

    //void setPlaneFeatureEnable(bool isOn) override;
    int getAddedPlanesCount() override;
    int getRemovedPlanesCount() override;
    int getUpdatedPlanesCount() override;
    void updatePlanesInfo() override;
    float* getAddedPlanesInfo() override;
    int* getRemovedPlanesInfo() override;
    float* getUpdatedPlanesInfo() override;
    
protected:
    _jobject* _impl;
    Pose *_cameraPose = new Pose();
    Matrix *_viewMatrix = new Matrix();
    Matrix *_projMatrix = new Matrix();
    TexCoords *_cameraTexCoords = new TexCoords();

    float* _addedPlanesInfo = new float[60];
    int* _removedPlanesInfo = new int[5];
    float* _updatedPlanesInfo = new float[60];
};

} // namespace ar
} // namespace cc