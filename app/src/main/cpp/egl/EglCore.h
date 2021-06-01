//
// Created by Administrator on 2018/2/10.
//

#ifndef EGL_HELPER_EGLCORE_H
#define EGL_HELPER_EGLCORE_H

#include <android/native_window.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglplatform.h>

/**
 * Constructor flag: surface must be recordable.  This discourages EGL from using a
 * pixel format that cannot be converted efficiently to something usable by the video
 * encoder.
 */
#define FLAG_RECORDABLE 0x01

/**
 * Constructor flag: ask for GLES3, fall back to GLES2 if not available.  Without this
 * flag, GLES2 is used.
 */
#define FLAG_TRY_GLES3 002

// Android-specific extension
#define EGL_RECORDABLE_ANDROID 0x3142

typedef EGLBoolean (EGLAPIENTRYP EGL_PRESENTATION_TIME_ANDROIDPROC)(EGLDisplay display, EGLSurface surface, khronos_stime_nanoseconds_t time);

class EglCore {

private:
    EGLDisplay mEGLDisplay = EGL_NO_DISPLAY;
    EGLConfig  mEGLConfig = NULL;
    EGLContext mEGLContext = EGL_NO_CONTEXT;

    // 设置时间戳方法
    EGL_PRESENTATION_TIME_ANDROIDPROC eglPresentationTimeANDROID = NULL;

    int mGlVersion = -1;
    // 查找合适的EGLConfig
    EGLConfig getConfig(int flags, int version);

public:
    EglCore();
    ~EglCore();
    EglCore(EGLContext sharedContext, int flags);
    bool init(EGLContext sharedContext, int flags);
    // 释放资源
    void release();
    // 获取EglContext
    EGLContext getEGLContext();
    // 销毁Surface
    void releaseSurface(EGLSurface eglSurface);
    //  创建EGLSurface
    EGLSurface createWindowSurface(ANativeWindow *surface);
    // 创建离屏EGLSurface
    EGLSurface createOffscreenSurface(int width, int height);
    // 切换到当前上下文
    void makeCurrent(EGLSurface eglSurface);
    // 切换到某个上下文
    void makeCurrent(EGLSurface drawSurface, EGLSurface readSurface);
    // 没有上下文
    void makeNothingCurrent();
    // 交换显示
    bool swapBuffers(EGLSurface eglSurface);
    // 设置pts
    void setPresentationTime(EGLSurface eglSurface, long nsecs);
    // 判断是否属于当前上下文
    bool isCurrent(EGLSurface eglSurface);
    // 执行查询
    int querySurface(EGLSurface eglSurface, int what);
    // 查询字符串
    const char *queryString(int what);
    // 获取当前的GLES 版本号
    int getGlVersion();
    // 检查是否出错
    void checkEglError(const char *msg);
};


#endif //EGL_HELPER_EGLCORE_H
