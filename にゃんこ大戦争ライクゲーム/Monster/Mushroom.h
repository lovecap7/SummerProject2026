#pragma once
#include "MonsterBase.h"
class Mushroom :
    public MonsterBase
{
public:
    Mushroom();
    ~Mushroom();
    virtual void Init() override;
	virtual void Draw() const override;
};

