//FollowBall.h
#pragma once
#include "AIStates.h"
#include "Stalker.h"

#include "Thor/Shapes/ConcaveShape.hpp"
#include "Thor/Shapes/Shapes.hpp"
#include "Thor/Math/Random.hpp"
#include "Thor/Input.hpp"
#include "Thor/Input/ActionMap.hpp"
#include "Thor/Math/Trigonometry.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <Windows.h>
#include <stdio.h>
#include <assert.h>

#include "Box2D/Box2D.h"

class Stalker;

class FollowBall : public AIState<Stalker>
{
private:
	FollowBall() {}
public:
	static FollowBall* Instance();
	void Enter(Stalker* p_stalker){}
	void Execute(Stalker* p_stalker);
	void Exit(Stalker* p_stalker){}

	~FollowBall(){}
};