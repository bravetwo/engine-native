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
#include "base/threading/Semaphore.h"
#include <array>

class _jobject;

namespace cc {
namespace ar {

class ARAndroidAPIImpl : public IARAPI{
public:
    ARAndroidAPIImpl();
    ~ARAndroidAPIImpl() override;
    void start() override;
    void resume() override;
    void pause() override;
    void update() override;
    int getAPIState() override;
    void beforeUpdate() override;

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
    int getInfoLength() override;
    
protected:
    _jobject* _impl;
    Pose *_cameraPose = new Pose();
    Matrix *_viewMatrix = new Matrix();
    Matrix *_projMatrix = new Matrix();
    TexCoords *_cameraTexCoords = new TexCoords();
    void onBeforeUpdate();

    //float* _addedPlanesInfo = new float[60];
    float* _addedPlanesInfo = nullptr;
    int* _removedPlanesInfo = new int[5];
    //'float* _updatedPlanesInfo = new float[60];
    float* _updatedPlanesInfo = nullptr;
    int _infoLength = 0;

    Semaphore _frameBoundarySemaphore{1};
};

} // namespace ar
} // namespace cc