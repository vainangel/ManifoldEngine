#version 460 core
out float FragColor;
  
in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;

uniform vec3 samples[64];
uniform mat4 projection;

// tile noise texture over screen, based on screen dimensions divided by noise size
const vec2 noiseScale = vec2(800.0/2.0, 600.0/2.0); // screen = 800x600

void main()
{
	int kernelSize = 32;
	float radius = 0.5;
	float bias = 0.025;

	//vec3 normal    = normalize(texture(gNormal, TexCoords).rgb) * 2.0 - 1.0;
	//vec3 normal = vec3(1.0f, 1.0f, 1.0f); // TODO: Read up on tangent space, normals https://learnopengl.com/Advanced-Lighting/Normal-Mapping
	//vec3 normal = normalize(texture(gNormal, TexCoords).rrr + texture(gNormal, TexCoords).ggg + texture(gNormal, TexCoords).bbb);
	//vec3 randomVec = texture(texNoise, TexCoords * noiseScale).xyz;  
	//vec3 randomVec = normalize(texture(texNoise, TexCoords * noiseScale).xyz);

	vec3 fragPos   = texture(gPosition, TexCoords).xyz;
	vec3 normal    = texture(gNormal, TexCoords).rgb;
	vec3 randomVec = texture(texNoise, TexCoords * noiseScale).xyz; 

	vec4 screenPos = vec4(fragPos, 1.0);
	screenPos = projection * screenPos; // from view to clip-space
	screenPos.xyz /= screenPos.w; // perspective divide
	screenPos.xyz = screenPos.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0

	vec3 tangent   = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN       = mat3(tangent, bitangent, normal);

	float occlusion = 0.5;
	for(int i = 0; i < kernelSize; ++i)
	{
		// get sample position
		vec3 samplePos = TBN * samples[i]; // from tangent to view-space
		samplePos = fragPos + samplePos * radius; 
		
		vec4 offset = vec4(samplePos, 1.0);
		offset      = projection * offset;    // from view to clip-space
		offset.xyz /= offset.w;               // perspective divide
		offset.xyz  = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0  

		float sampleDepth = texture(gPosition, offset.xy).z; 
		//occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0);  

		float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
		occlusion       += 2 * (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck; 
	}  

	occlusion = 1.0 - (occlusion / kernelSize);
	//FragColor = occlusion;

	if (screenPos.z < 1.0) // not on background
	{
	FragColor = pow(occlusion, 1);
	//FragColor = occlusion;
	}
	else
	{
	FragColor = 1.0;
	}
}