#version 450

in vec2 UV_VS;

out vec4 finalCol;

uniform sampler2D PosTex;

uniform float radSample;

uniform mat4 PROJ;

const int MAX_KERNEL_SIZE = 128;
uniform vec3 kernel[MAX_KERNEL_SIZE];

void main()
{
    vec3 Pos = texture(PosTex, TexCoord).xyz;

    float AO = 0.0;

    for (int i = 0 ; i < MAX_KERNEL_SIZE ; i++)
	{
        vec3 samplePos = Pos + gKernel[i];
        vec4 offset = vec4(samplePos, 1.0);
        offset = PROJ * offset;
        offset.xy /= offset.w;
        offset.xy = offset.xy * 0.5 + vec2(0.5);

        float sampleDepth = texture(PosTex, offset.xy).b;

        if (abs(Pos.z - sampleDepth) < gSampleRad) {
            AO += step(sampleDepth,samplePos.z);
        }
    }

    AO = 1.0 - AO/128.0;

    finalCol = vec4(pow(AO, 2.0));
}