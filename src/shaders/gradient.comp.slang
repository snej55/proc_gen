#language slang 2026

[[vk::binding(0, 0)]]
RWTexture2D<float4> image;

[shader("compute")]
[numthreads(16, 16, 1)]

void main(uint3 GL_GlobalInvocationID : SV_DispatchThreadID, uint3 GL_LocalInvocationID : SV_GroupThreadID)
{
    int2 texelCoord = int2(GL_GlobalInvocationID.xy);
    uint2 size;
    image.GetDimensions(size.x, size.y);

    if (texelCoord.x < size.x && texelCoord.y < size.y)
    {
        float4 color = float4(0.0, 0.0, 0.0, 1.0);
        if (GL_LocalInvocationID.x != 0 && GL_LocalInvocationID.y != 0)
        {
            color.x = float(texelCoord.x) / (size.x);
            color.y = float(texelCoord.y) / (size.y);
        }

        image[texelCoord] = color;
    }
}
