//FollowBall.cpp

#include "AIStates.h"
#include "FollowBall.h"
#include "Stalker.h"

FollowBall* FollowBall::Instance()
{
	static FollowBall Instance;

	return &Instance;
}

void FollowBall::Execute(Stalker* p_stalker)
{
	p_stalker->Follow(p_stalker->getPlayerPosition());
}