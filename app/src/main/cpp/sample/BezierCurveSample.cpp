/**
 *
 * Created by 公众号：字节流动 on 2020/4/18.
 * https://github.com/githubhaohao/NDK_OpenGLES_3_0
 * 最新文章首发于公众号：字节流动，有疑问或者技术交流可以添加微信 Byte-Flow ,领取视频教程, 拉你进技术交流群
 *
 * */

#include <gtc/matrix_transform.hpp>
#include "BezierCurveSample.h"
#include "../util/GLUtils.h"

//#define DRAW_POINTS
#define POINTS_NUM           256
#define POINTS_PRE_TRIANGLES 3

BezierCurveSample::BezierCurveSample() {

    m_SamplerLoc = GL_NONE;
    m_MVPMatLoc = GL_NONE;

    m_TextureId = GL_NONE;
    m_VaoId = GL_NONE;

    m_AngleX = 0;
    m_AngleY = 0;

    m_ScaleX = 1.0f;
    m_ScaleY = 1.0f;

    m_pCoordSystemSample = new CoordSystemSample();

    m_FrameIndex = 0;

}

BezierCurveSample::~BezierCurveSample() {
    NativeImageUtil::FreeNativeImage(&m_RenderImage);
    if (m_pCoordSystemSample != nullptr) {
        delete m_pCoordSystemSample;
        m_pCoordSystemSample = nullptr;
    }

}

void BezierCurveSample::Init() {
    if (m_ProgramObj)
        return;

    if (m_pCoordSystemSample != nullptr) {
        m_pCoordSystemSample->Init();
    }

    char vShaderStr[] =
            "#version 300 es\n"
            "layout(location = 0) in float a_tData;\n"
            "uniform vec4 u_StartEndPoints;\n"
            "uniform vec4 u_ControlPoints;\n"
            "uniform mat4 u_MVPMatrix;\n"
            "uniform float u_Offset;\n"
            "\n"
            "vec2 bezier3(in vec2 p0, in vec2 p1, in vec2 p2, in vec2 p3, in float t){\n"
            "    float tt = (1.0 - t) * (1.0 -t);\n"
            "    return tt * (1.0 -t) *p0 + 3.0 * t * tt * p1 + 3.0 * t *t *(1.0 -t) *p2 + t *t *t *p3;\n"
            "}\n"
            "\n"
            "vec2 bezier3_(in vec2 p0, in vec2 p1, in vec2 p2, in vec2 p3, in float t)\n"
            "{\n"
            "    vec2 q0 = mix(p0, p1, t);\n"
            "    vec2 q1 = mix(p1, p2, t);\n"
            "    vec2 q2 = mix(p2, p3, t);\n"
            "\n"
            "    vec2 r0 = mix(q0, q1, t);\n"
            "    vec2 r1 = mix(q1, q2, t);\n"
            "\n"
            "    return mix(r0, r1, t);\n"
            "}\n"
            "\n"
            "void main() {\n"
            "\n"
            "    vec4 pos;\n"
            "    pos.w = 1.0;\n"
            "\n"
            "    vec2 p0 = u_StartEndPoints.xy;\n"
            "    vec2 p3 = u_StartEndPoints.zw;\n"
            "\n"
            "    vec2 p1 = u_ControlPoints.xy;\n"
            "    vec2 p2 = u_ControlPoints.zw;\n"
            "\n"
            "    p0.y *= u_Offset;\n"
            "    p1.y *= u_Offset;\n"
            "    p2.y *= u_Offset;\n"
            "    p3.y *= u_Offset;\n"
            "\n"
            "    float t = a_tData;\n"
            "\n"
            "    vec2 point = bezier3_(p0, p1, p2, p3, t);\n"
            "\n"
            "    if (t < 0.0)\n"
            "    {\n"
            "        pos.xy = vec2(0.0, 0.0);\n"
            "    }\n"
            "    else\n"
            "    {\n"
            "        pos.xy = point;\n"
            "    }\n"
            "    gl_PointSize = 4.0;\n"
            "    gl_Position = u_MVPMatrix * pos;\n"
            "}";

    char fShaderStr[] =
            "#version 300 es\n"
            "precision mediump float;\n"
            "layout(location = 0) out vec4 outColor;\n"
            "uniform vec4 u_Color;\n"
            "void main()\n"
            "{\n"
            "    //outColor = texture(s_TextureMap, v_texCoord);\n"
            "    outColor = u_Color;\n"
            "}";

    m_ProgramObj = GLUtils::CreateProgram(vShaderStr, fShaderStr, m_VertexShader, m_FragmentShader);
    if (!m_ProgramObj) {
        LOGCATE("BezierCurveSample::Init create program fail");
    }

    int tDataSize = POINTS_NUM * POINTS_PRE_TRIANGLES;
    float *p_tData = new float[tDataSize];

    for (int i = 0; i < tDataSize; i += POINTS_PRE_TRIANGLES) {
#ifdef DRAW_POINTS
        float t0 = (float) i / tDataSize;
        float t1 = (float) (i + 1) / tDataSize;
        float t2 = (float) (i + 2) / tDataSize;

        p_tData[i] = t0;
        p_tData[i + 1] = t1;
        p_tData[i + 2] = t2;
#else
        float t = (float) i / tDataSize;
        float t1 = (float) (i + 3) / tDataSize;

        p_tData[i] = t;
        p_tData[i + 1] = t1;
        p_tData[i + 2] = -1;
#endif

    }

    // Generate VBO Ids and load the VBOs with data
    glGenBuffers(1, &m_VboId);
    glBindBuffer(GL_ARRAY_BUFFER, m_VboId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * tDataSize, p_tData, GL_STATIC_DRAW);

    delete[] p_tData;

    // Generate VAO Id
    glGenVertexArrays(1, &m_VaoId);
    glBindVertexArray(m_VaoId);

    glBindBuffer(GL_ARRAY_BUFFER, m_VboId);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat), (const void *) 0);

    glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);
    glBindVertexArray(GL_NONE);

}

void BezierCurveSample::LoadImage(NativeImage *pImage) {
    LOGCATE("BezierCurveSample::LoadImage pImage = %p", pImage->ppPlane[0]);
    if (m_pCoordSystemSample != nullptr) {
        m_pCoordSystemSample->LoadImage(pImage);
    }
}

void BezierCurveSample::Draw(int screenW, int screenH) {
    LOGCATE("BezierCurveSample::Draw()");

    if (m_pCoordSystemSample != nullptr) {
        //m_pCoordSystemSample->Draw(screenW, screenH);
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.1, 0.1, 0.1, 0.1);


    if (m_ProgramObj == GL_NONE) return;

    m_FrameIndex++;

    UpdateMVPMatrix(m_MVPMatrix, m_AngleX, m_AngleY, (float) screenW / screenH);

    // Use the program object
    glUseProgram(m_ProgramObj);

    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_COLOR, GL_ONE, GL_ONE_MINUS_SRC_ALPHA); // Screen blend mode
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);

    glBindVertexArray(m_VaoId);

    //draw one
    GLUtils::setMat4(m_ProgramObj, "u_MVPMatrix", m_MVPMatrix);
    GLUtils::setVec4(m_ProgramObj, "u_StartEndPoints", glm::vec4(-1, 0,
                                                               1, 0));
    GLUtils::setVec4(m_ProgramObj, "u_ControlPoints", glm::vec4(-0.04f, 0.99f,
                                                              0.0f, 0.99f));
    GLUtils::setVec4(m_ProgramObj, "u_Color", glm::vec4(1.0f, 0.3f, 0.0f, 1.0f));
    float offset = (m_FrameIndex % 100) * 1.0f / 100;
    offset = (m_FrameIndex / 100) % 2 == 1 ? (1 - offset) : offset;
    GLUtils::setFloat(m_ProgramObj, "u_Offset", offset);

    DrawArray();

    UpdateMVPMatrix(m_MVPMatrix, 180, m_AngleY, (float) screenW / screenH);
    GLUtils::setMat4(m_ProgramObj, "u_MVPMatrix", m_MVPMatrix);

    DrawArray();

    //draw two
    int newIndex = m_FrameIndex + 33;
    offset = (newIndex % 100) * 1.0f / 100;
    offset = (newIndex / 100) % 2 == 1 ? (1 - offset) : offset;
    GLUtils::setFloat(m_ProgramObj, "u_Offset", offset);
    GLUtils::setVec4(m_ProgramObj, "u_Color", glm::vec4(0.0f, 0.3f, 0.8f, 1.0f));
    GLUtils::setVec4(m_ProgramObj, "u_ControlPoints", glm::vec4(-0.8f, 0.99f,
                                                              0.0f, 0.0f));
    UpdateMVPMatrix(m_MVPMatrix, m_AngleX, m_AngleY, (float) screenW / screenH);
    GLUtils::setMat4(m_ProgramObj, "u_MVPMatrix", m_MVPMatrix);

    DrawArray();

    UpdateMVPMatrix(m_MVPMatrix, 180, m_AngleY, (float) screenW / screenH);
    GLUtils::setMat4(m_ProgramObj, "u_MVPMatrix", m_MVPMatrix);

    DrawArray();

    //draw three
    newIndex = newIndex + 33;
    offset = (newIndex % 100) * 1.0f / 100;
    offset = (newIndex / 100) % 2 == 1 ? (1 - offset) : offset;
    GLUtils::setFloat(m_ProgramObj, "u_Offset", offset);
    GLUtils::setVec4(m_ProgramObj, "u_Color", glm::vec4(0.1f, 0.6f, 0.3f, 1.0f));
    GLUtils::setVec4(m_ProgramObj, "u_ControlPoints", glm::vec4(0.0f, 0.0f, 0.8f, 0.99f));
    UpdateMVPMatrix(m_MVPMatrix, m_AngleX, m_AngleY, (float) screenW / screenH);
    GLUtils::setMat4(m_ProgramObj, "u_MVPMatrix", m_MVPMatrix);

    DrawArray();

    UpdateMVPMatrix(m_MVPMatrix, 180, m_AngleY, (float) screenW / screenH);
    GLUtils::setMat4(m_ProgramObj, "u_MVPMatrix", m_MVPMatrix);

    DrawArray();

    //draw four
    newIndex = newIndex + 33;
    offset = (newIndex % 100) * 1.0f / 100;
    offset = (newIndex / 100) % 2 == 1 ? (1 - offset) : offset;
    GLUtils::setFloat(m_ProgramObj, "u_Offset", offset);
    GLUtils::setVec4(m_ProgramObj, "u_Color", glm::vec4(1.0f, 0.0f, 0.3f, 1.0f));
    GLUtils::setVec4(m_ProgramObj, "u_ControlPoints", glm::vec4(-0.2f, 0.99f, 0.0f, 0.0f));
    UpdateMVPMatrix(m_MVPMatrix, m_AngleX, m_AngleY, (float) screenW / screenH);
    GLUtils::setMat4(m_ProgramObj, "u_MVPMatrix", m_MVPMatrix);

    DrawArray();

    UpdateMVPMatrix(m_MVPMatrix, 180, m_AngleY, (float) screenW / screenH);
    GLUtils::setMat4(m_ProgramObj, "u_MVPMatrix", m_MVPMatrix);

    DrawArray();

    //draw five
    newIndex = newIndex + 33;
    offset = (newIndex % 100) * 1.0f / 100;
    offset = (newIndex / 100) % 2 == 1 ? (1 - offset) : offset;
    GLUtils::setFloat(m_ProgramObj, "u_Offset", offset);
    GLUtils::setVec4(m_ProgramObj, "u_Color", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
    GLUtils::setVec4(m_ProgramObj, "u_ControlPoints", glm::vec4(0.0f, 0.0f, 0.2f, 0.99f));
    UpdateMVPMatrix(m_MVPMatrix, m_AngleX, m_AngleY, (float) screenW / screenH);
    GLUtils::setMat4(m_ProgramObj, "u_MVPMatrix", m_MVPMatrix);

    DrawArray();

    UpdateMVPMatrix(m_MVPMatrix, 180, m_AngleY, (float) screenW / screenH);
    GLUtils::setMat4(m_ProgramObj, "u_MVPMatrix", m_MVPMatrix);

    DrawArray();

    glDisable(GL_BLEND);

}

void BezierCurveSample::Destroy() {
    if (m_ProgramObj) {
        glDeleteProgram(m_ProgramObj);
        glDeleteBuffers(1, &m_VboId);
        glDeleteVertexArrays(1, &m_VaoId);
    }

    if (m_pCoordSystemSample != nullptr) {
        m_pCoordSystemSample->Destroy();
    }
}


/**
 * @param angleX 绕X轴旋转度数
 * @param angleY 绕Y轴旋转度数
 * @param ratio 宽高比
 * */
void BezierCurveSample::UpdateMVPMatrix(glm::mat4 &mvpMatrix, int angleX, int angleY, float ratio) {
    LOGCATE("BezierCurveSample::UpdateMVPMatrix angleX = %d, angleY = %d, ratio = %f", angleX,
            angleY, ratio);
    angleX = angleX % 360;
    angleY = angleY % 360;

    //转化为弧度角
    float radiansX = static_cast<float>(MATH_PI / 180.0f * angleX);
    float radiansY = static_cast<float>(MATH_PI / 180.0f * angleY);


    // Projection matrix
    glm::mat4 Projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 100.0f);
    //glm::mat4 Projection = glm::frustum(-ratio, ratio, -1.0f, 1.0f, 4.0f, 100.0f);
    //glm::mat4 Projection = glm::perspective(45.0f, ratio, 0.1f, 100.f);

    // View matrix
    glm::mat4 View = glm::lookAt(
            glm::vec3(0, 0, 4), // Camera is at (0,0,1), in World Space
            glm::vec3(0, 0, 0), // and looks at the origin
            glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
    );

    // Model matrix
    glm::mat4 Model = glm::mat4(1.0f);
    Model = glm::scale(Model, glm::vec3(m_ScaleX, m_ScaleY, 1.0f));
    Model = glm::rotate(Model, radiansX, glm::vec3(1.0f, 0.0f, 0.0f));
    Model = glm::rotate(Model, radiansY, glm::vec3(0.0f, 1.0f, 0.0f));
    Model = glm::translate(Model, glm::vec3(0.0f, 0.0f, 0.0f));

    mvpMatrix = Projection * View * Model;

}

void
BezierCurveSample::UpdateTransformMatrix(float rotateX, float rotateY, float scaleX, float scaleY) {
    GLSampleBase::UpdateTransformMatrix(rotateX, rotateY, scaleX, scaleY);
    m_AngleX = static_cast<int>(rotateX);
    m_AngleY = static_cast<int>(rotateY);
    m_ScaleX = scaleX;
    m_ScaleY = scaleY;

    if (m_pCoordSystemSample != nullptr) {
        m_pCoordSystemSample->UpdateTransformMatrix(rotateX, rotateY, scaleX, scaleY);
    }
}

void BezierCurveSample::DrawArray() {
#ifdef DRAW_POINTS
    glDrawArrays(GL_POINTS, 0, POINTS_NUM * TRIANGLES_PER_POINT);
#else
    glDrawArrays(GL_TRIANGLES, 0, POINTS_NUM * POINTS_PRE_TRIANGLES);
    glDrawArrays(GL_LINES, 0, POINTS_NUM * POINTS_PRE_TRIANGLES);
#endif
}
