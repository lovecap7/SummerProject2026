#pragma once
#include "SceneBase.h"
#include "../General/Math/Vector2.h"
#include <memory>
#include <vector>

class Input;
class SceneController;
class House;
class MonsterBase;

class GameScene :
    public SceneBase
{
public:
    GameScene(SceneController& controller);
    ~GameScene();
    /// <summary>
    /// 派生クラスで実装を実装
    /// virtual はこれを継承するかもしれないから
    /// overrideは明示的にオーバーライドをエディタに示すため
    /// </summary>
    virtual void Init()override;
    virtual void Update() override;
    virtual void Draw() override;
    virtual void End() override;
    virtual void Restart() override;
    //デバッグ用
    virtual void DebugDraw() const override;
  
private:
    //ボタン座標
	std::vector<Vector2> m_buttonPosList;
    //ボタン画像
	std::vector<int> m_buttonImageList;

    //背景
	int m_backgroundImage = -1;

    //プレイヤーモンスター
	std::vector<std::shared_ptr<MonsterBase>> m_playerMonsterList;

	//敵モンスター
	std::vector<std::shared_ptr<MonsterBase>> m_enemyMonsterList;

    //プレイヤーの家
	std::shared_ptr<House> m_playerHouse;
	//敵の家
	std::shared_ptr<House> m_enemyHouse;

    enum class MonsterIndex:int
    {
        Mushroom1,
        Mushroom2,
        Mushroom3,
        Mushroom4,
        MaxNum
	};

    //敵出現フレーム
	int m_enemySpawnFrame = 0;

    //敵1の出現間隔
	int m_enemy1SpawnInterval = 300;

private:
	//どのボタンが押されたかチェック
    int CheckPushButton();

	//プレイヤーのモンスターを生成
	void CreatePlayerMonster(MonsterIndex index);

	//敵モンスターを生成
	void CreateEnemyMonster();
};

