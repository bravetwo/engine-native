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

#include "./ARAndroidAPIImpl.h"
#include "base/threading/MessageQueue.h"
#include "platform/java/jni/JniHelper.h"
#include "platform/java/jni/JniImp.h"
#include "renderer/gfx-base/GFXDevice.h"

#ifndef JCLS_ARAPI
    // arengine only supports gles2, arcore supports gles2 and gles3
    // setting the CC_USE_GLES3 off is needed while using USE_AR_AUTO or USE_AR_ENGINE
    #if USE_AR_AUTO
        #define JCLS_ARAPI "com/cocos/lib/CocosARAutoImpl"
    #elif USE_AR_ENGINE
        #define JCLS_ARAPI "com/cocos/lib/CocosAREngineAPI"
    #elif USE_AR_CORE
        #define JCLS_ARAPI "com/cocos/lib/CocosARCoreAPI"
    #endif

#endif
#define JARG_ARAPI "L" JCLS_ARAPI ";"

namespace cc {
namespace ar {

ARAndroidAPIImpl::ARAndroidAPIImpl() : _impl(nullptr) {
    DLLOG("Construct ARAndroidImpl: %p", this);
    JniMethodInfo methodInfo;
    if (JniHelper::getStaticMethodInfo(methodInfo,
                                       JCLS_ARAPI,
                                       "init",
                                       "()" JARG_ARAPI)) {
        jobject jObj = methodInfo.env->CallStaticObjectMethod(
            methodInfo.classID,
            methodInfo.methodID);
        _impl = methodInfo.env->NewGlobalRef(jObj);
        DLLOG("ARAndroidImpl: jObj: %p, _impl: %p", jObj, _impl);
        methodInfo.env->DeleteLocalRef(jObj);
        methodInfo.env->DeleteLocalRef(methodInfo.classID);

        _cameraPose      = new Pose();
        _viewMatrix      = new Matrix();
        _projMatrix      = new Matrix();
        _cameraTexCoords = new TexCoords();
    }
}

ARAndroidAPIImpl::~ARAndroidAPIImpl() {
    if (_impl != nullptr) {
        JniMethodInfo methodInfo;
        if (JniHelper::getStaticMethodInfo(methodInfo,
                                           JCLS_ARAPI,
                                           "onDestroy",
                                           "(" JARG_ARAPI ")V")) {
            methodInfo.env->CallStaticVoidMethod(
                methodInfo.classID,
                methodInfo.methodID,
                _impl);
            methodInfo.env->DeleteLocalRef(methodInfo.classID);
        }
        JniHelper::getEnv()->DeleteGlobalRef(_impl);
    }
    DLLOG("Destruct ARAndroidImpl: %p", this);
}

void ARAndroidAPIImpl::start() {
    if (_impl != nullptr) {
        JniMethodInfo methodInfo;
        if (JniHelper::getStaticMethodInfo(methodInfo,
                                           JCLS_ARAPI,
                                           "start",
                                           "(" JARG_ARAPI ")V")) {
            methodInfo.env->CallStaticVoidMethod(
                methodInfo.classID,
                methodInfo.methodID,
                _impl);
            methodInfo.env->DeleteLocalRef(methodInfo.classID);
        }
    }
}

void ARAndroidAPIImpl::resume() {
    if (_impl != nullptr) {
        JniMethodInfo methodInfo;
        if (JniHelper::getStaticMethodInfo(methodInfo,
                                           JCLS_ARAPI,
                                           "resume",
                                           "(" JARG_ARAPI ")V")) {
            methodInfo.env->CallStaticVoidMethod(
                methodInfo.classID,
                methodInfo.methodID,
                _impl);
            methodInfo.env->DeleteLocalRef(methodInfo.classID);
        }
    }
}

void ARAndroidAPIImpl::pause() {
    if (_impl != nullptr) {
        JniMethodInfo methodInfo;
        if (JniHelper::getStaticMethodInfo(methodInfo,
                                           JCLS_ARAPI,
                                           "pause",
                                           "(" JARG_ARAPI ")V")) {
            methodInfo.env->CallStaticVoidMethod(
                methodInfo.classID,
                methodInfo.methodID,
                _impl);
            methodInfo.env->DeleteLocalRef(methodInfo.classID);
        }
    }
}

void ARAndroidAPIImpl::update() {
    if (_impl != nullptr) {
        JniMethodInfo methodInfo;
        if (JniHelper::getStaticMethodInfo(methodInfo,
                                           JCLS_ARAPI,
                                           "update",
                                           "(" JARG_ARAPI ")V")) {
            methodInfo.env->CallStaticVoidMethod(
                methodInfo.classID,
                methodInfo.methodID,
                _impl);
            methodInfo.env->DeleteLocalRef(methodInfo.classID);
        }
    }
}

int ARAndroidAPIImpl::getAPIState() {
    if (_impl != nullptr) {
        JniMethodInfo methodInfo;
        if (JniHelper::getStaticMethodInfo(methodInfo,
                                           JCLS_ARAPI,
                                           "getAPIState",
                                           "(" JARG_ARAPI ")I")) {
            auto result = static_cast<int>(methodInfo.env->CallStaticIntMethod(
                methodInfo.classID,
                methodInfo.methodID,
                _impl));
            methodInfo.env->DeleteLocalRef(methodInfo.classID);
            return result;
        }
    }
    return -1;
}

void ARAndroidAPIImpl::setCameraTextureName(int id) {
    if (_impl != nullptr) {
        JniMethodInfo methodInfo;
        if (JniHelper::getStaticMethodInfo(methodInfo,
                                           JCLS_ARAPI,
                                           "setCameraTextureName",
                                           "(" JARG_ARAPI "I"
                                           ")V")) {
            methodInfo.env->CallStaticVoidMethod(
                methodInfo.classID,
                methodInfo.methodID,
                _impl,
                id);
            methodInfo.env->DeleteLocalRef(methodInfo.classID);
        }
    }
}

float* ARAndroidAPIImpl::getCameraPose() {
    if (_impl != nullptr) {
        JniMethodInfo methodInfo;
        if (JniHelper::getStaticMethodInfo(methodInfo,
                                           JCLS_ARAPI,
                                           "getCameraPose",
                                           "(" JARG_ARAPI ")[F")) {
            auto* array = static_cast<jfloatArray>(methodInfo.env->CallStaticObjectMethod(
                methodInfo.classID,
                methodInfo.methodID,
                _impl));
            jsize len   = methodInfo.env->GetArrayLength(array);
            if (len <= 7) {
                jfloat* elems = methodInfo.env->GetFloatArrayElements(array, nullptr);
                if (elems) {
                    memcpy(_cameraPose, elems, sizeof(float) * len);
                    methodInfo.env->ReleaseFloatArrayElements(array, elems, 0);
                };
            }
            methodInfo.env->DeleteLocalRef(methodInfo.classID);
        }
    }
    return _cameraPose->data();
}

float* ARAndroidAPIImpl::getCameraViewMatrix() {
    if (_impl != nullptr) {
        JniMethodInfo methodInfo;
        if (JniHelper::getStaticMethodInfo(methodInfo,
                                           JCLS_ARAPI,
                                           "getCameraViewMatrix",
                                           "(" JARG_ARAPI ")[F")) {
            auto* array = static_cast<jfloatArray>(methodInfo.env->CallStaticObjectMethod(
                methodInfo.classID,
                methodInfo.methodID,
                _impl));
            jsize len   = methodInfo.env->GetArrayLength(array);
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

float* ARAndroidAPIImpl::getCameraProjectionMatrix() {
    if (_impl != nullptr) {
        JniMethodInfo methodInfo;
        if (JniHelper::getStaticMethodInfo(methodInfo,
                                           JCLS_ARAPI,
                                           "getCameraProjectionMatrix",
                                           "(" JARG_ARAPI ")[F")) {
            auto* array = static_cast<jfloatArray>(methodInfo.env->CallStaticObjectMethod(
                methodInfo.classID,
                methodInfo.methodID,
                _impl));
            jsize len   = methodInfo.env->GetArrayLength(array);
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

float* ARAndroidAPIImpl::getCameraTexCoords() {
    if (_impl != nullptr) {
        JniMethodInfo methodInfo;
        if (JniHelper::getStaticMethodInfo(methodInfo,
                                           JCLS_ARAPI,
                                           "getCameraTexCoords",
                                           "(" JARG_ARAPI ")[F")) {
            auto* array = static_cast<jfloatArray>(methodInfo.env->CallStaticObjectMethod(
                methodInfo.classID,
                methodInfo.methodID,
                _impl));
            jsize len   = methodInfo.env->GetArrayLength(array);
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

void* ARAndroidAPIImpl::getCameraTextureRef() {
    return nullptr;
}

void ARAndroidAPIImpl::updatePlanesInfo() {
    if (_impl != nullptr) {
        JniMethodInfo methodInfo;
        if (JniHelper::getStaticMethodInfo(methodInfo,
                                           JCLS_ARAPI,
                                           "updatePlaneDetection",
                                           "(" JARG_ARAPI ")V")) {
            methodInfo.env->CallStaticVoidMethod(
                methodInfo.classID,
                methodInfo.methodID,
                _impl);
            methodInfo.env->DeleteLocalRef(methodInfo.classID);
        }
    }
}
float* ARAndroidAPIImpl::getAddedPlanesInfo() {
    delete[] _addedPlanesInfo;
    _addedPlanesInfo = nullptr;

    if (_impl != nullptr) {
        JniMethodInfo methodInfo;
        if (JniHelper::getStaticMethodInfo(methodInfo,
                                           JCLS_ARAPI,
                                           "getAddedPlanesInfo",
                                           "(" JARG_ARAPI ")[F")) {
            jfloatArray array = (jfloatArray)methodInfo.env->CallStaticObjectMethod(
                methodInfo.classID,
                methodInfo.methodID,
                _impl);
            jsize len = methodInfo.env->GetArrayLength(array);
            jfloat* elems = methodInfo.env->GetFloatArrayElements(array, nullptr);
            if (elems) {
                auto* info  = new float[len];
                _infoLength = len;
                memcpy(info, elems, sizeof(float) * len);
                methodInfo.env->ReleaseFloatArrayElements(array, elems, 0);
                _addedPlanesInfo = info;
            }
            methodInfo.env->DeleteLocalRef(methodInfo.classID);
        }
    }
    return _addedPlanesInfo;
}
int* ARAndroidAPIImpl::getRemovedPlanesInfo() {
    if (_impl != nullptr) {
        JniMethodInfo methodInfo;
        if (JniHelper::getStaticMethodInfo(methodInfo,
                                           JCLS_ARAPI,
                                           "getRemovedPlanesInfo",
                                           "(" JARG_ARAPI ")[I")) {
            jintArray array = (jintArray)methodInfo.env->CallStaticObjectMethod(
                methodInfo.classID,
                methodInfo.methodID,
                _impl);
            jsize len = methodInfo.env->GetArrayLength(array);
            jint* elems = methodInfo.env->GetIntArrayElements(array, nullptr);
            if (elems) {
                auto* info  = new int[len];
                _infoLength = len;
                memcpy(info, elems, sizeof(int) * len);
                methodInfo.env->ReleaseIntArrayElements(array, elems, 0);
                _removedPlanesInfo = info;
            };
            methodInfo.env->DeleteLocalRef(methodInfo.classID);
        }
    }
    return _removedPlanesInfo;
}
float* ARAndroidAPIImpl::getUpdatedPlanesInfo() {
    if (_impl != nullptr) {
        JniMethodInfo methodInfo;
        if (JniHelper::getStaticMethodInfo(methodInfo,
                                           JCLS_ARAPI,
                                           "getUpdatedPlanesInfo",
                                           "(" JARG_ARAPI ")[F")) {
            jfloatArray array = (jfloatArray)methodInfo.env->CallStaticObjectMethod(
                methodInfo.classID,
                methodInfo.methodID,
                _impl);
            jsize len = methodInfo.env->GetArrayLength(array);
            jfloat* elems = methodInfo.env->GetFloatArrayElements(array, nullptr);
            if (elems) {
                auto* info  = new float[len];
                _infoLength = len;
                memcpy(info, elems, sizeof(float) * len);
                methodInfo.env->ReleaseFloatArrayElements(array, elems, 0);
                _updatedPlanesInfo = info;
            };
            methodInfo.env->DeleteLocalRef(methodInfo.classID);
        }
    }
    return _updatedPlanesInfo;
}

int ARAndroidAPIImpl::getInfoLength() {
    return _infoLength;
}

} // namespace ar
} // namespace cc