//returnhome.cpp

#include "returnhome.h"
#include "Box2D\Box2D.h"

returnhome::returnhome(b2Vec2 start, b2Vec2 end, b2Body& body)
{
	body.ApplyForce(b2Vec2(start - end), body.GetWorldCenter(), true);
	if (body.GetLinearVelocity().Length() <= 0.5f)
	{
		
	}
}