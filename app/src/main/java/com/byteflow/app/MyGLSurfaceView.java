package com.byteflow.app;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.util.Log;
import android.view.MotionEvent;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import static com.byteflow.app.MyNativeRender.PARAM_TYPE_ROTATE_ANGLE;

public class MyGLSurfaceView extends GLSurfaceView {
    private static final String TAG = "MyGLSurfaceView";

    private final float TOUCH_SCALE_FACTOR = 180.0f/320;

    public static final int IMAGE_FORMAT_RGBA = 0x01;
    public static final int IMAGE_FORMAT_NV21 = 0x02;
    public static final int IMAGE_FORMAT_NV12 = 0x03;
    public static final int IMAGE_FORMAT_I420 = 0x04;

    private float mPreviousY;
    private float mPreviousX;
    private int mXAngle;
    private int mYAngle;

    private MyGLRender mGLRender;
    private MyNativeRender mNativeRender;

    private int mRatioWidth = 0;
    private int mRatioHeight = 0;

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

    @Override
    public boolean onTouchEvent(MotionEvent e) {
        float y = e.getY();
        float x = e.getX();
        switch (e.getAction()) {
            case MotionEvent.ACTION_MOVE:
                float dy = y - mPreviousY;
                float dx = x - mPreviousX;
                mYAngle += dx * TOUCH_SCALE_FACTOR;
                mXAngle += dy * TOUCH_SCALE_FACTOR;
        }
        mPreviousY = y;
        mPreviousX = x;

        mNativeRender.native_SetParamsInt(PARAM_TYPE_ROTATE_ANGLE, mXAngle, mYAngle);
        requestRender();
        return true;
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        super.onMeasure(widthMeasureSpec, heightMeasureSpec);
        int width = MeasureSpec.getSize(widthMeasureSpec);
        int height = MeasureSpec.getSize(heightMeasureSpec);

        if (0 == mRatioWidth || 0 == mRatioHeight) {
            setMeasuredDimension(width, height);
        } else {
            if (width < height * mRatioWidth / mRatioHeight) {
                setMeasuredDimension(width, width * mRatioHeight / mRatioWidth);
            } else {
                setMeasuredDimension(height * mRatioWidth / mRatioHeight, height);
            }
        }
    }

    public void setAspectRatio(int width, int height) {
        if (width < 0 || height < 0) {
            throw new IllegalArgumentException("Size cannot be negative.");
        }

        mRatioWidth = width;
        mRatioHeight = height;
        requestLayout();
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
            mNativeRender.native_OnSurfaceCreated();

        }

        @Override
        public void onSurfaceChanged(GL10 gl, int width, int height) {
            mNativeRender.native_OnSurfaceChanged(width, height);

        }

        @Override
        public void onDrawFrame(GL10 gl) {
            mNativeRender.native_OnDrawFrame();

        }

    }
}
