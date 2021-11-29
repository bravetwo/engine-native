
#include "ARKitAPIImpl.h"
#import <ARKit/ARKit.h>
#import <CoreVideo/CVPixelBuffer.h>
#import <dispatch/dispatch.h>

@interface ARKitAPI : NSObject<ARSessionDelegate> {
    ARSession* mARSession;
    ARFrame* mFrame;
    CVPixelBufferRef mPixelBuffer;
    dispatch_semaphore_t mSemaphore;
}

- (id)init;
//- (void)session : (ARSession*) session didUpdateFrame : (ARFrame*) frame;
- (void)startSession;
- (void)updateSession;
- (void)updateTexture : (ARFrame*) frame;
- (bool)checkStart;
- (CVPixelBufferRef)getCameraTextureRef;

@end

@implementation ARKitAPI

- (id) init {
    self = [super init];
    mARSession = [ARSession new];
    mARSession.delegate = self;
    
    //mSemaphore = dispatch_semaphore_create(1);
    
    return self;
}
//*
- (void) session : (ARSession*) session  didUpdateFrame : (ARFrame*) frame{
    mFrame = frame;
    CVPixelBufferRef capturedBuffer = frame.capturedImage;
    if(!mPixelBuffer) {
        CVPixelBufferCreate(nil,
                            CVPixelBufferGetWidth(capturedBuffer),
                            CVPixelBufferGetHeight(capturedBuffer),
                            //kCVPixelFormatType_422YpCbCr16,
                            CVPixelBufferGetPixelFormatType(capturedBuffer),
                            CVBufferGetAttachments(capturedBuffer, kCVAttachmentMode_ShouldPropagate),
                            &mPixelBuffer);
    }
    //dispatch_semaphore_wait(mSemaphore, DISPATCH_TIME_FOREVER);
    CVPixelBufferLockBaseAddress(mPixelBuffer, 0);
    CVPixelBufferLockBaseAddress(capturedBuffer, kCVPixelBufferLock_ReadOnly);
    for(int i = 0; i < CVPixelBufferGetPlaneCount(capturedBuffer); ++i) {
        void* dest = CVPixelBufferGetBaseAddressOfPlane(mPixelBuffer, i);
        void* source = CVPixelBufferGetBaseAddressOfPlane(capturedBuffer, i);
        size_t height = CVPixelBufferGetHeightOfPlane(capturedBuffer, i);
        size_t bytesPerRowSrc = CVPixelBufferGetBytesPerRowOfPlane(capturedBuffer, i);
        //size_t bytesPerRowDst = CVPixelBufferGetBytesPerRowOfPlane(mPixelBuffer, i);
        memcpy(dest, source, height * bytesPerRowSrc);
    }
    CVPixelBufferUnlockBaseAddress(capturedBuffer, kCVPixelBufferLock_ReadOnly);
    CVPixelBufferUnlockBaseAddress(mPixelBuffer, 0);
}
//*/
- (void) startSession {
    ARWorldTrackingConfiguration* config = [ARWorldTrackingConfiguration new];
    [mARSession runWithConfiguration : config];
}

- (void) updateSession {
    /*
    mFrame = mARSession.currentFrame;
    if(!mFrame) return;
    [self updateTexture : mFrame];
    //*/
}

- (void) updateTexture : (ARFrame*) frame {
    CVPixelBufferRef capturedBuffer = frame.capturedImage;
    if(!mPixelBuffer) {
        CVPixelBufferCreate(nil,
                            CVPixelBufferGetWidth(capturedBuffer),
                            CVPixelBufferGetHeight(capturedBuffer),
                            //kCVPixelFormatType_422YpCbCr16,
                            CVPixelBufferGetPixelFormatType(capturedBuffer),
                            CVBufferGetAttachments(capturedBuffer, kCVAttachmentMode_ShouldPropagate),
                            &mPixelBuffer);
    }
    dispatch_semaphore_wait(mSemaphore, DISPATCH_TIME_FOREVER);
    CVPixelBufferLockBaseAddress(mPixelBuffer, 0);
    CVPixelBufferLockBaseAddress(capturedBuffer, kCVPixelBufferLock_ReadOnly);
    for(int i = 0; i < CVPixelBufferGetPlaneCount(capturedBuffer); ++i) {
        void* dest = CVPixelBufferGetBaseAddressOfPlane(mPixelBuffer, i);
        void* source = CVPixelBufferGetBaseAddressOfPlane(capturedBuffer, i);
        size_t height = CVPixelBufferGetHeightOfPlane(capturedBuffer, i);
        size_t bytesPerRowSrc = CVPixelBufferGetBytesPerRowOfPlane(capturedBuffer, i);
        //size_t bytesPerRowDst = CVPixelBufferGetBytesPerRowOfPlane(mPixelBuffer, i);
        memcpy(dest, source, height * bytesPerRowSrc);
    }
    CVPixelBufferUnlockBaseAddress(capturedBuffer, kCVPixelBufferLock_ReadOnly);
    CVPixelBufferUnlockBaseAddress(mPixelBuffer, 0);
}

- (bool) checkStart {
    return mARSession && mFrame;
}

- (CVPixelBufferRef) getCameraTextureRef {
    //dispatch_semaphore_wait(mSemaphore, DISPATCH_TIME_FOREVER);
    //mPixelBuffer = mFrame.capturedImage;
    //mSemaphore = dispatch_semaphore_create(1);
    return mPixelBuffer;
}

- (void) syncTextureRef {
    //dispatch_semaphore_signal(mSemaphore);
}

@end

#define TransferImpl ARKitAPI *impl = (__bridge ARKitAPI *)_impl;

namespace cc {
namespace ar {

ARKitAPIImpl::ARKitAPIImpl() : _impl(nil) {
    _impl = [ARKitAPI new];
}

ARKitAPIImpl::~ARKitAPIImpl() {
    _impl = nullptr;
}

void ARKitAPIImpl::start() {
    TransferImpl;
    [impl startSession];
}

void ARKitAPIImpl::update() {
    TransferImpl;
    [impl updateSession];
}

void ARKitAPIImpl::resume() {}

void ARKitAPIImpl::pause() {}

bool ARKitAPIImpl::checkStart() {
    TransferImpl;
    return [impl checkStart];
}

void ARKitAPIImpl::setCameraTextureName(int id) {}

float* ARKitAPIImpl::getCameraPose() {}

float* ARKitAPIImpl::getCameraViewMatrix() {}

float* ARKitAPIImpl::getCameraProjectionMatrix() {}

float* ARKitAPIImpl::getCameraTexCoords() {}

void* ARKitAPIImpl::getCameraTextureRef() {
    TransferImpl;
    return (__bridge void *)[impl getCameraTextureRef];
}

void ARKitAPIImpl::syncTextureRef() {
    TransferImpl;
    [impl syncTextureRef];
}

} // namespace ar
} // namespace cc
