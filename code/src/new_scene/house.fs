#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoord;
} fs_in;

uniform sampler2D texture_diffuse1;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;

uniform float ambientStrength;   // e.g. 0.15
uniform float specularStrength;  // e.g. 0.6
uniform float shininess;         // e.g. 64.0 (Blinn-Phong)

void main()
{
    vec3 albedo = texture(texture_diffuse1, fs_in.TexCoord).rgb;

    vec3 N = normalize(fs_in.Normal);
    vec3 L = normalize(lightPos - fs_in.FragPos);

    // Ambient
    vec3 ambient = ambientStrength * albedo * lightColor;

    // Diffuse
    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = diff * albedo * lightColor;

    // Specular (Blinn-Phong)
    vec3 V = normalize(viewPos - fs_in.FragPos);
    vec3 H = normalize(L + V);
    float spec = pow(max(dot(N, H), 0.0), shininess);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 color = ambient + diffuse + specular;
    FragColor = vec4(color, 1.0);
}
