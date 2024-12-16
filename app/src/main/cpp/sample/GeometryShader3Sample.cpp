/**
 *
 * Created by 公众号：字节流动 on 2024/12/18.
 * https://github.com/githubhaohao/NDK_OpenGLES_3_0
 * 最新文章首发于公众号：字节流动，有疑问或者技术交流可以添加微信 Byte-Flow ,领取视频教程, 拉你进技术交流群
 *
 * */

#include <gtc/matrix_transform.hpp>
#include "GeometryShader3Sample.h"
#include "../util/GLUtils.h"

GeometryShader3Sample::GeometryShader3Sample()
{
	m_AngleX = 0;
	m_AngleY = 0;

	m_ScaleX = 1.0f;
	m_ScaleY = 1.0f;

	m_pModel = nullptr;
	m_pShader = nullptr;
    m_pShader2 = nullptr;

    m_FrameIndex = 0;
}

GeometryShader3Sample::~GeometryShader3Sample()
{

}

void GeometryShader3Sample::Init()
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
            "out vec2 v_texCoord;\n"
            "out vec3 ambient;\n"
            "out vec3 diffuse;\n"
            "out vec3 specular;\n"
            "\n"
            "void main()\n"
            "{\n"
            "    v_texCoord = a_texCoord;\n"
            "    vec4 position = vec4(a_position, 1.0);\n"
            "    gl_Position = u_MVPMatrix * position;\n"
            "    vec3 fragPos = vec3(u_ModelMatrix * position);\n"
            "\n"
            "    // Ambient\n"
            "    float ambientStrength = 0.25;\n"
            "    ambient = ambientStrength * lightColor;\n"
            "\n"
            "    // Diffuse\n"
            "    float diffuseStrength = 0.5;\n"
            "    vec3 unitNormal = normalize(vec3(u_ModelMatrix * vec4(a_normal, 1.0)));\n"
            "    vec3 lightDir = normalize(lightPos - fragPos);\n"
            "    float diff = max(dot(unitNormal, lightDir), 0.0);\n"
            "    diffuse = diffuseStrength * diff * lightColor;\n"
            "\n"
            "    // Specular\n"
            "    float specularStrength = 0.3;\n"
            "    vec3 viewDir = normalize(viewPos - fragPos);\n"
            "    vec3 reflectDir = reflect(-lightDir, unitNormal);\n"
            "    float spec = pow(max(dot(unitNormal, reflectDir), 0.0), 16.0);\n"
            "    specular = specularStrength * spec * lightColor;\n"
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

    char vShaderStr2[] = "#version 320 es\n"
                         "precision mediump float;\n"
                         "layout (location = 0) in vec3 a_position;\n"
                         "layout (location = 1) in vec3 a_normal;\n"
                         "layout (location = 2) in vec2 a_texCoord;\n"
                         "uniform mat4 u_ViewMatrix;\n"
                         "uniform mat4 u_ModelMatrix;\n"
                         "\n"
                         "out VS_OUT {\n"
                         "    vec3 normal;\n"
                         "} vs_out;\n"
                         "\n"
                         "void main()\n"
                         "{\n"
                         "    vec4 position = vec4(a_position, 1.0);\n"
                         "    gl_Position = u_ViewMatrix * u_ModelMatrix * position;\n"
                         "    mat3 normalMatrix = mat3(transpose(inverse(u_ViewMatrix * u_ModelMatrix)));\n"
                         "    vs_out.normal = normalize(vec3(vec4(normalMatrix * a_normal, 0.0)));\n"
                         "}";

    char gShaderStr2[] = "#version 320 es\n"
                         "layout (triangles) in;\n"
                         "layout (line_strip, max_vertices = 6) out;\n"
                         "\n"
                         "in VS_OUT {\n"
                         "    vec3 normal;\n"
                         "} gs_in[];\n"
                         "\n"
                         "uniform mat4 u_ProjectionMatrix;\n"
                         "\n"
                         "void GenerateLine(int index)\n"
                         "{\n"
                         "    gl_Position = u_ProjectionMatrix * gl_in[index].gl_Position;\n"
                         "    EmitVertex();\n"
                         "    gl_Position = u_ProjectionMatrix * (gl_in[index].gl_Position +\n"
                         "    vec4(gs_in[index].normal, 0.0) * 0.3);\n"
                         "    EmitVertex();\n"
                         "    EndPrimitive();\n"
                         "}\n"
                         "\n"
                         "void main()\n"
                         "{\n"
                         "    GenerateLine(0); // 第一个顶点法线\n"
                         "    GenerateLine(1); // 第二个顶点法线\n"
                         "    GenerateLine(2); // 第三个顶点法线\n"
                         "}";
    char fShaderStr2[] = "#version 320 es\n"
                         "precision highp float;\n"
                         "out vec4 outColor;\n"
                         "void main()\n"
                         "{\n"
                         "    outColor = vec4(1.0, 0.1, 0.0, 1.0);\n"
                         "}";

    //TODO 先把 model 文件夹拷贝到 /sdcard/Android/data/com.byteflow.app/files/Download 路径下，然后可以选择你要加载的模型
	//m_pModel = new Model(path + "/model/nanosuit/nanosuit.obj");
	std::string path(DEFAULT_OGL_ASSETS_DIR);
    m_pModel = new Model(path + "/model/poly/Apricot_02_hi_poly.obj");
    //m_pModel = new Model(path + "/model/tank/Abrams_BF3.obj");
    //m_pModel = new Model(path + "/model/girl/091_W_Aya_10K.obj");//2000 1500
    //m_pModel = new Model(path + "/model/new/camaro.obj");
    //m_pModel = new Model(path + "/model/bird/12214_Bird_v1max_l3.obj");

    m_pShader = new Shader(vShaderStr, fShaderStr);
    m_pShader2 = new Shader(vShaderStr2, fShaderStr2, gShaderStr2);
}

void GeometryShader3Sample::LoadImage(NativeImage *pImage)
{
	LOGCATE("GeometryShader3Sample::LoadImage pImage = %p", pImage->ppPlane[0]);

}

void GeometryShader3Sample::Draw(int screenW, int screenH)
{
	if(m_pModel == nullptr || m_pShader == nullptr) return;
    LOGCATE("GeometryShader3Sample::Draw()");
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

    m_pShader2->use();
    m_pShader2->setMat4("u_ViewMatrix", m_ViewMatrix);
    m_pShader2->setMat4("u_ModelMatrix", m_ModelMatrix);
    m_pShader2->setMat4("u_ProjectionMatrix", m_ProjectionMatrix);
    m_pModel->Draw((*m_pShader2));

    m_FrameIndex ++;
}

void GeometryShader3Sample::Destroy()
{
    LOGCATE("GeometryShader3Sample::Destroy");
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

    if (m_pShader2 != nullptr) {
        m_pShader2->Destroy();
        delete m_pShader2;
        m_pShader2 = nullptr;
    }
}

void GeometryShader3Sample::UpdateMVPMatrix(glm::mat4 &mvpMatrix, int angleX, int angleY, float ratio)
{
	LOGCATE("GeometryShader3Sample::UpdateMVPMatrix angleX = %d, angleY = %d, ratio = %f", angleX, angleY, ratio);
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
    m_ViewMatrix = View;
    m_ProjectionMatrix = Projection;
	mvpMatrix = Projection * View * Model;

}

void GeometryShader3Sample::UpdateTransformMatrix(float rotateX, float rotateY, float scaleX, float scaleY)
{
	GLSampleBase::UpdateTransformMatrix(rotateX, rotateY, scaleX, scaleY);
	m_AngleX = static_cast<int>(rotateX);
	m_AngleY = static_cast<int>(rotateY);
	m_ScaleX = scaleX;
	m_ScaleY = scaleY;
}
