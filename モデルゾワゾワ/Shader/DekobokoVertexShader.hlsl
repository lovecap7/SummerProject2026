#define SKINMESH 1

#include "VertexShader.h"


// ============================================================
// ディゾルブテクスチャ
// C++側:
//
// SetUseTextureToShader(1, dissolveTexture);
//
// で設定したテクスチャを使用
// ============================================================

Texture2D g_DissolveTexture : register(t1);


// ============================================================
// ディゾルブ用サンプラー
// ============================================================

SamplerState g_DissolveSampler : register(s0)
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Clamp;
    AddressV = Clamp;
};

cbuffer DissolveBuffer : register(b5)
{
    float4 g_DissolveParam;
};


// ============================================================
// 頂点シェーダー入力
// ============================================================

struct VS_INPUT
{
    float3 Position : POSITION;
    float3 Normal : NORMAL0;

    float4 Diffuse : COLOR0;
    float4 Specular : COLOR1;

    float4 TexCoords0 : TEXCOORD0;
    float4 TexCoords1 : TEXCOORD1;

#ifdef BUMPMAP

    float3 Tan : TANGENT0;
    float3 Bin : BINORMAL0;

#endif // BUMPMAP


#ifdef SKINMESH

    int4 BlendIndices0 : BLENDINDICES0;
    float4 BlendWeight0 : BLENDWEIGHT0;

#ifdef BONE8

    int4 BlendIndices1 : BLENDINDICES1;
    float4 BlendWeight1 : BLENDWEIGHT1;

#endif // BONE8

#endif // SKINMESH
};


// ============================================================
// 頂点シェーダー出力
// ============================================================

struct VS_OUTPUT
{
    float4 Diffuse : COLOR0;
    float4 Specular : COLOR1;

    float4 TexCoords0_1 : TEXCOORD0;

    float3 VPosition : TEXCOORD1;
    float3 VNormal : TEXCOORD2;

#ifdef BUMPMAP

    float3 VTan : TEXCOORD3;
    float3 VBin : TEXCOORD4;

#endif // BUMPMAP

    float2 Fog : TEXCOORD5;

#if SHADOWMAP

    float3 ShadowMap0Pos : TEXCOORD6;
    float3 ShadowMap1Pos : TEXCOORD7;
    float3 ShadowMap2Pos : TEXCOORD8;

#endif // SHADOWMAP

    float4 Position : SV_POSITION;
};


// ============================================================
// ローカルワールド行列
// ============================================================

#ifdef SKINMESH

#define LOCAL_WORLD_MAT lLocalWorldMatrix

#else

#define LOCAL_WORLD_MAT g_Base.LocalWorldMatrix

#endif


// ============================================================
// main
// ============================================================

VS_OUTPUT main(VS_INPUT VSInput)
{
    VS_OUTPUT VSOutput;

    int4 lBoneFloatIndex;

    float4 lLocalWorldMatrix[3];

    float4 lLocalPosition;
    float4 lWorldPosition;
    float4 lViewPosition;

    float3 lWorldNrm;
    float3 lWorldTan;
    float3 lWorldBin;

    float3 lViewNrm;
    float3 lViewTan;
    float3 lViewBin;

    float lVerticalFogY;
    float lFogDensity;


    // ========================================================
    // スキニング
    // ========================================================

#ifdef SKINMESH

    lBoneFloatIndex = VSInput.BlendIndices0;


    lLocalWorldMatrix[0] =
        g_LocalWorldMatrix.
    Matrix[ lBoneFloatIndex.
    x + 0]
        * VSInput.BlendWeight0.xxxx;

    lLocalWorldMatrix[1] =
        g_LocalWorldMatrix.
    Matrix[ lBoneFloatIndex.
    x + 1]
        * VSInput.BlendWeight0.xxxx;

    lLocalWorldMatrix[2] =
        g_LocalWorldMatrix.
    Matrix[ lBoneFloatIndex.
    x + 2]
        * VSInput.BlendWeight0.xxxx;


    lLocalWorldMatrix[0] +=
        g_LocalWorldMatrix.
    Matrix[ lBoneFloatIndex.
    y + 0]
        * VSInput.BlendWeight0.yyyy;

    lLocalWorldMatrix[1] +=
        g_LocalWorldMatrix.
    Matrix[ lBoneFloatIndex.
    y + 1]
        * VSInput.BlendWeight0.yyyy;

    lLocalWorldMatrix[2] +=
        g_LocalWorldMatrix.
    Matrix[ lBoneFloatIndex.
    y + 2]
        * VSInput.BlendWeight0.yyyy;


    lLocalWorldMatrix[0] +=
        g_LocalWorldMatrix.
    Matrix[ lBoneFloatIndex.
    z + 0]
        * VSInput.BlendWeight0.zzzz;

    lLocalWorldMatrix[1] +=
        g_LocalWorldMatrix.
    Matrix[ lBoneFloatIndex.
    z + 1]
        * VSInput.BlendWeight0.zzzz;

    lLocalWorldMatrix[2] +=
        g_LocalWorldMatrix.
    Matrix[ lBoneFloatIndex.
    z + 2]
        * VSInput.BlendWeight0.zzzz;


    lLocalWorldMatrix[0] +=
        g_LocalWorldMatrix.
    Matrix[ lBoneFloatIndex.
    w + 0]
        * VSInput.BlendWeight0.wwww;

    lLocalWorldMatrix[1] +=
        g_LocalWorldMatrix.
    Matrix[ lBoneFloatIndex.
    w + 1]
        * VSInput.BlendWeight0.wwww;

    lLocalWorldMatrix[2] +=
        g_LocalWorldMatrix.
    Matrix[ lBoneFloatIndex.
    w + 2]
        * VSInput.BlendWeight0.wwww;


#ifdef BONE8

    lBoneFloatIndex = VSInput.BlendIndices1;


    lLocalWorldMatrix[0] +=
        g_LocalWorldMatrix.Matrix[lBoneFloatIndex.x + 0]
        * VSInput.BlendWeight1.xxxx;

    lLocalWorldMatrix[1] +=
        g_LocalWorldMatrix.Matrix[lBoneFloatIndex.x + 1]
        * VSInput.BlendWeight1.xxxx;

    lLocalWorldMatrix[2] +=
        g_LocalWorldMatrix.Matrix[lBoneFloatIndex.x + 2]
        * VSInput.BlendWeight1.xxxx;


    lLocalWorldMatrix[0] +=
        g_LocalWorldMatrix.Matrix[lBoneFloatIndex.y + 0]
        * VSInput.BlendWeight1.yyyy;

    lLocalWorldMatrix[1] +=
        g_LocalWorldMatrix.Matrix[lBoneFloatIndex.y + 1]
        * VSInput.BlendWeight1.yyyy;

    lLocalWorldMatrix[2] +=
        g_LocalWorldMatrix.Matrix[lBoneFloatIndex.y + 2]
        * VSInput.BlendWeight1.yyyy;


    lLocalWorldMatrix[0] +=
        g_LocalWorldMatrix.Matrix[lBoneFloatIndex.z + 0]
        * VSInput.BlendWeight1.zzzz;

    lLocalWorldMatrix[1] +=
        g_LocalWorldMatrix.Matrix[lBoneFloatIndex.z + 1]
        * VSInput.BlendWeight1.zzzz;

    lLocalWorldMatrix[2] +=
        g_LocalWorldMatrix.Matrix[lBoneFloatIndex.z + 2]
        * VSInput.BlendWeight1.zzzz;


    lLocalWorldMatrix[0] +=
        g_LocalWorldMatrix.Matrix[lBoneFloatIndex.w + 0]
        * VSInput.BlendWeight1.wwww;

    lLocalWorldMatrix[1] +=
        g_LocalWorldMatrix.Matrix[lBoneFloatIndex.w + 1]
        * VSInput.BlendWeight1.wwww;

    lLocalWorldMatrix[2] +=
        g_LocalWorldMatrix.Matrix[lBoneFloatIndex.w + 2]
        * VSInput.BlendWeight1.wwww;

#endif // BONE8

#endif // SKINMESH


    // ========================================================
    // ローカル座標
    // ========================================================

    lLocalPosition.xyz = VSInput.Position;
    lLocalPosition.w = 1.0f;


    // ========================================================
    // ディゾルブテクスチャから値を取得
    // ========================================================

    float dissolveValue = g_DissolveTexture.SampleLevel(g_DissolveSampler,VSInput.TexCoords0.xy,0).r;

    // ========================================================
    // ディゾルブ設定
    // ========================================================

    float dissolveThreshold = g_DissolveParam.x;
    float edgeWidth = 0.15f;
    float bumpPower = g_DissolveParam.y;

    //ディゾルブテクスチャの境界からどれくらい近いか
    float distanceFromEdge = abs(dissolveValue - dissolveThreshold);
    //ret smoothstep(min, max, x)
    float edge = 1.0f - smoothstep(0.0f,edgeWidth,distanceFromEdge);

    lLocalPosition.xyz += VSInput.Normal * edge * bumpPower;


    // ========================================================
    // ローカル → ワールド
    // ========================================================

    lWorldPosition.x =
        dot(
            lLocalPosition,
            LOCAL_WORLD_MAT[0]
        );

    lWorldPosition.y =
        dot(
            lLocalPosition,
            LOCAL_WORLD_MAT[1]
        );

    lWorldPosition.z =
        dot(
            lLocalPosition,
            LOCAL_WORLD_MAT[2]
        );

    lWorldPosition.w = 1.0f;


    // ========================================================
    // ワールド → ビュー
    // ========================================================

    lViewPosition.x =
        dot(
            lWorldPosition,
            g_Base.ViewMatrix[0]
        );

    lViewPosition.y =
        dot(
            lWorldPosition,
            g_Base.ViewMatrix[1]
        );

    lViewPosition.z =
        dot(
            lWorldPosition,
            g_Base.ViewMatrix[2]
        );

    lViewPosition.w = 1.0f;


    // ========================================================
    // ビュー → プロジェクション
    // ========================================================

    VSOutput.Position.x =
        dot(
            lViewPosition,
            g_Base.ProjectionMatrix[0]
        );

    VSOutput.Position.y =
        dot(
            lViewPosition,
            g_Base.ProjectionMatrix[1]
        );

    VSOutput.Position.z =
        dot(
            lViewPosition,
            g_Base.ProjectionMatrix[2]
        );

    VSOutput.Position.w =
        dot(
            lViewPosition,
            g_Base.ProjectionMatrix[3]
        );


    // ========================================================
    // ビュー空間座標
    // ========================================================

    VSOutput.VPosition =
        lViewPosition.xyz;


    // ========================================================
    // 法線
    // ========================================================

    lWorldNrm.x =
        dot(
            VSInput.Normal,
            LOCAL_WORLD_MAT[0].xyz
        );

    lWorldNrm.y =
        dot(
            VSInput.Normal,
            LOCAL_WORLD_MAT[1].xyz
        );

    lWorldNrm.z =
        dot(
            VSInput.Normal,
            LOCAL_WORLD_MAT[2].xyz
        );


    lViewNrm.x =
        dot(
            lWorldNrm,
            g_Base.ViewMatrix[0].xyz
        );

    lViewNrm.y =
        dot(
            lWorldNrm,
            g_Base.ViewMatrix[1].xyz
        );

    lViewNrm.z =
        dot(
            lWorldNrm,
            g_Base.ViewMatrix[2].xyz
        );


    VSOutput.VNormal =
        lViewNrm;


    // ========================================================
    // バンプマップ
    // ========================================================

#ifdef BUMPMAP

    lWorldTan.x =
        dot(
            VSInput.Tan,
            LOCAL_WORLD_MAT[0].xyz
        );

    lWorldTan.y =
        dot(
            VSInput.Tan,
            LOCAL_WORLD_MAT[1].xyz
        );

    lWorldTan.z =
        dot(
            VSInput.Tan,
            LOCAL_WORLD_MAT[2].xyz
        );


    lWorldBin.x =
        dot(
            VSInput.Bin,
            LOCAL_WORLD_MAT[0].xyz
        );

    lWorldBin.y =
        dot(
            VSInput.Bin,
            LOCAL_WORLD_MAT[1].xyz
        );

    lWorldBin.z =
        dot(
            VSInput.Bin,
            LOCAL_WORLD_MAT[2].xyz
        );


    lViewTan.x =
        dot(
            lWorldTan,
            g_Base.ViewMatrix[0].xyz
        );

    lViewTan.y =
        dot(
            lWorldTan,
            g_Base.ViewMatrix[1].xyz
        );

    lViewTan.z =
        dot(
            lWorldTan,
            g_Base.ViewMatrix[2].xyz
        );


    lViewBin.x =
        dot(
            lWorldBin,
            g_Base.ViewMatrix[0].xyz
        );

    lViewBin.y =
        dot(
            lWorldBin,
            g_Base.ViewMatrix[1].xyz
        );

    lViewBin.z =
        dot(
            lWorldBin,
            g_Base.ViewMatrix[2].xyz
        );


    VSOutput.VTan = lViewTan;
    VSOutput.VBin = lViewBin;

#endif // BUMPMAP


    // ========================================================
    // ディフューズ
    // ========================================================

    VSOutput.Diffuse =
        g_Base.DiffuseSource > 0.5f
        ?
        VSInput.Diffuse
        :
        g_Common.Material.Diffuse;


    // ========================================================
    // スペキュラ
    // ========================================================

    VSOutput.Specular =
        (
            g_Base.SpecularSource > 0.5f
            ?
            VSInput.Specular
            :
            g_Common.Material.Specular
        )
        *
        g_Base.MulSpecularColor;


    // ========================================================
    // フォグ
    // ========================================================

#if FOG_LINEAR || FOG_EXP || FOG_EXP2

#ifdef FOG_LINEAR

    VSOutput.Fog.x =
        lViewPosition.z *
        g_Common.Fog.LinearDiv +
        g_Common.Fog.LinearAdd;

#endif

#ifdef FOG_EXP

    VSOutput.Fog.x =
        1.0f /
        pow(
            abs(g_Common.Fog.E),
            lViewPosition.z *
            g_Common.Fog.Density
        );

#endif

#ifdef FOG_EXP2

    VSOutput.Fog.x =
        1.0f /
        pow(
            abs(g_Common.Fog.E),
            (
                lViewPosition.z *
                g_Common.Fog.Density
            )
            *
            (
                lViewPosition.z *
                g_Common.Fog.Density
            )
        );

#endif

#else

    VSOutput.Fog.x = 1.0f;

#endif


    // ========================================================
    // 高さフォグ
    // ========================================================

    VSOutput.Fog.y = 1.0f;

    if (
        g_Common.VerticalFog.Mode == 1 ||
        g_Common.VerticalFog.Mode == 2
    )
    {
        if (g_Common.VerticalFog.Density < 0.0f)
        {
            lVerticalFogY =
                lWorldPosition.y -
                g_Common.VerticalFog.DensityStart;

            lFogDensity =
                -g_Common.VerticalFog.Density;
        }
        else
        {
            lVerticalFogY =
                g_Common.VerticalFog.DensityStart -
                lWorldPosition.y;

            lFogDensity =
                g_Common.VerticalFog.Density;
        }


        if (lVerticalFogY > 0.0f)
        {
            if (g_Common.VerticalFog.Mode == 1)
            {
                VSOutput.Fog.y =
                    1.0f /
                    pow(
                        abs(g_Common.VerticalFog.E),
                        lVerticalFogY *
                        lFogDensity
                    );
            }
            else
            {
                VSOutput.Fog.y =
                    1.0f /
                    pow(
                        abs(g_Common.VerticalFog.E),
                        (
                            lVerticalFogY *
                            lFogDensity
                        )
                        *
                        (
                            lVerticalFogY *
                            lFogDensity
                        )
                    );
            }
        }
    }
    else if (g_Common.VerticalFog.Mode == 3)
    {
        VSOutput.Fog.y =
            lWorldPosition.y *
            g_Common.VerticalFog.LinearDiv +
            g_Common.VerticalFog.LinearAdd;
    }


    // ========================================================
    // シャドウマップ
    // ========================================================

#if SHADOWMAP

    VSOutput.ShadowMap0Pos.x =
        dot(
            lWorldPosition,
            g_OtherMatrix.ShadowMapLightViewProjectionMatrix[0][0]
        );

    VSOutput.ShadowMap0Pos.y =
        dot(
            lWorldPosition,
            g_OtherMatrix.ShadowMapLightViewProjectionMatrix[0][1]
        );

    VSOutput.ShadowMap0Pos.z =
        dot(
            lWorldPosition,
            g_OtherMatrix.ShadowMapLightViewProjectionMatrix[0][2]
        );


    VSOutput.ShadowMap1Pos.x =
        dot(
            lWorldPosition,
            g_OtherMatrix.ShadowMapLightViewProjectionMatrix[1][0]
        );

    VSOutput.ShadowMap1Pos.y =
        dot(
            lWorldPosition,
            g_OtherMatrix.ShadowMapLightViewProjectionMatrix[1][1]
        );

    VSOutput.ShadowMap1Pos.z =
        dot(
            lWorldPosition,
            g_OtherMatrix.ShadowMapLightViewProjectionMatrix[1][2]
        );


    VSOutput.ShadowMap2Pos.x =
        dot(
            lWorldPosition,
            g_OtherMatrix.ShadowMapLightViewProjectionMatrix[2][0]
        );

    VSOutput.ShadowMap2Pos.y =
        dot(
            lWorldPosition,
            g_OtherMatrix.ShadowMapLightViewProjectionMatrix[2][1]
        );

    VSOutput.ShadowMap2Pos.z =
        dot(
            lWorldPosition,
            g_OtherMatrix.ShadowMapLightViewProjectionMatrix[2][2]
        );

#endif // SHADOWMAP


    // ========================================================
    // テクスチャ座標
    // ========================================================

    VSOutput.TexCoords0_1.x =
        dot(
            VSInput.TexCoords0,
            g_OtherMatrix.TextureMatrix[0][0]
        );

    VSOutput.TexCoords0_1.y =
        dot(
            VSInput.TexCoords0,
            g_OtherMatrix.TextureMatrix[0][1]
        );


    VSOutput.TexCoords0_1.z =
        dot(
            VSInput.TexCoords1,
            g_OtherMatrix.TextureMatrix[0][0]
        );

    VSOutput.TexCoords0_1.w =
        dot(
            VSInput.TexCoords1,
            g_OtherMatrix.TextureMatrix[0][1]
        );


    return VSOutput;
}