//returnhome.h
#pragma once
#include "Box2D\Box2D.h"
#include "SFML\Graphics.hpp"

class returnhome
{
public:
	returnhome(b2Vec2 start, b2Vec2 end, b2Body& body);
	//move towards and calculate distance. when distance is less than 0.1
	//stop motion, return return home value to false

	~returnhome();

	void BackToRespawn();

private:
	b2Vec2 endPos;
	b2Vec2 startPos;

};