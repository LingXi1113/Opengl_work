#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

void main()
{
    // Light blue gradient
    vec3 topColor = vec3(0.0f, 0.5f, 1.0f); // Deep Sky Blue
    vec3 bottomColor = vec3(0.8f, 0.9f, 1.0f); // Light Blue / White at horizon
    
    vec3 dir = normalize(TexCoords);
    float t = clamp(dir.y * 0.5 + 0.5, 0.0, 1.0);
    
    // Mix based on height
    vec3 color = mix(bottomColor, topColor, t);
    
    FragColor = vec4(color, 1.0);
}