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
#include <StencilTestingSample.h>
#include <BlendingSample.h>
#include <ParticlesSample.h>
#include <Noise3DSample.h>
#include <SkyBoxSample.h>
#include <Model3DSample.h>
#include <PBOSample.h>
#include "MyGLRenderContext.h"
#include "LogUtil.h"

MyGLRenderContext* MyGLRenderContext::m_pContext = nullptr;

MyGLRenderContext::MyGLRenderContext()
{
	m_pCurSample = new TriangleSample();
	m_pBeforeSample = nullptr;

}

MyGLRenderContext::~MyGLRenderContext()
{
	if (m_pCurSample)
	{
		delete m_pCurSample;
		m_pCurSample = nullptr;
	}

	if (m_pBeforeSample)
	{
		delete m_pBeforeSample;
		m_pBeforeSample = nullptr;
	}

}


void MyGLRenderContext::SetParamsInt(int paramType, int value0, int value1)
{
	LOGCATE("MyGLRenderContext::SetParamsInt paramType = %d, value0 = %d, value1 = %d", paramType, value0, value1);

	if (paramType == SAMPLE_TYPE)
	{
		m_pBeforeSample = m_pCurSample;

		LOGCATE("MyGLRenderContext::SetParamsInt 0 m_pBeforeSample = %p", m_pBeforeSample);

		switch (value0)
		{
			case SAMPLE_TYPE_KEY_TRIANGLE:
				m_pCurSample = new TriangleSample();
				break;
			case SAMPLE_TYPE_KEY_TEXTURE_MAP:
				m_pCurSample = new TextureMapSample();
				break;
			case SAMPLE_TYPE_KEY_YUV_TEXTURE_MAP:
				m_pCurSample = new NV21TextureMapSample();
				break;
			case SAMPLE_TYPE_KEY_VAO:
				m_pCurSample = new VaoSample();
				break;
			case SAMPLE_TYPE_KEY_FBO:
				m_pCurSample = new FBOSample();
				break;
			case SAMPLE_TYPE_KEY_FBO_LEG_LENGTHEN:
				m_pCurSample = new FBOLegLengthenSample();
				break;
			case SAMPLE_TYPE_KEY_COORD_SYSTEM:
				m_pCurSample = new CoordSystemSample();
				break;
			case SAMPLE_TYPE_KEY_BASIC_LIGHTING:
				m_pCurSample = new BasicLightingSample();
				break;
			case SAMPLE_TYPE_KEY_TRANSFORM_FEEDBACK:
				m_pCurSample = new TransformFeedbackSample();
				break;
			case SAMPLE_TYPE_KEY_MULTI_LIGHTS:
				m_pCurSample = new MultiLightsSample();
				break;
			case SAMPLE_TYPE_KEY_DEPTH_TESTING:
				m_pCurSample = new DepthTestingSample();
				break;
			case SAMPLE_TYPE_KEY_INSTANCING:
				m_pCurSample = new Instancing3DSample();
				break;
			case SAMPLE_TYPE_KEY_STENCIL_TESTING:
				m_pCurSample = new StencilTestingSample();
				break;
			case SAMPLE_TYPE_KEY_BLENDING:
				m_pCurSample = new BlendingSample();
				break;
			case SAMPLE_TYPE_KEY_PARTICLES:
				m_pCurSample = new ParticlesSample();
				break;
			case SAMPLE_TYPE_KEY_SKYBOX:
				m_pCurSample = new SkyBoxSample();
				break;
			case SAMPLE_TYPE_KEY_3D_MODEL:
				m_pCurSample = new Model3DSample();
				break;
			case SAMPLE_TYPE_KEY_PBO:
				m_pCurSample = new PBOSample();
				break;
			default:
			    m_pCurSample = nullptr;
				break;
		}

		LOGCATE("MyGLRenderContext::SetParamsInt m_pBeforeSample = %p, m_pCurSample=%p", m_pBeforeSample, m_pCurSample);
	}
}

void MyGLRenderContext::UpdateTransformMatrix(float rotateX, float rotateY, float scaleX, float scaleY)
{
	LOGCATE("MyGLRenderContext::UpdateTransformMatrix [rotateX, rotateY, scaleX, scaleY] = [%f, %f, %f, %f]", rotateX, rotateY, scaleX, scaleY);
	if (m_pCurSample)
	{
		m_pCurSample->UpdateTransformMatrix(rotateX, rotateY, scaleX, scaleY);
	}
}

void MyGLRenderContext::SetImageDataWithIndex(int index, int format, int width, int height, uint8_t *pData)
{
	LOGCATE("MyGLRenderContext::SetImageDataWithIndex index=%d, format=%d, width=%d, height=%d, pData=%p", index, format, width, height, pData);
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

	if (m_pCurSample)
	{
		m_pCurSample->LoadMultiImageWithIndex(index, &nativeImage);
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

	if (m_pCurSample)
	{
		m_pCurSample->LoadImage(&nativeImage);
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

	if (m_pBeforeSample)
	{
		m_pBeforeSample->Destroy();
		delete m_pBeforeSample;
		m_pBeforeSample = nullptr;
	}

	if (m_pCurSample)
	{
		m_pCurSample->Init();
		m_pCurSample->Draw(m_ScreenW, m_ScreenH);
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


