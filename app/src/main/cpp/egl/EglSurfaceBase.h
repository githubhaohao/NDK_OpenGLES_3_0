//
// Created by Administrator on 2018/3/21.
//

#ifndef EGL_HELPER_EGLSURFACEBASE_H
#define EGL_HELPER_EGLSURFACEBASE_H

#include "EglCore.h"

class EglSurfaceBase {

public:
    EglSurfaceBase(EglCore *eglCore);
    // 创建窗口Surface
    void createWindowSurface(ANativeWindow *nativeWindow);
    // 创建离屏Surface
    void createOffscreenSurface(int width, int height);
    // 获取宽度
    int getWidth();
    // 获取高度
    int getHeight();
    // 释放EGLSurface
    void releaseEglSurface();
    // 切换到当前上下文
    void makeCurrent();
    // 交换缓冲区，显示图像
    bool swapBuffers();
    // 设置显示时间戳
    void setPresentationTime(long nsecs);
    // 获取当前帧缓冲
    char *getCurrentFrame();

protected:
    EglCore *mEglCore;
    EGLSurface mEglSurface;
    int mWidth;
    int mHeight;

};


#endif //EGL_HELPER_EGLSURFACEBASE_H
