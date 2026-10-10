#version 450 core
out vec4 FragColor;

//in vec3 world_pos;
//in vec3 world_normal;
//in vec4 viewSpace;
in vec2 TexCoord;

flat in ivec4 color;
flat in int c;

uniform vec3 eye_position;

void main() {
	/*float dist = 0;
	float fogFactor = 0;

	vec3 fogColor = vec3(0.0, 0.0, 0.0);

	dist = length(viewSpace);

	fogFactor = 1.0 / exp((dist * 0.002) * (dist * 0.002));
	fogFactor = clamp(fogFactor, 0.0, 1.0);*/

	//FragColor = vec4(0.996078431372549 * TexCoord.x, 0.396078431372549, 0.9921568627450981 * TexCoord.y, 1.0f);
	vec4 colorOut = vec4(1.0f, 0.0f, 0.0f, 1.0f);
	int d = c;
	if (d == 1) colorOut = vec4(0.95f, 0.95f, 0.95f, 1.0f);
	else if (d == 2) colorOut = vec4(0.95f, 0.95f, 0.95f, 1.0f);
	else if (d == 3) colorOut = vec4(0.90f, 0.90f, 0.90f, 1.0f);
	else if (d == 4) colorOut = vec4(0.90f, 0.90f, 0.90f, 1.0f);
	else if (d == 5) colorOut = vec4(1.0f);
	else if (d == 6) colorOut = vec4(0.85f, 0.85f, 0.85f, 1.0f);

	//colorOut = vec4(c << 8);

	FragColor = colorOut - vec4(vec3(0.05), 0); 

	//FragColor = vec4(TexCoord, 0.0f, 0.0f);
	//FragColor = vec4(mix(fogColor, vec3(TexCoord, 0.0f), fogFactor), 1);
}
