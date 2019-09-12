//
// Created by ByteFlow on 2019/7/9.
//

#include <TriangleSample.h>
#include <TextureMapSample.h>
#include <NV21TextureMapSample.h>
#include <VaoSample.h>
#include <FBOSample.h>
#include <FBOLegLengthenSample.h>
#include <CoordSystemSample.h>
#include <BasicLightingSample.h>
#include <TransformFeedbackSample.h>
#include <MultiLightsSample.h>
#include <DepthTestingSample.h>
#include <InstancingSample.h>
#include <Instancing3DSample.h>
#include "MyGLRenderContext.h"
#include "LogUtil.h"

MyGLRenderContext* MyGLRenderContext::m_pContext = nullptr;

MyGLRenderContext::MyGLRenderContext()
{
	m_Sample = new TriangleSample();

}

MyGLRenderContext::~MyGLRenderContext()
{
	if (m_Sample)
	{
		delete m_Sample;
		m_Sample = nullptr;
	}

}


void MyGLRenderContext::SetParamsInt(int paramType, int value0, int value1)
{
	LOGCATE("MyGLRenderContext::SetParamsInt paramType = %d, value0 = %d, value1 = %d", paramType, value0, value1);

	if (paramType == SAMPLE_TYPE)
	{
		if (m_Sample)
		{
			delete m_Sample;
			m_Sample = nullptr;
		}
		switch (value0)
		{
			case SAMPLE_TYPE_KEY_TRIANGLE:
				m_Sample = new TriangleSample();
				break;
			case SAMPLE_TYPE_KEY_TEXTURE_MAP:
				m_Sample = new TextureMapSample();
				break;
			case SAMPLE_TYPE_KEY_YUV_TEXTURE_MAP:
				m_Sample = new NV21TextureMapSample();
				break;
			case SAMPLE_TYPE_KEY_VAO:
				m_Sample = new VaoSample();
				break;
			case SAMPLE_TYPE_KEY_FBO:
				m_Sample = new FBOSample();
				break;
			case SAMPLE_TYPE_KEY_FBO_LEG_LENGTHEN:
				m_Sample = new FBOLegLengthenSample();
				break;
			case SAMPLE_TYPE_KEY_COORD_SYSTEM:
				m_Sample = new CoordSystemSample();
				break;
			case SAMPLE_TYPE_KEY_BASIC_LIGHTING:
				m_Sample = new BasicLightingSample();
				break;
			case SAMPLE_TYPE_KEY_TRANSFORM_FEEDBACK:
				m_Sample = new TransformFeedbackSample();
				break;
			case SAMPLE_TYPE_KEY_MULTI_LIGHTS:
				m_Sample = new MultiLightsSample();
				break;
			case SAMPLE_TYPE_KEY_DEPTH_TESTING:
				m_Sample = new DepthTestingSample();
				break;
			case SAMPLE_TYPE_KEY_INSTANCING:
				m_Sample = new Instancing3DSample();
				break;
			default:
				break;
		}
	}

	if (paramType == ROTATE_ANGLE_PARAM_TYPE && m_Sample)
	{
		m_Sample->SetParamsInt(paramType, value0, value1);
	}

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

	if (m_Sample)
	{
		m_Sample->LoadImage(&nativeImage);
	}

}

void MyGLRenderContext::OnSurfaceCreated()
{
	LOGCATE("MyGLRenderContext::OnSurfaceCreated");
	glClearColor(1.0f,1.0f,1.0f, 1.0f);
}

void MyGLRenderContext::OnSurfaceChanged(int width, int height)
{
	LOGCATE("MyGLRenderContext::OnSurfaceChanged [w, h] = [%d, %d]", width, height);
	glViewport(0, 0, width, height);
	m_ScreenW = width;
	m_ScreenH = height;
}

void MyGLRenderContext::OnDrawFrame()
{
	LOGCATE("MyGLRenderContext::OnDrawFrame");
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	if (m_Sample)
	{
		m_Sample->Init();
		m_Sample->Draw(m_ScreenW, m_ScreenH);
		m_Sample->Destroy();
	}
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

