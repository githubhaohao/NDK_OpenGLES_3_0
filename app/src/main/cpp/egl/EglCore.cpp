//
// Created by Administrator on 2018/2/10.
//

#include "LogUtil.h"
#include "EglCore.h"
#include <assert.h>

EglCore::EglCore() {
    init(NULL, 0);
}


EglCore::~EglCore() {
    release();
}

/**
 * 构造方法
 * @param sharedContext
 * @param flags
 */
EglCore::EglCore(EGLContext sharedContext, int flags) {
    init(sharedContext, flags);
}

/**
 * 初始化
 * @param sharedContext
 * @param flags
 * @return
 */
bool EglCore::init(EGLContext sharedContext, int flags) {
    assert(mEGLDisplay == EGL_NO_DISPLAY);
    if (mEGLDisplay != EGL_NO_DISPLAY) {
        LOGCATE("EGL already set up");
        return false;
    }
    if (sharedContext == NULL) {
        sharedContext = EGL_NO_CONTEXT;
    }

    mEGLDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    assert(mEGLDisplay != EGL_NO_DISPLAY);
    if (mEGLDisplay == EGL_NO_DISPLAY) {
        LOGCATE("unable to get EGL14 display.\n");
        return false;
    }

    if (!eglInitialize(mEGLDisplay, 0, 0)) {
        mEGLDisplay = EGL_NO_DISPLAY;
        LOGCATE("unable to initialize EGL14");
        return false;
    }

    // 尝试使用GLES3
    if ((flags & FLAG_TRY_GLES3) != 0) {
        EGLConfig config = getConfig(flags, 3);
        if (config != NULL) {
            int attrib3_list[] = {
                    EGL_CONTEXT_CLIENT_VERSION, 3,
                    EGL_NONE
            };
            EGLContext context = eglCreateContext(mEGLDisplay, config,
                                                  sharedContext, attrib3_list);
            checkEglError("eglCreateContext");
            if (eglGetError() == EGL_SUCCESS) {
                mEGLConfig = config;
                mEGLContext = context;
                mGlVersion = 3;
            }
        }
    }
    // 如果GLES3没有获取到，则尝试使用GLES2
    if (mEGLContext == EGL_NO_CONTEXT) {
        EGLConfig config = getConfig(flags, 2);
        assert(config != NULL);
        int attrib2_list[] = {
                EGL_CONTEXT_CLIENT_VERSION, 2,
                EGL_NONE
        };
        EGLContext context = eglCreateContext(mEGLDisplay, config,
                                              sharedContext, attrib2_list);
        checkEglError("eglCreateContext");
        if (eglGetError() == EGL_SUCCESS) {
            mEGLConfig = config;
            mEGLContext = context;
            mGlVersion = 2;
        }
    }

    // 获取eglPresentationTimeANDROID方法的地址
    eglPresentationTimeANDROID = (EGL_PRESENTATION_TIME_ANDROIDPROC)
            eglGetProcAddress("eglPresentationTimeANDROID");
    if (!eglPresentationTimeANDROID) {
        LOGCATE("eglPresentationTimeANDROID is not available!");
    }

    int values[1] = {0};
    eglQueryContext(mEGLDisplay, mEGLContext, EGL_CONTEXT_CLIENT_VERSION, values);
    LOGCATE("EGLContext created, client version %d", values[0]);

    return true;
}


/**
 * 获取合适的EGLConfig
 * @param flags
 * @param version
 * @return
 */
EGLConfig EglCore::getConfig(int flags, int version) {
    int renderableType = EGL_OPENGL_ES2_BIT;
    if (version >= 3) {
        renderableType |= EGL_OPENGL_ES3_BIT_KHR;
    }
    int attribList[] = {
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_ALPHA_SIZE, 8,
            EGL_DEPTH_SIZE, 16,
            EGL_STENCIL_SIZE, 8,
            EGL_RENDERABLE_TYPE, renderableType,
            EGL_NONE, 0,      // placeholder for recordable [@-3]
            EGL_NONE
    };
    int length = sizeof(attribList) / sizeof(attribList[0]);
    if ((flags & FLAG_RECORDABLE) != 0) {
        attribList[length - 3] = EGL_RECORDABLE_ANDROID;
        attribList[length - 2] = 1;
    }
    EGLConfig configs = NULL;
    int numConfigs;
    if (!eglChooseConfig(mEGLDisplay, attribList, &configs, 1, &numConfigs)) {
        LOGCATE("unable to find RGB8888 / %d  EGLConfig", version);
        return NULL;
    }
    return configs;
}

/**
 * 释放资源
 */
void EglCore::release() {
    if (mEGLDisplay != EGL_NO_DISPLAY) {
        eglMakeCurrent(mEGLDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglDestroyContext(mEGLDisplay, mEGLContext);
        eglReleaseThread();
        eglTerminate(mEGLDisplay);
    }

    mEGLDisplay = EGL_NO_DISPLAY;
    mEGLContext = EGL_NO_CONTEXT;
    mEGLConfig = NULL;
}

/**
 * 获取EGLContext
 * @return
 */
EGLContext EglCore::getEGLContext() {
    return mEGLContext;
}

/**
 * 销毁EGLSurface
 * @param eglSurface
 */
void EglCore::releaseSurface(EGLSurface eglSurface) {
    eglDestroySurface(mEGLDisplay, eglSurface);
}

/**
 * 创建EGLSurface
 * @param surface
 * @return
 */
EGLSurface EglCore::createWindowSurface(ANativeWindow *surface) {
    assert(surface != NULL);
    if (surface == NULL) {
        LOGCATE("ANativeWindow is NULL!");
        return NULL;
    }
    int surfaceAttribs[] = {
            EGL_NONE
    };
    LOGCATE("eglCreateWindowSurface start");
    EGLSurface eglSurface = eglCreateWindowSurface(mEGLDisplay, mEGLConfig, surface, surfaceAttribs);
    checkEglError("eglCreateWindowSurface");
    assert(eglSurface != NULL);
    if (eglSurface == NULL) {
        LOGCATE("EGLSurface is NULL!");
        return NULL;
    }
    return eglSurface;
}

/**
 * 创建离屏渲染的EGLSurface
 * @param width
 * @param height
 * @return
 */
EGLSurface EglCore::createOffscreenSurface(int width, int height) {
    int surfaceAttribs[] = {
            EGL_WIDTH, width,
            EGL_HEIGHT, height,
            EGL_NONE
    };
    EGLSurface eglSurface = eglCreatePbufferSurface(mEGLDisplay, mEGLConfig, surfaceAttribs);
    assert(eglSurface != NULL);
    if (eglSurface == NULL) {
        LOGCATE("Surface was null");
        return NULL;
    }
    return eglSurface;
}

/**
 * 切换到当前的上下文
 * @param eglSurface
 */
void EglCore::makeCurrent(EGLSurface eglSurface) {
    if (mEGLDisplay == EGL_NO_DISPLAY) {
        LOGCATE("Note: makeCurrent w/o display.\n");
    }
    if (!eglMakeCurrent(mEGLDisplay, eglSurface, eglSurface, mEGLContext)) {
        // TODO 抛出异常
    }
}

/**
 * 切换到某个上下文
 * @param drawSurface
 * @param readSurface
 */
void EglCore::makeCurrent(EGLSurface drawSurface, EGLSurface readSurface) {
    if (mEGLDisplay == EGL_NO_DISPLAY) {
        LOGCATE("Note: makeCurrent w/o display.\n");
    }
    if (!eglMakeCurrent(mEGLDisplay, drawSurface, readSurface, mEGLContext)) {
        // TODO 抛出异常
    }
}

/**
 *
 */
void EglCore::makeNothingCurrent() {
    if (!eglMakeCurrent(mEGLDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT)) {
        // TODO 抛出异常
    }
}

/**
 * 交换显示
 * @param eglSurface
 * @return
 */
bool EglCore::swapBuffers(EGLSurface eglSurface) {
    return eglSwapBuffers(mEGLDisplay, eglSurface);
}

/**
 * 设置显示时间戳pts
 * @param eglSurface
 * @param nsecs
 */
void EglCore::setPresentationTime(EGLSurface eglSurface, long nsecs) {
    eglPresentationTimeANDROID(mEGLDisplay, eglSurface, nsecs);
}

/**
 * 是否处于当前上下文
 * @param eglSurface
 * @return
 */
bool EglCore::isCurrent(EGLSurface eglSurface) {
    return mEGLContext == eglGetCurrentContext() &&
            eglSurface == eglGetCurrentSurface(EGL_DRAW);
}

/**
 * 查询surface
 * @param eglSurface
 * @param what
 * @return
 */
int EglCore::querySurface(EGLSurface eglSurface, int what) {
    int value;
    eglQuerySurface(mEGLContext, eglSurface, what, &value);
    return value;
}

/**
 * 查询字符串
 * @param what
 * @return
 */
const char* EglCore::queryString(int what) {
    return eglQueryString(mEGLDisplay, what);
}

/**
 * 获取GLES版本号
 * @return
 */
int EglCore::getGlVersion() {
    return mGlVersion;
}

/**
 * 检查是否出错
 * @param msg
 */
void EglCore::checkEglError(const char *msg) {
    int error;
    if ((error = eglGetError()) != EGL_SUCCESS) {
        // TODO 抛出异常
        LOGCATE("%s: EGL error: %x", msg, error);
    }
}
