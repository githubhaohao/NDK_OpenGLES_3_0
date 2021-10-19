/**
 *
 * Created by 公众号：字节流动 on 2022/4/18.
 * https://github.com/githubhaohao/NDK_OpenGLES_3_0
 * 最新文章首发于公众号：字节流动，有疑问或者技术交流可以添加微信 Byte-Flow ,领取视频教程, 拉你进技术交流群
 *
 * */

#ifndef NDK_OPENGLES_3_0_YUVP010EXAMPLE_H
#define NDK_OPENGLES_3_0_YUVP010EXAMPLE_H
#include "ImageDef.h"
#include "LogUtil.h"
#include "thread"
#include "GLSampleBase.h"

#define DEFAULT_YUV_IMAGE_NAME "IMAGE_4406x3108.NV21"

class YUVP010Example {
public:
    static void YUVP010Test() {
        NativeImage p010Img, nv21Img;
        p010Img.width = 4406;
        p010Img.height = 3108;
        p010Img.format = IMAGE_FORMAT_P010;

        nv21Img = p010Img;
        nv21Img.format = IMAGE_FORMAT_NV21;

        //申请内存
        NativeImageUtil::AllocNativeImage(&p010Img);
        NativeImageUtil::AllocNativeImage(&nv21Img);

        //加载 NV21 图片
        char filePath[512] = {0};
        sprintf(filePath, "%s/yuv/%s", DEFAULT_OGL_ASSETS_DIR, DEFAULT_YUV_IMAGE_NAME);
        NativeImageUtil::LoadNativeImage(&nv21Img, filePath);

        //NV21 转换为 P010
        {
            BEGIN_TIME("NativeImageUtil::ConvertNV21toP010")
            NativeImageUtil::ConvertNV21toP010(&nv21Img, &p010Img);
            END_TIME("NativeImageUtil::ConvertNV21toP010")
        }

        //保存 P010 图像到手机
        NativeImageUtil::DumpNativeImage(&p010Img, DEFAULT_OGL_ASSETS_DIR, "IMAGE_P010");

        //P010 转换为 NV21
        {
            BEGIN_TIME("NativeImageUtil::ConvertP010toNV21")
            NativeImageUtil::ConvertP010toNV21(&p010Img, &nv21Img);
            END_TIME("NativeImageUtil::ConvertP010toNV21")
        }

        //多线程实现 P010 转换为 NV21
        {
            BEGIN_TIME("NativeImageUtil::ConvertP010toNV21 MultiThread")
            std::thread *pThreads[2] = {nullptr};
            pThreads[0] = new std::thread(NativeImageUtil::ConvertP010PlaneTo8Bit, (u_int16_t*)p010Img.ppPlane[0], nv21Img.ppPlane[0], nv21Img.width, nv21Img.height / 2);
            pThreads[1] = new std::thread(NativeImageUtil::ConvertP010PlaneTo8Bit, (u_int16_t*)p010Img.ppPlane[0] + p010Img.height * p010Img.width / 2, nv21Img.ppPlane[0] + nv21Img.height * nv21Img.width / 2, nv21Img.width, nv21Img.height / 2);
            //pThreads[2] = new std::thread(NativeImageUtil::ConvertP010PlaneTo8Bit, (u_int16_t*)p010Img.ppPlane[1], nv21Img.ppPlane[1], nv21Img.width, nv21Img.height / 2);
            NativeImageUtil::ConvertP010PlaneTo8Bit((u_int16_t*)p010Img.ppPlane[1], nv21Img.ppPlane[1], nv21Img.width, nv21Img.height / 2);
            for (int i = 0; i < 2; ++i) {
                pThreads[i]->join();
            }

            for (int i = 0; i < 2; ++i) {
                delete pThreads[i];
            }
            END_TIME("NativeImageUtil::ConvertP010toNV21 MultiThread")

        }

        NativeImageUtil::DumpNativeImage(&nv21Img, DEFAULT_OGL_ASSETS_DIR, "IMAGE_NV21");

        //释放内存
        NativeImageUtil::FreeNativeImage(&p010Img);
        NativeImageUtil::FreeNativeImage(&nv21Img);
    }
};

/**  图像转换性能
 *  ByteFlow: NativeImageUtil::ConvertNV21toP010 func start
 *	ByteFlow: NativeImageUtil::ConvertNV21toP010 func cost time 63ms
 *	ByteFlow: NativeImageUtil::ConvertP010toNV21 func start
 *	ByteFlow: NativeImageUtil::ConvertP010toNV21 func cost time 52ms
 *	ByteFlow: NativeImageUtil::ConvertP010toNV21 MultiThread func start
 *	ByteFlow: NativeImageUtil::ConvertP010toNV21 MultiThread func cost time 23ms
 *
 *	ByteFlow: NativeImageUtil::ConvertNV21toP010 func start
 *	ByteFlow: NativeImageUtil::ConvertNV21toP010 func cost time 62ms
 *	ByteFlow: NativeImageUtil::ConvertP010toNV21 func start
 *	ByteFlow: NativeImageUtil::ConvertP010toNV21 func cost time 48ms
 *	ByteFlow: NativeImageUtil::ConvertP010toNV21 MultiThread func start
 *	ByteFlow: NativeImageUtil::ConvertP010toNV21 MultiThread func cost time 27ms
 *
 * */
#endif //NDK_OPENGLES_3_0_YUVP010EXAMPLE_H
