#include "DxLib.h"
#include <cassert>
#include <cmath>

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    // ウィンドウモードで起動
    ChangeWindowMode(TRUE);

    SetUseDirect3DVersion(DX_DIRECT3D_11);

    // DXライブラリ初期化
    if (DxLib_Init() == -1)
    {
        return -1;
    }

    // 裏画面に描画
    SetDrawScreen(DX_SCREEN_BACK);

    // シェーダーの読み込み(ボーン有り)
    int boneVertexShaderHandle = LoadVertexShader(L"DekobokoVertexShader.vso");
    assert(boneVertexShaderHandle != -1);
    // シェーダーの読み込み(ボーン無し)
    //int noBoneVertexShaderHandle = LoadVertexShader(L"MV1VertexShader_NoBone.vso");
    //assert(noBoneVertexShaderHandle != -1);

    //ディゾルブシェーダー
    //int pixelShaderHandle = LoadPixelShader(L"Dissolve.pso");
    int pixelShaderHandle = LoadPixelShader(L"MV1PixelShader_Simple.pso");
    assert(pixelShaderHandle != -1);
    //準備
    struct DissolveBuffer
    {
        float threshold = 0.0f;
        float bumpPower = 0.0f;
        float dummy2 = 0.0f;
        float dummy3 = 0.0f;
    };
    int dissolveCB = CreateShaderConstantBuffer(
        sizeof(DissolveBuffer)
    );
    //ディゾルブのコンスタントバッファ作成
    DissolveBuffer* dissolveBuffer =
        static_cast<DissolveBuffer*>(
            GetBufferShaderConstantBuffer(dissolveCB)
            );
    UpdateShaderConstantBuffer(dissolveCB);
    //ディゾルブ用の画像(パーリンノイズ)
    int dissolveTexture = LoadGraph(L"PerlinNoise.jpg");
    //テクスチャを渡す
    SetUseTextureToShader(1, dissolveTexture);

    ////ボーン無し
    //int starModelHandle = MV1LoadModel(L"Star.mv1");
    //assert(starModelHandle != -1);
    //MV1SetPosition(starModelHandle, VGet(100, 0, 0));

    //ボーン有り
    int humanModelHandle = MV1LoadModel(L"Human.mv1");
    assert(humanModelHandle != -1);
    MV1SetPosition(humanModelHandle, VGet(0, -100, 0));


    float angle = 0.0f;

    // モデル描画にオリジナルシェーダーを使う設定をON
    MV1SetUseOrigShader(TRUE);
    SetUsePixelShader(pixelShaderHandle);

    //ライト
    SetLightDirection(VGet(0.0f, 0.0f, -1.0f));

    // モーションを読み込む
    int animIndex = MV1AttachAnim(humanModelHandle, 1);

    // 再生時間
    float animTime = 0.0f;

    while (ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0)
    {
        // 画面をクリア
        ClearDrawScreen();

        DrawBox(0, 0, 1000, 1000, 0x555555,true);

        SetCameraPositionAndTarget_UpVecY(VGet(0, 0, 200), VGet(0, 0, 0));

        //回転
        angle += 0.01f;
        //MV1SetRotationYUseDir(starModelHandle, VGet(0, 1, 0), angle);
        MV1SetRotationYUseDir(humanModelHandle, VGet(0, 1, 0), angle);

        // 毎フレーム
        animTime += 1.0f;

        MV1SetAttachAnimTime(humanModelHandle, animIndex, animTime);

        // モーションの長さ
        float animTotalTime = MV1GetAttachAnimTotalTime(humanModelHandle, animIndex);

        // ループ
        if (animTime >= animTotalTime)
        {
            animTime = 0.0f;
        }

        //ディゾルブ
        dissolveBuffer->threshold = std::cosf(angle);
        dissolveBuffer->bumpPower = std::cosf(-angle) * 10.0f;
        UpdateShaderConstantBuffer(dissolveCB);
        SetShaderConstantBuffer(dissolveCB,DX_SHADERTYPE_PIXEL,5);
        SetShaderConstantBuffer(dissolveCB, DX_SHADERTYPE_VERTEX, 5);


        //ボーン無し描画
        //SetUseVertexShader(noBoneVertexShaderHandle);
        //MV1DrawModel(starModelHandle);

        //ボーン有り描画
        SetUseVertexShader(boneVertexShaderHandle);
        MV1DrawModel(humanModelHandle);

        // 描画内容を画面に反映
        ScreenFlip();
    }

    DeleteShader(boneVertexShaderHandle);
    DeleteShader(pixelShaderHandle);
    //MV1DeleteModel(starModelHandle);
    // DXライブラリ終了
    DxLib_End();

    return 0;
}