#version 450 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

//out vec4 FragColor;

//layout (binding=0) uniform sampler2D baseTexture;
layout (binding=1) uniform sampler2DArray arrayTexture;

in flat uvec3 normal;
in vec2 texCoord; 
in vec3 thisVertexCoord;
in flat int texID;
in float v_fogDepth;
in float v_flatDepth;
in vec3 FragPos;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

const uvec3 n_front =	uvec3(0, 0, 1);
const uvec3 n_back =	uvec3(0, 1, 0);
const uvec3 n_left =	uvec3(1, 0, 1);
const uvec3 n_right =	uvec3(1, 0, 0);
const uvec3 n_top =		uvec3(1, 1, 1);
const uvec3 n_bottom =	uvec3(0, 0, 0);

float linearizeDepth(float depth, float near, float far) {
    float z = depth * 2.0 - 1.0; 
    return (2.0 * near) / (far + near - z * (far - near));
}
vec2 rotateUV(vec2 uv, float angle) {
    // Shift UV to center (0.5, 0.5)
    vec2 center = vec2(0.5);
    vec2 shifted = uv - center;
    
    // Create rotation matrix
    float s = sin(angle);
    float c = cos(angle);
    mat2 rot = mat2(c, -s, s, c);
    
    // Rotate and shift back
    return (rot * shifted) + center;
}

vec3 adjustSaturation(vec3 rgb, float adjustment) {
    // Standard digital luminance weights (Rec. 709)
    const vec3 W = vec3(0.2125, 0.7154, 0.0721);
    
    // Calculate the perceived grayscale intensity
    vec3 intensity = vec3(dot(rgb, W));
    
    // Mix between grayscale and original color based on adjustment
    return mix(intensity, rgb, adjustment);
}

void main() {
	vec4 colorOut = vec4(1.0f, 0.0f, 0.0f, 1.0f);
	uvec3 normal = uvec3(normal.x >> 7, normal.y >> 7, normal.z >> 7);

	float shading_top = 1.00f;
	float shading_front_back = 0.90f;
	float shading_left_right = 0.80f;
	float shading_bottom = 0.7f;

	if (normal == n_front || normal == n_back) colorOut = vec4(vec3(shading_front_back), 1.0f);
	else if (normal == n_left || normal == n_right) colorOut = vec4(vec3(shading_left_right), 1.0f);
	else if (normal == n_top) colorOut = vec4(vec3(shading_top), 1.0f);
	else if (normal == n_bottom) colorOut = vec4(vec3(shading_bottom), 1.0f);

	vec3 quadNormal = vec3(1.0f);
	vec3 quadTangent = vec3(0.0f);
	
	/*if (normal == n_front || normal == n_back) {
		quadNormal = vec3(0.0f, 0.0f, 1.0f);
		quadTangent = vec3(1.0f, 1.0f, 0.0f);
	}

	if (normal == n_left || normal == n_right) {
		quadNormal = vec3(1.0f, 0.0f, 0.0f);
		quadTangent = vec3(0.0f, 1.0f, 1.0f);
	}

	if (normal == n_top) {
		quadNormal = vec3(0.0f, 1.0f, 0.0f);
		quadTangent = vec3(1.0f, 0.0f, 1.0f);
	}

	if (normal == n_bottom) {
		quadNormal = vec3(0.0f, -1.0f, 0.0f);
		quadTangent = vec3(1.0f, 0.0f, 1.0f);
	}*/

	if (normal == n_front) quadNormal = vec3(0.0f, 0.0f, 1.0f);
	if (normal == n_back) quadNormal = vec3(0.0f, 0.0f, -1.0f);

	if (normal == n_left) quadNormal = vec3(-1.0f, 0.0f, 0.0f);
	if (normal == n_right) quadNormal = vec3(1.0f, 0.0f, 0.0f);
	
	if (normal == n_top) quadNormal = vec3(0.0f, 1.0f, 0.0f);
	if (normal == n_bottom) quadNormal = vec3(0.0f, -1.0f, 0.0f);

	//colorOut = vec4(normal, 1.0f);

	//colorOut.rg = vec2(round(thisVertexCoord.y-0.49)/32);
	vec4 unshadedColor = vec4(1.0);
	unshadedColor.rgb = vec3(floor(thisVertexCoord.y-0.01)/64);
	if (thisVertexCoord.y > 56.0) unshadedColor.rgb = vec3(floor(56.0-0.01)/64);
	//if (floor(thisVertexCoord.y-0.01) >= 31) { unshadedColor.gb = vec2(0.3); }
	//if (floor(thisVertexCoord.y-0.01) >= 55) { unshadedColor.gb = vec2(0.5); }
	//if (floor(thisVertexCoord.y-0.01) >= 63) { unshadedColor.gb = vec2(1); }

	float t_f = texID;
	if (t_f == 3) {
		if (normal == n_front || normal == n_back || normal == n_left || normal == n_right) {
			t_f = 2;
		}
		else if (normal == n_bottom) {
			t_f = 1;
		}
	}
	//FragColor = vec4(0.7f, t_f / 4, 0.5f, 1.0f) - (vec4(1, 1, 1, 1) - colorOut);
	
	//if (texID == 0) FragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
	//else FragColor = vec4(vec3(t_f / 4), 1.0f) -  (vec4(1, 1, 1, 1) - colorOut);

	vec2 finalTex = texCoord;
	//finalTex.x = 1.0f - finalTex.x;
	//if (normal == n_front || normal == n_left || normal == n_bottom) finalTex.x = 1.0f - finalTex.x;
	if (normal == n_left || normal == n_right) {
		finalTex = rotateUV(finalTex, radians(-90));
	}
	if (normal == n_left || normal == n_back || normal == n_top) {
		finalTex.x = 1.0 - finalTex.x;
	}

	float shadingOffset = 0.0f;
	if (normal == n_left || normal == n_right) {
		shadingOffset = 0.2f;
	}

	if (normal == n_front || normal == n_back) {
		shadingOffset = 0.1f;
	}

	if (normal == n_bottom) {
		shadingOffset = 0.3f;
	}

	// temporary, will become proper uniforms 
	vec4 u_fogColor = vec4(0.5f, 1.0f, 1.0f, 1.0f);
	float u_fogNear = 128.0f;
	float u_fogFar = 512.0f;

	float shadingFogNear = 0.25f;
	float shadingFogFar = 40.0f;
	float shadingFogFactor = smoothstep(shadingFogNear, shadingFogFar, ceil(v_flatDepth));
	//colorOut -= min(0.3, shadingFogFactor) + shadingOffset;
	colorOut -= shadingOffset;

	//colorOut -= ceil(v_flatDepth) / 64;
	//if (normal != n_top) colorOut -= min(0.3, shadingFogFactor); (depth fog disabled for now) 
	//else colorOut -= min(0.2, shadingFogFactor);

	// Elevation-based cell shading 
	//float thisClampedYCoord = ceil(thisVertexCoord.y);
	//if (normal != n_top) colorOut = colorOut * (thisClampedYCoord / 64.0);


	// Sample the base texture color
    vec4 texColor = texture(arrayTexture, vec3(finalTex, t_f));
    
    // Calculate fog factor (0.0 = no fog, 1.0 = full fog)
    float fogFactor = smoothstep(u_fogNear, u_fogFar, v_fogDepth);
    
    // Blend original color with fog color
    colorOut = mix(colorOut * texColor, u_fogColor, fogFactor);

	// lighting stuff 

	vec3 norm = normalize(normal); // redundant but whatever 
	vec3 lightDir = normalize(vec3(64.0f, 64.0f, 64.0f) - FragPos);
	float diff = max((dot(norm, lightDir)), 0.0);
	vec3 diffuse = diff * vec3(1.0f, 0.0f, 1.0f);
	vec3 result = (1 + diffuse) * colorOut.xyz;

	//colorOut.xyz = ceil(result * 10) / 10;

	//FragColor = colorOut;
	
	/* normal calculation 
		vec3 Normal        = normalize(Normal0);  // The geometry's physical normal, in world space
		vec3 Tangent       = normalize(Tangent0); // The geometry's physical tangent, in world space
		Tangent            = (Tangent - dot(Tangent, Normal) * Normal);

		vec3 Bitangent     = cross(Tangent, Normal);
		vec3 BumpMapNormal = texture(NormalTexture, TexCoord0.xy).xyz; // The bumpmap texture for the skin of the object
		BumpMapNormal      = (BumpMapNormal * 2.0 - 1.0);    
		mat3 TBN           = mat3(Tangent, Bitangent, Normal);
		vec3 NormalFinal   = normalize((TBN * BumpMapNormal));
		NormalOut          = normalize(0.5 * NormalFinal + 0.5); // Final normal
	*/

	mat3 normalMatrix = transpose(inverse(mat3(view) * mat3(model)));
	//quadNormal = (view * model * vec4(quadNormal + thisVertexCoord, 1.0f)).xyz;
	//quadTangent = (quadTangent - dot(quadTangent, quadNormal) * quadNormal);

	//vec3 bitTangent = cross(quadTangent, quadNormal);
	//mat3 TBN = mat3(quadTangent, bitTangent, quadNormal);
	//vec3 normalFinal = normalize(TBN * vec3(1.0f));
	//gNormal = normalize(0.5 * normalFinal + 0.5);

	gPosition = FragPos;
	gNormal = normalize(normalMatrix * quadNormal);
	gAlbedoSpec = colorOut;
	
	//FragColor = colorOut * texture(arrayTexture, vec3(finalTex, 0));

	//FragColor = (unshadedColor - (vec4(1, 1, 1, 1) - colorOut) - vec4(vec3(0.05), 0) + vec4(0.2, 0.2, 0.2, 1.0)) * texture(baseTexture, texCoord); 
	//FragColor = colorOut * texture(arrayTexture, texCoord);
	//FragColor = vec4(texCoord, 0.0, 1.0);


}
