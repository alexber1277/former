#version 460 core

in vec2 uv;
out vec4 FragColor;

uniform sampler2D screenTex;
uniform sampler2D hbaoTex;
uniform vec2 uResolution;

#define FXAA_REDUCE_MIN   1.0/128.0
#define FXAA_REDUCE_MUL   1.0/8.0
#define FXAA_SPAN_MAX     8.0

void main()
{
    vec2 uv = gl_FragCoord.xy / uResolution;
    vec2 texel = 1.0 / uResolution;

    // ----------------------------
    // 2D Gaussian Blur 3x3
    // ----------------------------
    float kernel[9] = float[](
        1, 2, 1,
        2, 4, 2,
        1, 2, 1
    );

    float sum = 0.0;
    float weightSum = 0.0;
    int k = 0;

    for(int y = -1; y <= 1; y++)
    {
        for(int x = -1; x <= 1; x++)
        {
            float s = texture(hbaoTex, uv + texel * vec2(x,y)).r;
            sum += s * kernel[k];
            weightSum += kernel[k];
            k++;
        }
    }

    float ao = sum / weightSum;

    // ----------------------------
    // Threshold
    // ----------------------------
    float threshold = 0.55;
    if(ao < threshold)
        ao = 0.0;

    // ----------------------------
    // Remove isolated pixels
    // ----------------------------
    if(ao > 0.0)
    {
        int neighbors = 0;

        for(int y = -1; y <= 1; y++)
        {
            for(int x = -1; x <= 1; x++)
            {
                if(x == 0 && y == 0) continue;

                float n = texture(hbaoTex, uv + texel * vec2(x,y)).r;
                if(n >= threshold)
                    neighbors++;
            }
        }

        if(neighbors == 0)
            ao = 0.0;
    }

    // ----------------------------
    // FXAA (unchanged)
    // ----------------------------
    vec3 rgbNW = texture(screenTex, uv + vec2(-1.0, -1.0)/uResolution).rgb;
    vec3 rgbNE = texture(screenTex, uv + vec2( 1.0, -1.0)/uResolution).rgb;
    vec3 rgbSW = texture(screenTex, uv + vec2(-1.0,  1.0)/uResolution).rgb;
    vec3 rgbSE = texture(screenTex, uv + vec2( 1.0,  1.0)/uResolution).rgb;
    vec3 rgbM  = texture(screenTex, uv).rgb;

    vec3 luma = vec3(0.299, 0.587, 0.114);

    float lumaNW = dot(rgbNW, luma);
    float lumaNE = dot(rgbNE, luma);
    float lumaSW = dot(rgbSW, luma);
    float lumaSE = dot(rgbSE, luma);
    float lumaM  = dot(rgbM,  luma);

    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

    vec2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));

    float dirReduce = max(
        (lumaNW + lumaNE + lumaSW + lumaSE) *
        (0.25 * FXAA_REDUCE_MUL),
        FXAA_REDUCE_MIN);

    float rcpDirMin = 1.0 /
        (min(abs(dir.x), abs(dir.y)) + dirReduce);

    dir = clamp(dir * rcpDirMin,
                vec2(-FXAA_SPAN_MAX),
                vec2( FXAA_SPAN_MAX)) / uResolution;

    vec3 rgbA = 0.5 * (
        texture(screenTex, uv + dir * (1.0/3.0 - 0.5)).rgb +
        texture(screenTex, uv + dir * (2.0/3.0 - 0.5)).rgb
    );

    vec3 rgbB = rgbA * 0.5 + 0.25 * (
        texture(screenTex, uv + dir * -0.5).rgb +
        texture(screenTex, uv + dir *  0.5).rgb
    );

    float lumaB = dot(rgbB, luma);
    vec3 finalColor =
        (lumaB < lumaMin || lumaB > lumaMax) ? rgbA : rgbB;

    // ----------------------------
    // Apply AO darkening
    // ----------------------------
    float aoStrength = 1.0;
    finalColor *= (1.0 - ao * aoStrength);

    FragColor = vec4(finalColor, 1.0);
}