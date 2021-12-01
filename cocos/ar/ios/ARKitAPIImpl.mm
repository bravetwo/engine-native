
#include "ARKitAPIImpl.h"
#import <ARKit/ARKit.h>
#import <CoreVideo/CVPixelBuffer.h>
#import <dispatch/dispatch.h>

static const float kCameraTexCoordData[8] = {
    0.0, 0.0,  0.0, 1.0,
    1.0, 0.0,  1.0, 1.0,
};

@interface ARKitAPI : NSObject<ARSessionDelegate> {
    ARSession* mARSession;
    ARFrame* mFrame;
    CVPixelBufferRef mPixelBuffer;
    dispatch_semaphore_t mSemaphore;
    float* mCameraProjMat;
    float* mCameraTexCoords;
    CGSize mViewportSize;
}

- (id)init;
//- (void)session : (ARSession*) session didUpdateFrame : (ARFrame*) frame;
- (void)startSession;
- (void)updateSession;
- (void)updateTexture : (ARFrame*) frame;
- (bool)checkStart;
- (CVPixelBufferRef)getCameraTextureRef;
- (float*)getCameraTexCoords;
- (void)updateCameraTexCoords;
@end

@implementation ARKitAPI

- (id) init {
    self = [super init];
    mARSession = [ARSession new];
    mARSession.delegate = self;
    
    //mSemaphore = dispatch_semaphore_create(1);
    mCameraProjMat = (float *)malloc(sizeof(float) * 16);
    mCameraTexCoords = (float *)malloc(sizeof(float) * 8);
    
    
    return self;
}
//*
- (void) session : (ARSession*) session  didUpdateFrame : (ARFrame*) frame{
    mFrame = frame;
    
    CVPixelBufferRef capturedBuffer = frame.capturedImage;
    
    CGFloat width = CVPixelBufferGetWidth(capturedBuffer);
    CGFloat height = CVPixelBufferGetHeight(capturedBuffer);
    mViewportSize = CGSizeMake(width, height);
    
    if(!mPixelBuffer) {
        
        NSDictionary* cvBufferProperties = @{
            (__bridge NSString*)kCVPixelBufferOpenGLCompatibilityKey : @YES,
            (__bridge NSString*)kCVPixelBufferMetalCompatibilityKey : @YES,
        };
        CVPixelBufferCreate(nil,
                            width,
                            height,
                            //kCVPixelFormatType_422YpCbCr16,
                            CVPixelBufferGetPixelFormatType(capturedBuffer),
                            //CVBufferGetAttachments(capturedBuffer, kCVAttachmentMode_ShouldPropagate),
                            (__bridge CFDictionaryRef)cvBufferProperties,
                            &mPixelBuffer);
    }
    
    [self updateCameraTexCoords];
    
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
    
    matrix_float4x4 projMat = [mFrame.camera projectionMatrix];
    for(int i = 0; i < 4; ++i) {
        vector_float4 col = projMat.columns[i];
        mCameraProjMat[i * 4] = col.x;
        mCameraProjMat[i * 4 + 1] = col.y;
        mCameraProjMat[i * 4 + 2] = col.z;
        mCameraProjMat[i * 4 + 3] = col.w;
    }
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

- (float*) getCameraTexCoords {
    
    return mCameraTexCoords;
}

- (void) updateCameraTexCoords {
    CGAffineTransform displayToCameraTransform = CGAffineTransformInvert([mFrame displayTransformForOrientation : UIInterfaceOrientationLandscapeRight viewportSize : mViewportSize]);

    for (NSInteger index = 0; index < 4; index++) {
        NSInteger textureCoordIndex = index * 2;
        CGPoint textureCoord = CGPointMake(kCameraTexCoordData[textureCoordIndex], kCameraTexCoordData[textureCoordIndex + 1]);
        CGPoint transformedCoord = CGPointApplyAffineTransform(textureCoord, displayToCameraTransform);
        mCameraTexCoords[textureCoordIndex] = transformedCoord.x;
        mCameraTexCoords[textureCoordIndex + 1] = transformedCoord.y;
    }
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

float* ARKitAPIImpl::getCameraTexCoords() {
    TransferImpl;
    return (__bridge float*)[impl getCameraTexCoords];
}

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
