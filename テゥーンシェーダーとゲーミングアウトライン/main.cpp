#include "DxLib.h"
#include "MyDraw.h"
#include <cassert>
#include <array>

//ゲーミングカラー
std::array<float, 3> GetRainbowColor(float time)
{
    float hue = std::fmod(time, 1.0f);
    float h = hue * 6.0f;

    int i = static_cast<int>(h);
    float f = h - i;

    float r, g, b;

    switch (i)
    {
    case 0:
        r = 1.0f;
        g = f;
        b = 0.0f;
        break;

    case 1:
        r = 1.0f - f;
        g = 1.0f;
        b = 0.0f;
        break;

    case 2:
        r = 0.0f;
        g = 1.0f;
        b = f;
        break;

    case 3:
        r = 0.0f;
        g = 1.0f - f;
        b = 1.0f;
        break;

    case 4:
        r = f;
        g = 0.0f;
        b = 1.0f;
        break;

    default:
        r = 1.0f;
        g = 0.0f;
        b = 1.0f - f;
        break;
    }

    return { r, g, b };
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    // ウィンドウモードで起動
    ChangeWindowMode(TRUE);

    // DXライブラリ初期化
    if (DxLib_Init() == -1)
    {
        return -1;
    }

    // 裏画面に描画
    SetDrawScreen(DX_SCREEN_BACK);

    int psH = LoadPixelShader(L"MV1PixelShader_Simple.pso");
    assert(psH != -1);
    int outLinePSH = LoadPixelShader(L"ToonShader_OutLine.pso");
    assert(outLinePSH != -1);
    int vsH = LoadVertexShader(L"MV1VertexShader_Bone.vso");
    assert(vsH != -1);
    int outLineVSH = LoadVertexShader(L"VertexShader_Outline.vso");
    assert(outLineVSH != -1);

    int modelH = MV1LoadModel(L"Human.mv1");
    assert(modelH != -1);

    MV1SetUseOrigShader(true);
  
    // モーションを読み込む
    int animIndex = MV1AttachAnim(modelH, 1);

    // 再生時間
    float animTime = 0.0f;
    float angle = 0.0f;

    //アウトライン用の定数バッファVS
    struct VSOutLine
    {
        float width = 0.0f;
        float temp1 = 0.0f;
        float temp2 = 0.0f;
        float temp3 = 0.0f;
    };
    int outlineVSCBH = CreateShaderConstantBuffer(sizeof(VSOutLine)); // 16byteアライメント必須
    VSOutLine* vsPointer = (VSOutLine*)GetBufferShaderConstantBuffer(outlineVSCBH);
    //アウトライン用の定数バッファPS
    struct PSOutLine
    {
        float color1 = 0.0f;
        float color2 = 0.0f;
        float color3 = 0.0f;
        float color4 = 0.0f;
    };
    int outlinePSCBH = CreateShaderConstantBuffer(sizeof(PSOutLine)); // 16byteアライメント必須
    PSOutLine* psPointer = (PSOutLine*)GetBufferShaderConstantBuffer(outlinePSCBH);

    // 画面サイズ取得
    int width, height;
    GetDrawScreenSize(&width, &height);

    int rt1 = MakeScreen(width, height, true);
    int rt2 = MakeScreen(width, height, true);

    while (ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0)
    {
        // 画面をクリア
        ClearDrawScreen();
      

        //回転
        angle += 0.01f;
        //MV1SetRotationYUseDir(starModelHandle, VGet(0, 1, 0), angle);
        MV1SetRotationYUseDir(modelH, VGet(0, 1, 0), angle);

        // 毎フレーム
        animTime += 1.0f;

        MV1SetAttachAnimTime(modelH, animIndex, animTime);

        // モーションの長さ
        float animTotalTime = MV1GetAttachAnimTotalTime(modelH, animIndex);

        // ループ
        if (animTime >= animTotalTime)
        {
            animTime = 0.0f;
        }

        //アウトラインの色
        auto color = GetRainbowColor(angle);
        psPointer->color1 = color[0];
        psPointer->color2 = color[1];
        psPointer->color3 = color[2];
        psPointer->color4 = 1.0f;
        UpdateShaderConstantBuffer(outlinePSCBH);
        SetShaderConstantBuffer(outlinePSCBH, DX_SHADERTYPE_PIXEL, 6); // b6スロットへ
        //アウトラインの太さ
        vsPointer->width = 1.0f;
        UpdateShaderConstantBuffer(outlineVSCBH);
        SetShaderConstantBuffer(outlineVSCBH, DX_SHADERTYPE_VERTEX, 7); // b7スロットへ


        SetDrawScreen(rt1);
        ClearDrawScreen();
        SetCameraPositionAndTargetAndUpVec(VGet(0, 100, -200), VGet(0, 100, 0), VGet(0, 1, 0));

        // 1パス目: アウトライン(背面のみ描画、法線方向に膨らませ、単色)
        SetUseVertexShader(outLineVSH);
        SetUsePixelShader(outLinePSH);
        MV1DrawModel(modelH);

        SetDrawScreen(rt2);
        ClearDrawScreen();
        SetCameraPositionAndTargetAndUpVec(VGet(0, 100, -200), VGet(0, 100, 0), VGet(0, 1, 0));

        // 2パス目: 通常描画(表面のみ)
        SetUseVertexShader(vsH);
        SetUsePixelShader(psH);
        MV1DrawModel(modelH);

        SetDrawScreen(DX_SCREEN_BACK);
        ClearDrawScreen();
        SetCameraPositionAndTargetAndUpVec(VGet(0, 100, -200), VGet(0, 100, 0), VGet(0, 1, 0));

        //DrawBox(0, 0, width, height, 0xaaaaaa, true);
        DrawGraph(0, 0, rt1,true);
        DrawGraph(0, 0, rt2, true);
        

        // 描画内容を画面に反映
        ScreenFlip();
    }

    // DXライブラリ終了
    DxLib_End();

    return 0;
}