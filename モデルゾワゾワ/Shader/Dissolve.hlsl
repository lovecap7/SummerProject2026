#include "PixelShader.h"

// ピクセルシェーダーの入力
struct PS_INPUT
{
    float4 Diffuse : COLOR0;
    float4 Specular : COLOR1;
    float4 TexCoords0_1 : TEXCOORD0;
    float3 VPosition : TEXCOORD1;
    float3 VNormal : TEXCOORD2;
    float2 Fog : TEXCOORD5;
};

// ピクセルシェーダーの出力
struct PS_OUTPUT
{
    float4 Color0 : SV_TARGET0;
    float4 Normal : SV_TARGET1;
    float4 Depth : SV_TARGET2;
};

SamplerState g_DiffuseMapSampler : register(s0);

// 通常のモデルテクスチャ
Texture2D g_DiffuseMapTexture : register(t0);


// ディゾルブ用テクスチャ
Texture2D g_DissolveTexture : register(t1);
// ディゾルブパラメータ
// x = ディゾルブ進行度 0=表示 1=消える
cbuffer DissolveBuffer : register(b5)
{
    float4 g_DissolveParam;
};


// main
PS_OUTPUT main(PS_INPUT PSInput)
{
    PS_OUTPUT PSOutput;

    float4 TextureDiffuseColor;

    float DiffuseAngleGen;

    float3 TotalDiffuse;
    float3 Normal;

    float3 lLightDir;
    float3 lLightTemp;

    float lLightDistancePow2;
    float lLightGen;
    float lLightDirectionCosA;

    float OutputAlpha;


    // ------------------------------------------------------------
    // 法線を正規化
    // ------------------------------------------------------------

    Normal = normalize(PSInput.VNormal);


    // ------------------------------------------------------------
    // ディフューズテクスチャ
    // ------------------------------------------------------------

    TextureDiffuseColor =
        g_DiffuseMapTexture.Sample(
            g_DiffuseMapSampler,
            PSInput.TexCoords0_1.xy
        );


    // ------------------------------------------------------------
    // ディゾルブ
    // ------------------------------------------------------------

    //ディゾルブの赤
    float dissolveValue = g_DissolveTexture.Sample(g_DiffuseMapSampler,PSInput.TexCoords0_1.xy).r;
    //現在のディゾルブの度合
    float dissolveThreshold = g_DissolveParam.x;

    // ディゾルブ画像の値が
    // threshold より小さければ消す
    if (dissolveValue < dissolveThreshold)
    {
        discard;
    }

    // ------------------------------------------------------------
    // ライト0
    // ------------------------------------------------------------

    TotalDiffuse = 0.0f;


    // ライトの方向
    if (g_Common.Light[0].Type == 3)
    {
        lLightDir = g_Common.Light[0].Direction;
        lLightGen = 1.0f;
    }
    else
    {
        lLightDir =
            normalize(
                PSInput.VPosition.xyz -
                g_Common.Light[0].Position
            );


        // 距離減衰
        lLightTemp =
            PSInput.VPosition.xyz -
            g_Common.Light[0].Position;

        lLightDistancePow2 =
            dot(lLightTemp, lLightTemp);

        lLightGen =
            1.0f /
            (
                g_Common.Light[0].Attenuation0 +
                g_Common.Light[0].Attenuation1 *
                sqrt(lLightDistancePow2) +
                g_Common.Light[0].Attenuation2 *
                lLightDistancePow2
            );


        // スポットライト
        if (g_Common.Light[0].Type == 2)
        {
            lLightDirectionCosA =
                dot(
                    lLightDir,
                    g_Common.Light[0].Direction
                );

            lLightGen *=
                saturate(
                    pow(
                        abs(
                            max(
                                lLightDirectionCosA -
                                g_Common.Light[0].SpotParam0,
                                0.0f
                            )
                            *
                            g_Common.Light[0].SpotParam1
                        ),
                        g_Common.Light[0].FallOff
                    )
                );
        }


        // 有効距離外
        lLightGen *=
            step(
                lLightDistancePow2,
                g_Common.Light[0].RangePow2
            );
    }


    // ------------------------------------------------------------
    // ディフューズ角度減衰
    // ------------------------------------------------------------

    if (g_Base.NoLightAngleAttenuation == 1)
    {
        DiffuseAngleGen = 1.0f;
    }
    else
    {
        float lLightDot =
            dot(
                Normal,
                -lLightDir
            );


        if (g_Base.UseHalfLambert == 1)
        {
            lLightDot =
                lLightDot * 0.5f + 0.5f;

            DiffuseAngleGen =
                lLightDot * lLightDot;
        }
        else
        {
            DiffuseAngleGen =
                saturate(lLightDot);
        }
    }


    // ------------------------------------------------------------
    // ディフューズカラー
    // ------------------------------------------------------------

    TotalDiffuse +=
        (
            g_Common.Light[0].Diffuse *
            PSInput.Diffuse.xyz *
            DiffuseAngleGen
            +
            g_Common.Light[0].Ambient.xyz
        )
        *
        lLightGen;


    // ------------------------------------------------------------
    // アンビエント + エミッシブ
    // ------------------------------------------------------------

    TotalDiffuse +=
        g_Common.Material.Ambient_Emissive.rgb;


    // ------------------------------------------------------------
    // 出力カラー
    // ------------------------------------------------------------

    PSOutput.Color0.rgb =
        TotalDiffuse *
        TextureDiffuseColor.rgb;


    OutputAlpha =
        TextureDiffuseColor.a *
        g_Base.FactorColor.a *
        PSInput.Diffuse.a;


    // ------------------------------------------------------------
    // アルファテスト
    // ------------------------------------------------------------

    AlphaTest(OutputAlpha);


    // ------------------------------------------------------------
    // フォグ
    // ------------------------------------------------------------

    PSOutput.Color0.rgb =
        lerp(
            g_Common.Fog.Color.rgb,
            PSOutput.Color0.rgb,
            saturate(PSInput.Fog.x)
        );


    // 高さフォグ
    PSOutput.Color0.rgb =
        lerp(
            g_Common.VerticalFog.Color.rgb,
            PSOutput.Color0.rgb,
            saturate(PSInput.Fog.y)
        );


    // ------------------------------------------------------------
    // 単純色加算
    // ------------------------------------------------------------

    PSOutput.Color0.r += g_Base.DrawAddColor.r;
    PSOutput.Color0.g += g_Base.DrawAddColor.g;
    PSOutput.Color0.b += g_Base.DrawAddColor.b;


    // ------------------------------------------------------------
    // アルファ乗算カラー
    // ------------------------------------------------------------

    if (g_Base.MulAlphaColor.x > 0.5f)
    {
        PSOutput.Color0.rgb *= OutputAlpha;
    }


    PSOutput.Color0.a = OutputAlpha;


    // ------------------------------------------------------------
    // Depth
    // ------------------------------------------------------------

    PSOutput.Depth.x =
        PSInput.VPosition.z;

    PSOutput.Depth.y = 0.0f;
    PSOutput.Depth.z = 0.0f;
    PSOutput.Depth.w = 1.0f;


    // ------------------------------------------------------------
    // Normal
    // ------------------------------------------------------------

    PSOutput.Normal.x =
        (Normal.x + 1.0f) / 2.0f;

    PSOutput.Normal.y =
        (Normal.y + 1.0f) / 2.0f;

    PSOutput.Normal.z =
        (Normal.z + 1.0f) / 2.0f;

    PSOutput.Normal.w = 1.0f;
    
    //ふちを明るく
    float edgeWidth = 0.05f;
    float edge = 1.0f - smoothstep(0.0f, edgeWidth, dissolveValue - dissolveThreshold);

    PSOutput.Color0.rgb = lerp(PSOutput.Color0.rgb, float3(1.0f, 0.3f, 0.0f), edge);

    return PSOutput;
}