#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in float Height;

// uniform sampler2D texture1; // No longer needed

void main()
{
    // Define a natural green palette
    vec3 colorLow = vec3(0.1, 0.35, 0.1);    // Darker Forest Green
    vec3 colorHigh = vec3(0.5, 0.7, 0.2);    // Lighter Grassy Green
    
    // Calculate mix factor based on height
    // Terrain height is roughly -6 to 6
    float t = smoothstep(-4.0, 4.0, Height);
    
    // Simple gradient based on height
    vec3 finalColor = mix(colorLow, colorHigh, t);
    
    FragColor = vec4(finalColor, 1.0);
}