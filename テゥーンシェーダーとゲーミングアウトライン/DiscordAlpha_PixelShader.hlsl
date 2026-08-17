struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float4 Diffuse : COLOR0;
    float2 TexCoord : TEXCOORD0;
};

Texture2D g_Texture : register(t0);
SamplerState g_Sampler : register(s0);

float4 main(PS_INPUT input) : SV_TARGET
{
    return g_Texture.Sample(g_Sampler, input.TexCoord);
}