//
// Created by ByteFlow on 2020/3/10.
//

#ifndef NDK_OPENGLES_3_0_COMMONDEF_H
#define NDK_OPENGLES_3_0_COMMONDEF_H

#include <unistd.h>
#include <cmath>
#include "stdio.h"
#include "stdint.h"

typedef struct _tag_PointF
{
    float x;
    float y;
    _tag_PointF()
    {
        x = y = 0;
    }

    _tag_PointF(float xBar, float yBar)
    {
        x = xBar;
        y = yBar;
    }
} PointF;

class PointUtil
{
public:
    static float Distance(PointF p0, PointF p1)
    {
        return static_cast<float>(sqrt(pow(p0.x - p1.x, 2) + pow(p0.y - p1.y, 2)));
    }

    static float DistanceSquare(PointF p0, PointF p1)
    {
        return static_cast<float>(pow(p0.x - p1.x, 2) + pow(p0.y - p1.y, 2));
    }

    static PointF PointMinus(PointF p0, PointF p1)
    {
        PointF pointF;
        pointF.x = p0.x - p1.x;
        pointF.y = p0.y - p1.y;
        return pointF;
    }

    static PointF PointAdd(PointF p0, PointF p1)
    {
        PointF pointF;
        pointF.x = p0.x + p1.x;
        pointF.y = p0.y + p1.y;
        return pointF;
    }

    static PointF PointTimes(PointF p0, PointF p1)
    {
        PointF pointF;
        pointF.x = p0.x * p1.x;
        pointF.y = p0.y * p1.y;
        return pointF;
    }

    static PointF PointTimes(PointF p0, float p1)
    {
        PointF pointF;
        pointF.x = p0.x * p1;
        pointF.y = p0.y * p1;
        return pointF;
    }

    static PointF PointDivide(PointF p0, PointF p1)
    {
        PointF pointF;
        pointF.x = p0.x / p1.x;
        pointF.y = p0.y / p1.y;
        return pointF;
    }

    static PointF PointDivide(PointF p0, float p1)
    {
        PointF pointF;
        pointF.x = p0.x / p1;
        pointF.y = p0.y / p1;
        return pointF;
    }
};

#endif //NDK_OPENGLES_3_0_COMMONDEF_H
