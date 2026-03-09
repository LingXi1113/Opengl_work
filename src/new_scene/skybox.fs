#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform float sunsetFactor; // 0.0 = Day, 1.0 = Sunset

void main()
{
    // Day colors (Blue Sky)
    vec3 dayTop = vec3(0.0f, 0.5f, 1.0f);
    vec3 dayBottom = vec3(0.8f, 0.9f, 1.0f);

    // Sunset colors (Purple/Orange)
    vec3 sunsetTop = vec3(0.2f, 0.1f, 0.4f);
    vec3 sunsetBottom = vec3(1.0f, 0.4f, 0.1f);

    // Interpolate based on sunsetFactor
    vec3 topColor = mix(dayTop, sunsetTop, sunsetFactor);
    vec3 bottomColor = mix(dayBottom, sunsetBottom, sunsetFactor);
    
    vec3 dir = normalize(TexCoords);
    float t = clamp(dir.y * 0.5 + 0.5, 0.0, 1.0);
    
    // Mix based on height
    vec3 color = mix(bottomColor, topColor, t);
    
    FragColor = vec4(color, 1.0);
}