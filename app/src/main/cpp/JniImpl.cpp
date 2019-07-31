//
// Created by ByteFlow on 2019/7/9.
//
#include "util/LogUtil.h"
#include <MyGLRenderContext.h>
#include <BgRender.h>
#include "jni.h"

#define NATIVE_RENDER_CLASS_NAME "com/byteflow/app/MyNativeRender"
#define NATIVE_BG_RENDER_CLASS_NAME "com/byteflow/app/egl/NativeBgRender"

#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_byteflow_app_MyNativeRender
 * Method:    native_OnInit
 * Signature: ()V
 */
JNIEXPORT void JNICALL native_OnInit(JNIEnv *env, jobject instance)
{
	MyGLRenderContext::GetInstance();

}

/*
 * Class:     com_byteflow_app_MyNativeRender
 * Method:    native_OnUnInit
 * Signature: ()V
 */
JNIEXPORT void JNICALL native_OnUnInit(JNIEnv *env, jobject instance)
{
	MyGLRenderContext::DestroyInstance();
}

/*
 * Class:     com_byteflow_app_MyNativeRender
 * Method:    native_SetImageData
 * Signature: (III[B)V
 */
JNIEXPORT void JNICALL native_SetImageData
(JNIEnv *env, jobject instance, jint format, jint width, jint height, jbyteArray imageData)
{
	int len = env->GetArrayLength (imageData);
	uint8_t* buf = new uint8_t[len];
	env->GetByteArrayRegion(imageData, 0, len, reinterpret_cast<jbyte*>(buf));
	MyGLRenderContext::GetInstance()->SetImageData(format, width, height, buf);
	delete[] buf;
	env->DeleteLocalRef(imageData);
}

/*
 * Class:     com_byteflow_app_MyNativeRender
 * Method:    native_SetParamsInt
 * Signature: (III)V
 */
JNIEXPORT void JNICALL native_SetParamsInt
		(JNIEnv *env, jobject instance, jint paramType, jint value0, jint value1)
{
	MyGLRenderContext::GetInstance()->SetParamsInt(paramType, value0, value1);
}

/*
 * Class:     com_byteflow_app_MyNativeRender
 * Method:    native_OnSurfaceCreated
 * Signature: ()V
 */
JNIEXPORT void JNICALL native_OnSurfaceCreated(JNIEnv *env, jobject instance)
{
	MyGLRenderContext::GetInstance()->OnSurfaceCreated();
}

/*
 * Class:     com_byteflow_app_MyNativeRender
 * Method:    native_OnSurfaceChanged
 * Signature: (II)V
 */
JNIEXPORT void JNICALL native_OnSurfaceChanged
(JNIEnv *env, jobject instance, jint width, jint height)
{
	MyGLRenderContext::GetInstance()->OnSurfaceChanged(width, height);

}

/*
 * Class:     com_byteflow_app_MyNativeRender
 * Method:    native_OnDrawFrame
 * Signature: ()V
 */
JNIEXPORT void JNICALL native_OnDrawFrame(JNIEnv *env, jobject instance)
{
	MyGLRenderContext::GetInstance()->OnDrawFrame();

}


/*
 * Class:     com_byteflow_app_egl_NativeBgRender
 * Method:    native_BgRenderInit
 * Signature: ()V
 */
JNIEXPORT void JNICALL native_BgRenderInit(JNIEnv *env, jobject instance)
{
	BgRender::GetInstance()->Init();

}

/*
 * Class:     com_byteflow_app_egl_NativeBgRender
 * Method:    native_BgRenderSetImageData
 * Signature: ([BII)V
 */
JNIEXPORT void JNICALL native_BgRenderSetImageData(JNIEnv *env, jobject instance, jbyteArray data, jint width, jint height)
{
	int len = env->GetArrayLength (data);
	uint8_t* buf = new uint8_t[len];
	env->GetByteArrayRegion(data, 0, len, reinterpret_cast<jbyte*>(buf));
	BgRender::GetInstance()->SetImageData(buf, width, height);
	delete[] buf;
	env->DeleteLocalRef(data);


}

/*
 * Class:     com_byteflow_app_egl_NativeBgRender
 * Method:    native_BgRenderSetIntParams
 * Signature: (II)V
 */
JNIEXPORT void JNICALL native_BgRenderSetIntParams(JNIEnv *env, jobject instance, jint type, jint param)
{
	BgRender::GetInstance()->SetIntParams(type, param);

}

/*
 * Class:     com_byteflow_app_egl_NativeBgRender
 * Method:    native_BgRenderDraw
 * Signature: ()V
 */
JNIEXPORT void JNICALL native_BgRenderDraw(JNIEnv *env, jobject instance)
{
	BgRender::GetInstance()->Draw();
}

/*
 * Class:     com_byteflow_app_egl_NativeBgRender
 * Method:    natuve_BgRenderUnInit
 * Signature: ()V
 */
JNIEXPORT void JNICALL native_BgRenderUnInit(JNIEnv *env, jobject instance)
{
	BgRender::GetInstance()->UnInit();
}

#ifdef __cplusplus
}
#endif

static JNINativeMethod g_RenderMethods[] = {
		{"native_OnInit",             "()V",       (void *)(native_OnInit)},
		{"native_OnUnInit",           "()V",       (void *)(native_OnUnInit)},
		{"native_SetImageData",       "(III[B)V",  (void *)(native_SetImageData)},
		{"native_SetParamsInt",       "(III)V",    (void *)(native_SetParamsInt)},
		{"native_OnSurfaceCreated",   "()V",       (void *)(native_OnSurfaceCreated)},
		{"native_OnSurfaceChanged",   "(II)V",     (void *)(native_OnSurfaceChanged)},
		{"native_OnDrawFrame",        "()V",       (void *)(native_OnDrawFrame)},
};

static JNINativeMethod g_BgRenderMethods[] = {
		{"native_BgRenderInit",          "()V",       (void *)(native_BgRenderInit)},
		{"native_BgRenderSetImageData",  "([BII)V",   (void *)(native_BgRenderSetImageData)},
		{"native_BgRenderSetIntParams",  "(II)V",     (void *)(native_BgRenderSetIntParams)},
		{"native_BgRenderDraw",          "()V",       (void *)(native_BgRenderDraw)},
		{"native_BgRenderUnInit",        "()V",       (void *)(native_BgRenderUnInit)},
};

static int RegisterNativeMethods(JNIEnv *env, const char *className, JNINativeMethod *methods, int methodNum)
{
	LOGCATE("RegisterNativeMethods");
	jclass clazz = env->FindClass(className);
	if (clazz == NULL)
	{
		LOGCATE("RegisterNativeMethods fail. clazz == NULL");
		return JNI_FALSE;
	}
	if (env->RegisterNatives(clazz, methods, methodNum) < 0)
	{
		LOGCATE("RegisterNativeMethods fail");
		return JNI_FALSE;
	}
	return JNI_TRUE;
}

static void UnregisterNativeMethods(JNIEnv *env, const char *className)
{
	LOGCATE("UnregisterNativeMethods");
	jclass clazz = env->FindClass(className);
	if (clazz == NULL)
	{
		LOGCATE("UnregisterNativeMethods fail. clazz == NULL");
		return;
	}
	if (env != NULL)
	{
		env->UnregisterNatives(clazz);
	}
}

// call this func when loading lib
extern "C" jint JNI_OnLoad(JavaVM *jvm, void *p)
{
	LOGCATE("===== JNI_OnLoad =====");
	jint jniRet = JNI_ERR;
	JNIEnv *env = NULL;
	if (jvm->GetEnv((void **) (&env), JNI_VERSION_1_6) != JNI_OK)
	{
		return jniRet;
	}

	jint regRet = RegisterNativeMethods(env, NATIVE_RENDER_CLASS_NAME, g_RenderMethods,
										sizeof(g_RenderMethods) /
										sizeof(g_RenderMethods[0]));
	if (regRet != JNI_TRUE)
	{
		return JNI_ERR;
	}

	regRet = RegisterNativeMethods(env, NATIVE_BG_RENDER_CLASS_NAME, g_BgRenderMethods,
										sizeof(g_BgRenderMethods) /
										sizeof(g_BgRenderMethods[0]));
	if (regRet != JNI_TRUE)
	{
		return JNI_ERR;
	}

	return JNI_VERSION_1_6;
}

extern "C" void JNI_OnUnload(JavaVM *jvm, void *p)
{
	JNIEnv *env = NULL;
	if (jvm->GetEnv((void **) (&env), JNI_VERSION_1_6) != JNI_OK)
	{
		return;
	}

	UnregisterNativeMethods(env, NATIVE_RENDER_CLASS_NAME);

	UnregisterNativeMethods(env, NATIVE_BG_RENDER_CLASS_NAME);
}
