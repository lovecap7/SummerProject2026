//サンプラーステート（テクスチャをどうサンプリングするか指定する）
SamplerState smp : register(s0);
//テクスチャ（シェーダーで扱う画像データ）
Texture2D tex : register(t0);
// 定数バッファ (CPU → GPU へ送る値)
cbuffer ConstantBuffer : register(b4)
{
    float value1;
    float value2;
    float value3;
    float value4;
}

//モノクロ化のための輝度係数
#define R_LUMINANCE 0.298912
#define G_LUMINANCE 0.586611
#define B_LUMINANCE 0.114478

//シェーダーの状態定数
#define STATE_NORMAL 0              //通常
#define STATE_GLITCH 1 << 0         //グリッチ効果
#define STATE_MONOCHROME 1 << 1     //モノクロ
#define STATE_JUSTAVOID 1 << 2      //ジャスト回避
#define STATE_GLITCHNOCOLOR 1 << 3  //色ずれなしグリッジ

//疑似乱数を生成する関数
float random(float2 seeds)
{
    return frac(sin(dot(seeds, float2(12.9898, 78.233))) * 43758.5453);
}

//ブロックノイズ（座標を整数化してノイズを生成）
float blockNoise(float2 seeds)
{
    return random(floor(seeds));
}

//-1.0 ～ 1.0 の範囲の乱数を生成
float noiserandom(float2 seeds)
{
    return -1.0 + 2.0 * blockNoise(seeds);
}

//ピクセルシェーダーの入力構造体
struct PS_INPUT
{   
    float4 pos      : SV_Position;  //ピクセルのスクリーン座標
    float4 color    : COLOR0;       //頂点カラー
    float2 uv       : TEXCOORD0;    //テクスチャ座標
};

//ピクセルシェーダーの出力構造体
struct PS_OUTPUT
{
    float4 color    : SV_TARGET0;  //出力カラー
};

PS_OUTPUT main(PS_INPUT input)
{
    PS_OUTPUT output;

    // 元のテクスチャ色を取得
    float4 color = tex.Sample(smp, input.uv);

    //最終的なカラーを出力
    output.color = color;
    return output;
}
