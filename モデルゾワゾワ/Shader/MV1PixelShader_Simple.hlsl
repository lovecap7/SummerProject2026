#include "PixelShader.h"

// このシンプル版は以下の構成に決め打ちしています:
//   ・ライトは1つだけ(g_Common.Light[0])。種類(平行/点/スポット)は実行時に判定
//   ・バンプマップ、トゥーン、シャドウマップ、スペキュラ、サブテクスチャは無し
// 別の機能が欲しくなったら、元のフル機能版から該当ブロックだけ持ってくる形にすると管理しやすいです。

// ピクセルシェーダーの入力
// (バンプマップ用のTANGENT/BINORMALやシャドウマップ用のTEXCOORD6-8は使わないので削除)
struct PS_INPUT
{
	float4 Diffuse         : COLOR0 ;		// ディフューズカラー
	float4 Specular        : COLOR1 ;		// スペキュラカラー(未使用だが頂点シェーダーの出力と構造体を合わせる必要がある)
	float4 TexCoords0_1    : TEXCOORD0 ;	// xy:テクスチャ座標 zw:サブテクスチャ座標(未使用)
	float3 VPosition       : TEXCOORD1 ;	// 座標( ビュー空間 )
	float3 VNormal         : TEXCOORD2 ;	// 法線( ビュー空間 )
	float2 Fog              : TEXCOORD5 ;	// フォグパラメータ( x )   高さフォグパラメータ( y )
} ;

// ピクセルシェーダーの出力
struct PS_OUTPUT
{
	float4 Color0          : SV_TARGET0 ;	// 色
	float4 Normal           : SV_TARGET1 ;	// 法線( ビュー空間 )
	float4 Depth            : SV_TARGET2 ;	// 深度
} ;

SamplerState g_DiffuseMapSampler : register( s0 ) ;	// ディフューズマップテクスチャ
Texture2D    g_DiffuseMapTexture : register( t0 ) ;	// ディフューズマップテクスチャ


// main関数
PS_OUTPUT main( PS_INPUT PSInput )
{
	PS_OUTPUT PSOutput ;
	float4 TextureDiffuseColor ;
	float  DiffuseAngleGen ;
	float3 TotalDiffuse ;
	float3 Normal ;
	float3 lLightDir ;
	float3 lLightTemp ;
	float  lLightDistancePow2 ;
	float  lLightGen ;
	float  lLightDirectionCosA ;
	float  OutputAlpha ;

	// 法線を正規化
	Normal = normalize( PSInput.VNormal ) ;

	// ディフューズテクスチャカラーを取得
	TextureDiffuseColor = g_DiffuseMapTexture.Sample( g_DiffuseMapSampler, PSInput.TexCoords0_1.xy ) ;

	// ライト0の処理 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++( 開始 )

	TotalDiffuse = 0.0f ;

	// ライトの方向を求める(平行光源なら固定方向、点/スポット光源なら頂点からライト位置への方向)
	if( g_Common.Light[ 0 ].Type == 3 /* DX_LIGHTTYPE_DIRECTIONAL */ )
	{
		lLightDir = g_Common.Light[ 0 ].Direction ;
		lLightGen = 1.0f ;
	}
	else
	{
		lLightDir = normalize( PSInput.VPosition.xyz - g_Common.Light[ 0 ].Position ) ;

		// 距離減衰計算
		lLightTemp = PSInput.VPosition.xyz - g_Common.Light[ 0 ].Position ;
		lLightDistancePow2 = dot( lLightTemp, lLightTemp ) ;
		lLightGen = 1.0f / ( g_Common.Light[ 0 ].Attenuation0 + g_Common.Light[ 0 ].Attenuation1 * sqrt( lLightDistancePow2 ) + g_Common.Light[ 0 ].Attenuation2 * lLightDistancePow2 ) ;

		// スポットライト減衰計算
		if( g_Common.Light[ 0 ].Type == 2 /* DX_LIGHTTYPE_SPOT */ )
		{
			lLightDirectionCosA = dot( lLightDir, g_Common.Light[ 0 ].Direction ) ;
			lLightGen *= saturate( pow( abs( max( lLightDirectionCosA - g_Common.Light[ 0 ].SpotParam0, 0.0f ) * g_Common.Light[ 0 ].SpotParam1 ), g_Common.Light[ 0 ].FallOff ) ) ;
		}

		// 有効距離外なら減衰率を最大にする
		lLightGen *= step( lLightDistancePow2, g_Common.Light[ 0 ].RangePow2 ) ;
	}

	// ディフューズ角度減衰
	if( g_Base.NoLightAngleAttenuation == 1 )
	{
		DiffuseAngleGen = 1.0f ;
	}
	else
	{
		float lLightDot = dot( Normal, -lLightDir ) ;

		if( g_Base.UseHalfLambert == 1 )
		{
			lLightDot = lLightDot * 0.5f + 0.5f ;
			DiffuseAngleGen = lLightDot * lLightDot ;
		}
		else
		{
			DiffuseAngleGen = saturate( lLightDot ) ;
		}
	}

	// ディフューズカラー蓄積
	TotalDiffuse += ( g_Common.Light[ 0 ].Diffuse * PSInput.Diffuse.xyz * DiffuseAngleGen + g_Common.Light[ 0 ].Ambient.xyz ) * lLightGen ;

	// ライト0の処理 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++( 終了 )

	// マテリアルのアンビエント+エミッシブを加算
	TotalDiffuse += g_Common.Material.Ambient_Emissive.rgb ;

	// 出力カラー = TotalDiffuse * テクスチャカラー
	PSOutput.Color0.rgb = TotalDiffuse * TextureDiffuseColor.rgb ;
	OutputAlpha = TextureDiffuseColor.a * g_Base.FactorColor.a * PSInput.Diffuse.a ;

	// アルファテスト
	AlphaTest( OutputAlpha ) ;

	// フォグ効果
	PSOutput.Color0.rgb = lerp( g_Common.Fog.Color.rgb, PSOutput.Color0.rgb, saturate( PSInput.Fog.x ) ) ;

	// 高さフォグ効果
	PSOutput.Color0.rgb = lerp( g_Common.VerticalFog.Color.rgb, PSOutput.Color0.rgb, saturate( PSInput.Fog.y ) ) ;

	// 単純色加算
	PSOutput.Color0.r += g_Base.DrawAddColor.r ;
	PSOutput.Color0.g += g_Base.DrawAddColor.g ;
	PSOutput.Color0.b += g_Base.DrawAddColor.b ;

	// アルファ乗算カラー
	if( g_Base.MulAlphaColor.x > 0.5f )
	{
		PSOutput.Color0.rgb *= OutputAlpha ;
	}

	PSOutput.Color0.a = OutputAlpha ;

	PSOutput.Depth.x = PSInput.VPosition.z ;
	PSOutput.Depth.y = 0.0f ;
	PSOutput.Depth.z = 0.0f ;
	PSOutput.Depth.w = 1.0f ;

	PSOutput.Normal.x = ( Normal.x + 1.0f ) / 2.0f ;
	PSOutput.Normal.y = ( Normal.y + 1.0f ) / 2.0f ;
	PSOutput.Normal.z = ( Normal.z + 1.0f ) / 2.0f ;
	PSOutput.Normal.w = 1.0f ;

	return PSOutput ;
}
