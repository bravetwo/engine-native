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

import com.google.ar.core.Anchor;
import com.google.ar.core.ArCoreApk;
import com.google.ar.core.Camera;
import com.google.ar.core.Config;
import com.google.ar.core.Config.InstantPlacementMode;
import com.google.ar.core.DepthPoint;
import com.google.ar.core.Frame;
import com.google.ar.core.Pose;
import com.google.ar.core.HitResult;
import com.google.ar.core.InstantPlacementPoint;
import com.google.ar.core.LightEstimate;
import com.google.ar.core.Plane;
import com.google.ar.core.Point;
import com.google.ar.core.Point.OrientationMode;
import com.google.ar.core.PointCloud;
import com.google.ar.core.Session;
import com.google.ar.core.Trackable;
import com.google.ar.core.TrackingFailureReason;
import com.google.ar.core.TrackingState;
import com.google.ar.core.Coordinates2d;
import com.google.ar.core.exceptions.CameraNotAvailableException;
import com.google.ar.core.exceptions.UnavailableApkTooOldException;
import com.google.ar.core.exceptions.UnavailableArcoreNotInstalledException;
import com.google.ar.core.exceptions.UnavailableDeviceNotCompatibleException;
import com.google.ar.core.exceptions.UnavailableSdkTooOldException;
import com.google.ar.core.exceptions.UnavailableUserDeclinedInstallationException;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;
import android.content.Context;
import android.app.Activity;
import android.widget.Toast;
import android.os.Handler;
import android.util.Log;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

public class CocosARCoreAPI implements CocosARAPI, ActivityCompat.OnRequestPermissionsResultCallback {
    private static final String TAG = CocosARCoreAPI.class.getSimpleName();
    private static CocosARCoreAPI api = null;
    
    private CocosARDisplayRotationHelper mDisplayRotationHelper;

    private Session mSession;
    private Frame mFrame;
    private Camera mCamera;

    private float[] mCameraPose = new float[7];
    private float[] mViewMatrix = new float[16];
    private float[] mProjMatrix = new float[16];

    private float[] mQuadCoords = {-1f, -1f, -1f, 1f, 1f, -1f, 1f, 1f};
    private float[] mCameraTexCoords = new float[8];

    // requestInstall(Activity, true) will triggers installation of
    // Google Play Services for AR if necessary.
    private boolean mUserRequestedInstall = true;
    private boolean mActive;

    public static CocosARCoreAPI init() {
        api = new CocosARCoreAPI();
        return api;
    }

    public static void start(final CocosARCoreAPI api) {
        api.checkCamera();
        if (api.checkSupportAndUpdate() > 0) {
            api.startSession();
        }
    }
    public static void onDestroy(final CocosARCoreAPI api) {
        api.closeSession();
    }
    public static void resume(final CocosARCoreAPI api) {
        api.checkCamera();
        if (api.checkSupportAndUpdate() > 0) {
            api.startSession();
        }
    }
    public static void pause(final CocosARCoreAPI api) {
        api.pauseSession();
    }
    public static void update(final CocosARCoreAPI api) {
        if (api.mSession == null) return;
        api.updateSession();
        api.updateCameraPose();
        api.updateCameraTexCoords();
    }

    public static boolean checkStart(final CocosARCoreAPI api) {
        if (api != null && api.mSession != null) {
            return true;
        }
        return false;
    }

    public static void setCameraTextureName(final CocosARCoreAPI api, int id) {
        api.mSession.setCameraTextureName(id);
    }

    public static float[] getCameraPose(final CocosARCoreAPI api) {
        return api.mCameraPose;
    }
    public static float[] getCameraViewMatrix(final CocosARCoreAPI api) {
        api.mCamera.getViewMatrix(api.mViewMatrix, 0);
        return api.mViewMatrix;
    }
    public static float[] getCameraProjectionMatrix(final CocosARCoreAPI api) {
        if (api.mSession != null) api.mCamera.getProjectionMatrix(api.mProjMatrix, 0, 0.01f, 1000.0f);
        return api.mProjMatrix;
    }
    public static float[] getCameraTexCoords(final CocosARCoreAPI api) {
        return api.mCameraTexCoords;
    }

    // for CocosARDisplayRotationHelper 
    public void setDisplayGeometry(int displayRotation, int width, int height) {
        mSession.setDisplayGeometry(displayRotation, width, height);
    }

    private void updateCameraPose() {
        Pose pose = mCamera.getDisplayOrientedPose();
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
            mFrame.transformCoordinates2d(
                Coordinates2d.OPENGL_NORMALIZED_DEVICE_COORDINATES,
                mQuadCoords,
                Coordinates2d.TEXTURE_NORMALIZED,
                mCameraTexCoords);
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
        ArCoreApk.Availability availability = ArCoreApk.getInstance().checkAvailability(activity);
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
                ArCoreApk.InstallStatus installStatus =
                    ArCoreApk.getInstance().requestInstall(activity, /*userRequestedInstall=*/ true);
                switch (installStatus) {
                  case INSTALL_REQUESTED:
                    Log.e(TAG, "ARCore installation requested.");
                    return 0;
                  case INSTALLED:
                    break;
                }
              } catch (UnavailableDeviceNotCompatibleException e) {
                Log.e(TAG, "ARCore not installed", e);
                return 0;
              } catch (UnavailableUserDeclinedInstallationException e) {
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
                mSession = new Session(GlobalObject.getActivity());
            } catch (UnavailableArcoreNotInstalledException e) {
                e.printStackTrace();
            } catch (UnavailableApkTooOldException e) {
                e.printStackTrace();
            } catch (UnavailableSdkTooOldException e) {
                e.printStackTrace();
            } catch (UnavailableDeviceNotCompatibleException e) {
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
                    //    Services for AR (market://details?id=com.google.ar.core).
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

        } catch (CameraNotAvailableException e) {
            mSession = null;
            return;
        }
    }

    private void closeSession() {
        mSession.close();
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
        } catch (CameraNotAvailableException e) {
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
}