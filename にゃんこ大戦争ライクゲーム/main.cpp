#include "DxLib.h"
#include "General/Input.h"
#include "Scene/SceneController.h"
#include "General/Fader.h"
#include "General/Game.h"

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    // ウィンドウモードで起動
    ChangeWindowMode(TRUE);

    SetGraphMode(Game::kScreenWidth, Game::kScreenHeight,Game::kColorBitNum);

    // DXライブラリ初期化
    if (DxLib_Init() == -1)
    {
        return -1;
    }

    // 裏画面に描画
    SetDrawScreen(DX_SCREEN_BACK);

    //入力
	Input& input = Input::GetInstance();
	input.Init();

    //フェード
    Fader& fader = Fader::GetInstance();
    fader.Init();

    //シーン
	SceneController* sceneController = new SceneController;
    

    while (ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0)
    {
        //今回のループが始まった時間を覚えておく
        LONGLONG time = GetNowHiPerformanceCount();

        // 画面をクリア
        ClearDrawScreen();

        //入力の更新
		input.Update();

		//更新
		sceneController->Update();
        fader.Update();


		//描画
		sceneController->Draw();
		fader.Draw();

        //マウスの座標
		auto mousePos = input.GetMousePos();

        // 描画内容を画面に反映
        ScreenFlip();

        //FPSを60に固定
        while (GetNowHiPerformanceCount() - time < 16667)
        {

        }
    }

    delete sceneController;
	sceneController = nullptr;

    // DXライブラリ終了
    DxLib_End();

    return 0;
}