#include "MonsterBase.h"
#include "../Game/House.h"
#include <algorithm>
#include <DxLib.h>

namespace
{
	//アニメーションのフレーム数
	constexpr int kAnimationFrameCount = 5;

	//死亡までのフレーム
	constexpr int kDeadFrameCount = 40;

	//死亡時の吹っ飛ばし量
	constexpr float kDeadFlyAmount = 5.0f;

	//死亡時のふっとばし曲線の周期
	constexpr float kDeadFlyPeriod = 0.1f;

	//死亡時の吹っ飛ばしの速度
	constexpr float kDeadFlySpeed = 2.0f;
}

void MonsterBase::Damage(int damage)
{
	m_hp = (std::max)(0, m_hp - damage);
}

void MonsterBase::CheckAttackRange(const std::vector<std::shared_ptr<MonsterBase>>& targetList, const std::weak_ptr<House> targetHouse)
{
	if (m_hp <= 0)return;

	for(auto& target : targetList)
	{
		if(target->IsDead())continue;
		float distance = std::abs(m_pos.x - target->GetPos().x);
		if(distance <= m_range)
		{
			//攻撃対象が範囲内にいる場合、攻撃状態に遷移
			if(m_updateFunc != &MonsterBase::UpdateAttack)
			{
				m_animationIndex = 0;
				m_attackTimer = 0;
				m_updateFunc = &MonsterBase::UpdateAttack;
			}
			//攻撃速度に達したら攻撃
			if (m_attackTimer >= m_attackSpeed)
			{
				m_attackTimer = 0;
				target->Damage(m_attack);
			}
			return;
		}
	}

	//家の攻撃範囲チェック
	if (!targetHouse.expired())
	{
		auto house = targetHouse.lock();
		float distance = std::abs(m_pos.x - house->GetPos().x);
		if (distance <= m_range)
		{
			//攻撃対象が範囲内にいる場合、攻撃状態に遷移
			if (m_updateFunc != &MonsterBase::UpdateAttack)
			{
				m_animationIndex = 0;
				m_attackTimer = 0;
				m_updateFunc = &MonsterBase::UpdateAttack;
			}
			//攻撃速度に達したら攻撃
			if (m_attackTimer >= m_attackSpeed)
			{
				m_attackTimer = 0;
				house->Damage(m_attack);
			}
			return;
		}
	}

	//攻撃対象がいない場合、歩き状態に遷移
	if (m_updateFunc != &MonsterBase::UpdateWalk && m_animationIndex <= 0)
	{
		m_updateFunc = &MonsterBase::UpdateWalk;
	}
}

MonsterBase::MonsterBase(Game::Team team):
	m_team(team),
	m_updateFunc(&MonsterBase::UpdateWalk)
{
}

void MonsterBase::Update()
{
	if(m_hp <= 0 && m_updateFunc != &MonsterBase::UpdateHit)
	{
		m_animationIndex = 0;
		m_deadTimer = 0;
		m_updateFunc = &MonsterBase::UpdateHit;
	}

	(this->*m_updateFunc)();
}

void MonsterBase::UpdateWalk()
{
	///現在のアニメーションを歩きアニメーションに設定
	m_currentAnim = m_walkAnim;

	///アニメーション
	PlayAnim(m_walkAnimFrameCount, true);

	///移動

	float speed = m_speed;
	//Playerチームなら左方向に移動
	if(m_team == Game::Team::Player)
	{
		speed *= -1;
	}
	//移動
	m_pos.x += speed;
}

void MonsterBase::UpdateAttack()
{
	//攻撃アニメーションを再生
	m_currentAnim = m_attackAnim;

	///アニメーション
	PlayAnim(m_attackAnimFrameCount, true);

	//攻撃カウント
	m_attackTimer++;
}

void MonsterBase::UpdateHit()
{
	//やられアニメーションを再生
	m_currentAnim = m_hitAnim;

	//アニメーション
	PlayAnim(m_hitAnimFrameCount, false);

	//タイマー
	m_deadTimer++;

	if(m_deadTimer >= kDeadFrameCount)
	{
		m_isDead = true;
		return;
	}

	//1秒ふっとばされる
	float speed = m_speed;
	//Enemyチームなら左方向に移動
	if (m_team == Game::Team::Enemy)
	{
		speed *= -1;
	}
	//移動
	m_pos.x += speed * kDeadFlySpeed;
	m_pos.y -= kDeadFlyAmount * std::sin(m_deadTimer * kDeadFlyPeriod);

}

void MonsterBase::PlayAnim(int animMaxFrame, bool isLoop)
{
	///アニメーション

	m_animationTimer++;

	//m_animationSpeedフレームごとに次の画像へ
	if (m_animationTimer >= m_animationSpeed)
	{
		m_animationTimer = 0;

		m_animationIndex++;

		//ループ再生
		if (m_animationIndex >= animMaxFrame)
		{
			if (isLoop)
			{
				m_animationIndex = 0;
			}
			else
			{
				m_animationIndex = animMaxFrame - 1;
			}
		}
	}
}
