//Stalker.h
#pragma once
#include "AIStateMachine.h"
#include "AIStates.h"
#include "FollowBall.h"

#include "SFML/Graphics.hpp"
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

class FollowBall;
//template class AIStateMachine<Stalker>;
class Stalker
{
public:
	Stalker();

	void Update();
	

	void Follow(sf::Vector2f p_vPlayerPos);


	void setPlayerPosition(sf::Vector2f vec)		{ playerPosition = vec; }
	sf::Vector2f getPlayerPosition()				{ return playerPosition; }

	void setPosition(sf::Vector2f vec)				{ position = vec; }
	sf::Vector2f getPosition()						{ return position; }

	void setvelocity(sf::Vector2f vec)				{ velocity = vec; }
	sf::Vector2f getvelocity()						{ return velocity; }

	sf::Vector2f getUnitVector()					{ return vNorm; }
	
	//Pallade inte göra fler accessors *_*
	sf::CircleShape shape;
	b2Body* body;
private:
	sf::Vector2f position, velocity, playerPosition;
	sf::Vector2f vNorm;

	AIStateMachine<Stalker>* FSM;

	
};