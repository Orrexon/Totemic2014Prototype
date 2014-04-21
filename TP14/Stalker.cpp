#include "Stalker.h"

Stalker::Stalker()
{
	position = { 0.f, 0.f };
	velocity = { 0.f, 0.f };
	vNorm = { 0.f, 0.f };

	playerPosition = { 0.f, 0.f };

	FSM = new AIStateMachine<Stalker>(this);
	FSM->SetCurrentState(FollowBall::Instance());
}

void Stalker::Update()
{
	FSM->Update();
}
void Stalker::Follow(sf::Vector2f p_vPlayerPos)
{
	sf::Vector2f vDistance = p_vPlayerPos - position;
	float fDistance = vDistance.x * vDistance.x + vDistance.y * vDistance.y;
	vNorm = vDistance / sqrtf(fDistance);
	setvelocity(vNorm * 4.f);
}