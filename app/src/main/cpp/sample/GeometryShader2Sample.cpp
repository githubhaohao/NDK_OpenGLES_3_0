/**
 *
 * Created by 公众号：字节流动 on 2024/12/18.
 * https://github.com/githubhaohao/NDK_OpenGLES_3_0
 * 最新文章首发于公众号：字节流动，有疑问或者技术交流可以添加微信 Byte-Flow ,领取视频教程, 拉你进技术交流群
 *
 * */

#include <gtc/matrix_transform.hpp>
#include "GeometryShader2Sample.h"
#include "../util/GLUtils.h"

GeometryShader2Sample::GeometryShader2Sample()
{
	m_AngleX = 0;
	m_AngleY = 0;

	m_ScaleX = 1.0f;
	m_ScaleY = 1.0f;

	m_pModel = nullptr;
	m_pShader = nullptr;

    m_FrameIndex = 0;
}

GeometryShader2Sample::~GeometryShader2Sample()
{

}

void GeometryShader2Sample::Init()
{
	if(m_pModel != nullptr && m_pShader != nullptr)
		return;

	char vShaderStr[] =
			"#version 320 es\n"
            "precision mediump float;\n"
            "layout (location = 0) in vec3 a_position;\n"
            "layout (location = 1) in vec3 a_normal;\n"
            "layout (location = 2) in vec2 a_texCoord;\n"
            "uniform mat4 u_MVPMatrix;\n"
            "uniform mat4 u_ModelMatrix;\n"
            "uniform vec3 lightPos;\n"
            "uniform vec3 lightColor;\n"
            "uniform vec3 viewPos;\n"
            "\n"
            "out VS_OUT {\n"
            "    vec2 v_texCoord;\n"
            "    vec3 ambient;\n"
            "    vec3 diffuse;\n"
            "    vec3 specular;\n"
            "} vs_out;\n"
            "\n"
            "void main()\n"
            "{\n"
            "    vs_out.v_texCoord = a_texCoord;\n"
            "    vec4 position = vec4(a_position, 1.0);\n"
            "    gl_Position = u_MVPMatrix * position;\n"
            "    vec3 fragPos = vec3(u_ModelMatrix * position);\n"
            "\n"
            "    // Ambient\n"
            "    float ambientStrength = 0.25;\n"
            "    vs_out.ambient = ambientStrength * lightColor;\n"
            "\n"
            "    // Diffuse\n"
            "    float diffuseStrength = 0.5;\n"
            "    vec3 unitNormal = normalize(vec3(u_ModelMatrix * vec4(a_normal, 1.0)));\n"
            "    vec3 lightDir = normalize(lightPos - fragPos);\n"
            "    float diff = max(dot(unitNormal, lightDir), 0.0);\n"
            "    vs_out.diffuse = diffuseStrength * diff * lightColor;\n"
            "\n"
            "    // Specular\n"
            "    float specularStrength = 0.3;\n"
            "    vec3 viewDir = normalize(viewPos - fragPos);\n"
            "    vec3 reflectDir = reflect(-lightDir, unitNormal);\n"
            "    float spec = pow(max(dot(unitNormal, reflectDir), 0.0), 16.0);\n"
            "    vs_out.specular = specularStrength * spec * lightColor;\n"
            "}";
    char gShaderStr[] = "#version 320 es\n"
                        "layout (triangles) in;\n"
                        "layout (triangle_strip, max_vertices = 3) out;\n"
                        "\n"
                        "in VS_OUT {\n"
                        "    vec2 v_texCoord;\n"
                        "    vec3 ambient;\n"
                        "    vec3 diffuse;\n"
                        "    vec3 specular;\n"
                        "} gs_in[];\n"
                        "\n"
                        "out vec2 v_texCoord;\n"
                        "out vec3 ambient;\n"
                        "out vec3 diffuse;\n"
                        "out vec3 specular;\n"
                        "\n"
                        "uniform float iTime;\n"
                        "\n"
                        "vec4 explode(vec4 position, vec3 normal)\n"
                        "{\n"
                        "    float magnitude = 0.75;\n"
                        "    vec3 direction = normal * ((sin(iTime) + 1.0) / 2.0) * magnitude;\n"
                        "    return position + vec4(direction, 0.0);\n"
                        "}\n"
                        "\n"
                        "vec3 GetNormal()\n"
                        "{\n"
                        "    vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);\n"
                        "    vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);\n"
                        "    return normalize(cross(a, b));\n"
                        "}\n"
                        "\n"
                        "void main() {\n"
                        "    vec3 normal = GetNormal();\n"
                        "\n"
                        "    gl_Position = explode(gl_in[0].gl_Position, normal);\n"
                        "    v_texCoord = gs_in[0].v_texCoord;\n"
                        "    ambient = gs_in[0].ambient;\n"
                        "    diffuse = gs_in[0].diffuse;\n"
                        "    specular = gs_in[0].specular;\n"
                        "    EmitVertex();\n"
                        "    gl_Position = explode(gl_in[1].gl_Position, normal);\n"
                        "    v_texCoord = gs_in[1].v_texCoord;\n"
                        "    ambient = gs_in[1].ambient;\n"
                        "    diffuse = gs_in[1].diffuse;\n"
                        "    specular = gs_in[1].specular;\n"
                        "    EmitVertex();\n"
                        "    gl_Position = explode(gl_in[2].gl_Position, normal);\n"
                        "    v_texCoord = gs_in[2].v_texCoord;\n"
                        "    ambient = gs_in[2].ambient;\n"
                        "    diffuse = gs_in[2].diffuse;\n"
                        "    specular = gs_in[2].specular;\n"
                        "    EmitVertex();\n"
                        "    EndPrimitive();\n"
                        "}";

	char fShaderStr[] =
			"#version 320 es\n"
            "precision mediump float;\n"
            "out vec4 outColor;\n"
            "in vec2 v_texCoord;\n"
            "in vec3 ambient;\n"
            "in vec3 diffuse;\n"
            "in vec3 specular;\n"
            "uniform sampler2D texture_diffuse1;\n"
            "void main()\n"
            "{    \n"
            "    vec4 objectColor = texture(texture_diffuse1, v_texCoord);\n"
            "    vec3 finalColor = (ambient + diffuse + specular) * vec3(objectColor);\n"
            "    outColor = vec4(finalColor, 1.0);\n"
            "}";

    char fNoTextureShaderStr[] =
            "#version 300 es\n"
            "precision highp float;\n"
            "out vec4 outColor;\n"
            "in vec3 ambient;\n"
            "in vec3 diffuse;\n"
            "in vec3 specular;\n"
            "void main()\n"
            "{    \n"
            "    vec4 objectColor = vec4(0.6, 0.6, 0.6, 1.0);\n"
            "    vec3 finalColor = (ambient + diffuse + specular) * vec3(objectColor);\n"
            "    outColor = vec4(finalColor, 1.0);\n"
            "}";
    //TODO 先把 model 文件夹拷贝到 /sdcard/Android/data/com.byteflow.app/files/Download 路径下，然后可以选择你要加载的模型
	//m_pModel = new Model(path + "/model/nanosuit/nanosuit.obj");
	std::string path(DEFAULT_OGL_ASSETS_DIR);
    m_pModel = new Model(path + "/model/poly/Apricot_02_hi_poly.obj");
    //m_pModel = new Model(path + "/model/tank/Abrams_BF3.obj");
    //m_pModel = new Model(path + "/model/girl/091_W_Aya_10K.obj");//2000 1500
    //m_pModel = new Model(path + "/model/new/camaro.obj");
    //m_pModel = new Model(path + "/model/bird/12214_Bird_v1max_l3.obj");

    if (m_pModel->ContainsTextures())
    {
        m_pShader = new Shader(vShaderStr, fShaderStr, gShaderStr);
    }
    else
    {
        m_pShader = new Shader(vShaderStr, fNoTextureShaderStr, gShaderStr);
    }
}

void GeometryShader2Sample::LoadImage(NativeImage *pImage)
{
	LOGCATE("GeometryShader2Sample::LoadImage pImage = %p", pImage->ppPlane[0]);

}

void GeometryShader2Sample::Draw(int screenW, int screenH)
{
	if(m_pModel == nullptr || m_pShader == nullptr) return;
    LOGCATE("GeometryShader2Sample::Draw()");
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

	UpdateMVPMatrix(m_MVPMatrix, m_AngleX, m_AngleY, (float)screenW / screenH);

    m_pShader->use();
    m_pShader->setMat4("u_MVPMatrix", m_MVPMatrix);
    m_pShader->setMat4("u_ModelMatrix", m_ModelMatrix);
    m_pShader->setVec3("lightPos", glm::vec3(0, 0, m_pModel->GetMaxViewDistance()));
    m_pShader->setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
    m_pShader->setVec3("viewPos", glm::vec3(0, 0, m_pModel->GetMaxViewDistance()));
    m_pShader->setFloat("iTime", m_FrameIndex * 0.01f);
    m_pModel->Draw((*m_pShader));

    m_FrameIndex ++;
}

void GeometryShader2Sample::Destroy()
{
    LOGCATE("GeometryShader2Sample::Destroy");
    if (m_pModel != nullptr) {
        m_pModel->Destroy();
        delete m_pModel;
        m_pModel = nullptr;
    }

    if (m_pShader != nullptr) {
        m_pShader->Destroy();
        delete m_pShader;
        m_pShader = nullptr;
    }
}

void GeometryShader2Sample::UpdateMVPMatrix(glm::mat4 &mvpMatrix, int angleX, int angleY, float ratio)
{
	LOGCATE("GeometryShader2Sample::UpdateMVPMatrix angleX = %d, angleY = %d, ratio = %f", angleX, angleY, ratio);
	angleX = angleX % 360;
	angleY = angleY % 360;

	//转化为弧度角
	float radiansX = static_cast<float>(MATH_PI / 180.0f * angleX);
	float radiansY = static_cast<float>(MATH_PI / 180.0f * angleY);


	// Projection matrix
	//glm::mat4 Projection = glm::ortho(-ratio, ratio, -1.0f, 1.0f, 0.1f, 100.0f);
	glm::mat4 Projection = glm::frustum(-ratio, ratio, -1.0f, 1.0f, 1.0f, m_pModel->GetMaxViewDistance() * 4);
	//glm::mat4 Projection = glm::perspective(45.0f,ratio, 0.1f,100.f);

	// View matrix
	glm::mat4 View = glm::lookAt(
			glm::vec3(0, 0, m_pModel->GetMaxViewDistance() * 1.8f), // Camera is at (0,0,1), in World Space
			glm::vec3(0, 0, 0), // and looks at the origin
			glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
	);

	// Model matrix
	glm::mat4 Model = glm::mat4(1.0f);
	Model = glm::scale(Model, glm::vec3(m_ScaleX, m_ScaleY, 1.0f));
	Model = glm::rotate(Model, radiansX, glm::vec3(1.0f, 0.0f, 0.0f));
	Model = glm::rotate(Model, radiansY, glm::vec3(0.0f, 1.0f, 0.0f));
	Model = glm::translate(Model, -m_pModel->GetAdjustModelPosVec());
    m_ModelMatrix = Model;
	mvpMatrix = Projection * View * Model;

}

void GeometryShader2Sample::UpdateTransformMatrix(float rotateX, float rotateY, float scaleX, float scaleY)
{
	GLSampleBase::UpdateTransformMatrix(rotateX, rotateY, scaleX, scaleY);
	m_AngleX = static_cast<int>(rotateX);
	m_AngleY = static_cast<int>(rotateY);
	m_ScaleX = scaleX;
	m_ScaleY = scaleY;
}
