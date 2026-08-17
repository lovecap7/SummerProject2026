#pragma once
#include "MonsterBase.h"
class Takenoko :
    public MonsterBase
{
public:
    Takenoko();
    ~Takenoko();
    virtual void Init() override;
	virtual void Draw() const override;

};

