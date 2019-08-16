#version 300 es
precision mediump float;
in vec2 v_texCoord;
in vec3 ambient;
in vec3 diffuse;
in vec3 specular;
layout(location = 0) out vec4 outColor;
uniform sampler2D s_TextureMap;
void main()
{
    vec4 objectColor = texture(s_TextureMap, v_texCoord);
    vec3 finalColor = (ambient + diffuse + specular) * vec3(objectColor);
    outColor = vec4(finalColor, 1.0);
}


#version 300 es
precision mediump float;
layout(location = 0) in vec4 a_position;
layout(location = 1) in vec2 a_texCoord;
layout(location = 2) in vec3 a_normal;
uniform mat4 u_MVPMatrix;
uniform mat4 u_ModelMatrix;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;
out vec2 v_texCoord;
out vec3 ambient;
out vec3 diffuse;
out vec3 specular;
void main()
{
    gl_Position = u_MVPMatrix * a_position;
    vec3 fragPos = vec3(u_ModelMatrix * a_position);

    // Ambient
    float ambientStrength = 0.3;
    ambient = ambientStrength * lightColor;

    // Diffuse
    vec3 unitNormal = normalize(u_ModelMatrix * vec4(a_normal, 1.0)).xyz;
    vec3 lightDir = normalize(lightPos - fragPos);
    float diff = max(dot(unitNormal, lightDir), 0.0);
    diffuse = diff * lightColor;

    // Specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - fragPos);
    //vec3 helfVector  = normalize(viewDir + lightDir);
    vec3 reflectDir = reflect(-lightDir, unitNormal);
    float spec = pow(max(dot(unitNormal, reflectDir), 0.0), 32.0);
    specular = specularStrength * spec * lightColor;

    v_texCoord = a_texCoord;
}