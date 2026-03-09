#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 5) in ivec4 aBoneIDs;
layout (location = 6) in vec4 aWeights;

out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

const int MAX_BONES = 100;
uniform mat4 finalBonesMatrices[MAX_BONES];

void main()
{
    mat4 skin = mat4(0.0);
    float totalWeight = 0.0;

    for (int i = 0; i < 4; ++i)
    {
        int id = aBoneIDs[i];
        float w = aWeights[i];
        if (id >= 0 && id < MAX_BONES && w > 0.0)
        {
            skin += finalBonesMatrices[id] * w;
            totalWeight += w;
        }
    }

    if (totalWeight < 0.0001)
        skin = mat4(1.0);

    vec4 skinnedPos = skin * vec4(aPos, 1.0);
    gl_Position = projection * view * model * skinnedPos;

    TexCoord = aTexCoord;
}
