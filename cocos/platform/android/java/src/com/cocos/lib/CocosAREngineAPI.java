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

package com.cocos.lib;

import com.huawei.hiar.ARAnchor;
import com.huawei.hiar.AREnginesApk;
import com.huawei.hiar.ARCamera;
//import com.huawei.hiar.Config;
//import com.huawei.hiar.Config.InstantPlacementMode;
import com.huawei.hiar.ARConfigBase;
import com.huawei.hiar.ARWorldTrackingConfig;
//import com.huawei.hiar.DepthPoint;
import com.huawei.hiar.ARFrame;
import com.huawei.hiar.ARPose;
import com.huawei.hiar.ARHitResult;
//import com.huawei.hiar.InstantPlacementPoint;
//import com.huawei.hiar.LightEstimate;
import com.huawei.hiar.ARPlane;
import com.huawei.hiar.ARPoint;
//import com.huawei.hiar.Point.OrientationMode;
//import com.huawei.hiar.PointCloud;
import com.huawei.hiar.ARSession;
import com.huawei.hiar.ARTrackable;
//import com.huawei.hiar.TrackingFailureReason;
//import com.huawei.hiar.TrackingState;
//import com.huawei.hiar.Coordinates2d;
import com.huawei.hiar.exceptions.ARCameraNotAvailableException;
import com.huawei.hiar.exceptions.ARNotYetAvailableException;
import com.huawei.hiar.exceptions.ARUnSupportedConfigurationException;
import com.huawei.hiar.exceptions.ARUnavailableClientSdkTooOldException;
import com.huawei.hiar.exceptions.ARUnavailableDeviceNotCompatibleException;
import com.huawei.hiar.exceptions.ARUnavailableEmuiNotCompatibleException;
import com.huawei.hiar.exceptions.ARUnavailableServiceApkTooOldException;
import com.huawei.hiar.exceptions.ARUnavailableServiceNotInstalledException;
//import com.huawei.hiar.exceptions.UnavailableApkTooOldException;
//import com.huawei.hiar.exceptions.UnavailableArcoreNotInstalledException;
//import com.huawei.hiar.exceptions.UnavailableDeviceNotCompatibleException;
//import com.huawei.hiar.exceptions.UnavailableSdkTooOldException;
import com.huawei.hiar.exceptions.ARUnavailableUserDeclinedInstallationException;

import java.nio.FloatBuffer;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.List;
import android.content.Context;
import android.app.Activity;
import android.widget.Toast;
import android.os.Handler;
import android.util.Log;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

public class CocosAREngineAPI implements CocosARAPI, ActivityCompat.OnRequestPermissionsResultCallback {
    private static final String TAG = CocosAREngineAPI.class.getSimpleName();
    private static CocosAREngineAPI api = null;
    
    private CocosARDisplayRotationHelper mDisplayRotationHelper;

    private ARSession mSession;
    private ARFrame mFrame;
    private ARCamera mCamera;

    private float[] mCameraPose = new float[7];
    private float[] mViewMatrix = new float[16];
    private float[] mProjMatrix = new float[16];

    private float[] mQuadCoords = {-1f, -1f, -1f, 1f, 1f, -1f, 1f, 1f};
    private float[] mCameraTexCoords = new float[8];

    // requestInstall(Activity, true) will triggers installation of
    // Google Play Services for AR if necessary.
    private boolean mUserRequestedInstall = true;
    private boolean mActive;

    // plane feature
    private final static int PLANE_INFOS_SIZE = 12;
    private int planesMaxSize = 5;
    private int planeTag = 0;
    private final HashMap<Integer, ARPlane> planesMap = new HashMap<>();
    private final HashMap<ARPlane, Integer> planesIndexMap = new HashMap<>();
    private List<Integer> mAddedPlanes = new ArrayList<>();
    private List<Integer> mRemovedPlanes = new ArrayList<>();
    private List<Integer> mUpdatedPlanes = new ArrayList<>();

    public static CocosAREngineAPI init() {
        api = new CocosAREngineAPI();
        return api;
    }

    public static void start(final CocosAREngineAPI api) {
        api.checkCamera();
        if (api.checkSupportAndUpdate() > 0) {
            api.startSession();
        }
    }
    public static void onDestroy(final CocosAREngineAPI api) {
        api.closeSession();
    }
    public static void resume(final CocosAREngineAPI api) {
        api.checkCamera();
        if (api.checkSupportAndUpdate() > 0) {
            api.startSession();
        }
    }
    public static void pause(final CocosAREngineAPI api) {
        api.pauseSession();
    }
    public static void update(final CocosAREngineAPI api) {
        if (api.mSession == null) return;
        api.updateSession();
        api.updateCameraPose();
        api.updateCameraTexCoords();
    }

    public static boolean checkStart(final CocosAREngineAPI api) {
        if (api != null && api.mSession != null) {
            return true;
        }
        return false;
    }

    public static void setCameraTextureName(final CocosAREngineAPI api, int id) {
        api.mSession.setCameraTextureName(id);
    }

    public static float[] getCameraPose(final CocosAREngineAPI api) {
        return api.mCameraPose;
    }
    public static float[] getCameraViewMatrix(final CocosAREngineAPI api) {
        api.mCamera.getViewMatrix(api.mViewMatrix, 0);
        return api.mViewMatrix;
    }
    public static float[] getCameraProjectionMatrix(final CocosAREngineAPI api) {
        if (api.mSession != null) api.mCamera.getProjectionMatrix(api.mProjMatrix, 0, 0.01f, 1000.0f);
        return api.mProjMatrix;
    }
    public static float[] getCameraTexCoords(final CocosAREngineAPI api) {
        return api.mCameraTexCoords;
    }

    // plane feature
    public static void updatePlanesInfo(final CocosAREngineAPI api) {
        api.updatePlanesInfo(api.planesMaxSize);
    }
    public static float[] getAddedPlanesInfo(final CocosAREngineAPI api) {
        return api.getPlanesInfoFromList(api.mAddedPlanes);
    }
    public static int[] getRemovedPlanesInfo(final CocosAREngineAPI api) {
        int size = api.mRemovedPlanes.size();
        //int[] result = new int[size];
        int[] result = new int[5];
        Integer[] temp = api.mRemovedPlanes.toArray(new Integer[size]);
        //size = size > 5 ? 5 : size; 
        //for (int n = 0; n < size; ++n) {
        for (int n = 0; n < 5; ++n) {
            if( n < size)
                result[n] = temp[n];
            else
                result[n] = -1;
        }
        return result;
    }
    public static float[] getUpdatedPlanesInfo(final CocosAREngineAPI api) {
        return api.getPlanesInfoFromList(api.mUpdatedPlanes);
    }
    public static int getAddedPlanesCount(final CocosAREngineAPI api) {
        return api.mAddedPlanes.size();
    }
    public static int getRemovedPlanesCount(final CocosAREngineAPI api) {
        return api.mRemovedPlanes.size();
    }
    public static int getUpdatedPlanesCount(final CocosAREngineAPI api) {
        return api.mUpdatedPlanes.size();
    }

    // for CocosARDisplayRotationHelper 
    public void setDisplayGeometry(int displayRotation, int width, int height) {
        mSession.setDisplayGeometry(displayRotation, width, height);
    }

    private void updateCameraPose() {
        ARPose pose = mCamera.getDisplayOrientedPose();
        mCameraPose[0] = pose.tx();
        mCameraPose[1] = pose.ty();
        mCameraPose[2] = pose.tz();
        mCameraPose[3] = pose.qx();
        mCameraPose[4] = pose.qy();
        mCameraPose[5] = pose.qz();
        mCameraPose[6] = pose.qw();
    }

    private void updateCameraTexCoords() {
        if (mFrame.hasDisplayGeometryChanged()) {
            // If display rotation changed (also includes view size change), we need to re-query the UV
            // coordinates for the screen rect, as they may have changed as well.
            FloatBuffer quadCoords = FloatBuffer.allocate(8);
            FloatBuffer texCoords = FloatBuffer.allocate(8);
            quadCoords.put(mQuadCoords);
            mFrame.transformDisplayUvCoords(quadCoords, texCoords);
            mCameraTexCoords = texCoords.array();
        }
    }

    private void checkCamera() {
        Activity activity = GlobalObject.getActivity();
        if (!CocosARCameraPermissionHelper.hasCameraPermission(activity)) {
            CocosARCameraPermissionHelper.requestCameraPermission(activity);
        }
    }

    // -1 not supported, 0 not installed, 1 ready
    private int checkSupportAndUpdate() {
        Activity activity = GlobalObject.getActivity();
        AREnginesApk.ARAvailability availability = AREnginesApk.checkAvailability(activity);
        if (availability.isTransient()) {
            // Continue to query availability at 5Hz while compatibility is checked in the background.
            new Handler().postDelayed(new Runnable() {
            @Override
            public void run() {
                checkSupportAndUpdate();
            }
            }, 200);
        }

        switch (availability) {
            case SUPPORTED_INSTALLED:
              break;
            case SUPPORTED_APK_TOO_OLD:
            case SUPPORTED_NOT_INSTALLED:
              try {
                // Request ARCore installation or update if needed.
                  AREnginesApk.ARInstallStatus installStatus =
                    AREnginesApk.requestInstall(activity, /*userRequestedInstall=*/ true);
                switch (installStatus) {
                  case INSTALL_REQUESTED:
                    Log.e(TAG, "ARCore installation requested.");
                    return 0;
                  case INSTALLED:
                    break;
                }
              } catch (ARUnavailableDeviceNotCompatibleException e) {
                Log.e(TAG, "ARCore not installed", e);
                return 0;
              } catch (ARUnavailableServiceNotInstalledException e) {
                  Log.e(TAG, "ARCore not installed", e);
                  return 0;
              }
              break;
            case UNKNOWN_ERROR:
            case UNKNOWN_CHECKING:
            case UNKNOWN_TIMED_OUT:
            case UNSUPPORTED_DEVICE_NOT_CAPABLE:
              Log.e(
                  TAG,
                  "ARCore is not supported on this device, ArCoreApk.checkAvailability() returned "
                      + availability);
              return -1;
          }
          return 1;
    }

    private void startSession() {
        CocosActivity activity = (CocosActivity)GlobalObject.getActivity();
        if (mSession == null) {
            try {
                mSession = new ARSession(GlobalObject.getActivity());
            } catch (ARUnavailableServiceNotInstalledException e) {
                e.printStackTrace();
            } catch (ARUnavailableClientSdkTooOldException e) {
                e.printStackTrace();
            } catch (ARUnavailableServiceApkTooOldException e) {
                e.printStackTrace();
            } catch (ARUnavailableEmuiNotCompatibleException e) {
                e.printStackTrace();
            }
        }
        /*if (mSession == null) {
            Activity activity = GlobalObject.getActivity();
            switch (ArCoreApk.getInstance().requestInstall(activity, mUserRequestedInstall)) {
                case INSTALLED:
                    // Success: Safe to create the AR session.
                    mSession = new Session(activity);
                    break;
                case INSTALL_REQUESTED:
                    // When this method returns `INSTALL_REQUESTED`:
                    // 1. ARCore pauses this activity.
                    // 2. ARCore prompts the user to install or update Google Play
                    //    Services for AR (market://details?id=com.huawei.hiar).
                    // 3. ARCore downloads the latest device profile data.
                    // 4. ARCore resumes this activity. The next invocation of
                    //    requestInstall() will either return `INSTALLED` or throw an
                    //    exception if the installation or update did not succeed.
                    mUserRequestedInstall = false;
                    return;
            }
        }*/
        if (mDisplayRotationHelper == null) {
            mDisplayRotationHelper = new CocosARDisplayRotationHelper(GlobalObject.getActivity());
        }
        mDisplayRotationHelper.registerDisplayListener();

        try {
            mSession.resume();
            //CocosActivity activity = (CocosActivity)GlobalObject.getActivity();
            int width = activity.getSurfaceView().getWidth();
            int height = activity.getSurfaceView().getHeight();
            mDisplayRotationHelper.updateViewportChanged(width, height);

        } catch (ARCameraNotAvailableException e) {
            mSession = null;
            return;
        }
    }

    private void closeSession() {
        mSession.stop();
        mSession = null;
    }

    private void pauseSession() {
        mSession.pause();
        mDisplayRotationHelper.unregisterDisplayListener();
    }

    private void updateSession() {
        mDisplayRotationHelper.updateDisplayGeometry(api);

        try {
            mFrame = mSession.update();
        } catch (ARCameraNotAvailableException e) {
            e.printStackTrace();
        }

        mCamera = mFrame.getCamera();
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] results) {
        Activity activity = GlobalObject.getActivity();
        if (!CocosARCameraPermissionHelper.hasCameraPermission(activity)) {
        Toast.makeText(
                activity,
                "Camera permission is needed to run this application",
                Toast.LENGTH_LONG)
            .show();
        if (!CocosARCameraPermissionHelper.shouldShowRequestPermissionRationale(activity)) {
            // Permission denied with checking "Do not ask again".
            CocosARCameraPermissionHelper.launchPermissionSettings(activity);
        }
        //finish();
        }
    }

    //#region plane feature
    private void updatePlanesInfo(int maxSize) {
        if (mSession == null || mCamera == null) return;

        Collection<ARPlane> allPlanes = mSession.getAllTrackables(ARPlane.class);
        
        // Planes must be sorted by distance from camera so that we draw closer planes first, and
        // they occlude the farther planes.
        List<SortablePlane> sortedPlanes = new ArrayList<>();

        ARPose cameraPose = mCamera.getDisplayOrientedPose();
        for (ARPlane plane : allPlanes) {
            //if (plane.getTrackingState() != TrackingState.TRACKING || plane.getSubsumedBy() != null) {
            if (plane.getTrackingState() != ARTrackable.TrackingState.TRACKING || plane.getType() == ARPlane.PlaneType.VERTICAL_FACING) {
              continue;
            }
            
            float distance = calculateDistanceToPlane(plane.getCenterPose(), cameraPose);
            if (distance < 0) { // Plane is back-facing.
              continue;
            }
            sortedPlanes.add(new SortablePlane(distance, plane));
        }
        Collections.sort(
            sortedPlanes,
            new Comparator<SortablePlane>() {
                @Override
                public int compare(SortablePlane a, SortablePlane b) {
                    return Float.compare(b.distance, a.distance);
                }
            }
        );
        
        int size = sortedPlanes.size();
        size = size > maxSize ? maxSize : size;
        int count = 0, offset = 0;

        mAddedPlanes.clear();
        mUpdatedPlanes.clear();
        mRemovedPlanes.clear();

        for (SortablePlane sortedPlane : sortedPlanes) {
            if (count >= size) continue;

            ARPlane plane = sortedPlane.plane;
            Integer planeIndex = planesIndexMap.get(plane);
            ARPlane subsumedByPlane = plane.getSubsumedBy();

            if (planeIndex == null) {
                planesMap.put(planeTag, plane);
                planesIndexMap.put(plane, planeTag);
                planeIndex = planeTag++;

                // add
                mAddedPlanes.add(planeIndex);
            } else {
                if(subsumedByPlane != null) {
                    // remove
                    mRemovedPlanes.add(planeIndex);
                    continue;
                } else {
                    // update
                    mUpdatedPlanes.add(planeIndex);
                }
            }

            ++count;
        }
    }

    private float[] getPlanesInfoFromList(List<Integer> planeIndices) {
        float[] planesInfo = new float[PLANE_INFOS_SIZE * planeIndices.size()];
        int offset = 0;
        int maxSize = 5;
        int n = 0;
        for (int index : planeIndices) {
            if (n >= maxSize) break;
            ARPlane plane = planesMap.get(index);
            copyPlaneToArray(plane, index, planesInfo, offset);
            offset += PLANE_INFOS_SIZE;
            ++n;
        }
        return planesInfo;
    }

    private static class SortablePlane {
        final float distance;
        final ARPlane plane;
    
        SortablePlane(float distance, ARPlane plane) {
            this.distance = distance;
            this.plane = plane;
        }
    }

    // Calculate the normal distance to plane from cameraPose, the given planePose should have y axis
    // parallel to plane's normal, for example plane's center pose or hit test pose.
    public static float calculateDistanceToPlane(ARPose planePose, ARPose cameraPose) {
        float[] normal = new float[3];
        float cameraX = cameraPose.tx();
        float cameraY = cameraPose.ty();
        float cameraZ = cameraPose.tz();
        // Get transformed Y axis of plane's coordinate system.
        planePose.getTransformedAxis(1, 1.0f, normal, 0);
        // Compute dot product of plane's normal with vector from camera to plane center.
        return (cameraX - planePose.tx()) * normal[0]
            + (cameraY - planePose.ty()) * normal[1]
            + (cameraZ - planePose.tz()) * normal[2];
    }

    //#endregion

    private static void copyPoseToArray(ARPose src, float[] arr) {
        copyPoseToArray(src, arr, 0);
    }

    private static void copyPoseToArray(ARPose src, float[] arr, int offset) {
        arr[offset] = src.tx();
        arr[offset+1] = src.ty();
        arr[offset+2] = src.tz();
        arr[offset+3] = src.qx();
        arr[offset+4] = src.qy();
        arr[offset+5] = src.qz();
        arr[offset+6] = src.qw();
    }

    private static void copyPlaneToArray(ARPlane src, int index, float[] arr, int offset) {
        arr[offset++] = index;
        arr[offset++] = src.getType().ordinal();
        arr[offset++] = src.getTrackingState().ordinal();
        arr[offset++] = src.getExtentX();
        arr[offset++] = src.getExtentZ();
        ARPose pose = src.getCenterPose();
        copyPoseToArray(pose, arr, offset);
    }
}