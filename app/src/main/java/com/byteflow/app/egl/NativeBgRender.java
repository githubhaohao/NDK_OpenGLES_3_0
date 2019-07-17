package com.byteflow.app.egl;

public class NativeBgRender {

    public native void native_BgRenderInit();

    public native void native_BgRenderSetImageData(byte[] data, int width, int height);

    public native void native_BgRenderSetIntParams(int paramType, int param);

    public native void native_BgRenderDraw();

    public native void native_BgRenderUnInit();
}
