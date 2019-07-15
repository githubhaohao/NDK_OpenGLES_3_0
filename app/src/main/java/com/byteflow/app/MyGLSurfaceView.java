package com.byteflow.app;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.util.Log;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class MyGLSurfaceView extends GLSurfaceView {
    private static final String TAG = "MyGLSurfaceView";

    public static final int IMAGE_FORMAT_RGBA = 0x01;
    public static final int IMAGE_FORMAT_NV21 = 0x02;
    public static final int IMAGE_FORMAT_NV12 = 0x03;
    public static final int IMAGE_FORMAT_I420 = 0x04;

    private MyGLRender mGLRender;
    private MyNativeRender mNativeRender;

    public MyGLSurfaceView(Context context) {
        this(context, null);
    }

    public MyGLSurfaceView(Context context, AttributeSet attrs) {
        super(context, attrs);
        this.setEGLContextClientVersion(3);
        mNativeRender = new MyNativeRender();
        mGLRender = new MyGLRender(mNativeRender);
        setRenderer(mGLRender);
        setRenderMode(RENDERMODE_WHEN_DIRTY);
    }

    public MyNativeRender getNativeRender() {
        return mNativeRender;
    }

    public static class MyGLRender implements GLSurfaceView.Renderer {
        private MyNativeRender mNativeRender;

        MyGLRender(MyNativeRender myNativeRender) {
            mNativeRender = myNativeRender;
        }

        @Override
        public void onSurfaceCreated(GL10 gl, EGLConfig config) {
            Log.d(TAG, "onSurfaceCreated() called with: gl = [" + gl + "], config = [" + config + "]");
            mNativeRender.native_OnSurfaceCreated();

        }

        @Override
        public void onSurfaceChanged(GL10 gl, int width, int height) {
            Log.d(TAG, "onSurfaceChanged() called with: gl = [" + gl + "], width = [" + width + "], height = [" + height + "]");
            mNativeRender.native_OnSurfaceChanged(width, height);

        }

        @Override
        public void onDrawFrame(GL10 gl) {
            Log.d(TAG, "onDrawFrame() called with: gl = [" + gl + "]");
            mNativeRender.native_OnDrawFrame();

        }
    }
}
