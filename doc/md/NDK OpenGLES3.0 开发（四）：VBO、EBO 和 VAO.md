# NDK OpenGLES3.0 开发（四）：VBO、EBO 和 VAO
> 该原创文章首发于微信公众号：字节流动
# VBO 和 EBO
**VBO（Vertex Buffer Object）是指顶点缓冲区对象，而 EBO（Element Buffer Object）是指图元索引缓冲区对象，VAO 和 EBO 实际上是对同一类 Buffer 按照用途的不同称呼**。

OpenGLES2.0 编程中，用于绘制的顶点数组数据首先保存在 CPU 内存，在调用 glDrawArrays 或者 glDrawElements 等进行绘制时，需要将顶点数组数据从 CPU 内存拷贝到显存。

但是很多时候我们没必要每次绘制的时候都去进行内存拷贝，如果可以在显存中缓存这些数据，就可以在很大程度上降低内存拷贝带来的开销。

OpenGLES3.0 VBO 和 EBO 的出现就是为了解决这个问题。 **VBO 和 EBO 的作用是在显存中提前开辟好一块内存，用于缓存顶点数据或者图元索引数据，从而避免每次绘制时的 CPU 与 GPU 之间的内存拷贝，可以改进渲染性能，降低内存带宽和功耗**。

**OpenGLES3.0 支持两类缓冲区对象：顶点数组缓冲区对象、图元索引缓冲区对象。GL_ARRAY_BUFFER 标志指定的缓冲区对象用于保存顶点数组，GL_ELEMENT_ARRAY_BUFFER 标志指定的缓存区对象用于保存图元索引**。

VBO（EBO）的创建和更新。
```c
// 创建 2 个 VBO（EBO 实际上跟 VBO 一样，只是按照用途的另一种称呼）
glGenBuffers(2, m_VboIds);

// 绑定第一个 VBO，拷贝顶点数组到显存
glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[0]);
glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

// 绑定第二个 VBO（EBO），拷贝图元索引数据到显存
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VboIds[1]);
glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
```
GL_STATIC_DRAW 标志标识缓冲区对象数据被修改一次，使用多次，用于绘制。


本例中顶点着色器和片段着色器增加 color 属性：
```c
//顶点着色器
#version 300 es                            
layout(location = 0) in vec4 a_position;   // 位置变量的属性位置值为 0 
layout(location = 1) in vec3 a_color;      // 颜色变量的属性位置值为 1
out vec3 v_color;                          // 向片段着色器输出一个颜色                          
void main()                                
{                                          
    v_color = a_color;                     
    gl_Position = a_position;              
};

//片段着色器
#version 300 es
precision mediump float;
in vec3 v_color;
out vec4 o_fragColor;
void main()
{
    o_fragColor = vec4(v_color, 1.0);
}
```
顶点数组数据和图元索引数据：
```c
// 4 vertices, with(x,y,z) ,(r, g, b, a) per-vertex
GLfloat vertices[] =
		{
				-0.5f,  0.5f, 0.0f,       // v0
				1.0f,  0.0f, 0.0f,        // c0
				-0.5f, -0.5f, 0.0f,       // v1
				0.0f,  1.0f, 0.0f,        // c1
				0.5f, -0.5f, 0.0f,        // v2
				0.0f,  0.0f, 1.0f,        // c2
				0.5f,  0.5f, 0.0f,        // v3
				0.5f,  1.0f, 1.0f,        // c3
		};
// Index buffer data
GLushort indices[6] = { 0, 1, 2, 0, 2, 3};
```
![VBO更新后内存中的数据结构](https://github.com/githubhaohao/NDK_OpenGLES_3_0/blob/master/doc/img/4/vertex_attribute_pointer_interleaved.png)

VBO更新后内存中的数据结构

由于顶点位置和颜色数据在同一个数组里，一起更新到 VBO 里面，所以需要知道 2 个属性的步长和偏移量。

为获得数据队列中下一个属性值（比如位置向量的下个 3 维分量）我们必须向右移动 6 个 float ，其中 3 个是位置值，另外 3 个是颜色值，那么步长就是 6 乘以 float 的字节数（= 24 字节）。

同样，也需要指定顶点位置属性和颜色属性在 VBO 内存中的偏移量。对于每个顶点来说，位置顶点属性在前，所以它的偏移量是 0 。而颜色属性紧随位置数据之后，所以偏移量就是 3 * sizeof(GLfloat) ，用字节来计算就是 12 字节。

使用 VBO 和 EBO 进行绘制。
```c
glUseProgram(m_ProgramObj);

//不使用 VBO 的绘制
glEnableVertexAttribArray(0);
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (3+3)*sizeof(GLfloat), vertices);

glEnableVertexAttribArray(1);
glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, (3+3)*sizeof(GLfloat), (vertices + 3));

glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

//使用 VBO 的绘制
glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[0]);
glEnableVertexAttribArray(0);
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (3+3)*sizeof(GLfloat), (const void *)0);
glEnableVertexAttribArray(1);
glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, (3+3)*sizeof(GLfloat), (const void *)(3 *sizeof(GLfloat)));

glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VboIds[1]);

glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (const void *)0);
```
# VAO
**VAO（Vertex Array Object）是指顶点数组对象，VAO 的主要作用是用于管理 VBO 或 EBO ，减少 glBindBuffer 、glEnableVertexAttribArray、 glVertexAttribPointer 这些调用操作，高效地实现在顶点数组配置之间切换**。

![VAO 与 VBO 之间的关系](https://github.com/githubhaohao/NDK_OpenGLES_3_0/blob/master/doc/img/4/vertex_array_objects.png)

VAO 与 VBO 之间的关系

基于上小节的例子创建 VAO ：
```c
// 创建并绑定 VAO
glGenVertexArrays(1, &m_VaoId);
glBindVertexArray(m_VaoId);

// 在绑定 VAO 之后，操作 VBO ，当前 VAO 会记录 VBO 的操作
glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[0]);
glEnableVertexAttribArray(0);
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (3+3)*sizeof(GLfloat), (const void *)0);
glEnableVertexAttribArray(1);
glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, (3+3)*sizeof(GLfloat), (const void *)(3 *sizeof(GLfloat)));

glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VboIds[1]);

glBindVertexArray(GL_NONE);
```

使用 VAO 进行绘制：
```c
// 是不是精简了很多？
glUseProgram(m_ProgramObj);

glBindVertexArray(m_VaoId);

glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (const void *)0);
```
# 联系与交流 #

微信公众号
![我的公众号](https://github.com/githubhaohao/NDK_OpenGLES_3_0/blob/master/doc/img/accountID.jpg#pic_center)
个人微信
![个人微信](https://github.com/githubhaohao/NDK_OpenGLES_3_0/blob/master/doc/img/WeChatID.jpg#pic_center)
