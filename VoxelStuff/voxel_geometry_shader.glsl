#version 460 core 
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

layout (location=2) in flat uvec3 normalIn[];
layout (location=3) in vec2 texCoordIn[]; 
layout (location=4) in vec3 thisVertexCoordIn[];
layout (location=5) in flat int texIDIn[];
layout (location=6) in float v_fogDepthIn[];
layout (location=7) flat in float v_flatDepthIn[];

layout (location=2) out flat uvec3 normal;
layout (location=3) out vec2 texCoord; 
layout (location=4) out vec3 thisVertexCoord;
layout (location=5) out flat int texID;
layout (location=6) out float v_fogDepth;
layout (location=7) flat out float v_flatDepth;

void main() {    
    normal = normalIn[0];
    texCoord = texCoordIn[0];
    thisVertexCoord = thisVertexCoordIn[0];
    texID = texIDIn[0];
    v_fogDepth = v_fogDepthIn[0];
    v_flatDepth = v_flatDepthIn[0];

     // Loop through all 3 vertices of the input triangle
    for (int i = 0; i < 3; i++) {
        // Copy the built-in position exactly
        gl_Position = gl_in[i].gl_Position;
        
        // Emit the vertex to the output stream
        EmitVertex();
    }
    
    // Finish the primitive
    EndPrimitive();
}  