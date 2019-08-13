# NDK OpenGLES3.0 开发（一）：绘制一个三角形
> 该原创文章首发于微信公众号：字节流动

# 什么是 OpenGLES 

OpenGLES 全称 OpenGL for Embedded Systems ，是三维图形应用程序接口 OpenGL 的子集，本质上是一个跨编程语言、跨平台的编程接口规范，主要应用于嵌入式设备，如手机、平板等。由[科纳斯（Khronos）](https://www.khronos.org/opengles/)组织定义和推广，科纳斯是一个图形软硬件行业协会，该协会主要关注图形和多媒体方面的开放标准。

# OpenGLES 3.0 的特点
OpenGLES 3.0 实际上是 OpenGLES 2.0 的扩展版本，向下兼容 OpenGLES 2.0 ，但不兼容 OpenGLES 1.0 。
![OpenGLES 3.0 图形管线.png](https://github.com/githubhaohao/NDK_OpenGLES_3_0/blob/master/doc/img/1/20190712151959897.png#pic_center)

## OpenGLES 3.0 主要新特性
### 纹理
- sRGB 纹理和帧缓冲区——允许应用程序执行伽马校正渲染。
- 2D 纹理数组——保存一组 2D 纹理的纹理目标。
- 3D 纹理。一些 OpenGL ES 2.0 实现通过扩展支持3D纹理，而 OpenGL ES3.0 将此作为强制的功能。
- 深度纹理和阴影比较——启用存储在纹理中的深度缓冲区。
- 无缝立方图。在 OpenGL ES 3.0 中，立方图可以进行采样如过滤来使用相邻面的数据并删除接缝处的伪像。
- 浮点纹理。

### 着色器
- 二进制程序文件。在 OpenGL ES 3.0 中，完全链接过的二进制程序文件可以保存为离线二进制格式，运行时不需要链接步骤。这有助于减少应用程序的加载时间。
- 非方矩阵。支持方阵之外的新矩阵类型，并在 API中 增加了相关的统一调用，以支持这些矩阵的加载。
- 全整数支持。支持整数（以及无符号整数）标量和向量类型以及全整数操作。
- 平面/平滑插值程序。 OpenGL ES 3.0 中插值程序可以显式声明为平面或者平滑着色。
- 统一变量块。统一变量值可以组合为统一变量块。统一变量块可以更高效地加载，也可在多个着色器程序间共享。
- 布局限定符。顶点着色器输入可以用布局限定符声明，以显式绑定着色器源代码中的位置，而不需要调用 API 。
### 几何形状

- 变换反馈。可以在缓冲区对象中捕捉顶点着色器的输出。
- 布尔遮挡查询。应用程序可以查询一个（或者一组）绘制调用的任何像素是否通过深度测试。
- 实例渲染。有效地渲染包含类似几何形状但是属性（例如变化矩阵、颜色或者大小）不同的对象。

### 缓冲区对象
- 统一变量缓冲区对象。为存储/绑定大的统一变量块提供高效的方法。统
- VAO 顶点数组对象。提供绑定和在顶点数组状态之间切换的高效方法。
- 采样器对象。将采样器状态（纹理循环模式和过滤）与纹理对象分离。
- 同步对象。为应用程序提供检查一组操作是否在GPU上完成执行的机制。
- 像素缓冲对象。使应用程序能够执行对像素操作和纹理传输操作的异步数据传输。
- 缓冲区对象间拷贝。提供了高效地从一个缓冲区对象向另一个缓冲区对象传输数据的机制，不需要CPU干预。

### 帧缓冲区
- 多重渲染目标（MRT）。允许应用程序同时渲染到多个颜色缓冲区。
- 多重采样渲染缓冲区。使应用程序能够渲染到具备多重采样抗锯齿功能的屏幕外帧缓冲区。
- 帧缓冲区失效提示。

## OpenGLES 3.0 着色器语言规范变化

OpenGLES 3.0 着色器脚本
```
#version 300 es                          
layout(location = 0) in vec4 vPosition;  
void main()                              
{                                        
   gl_Position = vPosition;              
}                                        
```
其中，`#version 300 es ` 为 OpenGLES 3.0 版本声明，3.0 中使用 `in` 和 `out` 关键字取代 `attribute` 和 `varying` ，`layout` 关键字直接为脚本中的属性指定位置，为属性赋值变成了：
```
GLfloat vVertices[] = {
			0.0f,  0.5f, 0.0f,
			-0.5f, -0.5f, 0.0f,
			0.5f, -0.5f, 0.0f,
			0.0f,  -1.0f, 0.0f,
			0.5f, -0.5f, 0.0f,
			-0.5f, -0.5f, 0.0f
	};

//第一个参数为对应属性的 location 值
glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, vVertices );
```
而原来 2.0 的赋值方式为：

```
positionLoc = glGetAttribLocation（program, "vPosition"）;
glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 0, vVertices );
```

# 绘制三角形
绘制一个三角形的步骤：
1. 创建 OpenGLES 环境（可以借助于 GLSurfaceView 创建的上下文对象）；
2. 编译并链接着色器程序；
3. 指定着色器程序，为着色器程序中的变量赋值；
4. 绘制。

## 基于 GLSurfaceView 搭建 OpenGLES 环境

简单自定义 GLSurfaceView。

```java
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
        setRenderMode(RENDERMODE_CONTINUOUSLY);
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

```

JNI 类。
```java
package com.byteflow.app;

public class MyNativeRender {
    static {
        System.loadLibrary("native-render");
    }

    public native void native_OnInit();

    public native void native_OnUnInit();

    public native void native_SetImageData(int format, int width, int height, byte[] bytes);

    public native void native_OnSurfaceCreated();

    public native void native_OnSurfaceChanged(int width, int height);

    public native void native_OnDrawFrame();
}
```

Native 层简单实现 JNI 。
```cpp
//
// Created by ByteFlow on 2019/7/9.
//
#include "util/LogUtil.h"
#include <MyGLRenderContext.h>
#include "jni.h"

#define NATIVE_RENDER_CLASS_NAME "com/byteflow/app/MyNativeRender"

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

#ifdef __cplusplus
}
#endif

static JNINativeMethod g_RenderMethods[] = {
		{"native_OnInit",             "()V",       (void *)(native_OnInit)},
		{"native_OnUnInit",           "()V",       (void *)(native_OnUnInit)},
		{"native_SetImageData",       "(III[B)V",  (void *)(native_SetImageData)},
		{"native_OnSurfaceCreated",   "()V",       (void *)(native_OnSurfaceCreated)},
		{"native_OnSurfaceChanged",   "(II)V",     (void *)(native_OnSurfaceChanged)},
		{"native_OnDrawFrame",        "()V",       (void *)(native_OnDrawFrame)},
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
}

```

Native 层的 Render 封装类，其他的 Sample 都在此类中实现。
```cpp
//
// Created by ByteFlow on 2019/7/9.
//

#include <TriangleSample.h>
#include "MyGLRenderContext.h"
#include "LogUtil.h"

MyGLRenderContext* MyGLRenderContext::m_pContext = nullptr;

MyGLRenderContext::MyGLRenderContext()
{

}

MyGLRenderContext::~MyGLRenderContext()
{

}

void MyGLRenderContext::SetImageData(int format, int width, int height, uint8_t *pData)
{
	LOGCATE("MyGLRenderContext::SetImageData format=%d, width=%d, height=%d, pData=%p", format, width, height, pData);
	NativeImage nativeImage;
	nativeImage.format = format;
	nativeImage.width = width;
	nativeImage.height = height;
	nativeImage.ppPlane[0] = pData;

	switch (format)
	{
		case IMAGE_FORMAT_NV12:
		case IMAGE_FORMAT_NV21:
			nativeImage.ppPlane[1] = nativeImage.ppPlane[0] + width * height;
			break;
		case IMAGE_FORMAT_I420:
			nativeImage.ppPlane[1] = nativeImage.ppPlane[0] + width * height;
			nativeImage.ppPlane[2] = nativeImage.ppPlane[1] + width * height / 4;
			break;
		default:
			break;
	}

	//m_TextureMapSample->LoadImage(&nativeImage);

}

void MyGLRenderContext::OnSurfaceCreated()
{
	LOGCATE("MyGLRenderContext::OnSurfaceCreated");
	glClearColor(1.0f,1.0f,0.5f, 1.0f);
	m_Sample.Init();
}

void MyGLRenderContext::OnSurfaceChanged(int width, int height)
{
	LOGCATE("MyGLRenderContext::OnSurfaceChanged [w, h] = [%d, %d]", width, height);
	glViewport(0, 0, width, height);
}

void MyGLRenderContext::OnDrawFrame()
{
	LOGCATE("MyGLRenderContext::OnDrawFrame");
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    m_Sample.Draw();


}

MyGLRenderContext *MyGLRenderContext::GetInstance()
{
	LOGCATE("MyGLRenderContext::GetInstance");
	if (m_pContext == nullptr)
	{
		m_pContext = new MyGLRenderContext();
	}
	return m_pContext;
}

void MyGLRenderContext::DestroyInstance()
{
	LOGCATE("MyGLRenderContext::DestroyInstance");
	if (m_pContext)
	{
		delete m_pContext;
		m_pContext = nullptr;
	}

}

```

## 编译链接着色器程序，进行绘制
三角形的绘制实现类。
```cpp
//
// Created by ByteFlow on 2019/7/9.
//

#include "TriangleSample.h"
#include "../util/GLUtils.h"
#include "../util/LogUtil.h"

TriangleSample::TriangleSample()
{

}

TriangleSample::~TriangleSample()
{
	if (m_ProgramObj)
	{
		glDeleteProgram(m_ProgramObj);
	}

}

void TriangleSample::Init()
{
	char vShaderStr[] =
			"#version 300 es                          \n"
			"layout(location = 0) in vec4 vPosition;  \n"
			"void main()                              \n"
			"{                                        \n"
			"   gl_Position = vPosition;              \n"
			"}                                        \n";

	char fShaderStr[] =
			"#version 300 es                              \n"
			"precision mediump float;                     \n"
			"out vec4 fragColor;                          \n"
			"void main()                                  \n"
			"{                                            \n"
			"   fragColor = vec4 ( 1.0, 0.0, 0.0, 1.0 );  \n"
			"}                                            \n";

	m_ProgramObj = GLUtils::CreateProgram(vShaderStr, fShaderStr, m_VertexShader, m_FragmentShader);

}

void TriangleSample::Draw()
{
	LOGCATE("TriangleSample::Draw");
	GLfloat vVertices[] = {
			0.0f,  0.5f, 0.0f,
			-0.5f, -0.5f, 0.0f,
			0.5f, -0.5f, 0.0f,
	};

	if(m_ProgramObj == 0)
		return;

	// Use the program object
	glUseProgram (m_ProgramObj);

	// Load the vertex data
	glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, vVertices );
	glEnableVertexAttribArray (0);

	glDrawArrays (GL_TRIANGLES, 0, 3);

}

```

编译和链接着色器程序的类
```cpp
#include "GLUtils.h"
#include "LogUtil.h"
#include <stdlib.h>

GLuint GLUtils::LoadShader(GLenum shaderType, const char *pSource)
{
    GLuint shader = 0;
        shader = glCreateShader(shaderType);
        if (shader)
        {
            glShaderSource(shader, 1, &pSource, NULL);
            glCompileShader(shader);
            GLint compiled = 0;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
            if (!compiled)
            {
                GLint infoLen = 0;
                glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
                if (infoLen)
                {
                    char* buf = (char*) malloc((size_t)infoLen);
                    if (buf)
                    {
                        glGetShaderInfoLog(shader, infoLen, NULL, buf);
                        LOGCATE("GLUtils::LoadShader Could not compile shader %d:\n%s\n", shaderType, buf);
                        free(buf);
                    }
                    glDeleteShader(shader);
                    shader = 0;
                }
            }
        }
	return shader;
}

GLuint GLUtils::CreateProgram(const char *pVertexShaderSource, const char *pFragShaderSource, GLuint &vertexShaderHandle, GLuint &fragShaderHandle)
{
    GLuint program = 0;
        vertexShaderHandle = LoadShader(GL_VERTEX_SHADER, pVertexShaderSource);
        if (!vertexShaderHandle) return program;

        fragShaderHandle = LoadShader(GL_FRAGMENT_SHADER, pFragShaderSource);
        if (!fragShaderHandle) return program;

        program = glCreateProgram();
        if (program)
        {
            glAttachShader(program, vertexShaderHandle);
            CheckGLError("glAttachShader");
            glAttachShader(program, fragShaderHandle);
            CheckGLError("glAttachShader");
            glLinkProgram(program);
            GLint linkStatus = GL_FALSE;
            glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

            glDetachShader(program, vertexShaderHandle);
            glDeleteShader(vertexShaderHandle);
            vertexShaderHandle = 0;
            glDetachShader(program, fragShaderHandle);
            glDeleteShader(fragShaderHandle);
            fragShaderHandle = 0;
            if (linkStatus != GL_TRUE)
            {
                GLint bufLength = 0;
                glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
                if (bufLength)
                {
                    char* buf = (char*) malloc((size_t)bufLength);
                    if (buf)
                    {
                        glGetProgramInfoLog(program, bufLength, NULL, buf);
                        LOGCATE("GLUtils::CreateProgram Could not link program:\n%s\n", buf);
                        free(buf);
                    }
                }
                glDeleteProgram(program);
                program = 0;
            }
        }
    LOGCATE("GLUtils::CreateProgram program = %d", program);
	return program;
}

void GLUtils::DeleteProgram(GLuint &program)
{
    LOGCATE("GLUtils::DeleteProgram");
    if (program)
    {
        glUseProgram(0);
        glDeleteProgram(program);
        program = 0;
    }
}

void GLUtils::CheckGLError(const char *pGLOperation)
{
    for (GLint error = glGetError(); error; error = glGetError())
    {
        LOGCATE("GLUtils::CheckGLError GL Operation %s() glError (0x%x)\n", pGLOperation, error);
    }

}

```

在 Init 函数中实现编译链接着色器程序 m_ProgramObj ，其中顶点着色器脚本：
```
#version 300 es                          
layout(location = 0) in vec4 vPosition;  
void main()                              
{                                        
   gl_Position = vPosition;              
}                                        
```
片元着色器脚本：
```
#version 300 es                              
precision mediump float;                     
out vec4 fragColor;                          
void main()                                  
{                                            
   fragColor = vec4 ( 1.0, 0.0, 0.0, 1.0 );  //填充三角形区域为红色
}                                            
```
在 Draw 函数中指定着色器程序，为着色器程序中的变量赋值，传入顶点坐标信息，然后绘制三角形。

opengles 坐标系中三角形顶点坐标：
![opengles 坐标系中三角形顶点坐标](https://github.com/githubhaohao/NDK_OpenGLES_3_0/blob/master/doc/img/1/20190712151914390.png#pic_center)

绘制结果图

![绘制的结果图](https://github.com/githubhaohao/NDK_OpenGLES_3_0/blob/master/doc/img/1/20190712151834578.jpg#pic_center)

# 参考
[OpenGLES 维基百科 https://zh.wikipedia.org/wiki/OpenGL_ES](https://zh.wikipedia.org/wiki/OpenGL_ES)

[OpenGLES 3.0 编程指南 https://book.douban.com/subject/26414014/](https://book.douban.com/subject/26414014/)


# 联系与交流 #

微信公众号
![我的公众号](https://github.com/githubhaohao/NDK_OpenGLES_3_0/blob/master/doc/img/accountID.jpg#pic_center)
个人微信
![个人微信](https://github.com/githubhaohao/NDK_OpenGLES_3_0/blob/master/doc/img/WeChatID.jpg#pic_center)
