#version 450 core
out vec4 FragColor;

void main() {
	float dist = 0;
	float fogFactor = 0;

	vec3 fogColor = vec3(0.0, 0.0, 0.0);

	FragColor = vec4(1.0);
	//FragColor = vec4(TexCoord, 0.0f, 0.0f);
	//FragColor = vec4(mix(fogColor, vec3(TexCoord, 0.0f), fogFactor), 1);
}
