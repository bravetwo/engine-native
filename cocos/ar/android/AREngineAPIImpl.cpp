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

#include "./AREngineAPIImpl.h"
//#include "platform/interfaces/modules/Device.h"
#include "platform/java/jni/JniHelper.h"
#include "platform/java/jni/JniImp.h"
#include "renderer/gfx-base/GFXDevice.h"

#ifndef JCLS_ARAPI
#define JCLS_ARAPI "com/cocos/lib/CocosAREngineAPI"
#endif
#define JARG_ARAPI "L" JCLS_ARAPI ";"

namespace cc {
namespace ar {

AREngineAPIImpl::AREngineAPIImpl() : _impl(nullptr) {
    DLLOG("Construct AREngineImpl: %p", this);
    //auto *deviceAgent = gfx::DeviceAgent::getInstance();
    //if (deviceAgent) deviceAgent->setMultithreaded(false);
    JniMethodInfo methodInfo;
    if (JniHelper::getStaticMethodInfo(methodInfo, 
                                        JCLS_ARAPI,
                                        "init",
                                        "()" JARG_ARAPI)) {
        jobject jObj = methodInfo.env->CallStaticObjectMethod(
                methodInfo.classID,
                methodInfo.methodID
        );
        _impl = methodInfo.env->NewGlobalRef(jObj);
        DLLOG("ARCoreImpl: jObj: %p, _impl: %p", jObj, _impl);
        methodInfo.env->DeleteLocalRef(jObj);
        methodInfo.env->DeleteLocalRef(methodInfo.classID);
    }
}

AREngineAPIImpl::~AREngineAPIImpl() {
    if(_impl != nullptr) {
        JniMethodInfo methodInfo;
        if (JniHelper::getStaticMethodInfo(methodInfo,
                                            JCLS_ARAPI,
                                            "onDestroy",
                                            "(" JARG_ARAPI ")V")) {
            methodInfo.env->CallStaticVoidMethod(
                    methodInfo.classID,
                    methodInfo.methodID,
                    _impl
            );
            methodInfo.env->DeleteLocalRef(methodInfo.classID);
        }
        JniHelper::getEnv()->DeleteGlobalRef(_impl);
    }
    //auto *deviceAgent = gfx::DeviceAgent::getInstance();
    //if (deviceAgent) deviceAgent->setMultithreaded(true);
    DLLOG("Destruct ARCoreImpl: %p", this);
}

void AREngineAPIImpl::start() {
    if(_impl != nullptr) {
        JniMethodInfo methodInfo;
        if (JniHelper::getStaticMethodInfo(methodInfo,
                                            JCLS_ARAPI,
                                            "start",
                                            "(" JARG_ARAPI ")V")) {
            methodInfo.env->CallStaticVoidMethod(
                    methodInfo.classID,
                    methodInfo.methodID,
                    _impl
            );
            methodInfo.env->DeleteLocalRef(methodInfo.classID);
        }
        //*
        auto* device = gfx::DeviceAgent::getInstance();
        auto updateExec = [this]() {
            update();
        };

        device->registerOnAcquireCallback(updateExec);//*/
    }
}

void AREngineAPIImpl::resume() {
    if(_impl != nullptr) {
        JniMethodInfo methodInfo;
        if (JniHelper::getStaticMethodInfo(methodInfo,
                                            JCLS_ARAPI,
                                            "resume",
                                            "(" JARG_ARAPI ")V")) {
            methodInfo.env->CallStaticVoidMethod(
                    methodInfo.classID,
                    methodInfo.methodID,
                    _impl
            );
            methodInfo.env->DeleteLocalRef(methodInfo.classID);
        }
    }
}

void AREngineAPIImpl::pause() {
    if(_impl != nullptr) {
        JniMethodInfo methodInfo;
        if (JniHelper::getStaticMethodInfo(methodInfo,
                                            JCLS_ARAPI,
                                            "pause",
                                            "(" JARG_ARAPI ")V")) {
            methodInfo.env->CallStaticVoidMethod(
                    methodInfo.classID,
                    methodInfo.methodID,
                    _impl
            );
            methodInfo.env->DeleteLocalRef(methodInfo.classID);
        }
    }
}

void AREngineAPIImpl::update() {
    if(_impl != nullptr) {
        JniMethodInfo methodInfo;
        if (JniHelper::getStaticMethodInfo(methodInfo,
                                            JCLS_ARAPI,
                                            "update",
                                            "(" JARG_ARAPI ")V")) {
            methodInfo.env->CallStaticVoidMethod(
                    methodInfo.classID,
                    methodInfo.methodID,
                    _impl
            );
            methodInfo.env->DeleteLocalRef(methodInfo.classID);
        }
    }
}

bool AREngineAPIImpl::checkStart() {
    if(_impl != nullptr) {
        JniMethodInfo methodInfo;
        if (JniHelper::getStaticMethodInfo(methodInfo,
                                            JCLS_ARAPI,
                                            "checkStart",
                                            "(" JARG_ARAPI ")Z")) {
            auto check = static_cast<bool>(methodInfo.env->CallStaticBooleanMethod(
                    methodInfo.classID,
                    methodInfo.methodID,
                    _impl
            ));
            methodInfo.env->DeleteLocalRef(methodInfo.classID);
            return check;
        }
    }
    return false;
}

void AREngineAPIImpl::setCameraTextureName(int id) {
    if(_impl != nullptr) {
        JniMethodInfo methodInfo;
        if (JniHelper::getStaticMethodInfo(methodInfo,
                                            JCLS_ARAPI,
                                            "setCameraTextureName",
                                            "(" JARG_ARAPI "I" ")V")) {
            methodInfo.env->CallStaticVoidMethod(
                    methodInfo.classID,
                    methodInfo.methodID,
                    _impl,
                    id
            );
            methodInfo.env->DeleteLocalRef(methodInfo.classID);
        }
    }
}

float* AREngineAPIImpl::getCameraPose() {
    if(_impl != nullptr) {
        //auto *deviceAgent = gfx::DeviceAgent::getInstance();
        //if (deviceAgent) deviceAgent->setMultithreaded(false);
        JniMethodInfo methodInfo;
        if (JniHelper::getStaticMethodInfo(methodInfo,
                                            JCLS_ARAPI,
                                            "getCameraPose",
                                            "(" JARG_ARAPI ")[F"))
        {
            auto *array = static_cast<jfloatArray>(methodInfo.env->CallStaticObjectMethod(
                    methodInfo.classID,
                    methodInfo.methodID,
                    _impl
            ));
            jsize len = methodInfo.env->GetArrayLength(array);
            if (len <= 7) {
                jfloat* elems = methodInfo.env->GetFloatArrayElements(array, nullptr);
                if (elems) {
                    memcpy(_cameraPose, elems, sizeof(float) * len);
                    methodInfo.env->ReleaseFloatArrayElements(array, elems, 0);
                };
            }
            methodInfo.env->DeleteLocalRef(methodInfo.classID); 
        }
        //if (deviceAgent) deviceAgent->setMultithreaded(true);
    }
    return _cameraPose->data();
}

float* AREngineAPIImpl::getCameraViewMatrix() {
    if(_impl != nullptr) {
        JniMethodInfo methodInfo;
        if (JniHelper::getStaticMethodInfo(methodInfo,
                                            JCLS_ARAPI,
                                            "getCameraViewMatrix",
                                            "(" JARG_ARAPI ")[F")) {
            auto *array = static_cast<jfloatArray>(methodInfo.env->CallStaticObjectMethod(
                    methodInfo.classID,
                    methodInfo.methodID,
                    _impl
            ));
            jsize len = methodInfo.env->GetArrayLength(array);
            if (len <= 16) {
                jfloat* elems = methodInfo.env->GetFloatArrayElements(array, nullptr);
                if (elems) {
                    memcpy(_viewMatrix, elems, sizeof(float) * len);
                    methodInfo.env->ReleaseFloatArrayElements(array, elems, 0);
                };
            }
            methodInfo.env->DeleteLocalRef(methodInfo.classID);
        }
    }
    return _viewMatrix->data();
}

float* AREngineAPIImpl::getCameraProjectionMatrix() {
    if(_impl != nullptr) {
        JniMethodInfo methodInfo;
        if (JniHelper::getStaticMethodInfo(methodInfo,
                                            JCLS_ARAPI,
                                            "getCameraProjectionMatrix",
                                            "(" JARG_ARAPI ")[F")) {
            auto *array = static_cast<jfloatArray>(methodInfo.env->CallStaticObjectMethod(
                    methodInfo.classID,
                    methodInfo.methodID,
                    _impl
            ));
            jsize len = methodInfo.env->GetArrayLength(array);
            if (len <= 16) {
                jfloat* elems = methodInfo.env->GetFloatArrayElements(array, nullptr);
                if (elems) {
                    memcpy(_projMatrix, elems, sizeof(float) * len);
                    methodInfo.env->ReleaseFloatArrayElements(array, elems, 0);
                };
            }
            methodInfo.env->DeleteLocalRef(methodInfo.classID);
        }
    }
    return _projMatrix->data();
}

float* AREngineAPIImpl::getCameraTexCoords() {
    if(_impl != nullptr) {
        JniMethodInfo methodInfo;
        if (JniHelper::getStaticMethodInfo(methodInfo,
                                            JCLS_ARAPI,
                                            "getCameraTexCoords",
                                            "(" JARG_ARAPI ")[F")) {
            auto *array = static_cast<jfloatArray>(methodInfo.env->CallStaticObjectMethod(
                    methodInfo.classID,
                    methodInfo.methodID,
                    _impl
            ));
            jsize len = methodInfo.env->GetArrayLength(array);
            if (len <= 8) {
                jfloat* elems = methodInfo.env->GetFloatArrayElements(array, nullptr);
                if (elems) {
                    memcpy(_cameraTexCoords, elems, sizeof(float) * len);
                    methodInfo.env->ReleaseFloatArrayElements(array, elems, 0);
                };
            }
            methodInfo.env->DeleteLocalRef(methodInfo.classID);
        }
    }
    return _cameraTexCoords->data();
}

void* AREngineAPIImpl::getCameraTextureRef() {
    return nullptr;
}

//void AREngineAPIImpl::setPlaneFeatureEnable(bool isOn) {}
int AREngineAPIImpl::getAddedPlanesCount() {
    if(_impl != nullptr) {
        JniMethodInfo methodInfo;
        if (JniHelper::getStaticMethodInfo(methodInfo,
                                            JCLS_ARAPI,
                                            "getAddedPlanesCount",
                                            "(" JARG_ARAPI ")I")) {
            auto result = static_cast<int>(methodInfo.env->CallStaticIntMethod(
                    methodInfo.classID,
                    methodInfo.methodID,
                    _impl
            ));
            methodInfo.env->DeleteLocalRef(methodInfo.classID);
            return result;
        }
    }
    return -1;
}
int AREngineAPIImpl::getRemovedPlanesCount() {
    if(_impl != nullptr) {
        JniMethodInfo methodInfo;
        if (JniHelper::getStaticMethodInfo(methodInfo,
                                            JCLS_ARAPI,
                                            "getRemovedPlanesCount",
                                            "(" JARG_ARAPI ")I")) {
            auto result = static_cast<int>(methodInfo.env->CallStaticIntMethod(
                    methodInfo.classID,
                    methodInfo.methodID,
                    _impl
            ));
            methodInfo.env->DeleteLocalRef(methodInfo.classID);
            return result;
        }
    }
    return -1;
}
int AREngineAPIImpl::getUpdatedPlanesCount() {
    if(_impl != nullptr) {
        JniMethodInfo methodInfo;
        if (JniHelper::getStaticMethodInfo(methodInfo,
                                            JCLS_ARAPI,
                                            "getUpdatedPlanesCount",
                                            "(" JARG_ARAPI ")I")) {
            auto result = static_cast<int>(methodInfo.env->CallStaticIntMethod(
                    methodInfo.classID,
                    methodInfo.methodID,
                    _impl
            ));
            methodInfo.env->DeleteLocalRef(methodInfo.classID);
            return result;
        }
    }
    return -1;
}
void AREngineAPIImpl::updatePlanesInfo() {
    if(_impl != nullptr) {
        JniMethodInfo methodInfo;
        if (JniHelper::getStaticMethodInfo(methodInfo,
                                            JCLS_ARAPI,
                                            "updatePlanesInfo",
                                            "(" JARG_ARAPI ")V")) {
            methodInfo.env->CallStaticVoidMethod(
                    methodInfo.classID,
                    methodInfo.methodID,
                    _impl
            );
            methodInfo.env->DeleteLocalRef(methodInfo.classID);
        }
    }
}
float* AREngineAPIImpl::getAddedPlanesInfo() {
    if(_impl != nullptr) {
        JniMethodInfo methodInfo;
        if (JniHelper::getStaticMethodInfo(methodInfo,
                                            JCLS_ARAPI,
                                            "getAddedPlanesInfo",
                                            "(" JARG_ARAPI ")[F")) {
            jfloatArray array = (jfloatArray)methodInfo.env->CallStaticObjectMethod(
                    methodInfo.classID,
                    methodInfo.methodID,
                    _impl
            );
            jsize len = methodInfo.env->GetArrayLength(array);
            if (len <= 5 * 12) {
                jfloat* elems = methodInfo.env->GetFloatArrayElements(array, 0);
                if (elems) {
                    //_addedPlanesInfo->reserve(len);
                    //memcpy(&_addedPlanesInfo[0], elems, sizeof(float) * len);
                    memcpy(_addedPlanesInfo, elems, sizeof(float) * len);
                    methodInfo.env->ReleaseFloatArrayElements(array, elems, 0);
                };
            }
            methodInfo.env->DeleteLocalRef(methodInfo.classID);
        }
    }
    return _addedPlanesInfo;
}
int* AREngineAPIImpl::getRemovedPlanesInfo() {
    if(_impl != nullptr) {
        JniMethodInfo methodInfo;
        if (JniHelper::getStaticMethodInfo(methodInfo,
                                            JCLS_ARAPI,
                                            "getRemovedPlanesInfo",
                                            "(" JARG_ARAPI ")[I")) {
            jintArray array = (jintArray)methodInfo.env->CallStaticObjectMethod(
                    methodInfo.classID,
                    methodInfo.methodID,
                    _impl
            );
            jsize len = methodInfo.env->GetArrayLength(array);
            if (len <= 5) {
                jint* elems = methodInfo.env->GetIntArrayElements(array, nullptr);
                if (elems) {
                    memcpy(_removedPlanesInfo, elems, sizeof(int) * len);
                    methodInfo.env->ReleaseIntArrayElements(array, elems, 0);
                };
            }
            methodInfo.env->DeleteLocalRef(methodInfo.classID);
        }
    }
    return _removedPlanesInfo;//->data();
}
float* AREngineAPIImpl::getUpdatedPlanesInfo() {
    if(_impl != nullptr) {
        JniMethodInfo methodInfo;
        if (JniHelper::getStaticMethodInfo(methodInfo,
                                            JCLS_ARAPI,
                                            "getUpdatedPlanesInfo",
                                            "(" JARG_ARAPI ")[F")) {
            jfloatArray array = (jfloatArray)methodInfo.env->CallStaticObjectMethod(
                    methodInfo.classID,
                    methodInfo.methodID,
                    _impl
            );
            jsize len = methodInfo.env->GetArrayLength(array);
            if (len <= 5 * 12) {
                jfloat* elems = methodInfo.env->GetFloatArrayElements(array, 0);
                if (elems) {
                    memcpy(_updatedPlanesInfo, elems, sizeof(float) * len);
                    methodInfo.env->ReleaseFloatArrayElements(array, elems, 0);
                };
            }
            methodInfo.env->DeleteLocalRef(methodInfo.classID);
        }
    }
    return _updatedPlanesInfo;
}

} // namespace ar
} // namespace cc