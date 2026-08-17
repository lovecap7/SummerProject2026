#pragma once
#include "MonsterBase.h"
class TankMushroom :
    public MonsterBase
{
public:
    TankMushroom();
    ~TankMushroom();
    virtual void Init() override;
    virtual void Draw() const override;
};

