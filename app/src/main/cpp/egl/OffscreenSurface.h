//
// Created by Administrator on 2018/3/21.
//

#ifndef EGL_HELPER_OFFSCREENSURFACE_H
#define EGL_HELPER_OFFSCREENSURFACE_H


#include "EglSurfaceBase.h"

class OffscreenSurface : public EglSurfaceBase {
public:
    OffscreenSurface(EglCore *eglCore, int width, int height);
    void release();
};


#endif //EGL_HELPER_OFFSCREENSURFACE_H
