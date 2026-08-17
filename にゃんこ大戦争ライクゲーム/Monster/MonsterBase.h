#pragma once
#include "../General/Math/Vector2.h"
#include "../General/Game.h"
#include <memory>
#include <vector>
class House;
class MonsterBase abstract
{
public:

	MonsterBase(Game::Team team);
	~MonsterBase() = default;
	virtual void Init() abstract;
	virtual void Update();
	virtual void Draw()const abstract;
	void Damage(int damage);
	bool IsDead() const { return m_isDead; }

	//座標
	Vector2 GetPos() const { return m_pos; }
	
	//攻撃範囲に入ったかチェック
	void CheckAttackRange(const std::vector<std::shared_ptr<MonsterBase>>& targetList,const std::weak_ptr<House> targetHouse);

protected:
	//チーム
	Game::Team m_team = Game::Team::Player;

	//体力
	int m_hp = 0;
	//攻撃力
	int m_attack = 0;
	//射程
	float m_range = 0.0f;
	//攻撃速度
	float m_attackSpeed = 0.0f;
	//攻撃の間隔を計るタイマー
	float m_attackTimer = 0.0f;
	//座標
	Vector2 m_pos = Vector2(0, 0);
	//移動量
	float m_speed = 0.0f;
	//死亡
	bool m_isDead = false;

	//現在のアニメーション
	int m_currentAnim = -1;

	//歩きアニメーション
	int m_walkAnim = -1;
	//攻撃アニメーション
	int m_attackAnim = -1;
	//やられアニメーション
	int m_hitAnim = -1;
	//アニメーション用のタイマー
	int m_animationTimer = 0;
	//アニメーション番号
	int m_animationIndex = 0;
	//アニメーション速度
	int m_animationSpeed = 5;
	//ヒットエフェクトの画像
	int m_hitEffectImage = -1;

	//歩きアニメーションのフレーム数
	int m_walkAnimFrameCount = 0;
	//攻撃アニメーションのフレーム数
	int m_attackAnimFrameCount = 0;
	//やられアニメーションのフレーム数
	int m_hitAnimFrameCount = 0;

	//死ぬまでのタイマー
	int m_deadTimer = 0;

	//状態遷移用の関数ポインタ
	using UpdateFunc = void(MonsterBase::*)();
	UpdateFunc m_updateFunc = nullptr;
	
protected:
	//歩き状態の更新
	virtual void UpdateWalk();
	//攻撃状態の更新
	virtual void UpdateAttack();
	//やられ状態の更新
	virtual void UpdateHit();

	//アニメーション再生
	void PlayAnim(int animMaxFrame, bool isLoop);
};

