#version 460 core

layout (location = 0) in vec3 aPos; // Position input from C++ mesh

out vec2 TexCoords; // Pass calculated UVs down to fragment shader

void main()
{
    // Pass raw NDC positions directly out to cover the whole window
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0); 

    // Convert [-1.0, 1.0] positions directly into [0.0, 1.0] texture space
    TexCoords = (aPos.xy + vec2(1.0)) * 0.5;
}