#version 460 core

out vec4 FragColor; // Final output color of the screen pixel

in vec2 TexCoords; // Received interpolated UV coordinates from vertex stage

uniform sampler2D screenTexture; // The texture sampler slot assigned from C++
uniform vec2 u_resolution;    // Screen resolution (width, height)
uniform float u_time;         // Elapsed time for animation

float random(vec2 co) {
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{
    // Look up the pixel color on the image texture based on coordinates
    //FragColor = texture(screenTexture, TexCoords);
    vec2 uv = TexCoords;

    //float shift = random(vec2(floor(uv.y * 20.0), u_time));
    //uv.x += (shift - 0.5) * 0.01 * step(0.8, random(vec2(u_time, 2.0)));
    
    // 1. Periodic Trigger Logic
    // Cycle every 3.0 seconds; pulse active for ~0.35s at the start of each interval
    float cycle = mod(u_time, 3.0);
    float burstWindow = 0.35;
    
    // Spikes from 0.0 to 1.0 quickly and fades back down
    float burstProgress = cycle / burstWindow;
    float burstMask = step(cycle, burstWindow) * sin(burstProgress * 3.14159265);

    // 2. Base Noise & Slicing
    // Finer slice bands during the burst for a chaotic tearing look
    float bandCount = mix(25.0, 60.0, burstMask);
    float sliceId = floor(uv.y * bandCount);
    
    // Base ambient glitch factor
    float glitchChance = step(0.75, random(vec2(floor(u_time * 8.0), 1.0)));
    float glitchOffset = (random(vec2(sliceId, floor(u_time * 12.0))) - 0.5);

    // 3. Dynamic Displacements
    // Baseline shifts vs major periodic burst shifts
    float xShift = glitchOffset * mix(0.02 * glitchChance, 0.25, burstMask);
    float rgbSpread = mix(0.004, 0.035, burstMask);

    // Apply horizontal tear
    float shift = random(vec2(floor(uv.y * 20.0), u_time));
    if (uv.x > 0.02 && uv.x < 0.98 && uv.y < 0.98 && uv.y > 0.02) uv.x += xShift + ((shift - 0.5) * 0.01 * step(0.8, random(vec2(u_time, 2.0))));

    // Chromatic aberration offsets
    float r = texture2D(screenTexture, uv + vec2(0.005, 0.0)).r;
    float g = texture2D(screenTexture, uv).g;
    float b = texture2D(screenTexture, uv - vec2(0.005, 0.0)).b;

    FragColor = vec4(r, g, b, 1.0f);

    // Normalize coordinates (0.0 to 1.0)
    //vec2 uv = gl_FragCoord.xy / u_resolution.xy;

    // Create a wave offset using sine
    //float wave = sin(uv.y * 10.0 + u_time * 3.0) * 0.01;
    
    // Apply distortion to the X coordinate
    //vec2 distortedUV = vec2(uv.x + wave, uv.y);

    // Sample the texture at the new distorted coordinate
    //vec4 color = texture2D(screenTexture, distortedUV);

    //FragColor = color;
    //FragColor = texture(screenTexture, TexCoords);
}