#version 450 core
out vec4 FragColor;

in vec2 TexCoord; 

void main() {
	//FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
	FragColor = vec4(0.5f, 1.0f, 1.0f, 1.0f);
	//FragColor = vec4(0.0, 0.0, 0.0, 1.0);
}
