/**
 *
 * Created by 公众号：字节流动 on 2021/3/12.
 * https://github.com/githubhaohao/NDK_OpenGLES_3_0
 * 最新文章首发于公众号：字节流动，有疑问或者技术交流可以添加微信 Byte-Flow ,领取视频教程, 拉你进技术交流群
 *
 * */

#ifndef NDK_OPENGLES_3_0_IMAGEDEF_H
#define NDK_OPENGLES_3_0_IMAGEDEF_H

#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include "stdio.h"
#include "sys/stat.h"
#include "stdint.h"
#include "LogUtil.h"

#define IMAGE_FORMAT_RGBA           0x01
#define IMAGE_FORMAT_NV21           0x02
#define IMAGE_FORMAT_NV12           0x03
#define IMAGE_FORMAT_I420           0x04
#define IMAGE_FORMAT_YUYV           0x05
#define IMAGE_FORMAT_GRAY           0x06
#define IMAGE_FORMAT_I444           0x07
#define IMAGE_FORMAT_P010           0x08

#define IMAGE_FORMAT_RGBA_EXT       "RGB32"
#define IMAGE_FORMAT_NV21_EXT       "NV21"
#define IMAGE_FORMAT_NV12_EXT       "NV12"
#define IMAGE_FORMAT_I420_EXT       "I420"
#define IMAGE_FORMAT_YUYV_EXT       "YUYV"
#define IMAGE_FORMAT_GRAY_EXT       "GRAY"
#define IMAGE_FORMAT_I444_EXT       "I444"
#define IMAGE_FORMAT_P010_EXT       "P010" //16bit NV21

typedef struct NativeRectF
{
	float left;
	float top;
	float right;
	float bottom;
	NativeRectF()
	{
		left = top = right = bottom = 0.0f;
	}
} RectF;

struct SizeF {
	float width;
	float height;
	SizeF() {
		width = height = 0;
	}
};

struct NativeImage
{
	int width;
	int height;
	int format;
	uint8_t *ppPlane[3];

	NativeImage()
	{
		width = 0;
		height = 0;
		format = 0;
		ppPlane[0] = nullptr;
		ppPlane[1] = nullptr;
		ppPlane[2] = nullptr;
	}
};

class NativeImageUtil
{
public:
	static void AllocNativeImage(NativeImage *pImage)
	{
		if (pImage->height == 0 || pImage->width == 0) return;

		switch (pImage->format)
		{
			case IMAGE_FORMAT_RGBA:
			{
				pImage->ppPlane[0] = static_cast<uint8_t *>(malloc(pImage->width * pImage->height * 4));
			}
				break;
			case IMAGE_FORMAT_YUYV:
			{
				pImage->ppPlane[0] = static_cast<uint8_t *>(malloc(pImage->width * pImage->height * 2));
			}
				break;
			case IMAGE_FORMAT_NV12:
			case IMAGE_FORMAT_NV21:
			{
				pImage->ppPlane[0] = static_cast<uint8_t *>(malloc(pImage->width * pImage->height * 1.5));
				pImage->ppPlane[1] = pImage->ppPlane[0] + pImage->width * pImage->height;
			}
				break;
			case IMAGE_FORMAT_I420:
			{
				pImage->ppPlane[0] = static_cast<uint8_t *>(malloc(pImage->width * pImage->height * 1.5));
				pImage->ppPlane[1] = pImage->ppPlane[0] + pImage->width * pImage->height;
				pImage->ppPlane[2] = pImage->ppPlane[1] + pImage->width * (pImage->height >> 2);
			}
				break;
			case IMAGE_FORMAT_GRAY:
			{
				pImage->ppPlane[0] = static_cast<uint8_t *>(malloc(pImage->width * pImage->height));
			}
				break;
			case IMAGE_FORMAT_I444:
			{
				pImage->ppPlane[0] = static_cast<uint8_t *>(malloc(pImage->width * pImage->height * 3));
			}
				break;
			case IMAGE_FORMAT_P010:
			{
				pImage->ppPlane[0] = static_cast<uint8_t *>(malloc(pImage->width * pImage->height * 3));
				pImage->ppPlane[1] = pImage->ppPlane[0] + pImage->width * pImage->height * 2;
			}
				break;
			default:
				LOGCATE("NativeImageUtil::AllocNativeImage do not support the format. Format = %d", pImage->format);
				break;
		}
	}

	static void FreeNativeImage(NativeImage *pImage)
	{
		if (pImage == nullptr || pImage->ppPlane[0] == nullptr) return;

		free(pImage->ppPlane[0]);
		pImage->ppPlane[0] = nullptr;
		pImage->ppPlane[1] = nullptr;
		pImage->ppPlane[2] = nullptr;
	}

	static void CopyNativeImage(NativeImage *pSrcImg, NativeImage *pDstImg)
	{
		if(pSrcImg == nullptr || pSrcImg->ppPlane[0] == nullptr) return;

		if(pSrcImg->format != pDstImg->format ||
		   pSrcImg->width != pDstImg->width ||
		   pSrcImg->height != pDstImg->height) return;

		if(pDstImg->ppPlane[0] == nullptr) AllocNativeImage(pDstImg);

		switch (pSrcImg->format)
		{
			case IMAGE_FORMAT_I420:
			case IMAGE_FORMAT_NV21:
			case IMAGE_FORMAT_NV12:
			{
				memcpy(pDstImg->ppPlane[0], pSrcImg->ppPlane[0], pSrcImg->width * pSrcImg->height * 1.5);
			}
				break;
			case IMAGE_FORMAT_YUYV:
			{
				memcpy(pDstImg->ppPlane[0], pSrcImg->ppPlane[0], pSrcImg->width * pSrcImg->height * 2);
			}
				break;
			case IMAGE_FORMAT_RGBA:
			{
				memcpy(pDstImg->ppPlane[0], pSrcImg->ppPlane[0], pSrcImg->width * pSrcImg->height * 4);
			}
				break;
			case IMAGE_FORMAT_GRAY:
			{
				memcpy(pDstImg->ppPlane[0], pSrcImg->ppPlane[0], pSrcImg->width * pSrcImg->height);
			}
				break;
			case IMAGE_FORMAT_P010:
			case IMAGE_FORMAT_I444:
			{
				memcpy(pDstImg->ppPlane[0], pSrcImg->ppPlane[0], pSrcImg->width * pSrcImg->height * 3);
			}
				break;
			default:
			{
				LOGCATE("NativeImageUtil::CopyNativeImage do not support the format. Format = %d", pSrcImg->format);
			}
				break;
		}

	}

	static void DumpNativeImage(NativeImage *pSrcImg, const char *pPath, const char *pFileName)
	{
		if (pSrcImg == nullptr || pPath == nullptr || pFileName == nullptr) return;

		if(access(pPath, 0) == -1)
		{
			mkdir(pPath, 0666);
		}

		char imgPath[256] = {0};
		const char *pExt = nullptr;
		switch (pSrcImg->format)
		{
			case IMAGE_FORMAT_I420:
				pExt = IMAGE_FORMAT_I420_EXT;
				break;
			case IMAGE_FORMAT_NV12:
				pExt = IMAGE_FORMAT_NV12_EXT;
				break;
			case IMAGE_FORMAT_NV21:
				pExt = IMAGE_FORMAT_NV21_EXT;
				break;
			case IMAGE_FORMAT_RGBA:
				pExt = IMAGE_FORMAT_RGBA_EXT;
				break;
			case IMAGE_FORMAT_YUYV:
				pExt = IMAGE_FORMAT_YUYV_EXT;
				break;
			case IMAGE_FORMAT_GRAY:
				pExt = IMAGE_FORMAT_GRAY_EXT;
				break;
			case IMAGE_FORMAT_I444:
				pExt = IMAGE_FORMAT_I444_EXT;
				break;
			case IMAGE_FORMAT_P010:
				pExt = IMAGE_FORMAT_P010_EXT;
				break;
			default:
				pExt = "Default";
				break;
		}

		sprintf(imgPath, "%s/IMG_%dx%d_%s.%s", pPath, pSrcImg->width, pSrcImg->height, pFileName, pExt);

		FILE *fp = fopen(imgPath, "wb");

		LOGCATE("DumpNativeImage fp=%p, file=%s", fp, imgPath);

		if(fp)
		{
			switch (pSrcImg->format)
			{
				case IMAGE_FORMAT_I420:
				{
					fwrite(pSrcImg->ppPlane[0],
						   static_cast<size_t>(pSrcImg->width * pSrcImg->height), 1, fp);
					fwrite(pSrcImg->ppPlane[1],
						   static_cast<size_t>((pSrcImg->width >> 1) * (pSrcImg->height >> 1)), 1, fp);
					fwrite(pSrcImg->ppPlane[2],
							static_cast<size_t>((pSrcImg->width >> 1) * (pSrcImg->height >> 1)),1,fp);
					break;
				}
				case IMAGE_FORMAT_NV21:
				case IMAGE_FORMAT_NV12:
				{
					fwrite(pSrcImg->ppPlane[0],
						   static_cast<size_t>(pSrcImg->width * pSrcImg->height), 1, fp);
					fwrite(pSrcImg->ppPlane[1],
						   static_cast<size_t>(pSrcImg->width * (pSrcImg->height >> 1)), 1, fp);
					break;
				}
				case IMAGE_FORMAT_RGBA:
				{
					fwrite(pSrcImg->ppPlane[0],
						   static_cast<size_t>(pSrcImg->width * pSrcImg->height * 4), 1, fp);
					break;
				}
				case IMAGE_FORMAT_YUYV:
				{
					fwrite(pSrcImg->ppPlane[0],
						   static_cast<size_t>(pSrcImg->width * pSrcImg->height * 2), 1, fp);
					break;
				}
				case IMAGE_FORMAT_GRAY:
				{
					fwrite(pSrcImg->ppPlane[0],
						   static_cast<size_t>(pSrcImg->width * pSrcImg->height), 1, fp);
					break;
				}
				case IMAGE_FORMAT_P010:
				{
					fwrite(pSrcImg->ppPlane[0],
						   static_cast<size_t>(pSrcImg->width * pSrcImg->height * 3), 1, fp);
					break;
				}
				case IMAGE_FORMAT_I444:
				{
					fwrite(pSrcImg->ppPlane[0],
						   static_cast<size_t>(pSrcImg->width * pSrcImg->height), 1, fp);
					fwrite(pSrcImg->ppPlane[1],
						   static_cast<size_t>(pSrcImg->width * pSrcImg->height), 1, fp);
					fwrite(pSrcImg->ppPlane[2],
						   static_cast<size_t>(pSrcImg->width * pSrcImg->height), 1, fp);
					break;
				}
				default:
				{
					fwrite(pSrcImg->ppPlane[0],
						   static_cast<size_t>(pSrcImg->width * pSrcImg->height), 1, fp);
					LOGCATE("DumpNativeImage default");
					break;
				}
			}

			fclose(fp);
			fp = NULL;
		}
	}

	static void LoadNativeImage(NativeImage *pSrcImg, const char *pPath)
	{
		if (pSrcImg == nullptr || pPath == nullptr) return;

		FILE *fp = fopen(pPath, "rb");
		LOGCATE("LoadNativeImage fp=%p, file=%s", fp, pPath);
		int dataSize = 0;
		if(fp)
		{
			switch (pSrcImg->format)
			{
				case IMAGE_FORMAT_I420:
				case IMAGE_FORMAT_NV21:
				case IMAGE_FORMAT_NV12:
				{
					dataSize = pSrcImg->width * pSrcImg->height * 1.5;
					fread(pSrcImg->ppPlane[0], dataSize, 1, fp);
					break;
				}
				case IMAGE_FORMAT_RGBA:
				{
					dataSize = pSrcImg->width * pSrcImg->height * 4;
					fread(pSrcImg->ppPlane[0], dataSize, 1, fp);
					break;
				}
				case IMAGE_FORMAT_YUYV:
				{
					dataSize = pSrcImg->width * pSrcImg->height * 2;
					fread(pSrcImg->ppPlane[0], dataSize, 1, fp);
					break;
				}
				case IMAGE_FORMAT_GRAY:
				{
					dataSize = pSrcImg->width * pSrcImg->height;
					fread(pSrcImg->ppPlane[0], dataSize, 1, fp);
					break;
				}
				case IMAGE_FORMAT_P010:
				case IMAGE_FORMAT_I444:
				{
					dataSize = pSrcImg->width * pSrcImg->height * 3;
					fread(pSrcImg->ppPlane[0], dataSize, 1, fp);
					break;
				}
				default:
				{
					LOGCATE("LoadNativeImage not support the format %d.", pSrcImg->format);
					break;
				}
			}

			fclose(fp);
			fp = NULL;
		}
	}

	static int ConvertP010toNV21(NativeImage* pP010Img, NativeImage* pNV21Img) {
		if(pP010Img == nullptr
		|| pNV21Img == nullptr
		|| pP010Img->format != IMAGE_FORMAT_P010
		|| pNV21Img->format != IMAGE_FORMAT_NV21) return -1;

		int width = pP010Img->width, height = pP010Img->height;
		for (int i = 0; i < height; ++i) {
			uint16_t *pu16YData = (uint16_t *)(pP010Img->ppPlane[0] + pP010Img->width * 2 * i);
			uint8_t  *pu8YData = pNV21Img->ppPlane[0] + pNV21Img->width * i;
			for (int j = 0; j < width; j++, pu8YData++, pu16YData++) {
				*pu8YData = (u_int8_t)(*pu16YData >> 8);
			}
		}

		width /= 2; height /= 2;
		for (int i = 0; i < height; ++i) {
			uint16_t *pu16UVData = (uint16_t *)(pP010Img->ppPlane[1] + pP010Img->width * 2 * i);
			uint8_t  *pu8UVData = pNV21Img->ppPlane[1] + pNV21Img->width * i;
			for (int j = 0; j < width; ++j, pu8UVData+=2, pu16UVData+=2) {
				*pu8UVData = *pu16UVData >> 8;
				*(pu8UVData + 1) = *(pu16UVData + 1) >> 8;
			}
		}
		return 0;
	}

	static void ConvertP010PlaneTo8Bit(uint16_t *pSrcData, uint8_t  *pDstData, int width, int height) {
		if(pSrcData == nullptr
		   || pDstData == nullptr
		   || width <= 0
		   || height <= 0) return;

		for (int i = 0; i < height; ++i) {
			uint16_t *pu16YData = pSrcData + width * i;
			uint8_t  *pu8YData = pDstData + width * i;
			for (int j = 0; j < width; j++, pu8YData++, pu16YData++) {
				*pu8YData = (u_int8_t)(*pu16YData >> 8);
			}
		}
	}

	static int ConvertNV21toP010(NativeImage* pNV21Img, NativeImage* pP010Img) {
		if(pP010Img == nullptr
		   || pNV21Img == nullptr
		   || pP010Img->format != IMAGE_FORMAT_P010
		   || pNV21Img->format != IMAGE_FORMAT_NV21) return -1;

		int width = pP010Img->width, height = pP010Img->height;
		for (int i = 0; i < height; ++i) {
			uint16_t *pu16YData = (uint16_t *)(pP010Img->ppPlane[0] + pP010Img->width * 2 * i);
			uint8_t  *pu8YData = pNV21Img->ppPlane[0] + pNV21Img->width * i;
			for (int j = 0; j < width; j++, pu8YData++, pu16YData++) {
				*pu16YData = (u_int16_t)*pu8YData << 8;
			}
		}

		width /= 2; height /= 2;
		for (int i = 0; i < height; ++i) {
			uint16_t *pu16UVData = (uint16_t *)(pP010Img->ppPlane[1] + pP010Img->width * 2 * i);
			uint8_t  *pu8UVData = pNV21Img->ppPlane[1] + pNV21Img->width * i;
			for (int j = 0; j < width; ++j, pu8UVData+=2, pu16UVData+=2) {
				*pu16UVData = (u_int16_t)*pu8UVData << 8;
				*(pu16UVData + 1) = (u_int16_t)*(pu8UVData + 1) << 8;
			}
		}

		return 0;
	}
};


#endif //NDK_OPENGLES_3_0_IMAGEDEF_H
