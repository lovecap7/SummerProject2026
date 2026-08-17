#pragma onece
#include "../General/Math/Vector2.h"
class HitEffect
{
public:
	HitEffect(Vector2 pos);
	~HitEffect();
	void Init();
	void Update();
	void Draw() const;

private:
	static int m_hitEffectImage;
	int m_animIndex = 0;
	int m_animSpeed = 2;

};