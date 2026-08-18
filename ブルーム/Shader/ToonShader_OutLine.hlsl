struct PS_INPUT
{
    float4 Position : SV_POSITION;
    bool IsFrontFace : SV_IsFrontFace;
};

cbuffer OutlineColor : register(b6) // 空いている番号を使う
{
    float4 g_OutlineColor; // 例: (0,0,0,1) で黒
};

float4 main(PS_INPUT PSInput) : SV_TARGET
{
    //ピクセルシェーダー内で、現在処理しているピクセルが
    //表向き（Front-facing）」のポリゴンに属しているか判定
   
    return g_OutlineColor;
}