static const float pi = 3.141593;

RWTexture2D<float4> outputTex : register(u0);
Texture2D inputTex : register(t0);


SamplerState gpointClampSampler : register(s0);

static const float weights[11] =
{
    0.05f, 0.05f, 0.1f, 0.1f, 0.1f, 0.2f, 0.1f, 0.1f, 0.1f, 0.05f, 0.05f,
};


static const int blurRadius = 5;

[numthreads(32, 32, 1)] //x,y의 모든 쓰레드 갯수가 화면 해상도를 기반으로 넘겼기에 SV_DispatchTheadID를 픽셀 좌표로 봐도 된다.
void main(uint3 gID : SV_GroupID, uint3 dtID : SV_DispatchThreadID)
{
    uint width, height;
    outputTex.GetDimensions(width, height);

    if (dtID.x >= width || dtID.y >= height)
        return;


    float dx = 1.0 / width;
    float dy = 1.0 / height;
    // 또는 float2 dx = float2(1.0 / 1280, 1.0 / 768)

    float2 uv = float2((dtID.x + 0.5) * dx, (dtID.y + 0.5) * dy);
    // float2 uv = float2((dtID.x + 0.5) * dx.x, (dtID.y + 0.5) * dx.y);

    float3 blurColor = float3(0, 0, 0);

    [unroll]
        for (int i = -blurRadius; i <= blurRadius; ++i)
        {
          
            float3 color = inputTex.SampleLevel(gpointClampSampler, uv + float2(float(i) * dx, 0.0), 0).rgb;

            blurColor += weights[i + blurRadius] * color;
        }

    outputTex[dtID.xy] = float4(blurColor, 1);
}