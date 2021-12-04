
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
    simd_float4x4 mCameraTransform;
    float* mCameraProjMat;
    float* mCameraTexCoords;
    CGSize mViewportSize;
    UIInterfaceOrientation mUIOrientation;
    
    NSMutableArray* mAddedPlanes;
}

- (id)init;
- (void)session : (ARSession*) session didUpdateFrame : (ARFrame*) frame;
- (void)startSession;
- (void)updateSession;
- (bool)checkStart;
- (float*)getCameraPose;
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
    // the device upright and the Home button on the Right
    mUIOrientation = UIInterfaceOrientationLandscapeRight;
    
    mAddedPlanes = [NSMutableArray new];
    
    return self;
}

- (void) session : (ARSession*) session didUpdateFrame : (ARFrame*)frame {
    mFrame = frame;
    
    //mViewportSize = CGSizeMake(width, height);
    mViewportSize = CGSizeMake(2532, 1170);
    
    [self updateCameraTexCoords];
    
    [self updateCameraTex];

    mCameraTransform = [frame.camera transform];
    
    matrix_float4x4 projMat = [mFrame.camera projectionMatrix];
    for(int i = 0; i < 4; ++i) {
        vector_float4 col = projMat.columns[i];
        mCameraProjMat[i * 4] = col.x;
        mCameraProjMat[i * 4 + 1] = col.y;
        mCameraProjMat[i * 4 + 2] = col.z;
        mCameraProjMat[i * 4 + 3] = col.w;
    }
}

- (void) session : (ARSession*) session didAddAnchors : (NSArray<__kindof ARAnchor *> *)anchors {
    for (ARAnchor* anchor in anchors) {
        if([anchor isKindOfClass:[ARPlaneAnchor class]]) {
            ARPlaneAnchor* plane = (ARPlaneAnchor*)anchor;
            [mAddedPlanes addObject : plane.identifier];
        }
    }
}

- (float *) getPlanesInfo {
    
}

- (void) startSession {
    ARWorldTrackingConfiguration* config = [ARWorldTrackingConfiguration new];
    
    config.planeDetection = ARPlaneDetectionHorizontal;
    
    [mARSession runWithConfiguration : config];
}

- (void) updateSession {
    /*
    mFrame = mARSession.currentFrame;
    if(!mFrame) return;
    [self updateTexture : mFrame];
    //*/
}

- (bool) checkStart {
    return mARSession && mFrame;
}

- (CVPixelBufferRef) getCameraTextureRef {
    return mPixelBuffer;
}

- (float*) getCameraPose {
    float * pose = (float *)malloc(sizeof(float) * 7);
    
    vector_float4 c0 = mCameraTransform.columns[0];
    vector_float4 c1 = mCameraTransform.columns[1];
    vector_float4 c2 = mCameraTransform.columns[2];
    vector_float4 c3 = mCameraTransform.columns[3];
    float qw = sqrt(1 + c0.x + c1.y + c2.z) * 0.5f;
    float qw_4 = 4.0f * qw;
    
    pose[0] = c3.x;
    pose[1] = c3.y;
    pose[2] = c3.z;
    pose[3] = -(c2.y - c1.z) / qw_4;
    pose[4] = -(c0.z - c2.x) / qw_4;
    pose[5] = -(c1.x - c0.y) / qw_4;
    pose[6] = qw;
    
    return pose;
}

- (float*) getCameraTexCoords {
    return mCameraTexCoords;
}

- (void) updateCameraTex {
    CVPixelBufferRef capturedBuffer = mFrame.capturedImage;
    
    if(!mPixelBuffer) {
        CGFloat width = CVPixelBufferGetWidth(capturedBuffer);
        CGFloat height = CVPixelBufferGetHeight(capturedBuffer);
        NSDictionary* cvBufferProperties = @{
            (__bridge NSString*)kCVPixelBufferOpenGLCompatibilityKey : @YES,
            (__bridge NSString*)kCVPixelBufferMetalCompatibilityKey : @YES,
        };
        CVPixelBufferCreate(kCFAllocatorDefault,
                            width,
                            height,
                            CVPixelBufferGetPixelFormatType(capturedBuffer),
                            (__bridge CFDictionaryRef)cvBufferProperties,
                            &mPixelBuffer);
    }

    CVPixelBufferLockBaseAddress(mPixelBuffer, 0);
    CVPixelBufferLockBaseAddress(capturedBuffer, kCVPixelBufferLock_ReadOnly);
    for(int i = 0; i < CVPixelBufferGetPlaneCount(capturedBuffer); ++i) {
        void* dest = CVPixelBufferGetBaseAddressOfPlane(mPixelBuffer, i);
        void* source = CVPixelBufferGetBaseAddressOfPlane(capturedBuffer, i);
        size_t height = CVPixelBufferGetHeightOfPlane(capturedBuffer, i);
        size_t bytesPerRowSrc = CVPixelBufferGetBytesPerRowOfPlane(capturedBuffer, i);
        memcpy(dest, source, height * bytesPerRowSrc);
    }
    CVPixelBufferUnlockBaseAddress(capturedBuffer, kCVPixelBufferLock_ReadOnly);
    CVPixelBufferUnlockBaseAddress(mPixelBuffer, 0);
}

- (void) updateCameraTexCoords {
    CGAffineTransform displayToCameraTransform = CGAffineTransformInvert([mFrame displayTransformForOrientation : mUIOrientation viewportSize : mViewportSize]);

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

float* ARKitAPIImpl::getCameraPose() {
    TransferImpl;
    return (__bridge float*)[impl getCameraPose];
}

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
