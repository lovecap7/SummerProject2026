// ピクセルシェーダーの入力
struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float4 Diffuse : COLOR0;
    float2 TexCoord : TEXCOORD0;
};
// ピクセルシェーダーの出力
struct PS_OUTPUT
{
    float4 color : SV_TARGET0; // 色
};

Texture2D g_Texture : register(t4);
SamplerState g_Sampler : register(s0);

PS_OUTPUT main(PS_INPUT input)
{
    PS_OUTPUT output;
    output.color = float4(1.0f, 0.0f, 0.0f, 1.0f);
    return output;
    
    //float4 color = g_Texture.Sample(
    //    g_Sampler,
    //    input.TexCoord
    //);

    //// 明るさを計算
    //float brightness = dot(
    //    color.rgb,
    //    float3(0.2126f, 0.7152f, 0.0722f)
    //);

    //// Bloomさせる明るさの閾値
    //float threshold = 0.7f;

    //// 閾値より暗ければ消す
    //if (brightness < threshold)
    //{
    //    return float4(0.0f, 0.0f, 0.0f, 0.0f);
    //}

    return output;
}