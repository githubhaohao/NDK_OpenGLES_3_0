//
// Created by Administrator on 2018/3/21.
//

#ifndef EGL_HELPER_WINDOWSURFACE_H
#define EGL_HELPER_WINDOWSURFACE_H

#include <android/native_window.h>
#include "EglSurfaceBase.h"

class WindowSurface : public EglSurfaceBase {

public:
    WindowSurface(EglCore *eglCore, ANativeWindow *window, bool releaseSurface);
    WindowSurface(EglCore *eglCore, ANativeWindow *window);
    // 释放资源
    void release();
    // 重新创建
    void recreate(EglCore *eglCore);

private:
    ANativeWindow  *mSurface;
    bool mReleaseSurface;
};


#endif //EGL_HELPER_WINDOWSURFACE_H
