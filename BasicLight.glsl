#version 300 es
precision mediump float;
in vec2 v_texCoord;
in vec3 normal;
in vec3 fragPos;
layout(location = 0) out vec4 outColor;
uniform sampler2D s_TextureMap;
uniform vec3 lightPos;
uniform vec3 lightColor;
void main()
{
    // Ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse
    vec3 unitNormal = normalize(normal);
    vec3 lightDir = normalize(lightPos - fragPos);
    float diff = max(dot(unitNormal, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    vec4 objectColor = texture(s_TextureMap, v_texCoord);

    vec3 finalColor = (ambient + diffuse) * vec3(objectColor);

    outColor = vec4(finalColor, 1.0);
}

#version 300 es
precision mediump float;
layout(location = 0) in vec4 a_position;
layout(location = 1) in vec2 a_texCoord;
layout(location = 2) in vec3 a_normal;
uniform mat4 u_MVPMatrix;
uniform mat4 u_ModelMatrix;
out vec2 v_texCoord;
out vec3 normal;
out vec3 fragPos;
void main()
{
    gl_Position = u_MVPMatrix * a_position;
    fragPos = vec3(u_ModelMatrix * a_position);
    v_texCoord = a_texCoord;
    normal = a_normal;
}