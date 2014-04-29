#include "SFML/Graphics.hpp"

#include <sstream>
#include "Thor/Shapes/ConcaveShape.hpp"
#include "Thor/Shapes/Shapes.hpp"
#include "Thor/Math/Random.hpp"
#include "Thor/Input.hpp"
#include "Thor/Input/ActionMap.hpp"
#include "Thor/Math/Trigonometry.hpp"
#include "Thor\Time.hpp"
#include "Thor\Animation.hpp"
#include "Thor\Particles.hpp"
#include "Thor\Math.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <Windows.h>
#include <stdio.h>
#include <assert.h>
#include <xmmintrin.h>
#include <math.h>

#include "Box2D/Box2D.h"
#include "manymouse.hpp"
#include "AudioSystem.h"

#include "Stalker.h"
#include "Box2D\Dynamics\b2Fixture.h"
#include "Box2DWorldDraw.h"
class b2Fixture;

//GLOBAL!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
sf::Vector2f def_coll_pos = { 0, 0 };
sf::Vector2f def_coll_dir = { 0, 0 };
sf::Vector2f deflect_coll_dir = { 0.f, 0.f };
b2Vec2 gameToPhysicsUnits(sf::Vector2f p_unit)
{
	return b2Vec2(p_unit.x / 32.f, p_unit.y / 32.f);
}
b2Vec2 gameToPhysicsUnits(sf::Vector2i p_unit)
{
	return b2Vec2(static_cast<float>(p_unit.x) / 32.f, static_cast<float>(p_unit.y) / 32.f);
}

float gameToPhysicsUnits(float p_unit)
{
	return p_unit / 32.f;
}

sf::Vector2f physicsToGameUnits(float p_x, float p_y)
{
	return sf::Vector2f(p_x * 32.f, p_y * 32.f);
}
sf::Vector2f physicsToGameUnits(b2Vec2 p_position)
{
	return sf::Vector2f(p_position.x * 32.f, p_position.y * 32.f);
}
void createWall(b2World* p_world, sf::Vector2f p_vec1, sf::Vector2f p_vec2)
{
	// The body
	b2BodyDef bodyDef;
	bodyDef.position = b2Vec2(0, 0);
	bodyDef.type = b2_staticBody;
	bodyDef.angle = 0;
	b2Body* body = p_world->CreateBody(&bodyDef);

	// The shape
	b2EdgeShape shape;
	b2Vec2 v1 = gameToPhysicsUnits(p_vec1);
	b2Vec2 v2 = gameToPhysicsUnits(p_vec2);
	shape.Set(v1, v2);

	// The fixture
	b2FixtureDef fixtureDef;
	fixtureDef.density = 1;
	fixtureDef.shape = &shape;
	fixtureDef.friction = 1;
	fixtureDef.restitution = 1;
	body->CreateFixture(&fixtureDef);
}
struct PlayerEntity
{
	std::string type;
	Audiosystem* m_audioSystem;
	b2Body* gatherer_body;
	b2Body* defender_body;
	PlayerEntity* m_link;

	sf::Vector2f m_textposition;
	bool IsGatheringTime = false;
	sf::CircleShape gatherer;
	sf::CircleShape defender;
	b2Vec2 m_respawn_pos;
	void SetLink(PlayerEntity* link)
	{
		m_link = link;
	}
	bool GathererIsHit = false;
	bool Particle = false;
	bool HasShield = false;
	bool defenderDeflected = false;
	bool Lightning = false;
	bool stunned;
	thor::StopWatch deflectiontimer;
};
struct Gatherer : public PlayerEntity
{

	sf::Sprite m_sprite;
	sf::Sprite m_animation;

};
struct Defender : public PlayerEntity
{
	sf::Sprite m_sprite;
	sf::Sprite m_animation;
	b2Vec2 previousVel, currentVel, DVel;


};
struct Player
{
	Player()
	{
		timer.reset();
		timer.stop();
	}
	thor::StopWatch timer;
	sf::CircleShape defender;
	sf::CircleShape gatherer;

	Gatherer* m_gatherer;
	Defender* m_defender;

	sf::Color* GetStartColor() { return m_Start_color; }
	sf::Color* m_Start_color;
	sf::Texture* texture;

};

struct Shield
{
	sf::Sprite shieldSprite;
	sf::CircleShape shieldShape;
	float shieldRadius;
	sf::Vector2f shieldPosition;
	bool taken = false;
	thor::StopWatch shieldTimer;
};

class Lightning
{
public:
	sf::Sprite lightningSprite;
	sf::CircleShape lightningShape;
	float lightningRadius;
	sf::Vector2f lightningPosition;
	thor::StopWatch respawnTimer;
	bool doDraw = true;
};
void SetStartingColors(std::vector<Player*> players, int value)
{
	int i = value;
	if (0 < i)
	{
		players[0]->m_gatherer->gatherer.setFillColor(sf::Color::Blue); players[0]->m_Start_color = new sf::Color(players[0]->m_gatherer->gatherer.getFillColor());
		players[0]->m_defender->defender.setFillColor(players[0]->m_gatherer->gatherer.getFillColor());
	}
	if (1 < i)
	{
		players[1]->m_gatherer->gatherer.setFillColor(sf::Color::Red); players[1]->m_Start_color = new sf::Color(players[1]->m_gatherer->gatherer.getFillColor());
		players[1]->m_defender->defender.setFillColor(players[1]->m_gatherer->gatherer.getFillColor());
	}
	if (2 < i)
	{
		players[2]->m_gatherer->gatherer.setFillColor(sf::Color::Magenta); players[2]->m_Start_color = new sf::Color(players[2]->m_gatherer->gatherer.getFillColor());
		players[2]->m_defender->defender.setFillColor(players[2]->m_gatherer->gatherer.getFillColor());
	}
	if (3 < i)
	{
		players[3]->m_gatherer->gatherer.setFillColor(sf::Color::Cyan); players[3]->m_Start_color = new sf::Color(players[3]->m_gatherer->gatherer.getFillColor());
		players[3]->m_defender->defender.setFillColor(players[3]->m_gatherer->gatherer.getFillColor());
	}
}

Stalker* CreateStalker(b2World* world)
{
	Stalker* stalker = new Stalker;
	stalker->shape.setFillColor(sf::Color::Blue);
	stalker->shape.setRadius(30.f);
	stalker->shape.setOrigin(30.f, 30.f);
	stalker->shape.setPosition(sf::Vector2f(thor::random(300.f, 600.f), thor::random(300.f, 600.f)));

	b2BodyDef bodyDef;
	bodyDef.position = gameToPhysicsUnits(stalker->shape.getPosition());
	bodyDef.type = b2_dynamicBody;
	bodyDef.linearDamping = 0.3f;
	bodyDef.userData = stalker;
	b2Body* body = world->CreateBody(&bodyDef);

	b2CircleShape shape1;
	shape1.m_radius = gameToPhysicsUnits(stalker->shape.getRadius());

	b2FixtureDef fixtureDef;
	fixtureDef.density = 1;
	fixtureDef.shape = &shape1;
	fixtureDef.friction = 0.3f;
	fixtureDef.restitution = 1;
	body->CreateFixture(&fixtureDef);
	stalker->body = body;

	return stalker;
}



void DefenderDirection(sf::Sprite);

class WorldContactListener : public b2ContactListener
{
	void BeginContact(b2Contact* p_contact)
	{
		playerContactBegin(p_contact);
	}

	void EndContact(b2Contact* p_contact)
	{
		playerContactEnd(p_contact);
	}

	void playerContactBegin(b2Contact* p_contact)
	{
		b2Fixture* fixtureA = p_contact->GetFixtureA();
		b2Fixture* fixtureB = p_contact->GetFixtureB();


		void* bodyUserDataA = fixtureA->GetBody()->GetUserData();
		void* bodyUserDataB = fixtureB->GetBody()->GetUserData();

		if (bodyUserDataA != nullptr && bodyUserDataB != nullptr)
		{

			PlayerEntity* playerA = static_cast<PlayerEntity*>(bodyUserDataA);
			PlayerEntity* playerB = static_cast<PlayerEntity*>(bodyUserDataB);
			playerA->m_audioSystem->playSound("Ball_Contact");
			if (playerA->type == "defender" && playerB->type == "gatherer")
			{
				if (playerA->m_link != playerB)
				{
					std::cout << "YOU ARE MY SWORN ENEMY!!!!!" << std::endl;
					if (!playerB->HasShield)
					{
						playerB->GathererIsHit = true;
					}
					else if (playerB->HasShield)
					{
						deflect_coll_dir = physicsToGameUnits(playerA->defender_body->GetWorldCenter() - playerB->gatherer_body->GetWorldCenter());
						float lenght = sqrtf(deflect_coll_dir.x*deflect_coll_dir.x + deflect_coll_dir.y*deflect_coll_dir.y);
						deflect_coll_dir /= lenght;
						playerA->defenderDeflected = true;
						playerA->deflectiontimer.restart();
					}

				}
				else if (playerA->m_link == playerB)
				{
					std::cout << "YOU ARE MY FRIEND?????" << std::endl;
				}

			}
			else if (playerB->type == "defender" && playerA->type == "gatherer")
			{
				if (playerB->m_link != playerA)
				{
					std::cout << "!!!!!!!!!!!!YOU ARE MY SWORN ENEMY!!!!!" << std::endl;
					if (!playerA->HasShield)
					{
						playerA->GathererIsHit = true;
					}
					else if (playerA->HasShield)
					{
						deflect_coll_dir = physicsToGameUnits(playerB->defender_body->GetWorldCenter() - playerA->gatherer_body->GetWorldCenter());
						float lenght = sqrtf(deflect_coll_dir.x*deflect_coll_dir.x + deflect_coll_dir.y*deflect_coll_dir.y);
						deflect_coll_dir /= lenght;
						playerB->defenderDeflected = true;
						playerB->deflectiontimer.restart();
					}
				}
				else if (playerB->m_link == playerA)
				{
					std::cout << "!!!!!!!!!!!!!!!YOU ARE MY FRIEND?????" << std::endl;
				}
			}
			else if (playerA->type == "defender" && playerB->type == "defender")
			{
				playerA->Particle = true;
				def_coll_dir = physicsToGameUnits(playerB->defender_body->GetWorldCenter() - playerA->defender_body->GetWorldCenter());
				float Lenght = sqrtf(def_coll_dir.x*def_coll_dir.x + def_coll_dir.y*def_coll_dir.y);
				def_coll_pos = physicsToGameUnits(playerA->defender_body->GetWorldCenter());
				def_coll_dir /= Lenght;
				def_coll_pos += def_coll_dir * 60.f;

			}
			else if (playerB->type == "defender" && playerA->type == "defender")
			{
				playerB->Particle = true;
				def_coll_dir = physicsToGameUnits(playerA->defender_body->GetWorldCenter() - playerB->defender_body->GetWorldCenter());
				float Lenght = sqrtf(def_coll_dir.x*def_coll_dir.x + def_coll_dir.y*def_coll_dir.y);
				def_coll_pos = physicsToGameUnits(playerB->defender_body->GetWorldCenter());
				def_coll_dir /= Lenght;
				def_coll_pos += def_coll_dir * 60.f;
			}
		}
	}

	void playerContactEnd(b2Contact* p_contact)
	{
		b2Fixture* fixtureA = p_contact->GetFixtureA();
		b2Fixture* fixtureB = p_contact->GetFixtureB();

		void* bodyUserDataA = fixtureA->GetBody()->GetUserData();
		void* bodyUserDataB = fixtureB->GetBody()->GetUserData();

		if (bodyUserDataA != nullptr && bodyUserDataB != nullptr)
		{
			PlayerEntity* playerA = static_cast<PlayerEntity*>(bodyUserDataA);
			PlayerEntity* playerB = static_cast<PlayerEntity*>(bodyUserDataB);

		}
	}
};

//FFW Declare
float DefenderVelocity(b2Vec2 vel1, b2Vec2 vel2);

int main(int argc, char *argv[])
{
	for (int i = 0; i < argc; i++)
	{
		std::cout << "Argument " << i << ": " << argv[i] << std::endl;
	}


	sf::Texture t;
	t.loadFromFile("../d.psd");
	sf::Sprite s;
	s.setTexture(t);

	int numDevices = ManyMouse_Init();

	printf("Driver: %s\n", ManyMouse_DriverName());
	for (int i = 0; i < numDevices; i++)
	{
		std::cout << "Mouse " << i << ": " << ManyMouse_DeviceName(i) << std::endl;
	}

	//Sound & music
	Audiosystem* audioSystem = new Audiosystem();
	audioSystem->createSound("Ball_Contact", "../assets/sound/test.ogg");
	audioSystem->createMusic("Music_one", "../assets/sound/mamaAfrica.wav");
	audioSystem->playMusic("Music_one", true);

	// load textures
	sf::Texture cursor;
	cursor.loadFromFile("../cursor.png");

	//background
	sf::Texture backgroundTex = sf::Texture();
	backgroundTex.loadFromFile("../assets/png/masker-mot-bakgrund.png", sf::IntRect(0, 0, 1920, 1080));
	sf::Sprite background = sf::Sprite(backgroundTex);

	//stone
	sf::Texture stone = sf::Texture();
	stone.loadFromFile("../assets/png/stone.png", sf::IntRect(0, 0, 194, 121));
	sf::Sprite s_stone1 = sf::Sprite(stone);
	s_stone1.setPosition(sf::Vector2f(thor::random(100.f, 1000.f), thor::random(100.f, 1000.f)));
	sf::Sprite s_stone2 = sf::Sprite(stone);
	s_stone2.setPosition(sf::Vector2f(thor::random(100.f, 1000.f), thor::random(100.f, 1000.f)));
	sf::Sprite s_stone3 = sf::Sprite(stone);
	s_stone3.setPosition(sf::Vector2f(thor::random(100.f, 1000.f), thor::random(100.f, 1000.f)));
	std::vector<sf::Sprite> stones;
	stones.push_back(s_stone1);
	stones.push_back(s_stone2);
	stones.push_back(s_stone3);

	//GUI
	sf::Texture GUI = sf::Texture();
	GUI.loadFromFile("../assets/png/gui2.png", sf::IntRect(0, 0, 1920, 1080));
	sf::Sprite s_GUI = sf::Sprite(GUI);

	//font
	sf::Font font; font.loadFromFile("../assets/BRLNSR.TTF");
	sf::Vector2f redsText(10.f, 580.f);
	sf::Vector2f bluesText(10.f, 810.f);
	sf::Vector2f yellowsText(10.f, 350.f);
	sf::Vector2f purplesText(10.f, 100.f);
	std::vector<sf::Vector2f> textpositions;
	textpositions.push_back(redsText);
	textpositions.push_back(bluesText);
	textpositions.push_back(yellowsText);
	textpositions.push_back(purplesText);

	//Red gatherer and defender
	sf::Texture red_g = sf::Texture();
	sf::Texture red_d = sf::Texture();
	red_g.loadFromFile("../assets/png/red_g.png", sf::IntRect(0, 0, 163, 164));
	red_d.loadFromFile("../assets/png/red_d.png", sf::IntRect(0, 0, 315, 336));

	//blue gatherer and defender
	sf::Texture blue_g = sf::Texture();
	sf::Texture blue_d = sf::Texture();
	blue_g.loadFromFile("../assets/png/blue_g.png", sf::IntRect(0, 0, 163, 164));
	blue_d.loadFromFile("../assets/png/blue_d.png", sf::IntRect(0, 0, 315, 336));

	//yellow gatherer and defender
	sf::Texture yellow_g = sf::Texture();
	sf::Texture yellow_d = sf::Texture();
	yellow_g.loadFromFile("../assets/png/yellow_tiny.png", sf::IntRect(0, 0, 163, 164));
	yellow_d.loadFromFile("../assets/png/yellow.png", sf::IntRect(0, 0, 315, 336));

	//purple gatherer and defender
	sf::Texture purple_g = sf::Texture();
	sf::Texture purple_d = sf::Texture();
	purple_g.loadFromFile("../assets/png/purple_g.png", sf::IntRect(0, 0, 163, 164));
	purple_d.loadFromFile("../assets/png/purple_d.png", sf::IntRect(0, 0, 315, 336));

	std::vector<sf::Sprite> g_sprites;
	std::vector<sf::Sprite> d_sprites;
	
	//the sprites
	sf::Sprite s_red_g = sf::Sprite(red_g);
	sf::Sprite s_red_d = sf::Sprite(red_d);
	/*s_red_g.setScale(0.75f,0.75f);
	s_red_d.setScale(0.10f, 0.10f);*/
	g_sprites.push_back(s_red_g);
	d_sprites.push_back(s_red_d);

	sf::Sprite s_blue_g = sf::Sprite(blue_g);
	sf::Sprite s_blue_d = sf::Sprite(blue_d);
	//s_blue_g.setScale(0.75f, 0.75f);
	//s_blue_d.setScale(0.75f, 0.75f);
	g_sprites.push_back(s_blue_g);
	d_sprites.push_back(s_blue_d);

	sf::Sprite s_yellow_g = sf::Sprite(yellow_g);
	sf::Sprite s_yellow_d = sf::Sprite(yellow_d);
	g_sprites.push_back(s_yellow_g);
	d_sprites.push_back(s_yellow_d);

	sf::Sprite s_purple_g = sf::Sprite(purple_g);
	sf::Sprite s_purple_d = sf::Sprite(purple_d);
	g_sprites.push_back(s_purple_g);
	d_sprites.push_back(s_purple_d);


	//Animations
	sf::Clock clock;
	sf::Texture texture;
	sf::Texture texture1;
	sf::Texture texture2;
	sf::Texture texture3;
	texture.loadFromFile("../assets/png/def_red.png", sf::IntRect(0, 0, 1792, 256));
	texture1.loadFromFile("../assets/png/def_blue.png", sf::IntRect(0, 0, 1792, 256));
	texture2.loadFromFile("../assets/png/def_yellow.png", sf::IntRect(0, 0, 1792, 256));
	texture3.loadFromFile("../assets/png/def_purple.png", sf::IntRect(0, 0, 1792, 256));
	sf::Sprite red_ani(texture);
	sf::Sprite blue_ani(texture1);
	sf::Sprite yellow_ani(texture2);
	sf::Sprite purple_ani(texture3);

	std::vector<sf::Sprite> animatedSprites;
	animatedSprites.push_back(red_ani);
	animatedSprites.push_back(blue_ani);
	animatedSprites.push_back(yellow_ani);
	animatedSprites.push_back(purple_ani);

	thor::FrameAnimation move;

	move.addFrame(0.1f, sf::IntRect(0, 0, 256, 256));
	move.addFrame(0.1f, sf::IntRect(256 + 1, 0, 256, 256));
	move.addFrame(0.1f, sf::IntRect(256 * 2 + 1, 0, 256, 256));
	move.addFrame(0.1f, sf::IntRect(256 * 3 + 1, 0, 256, 256));
	move.addFrame(0.1f, sf::IntRect(256 * 4 + 1, 0, 256, 256));
	move.addFrame(0.1f, sf::IntRect(256 * 5 + 1, 0, 256, 256));
	move.addFrame(0.1f, sf::IntRect(256 * 6 + 1, 0, 256, 256));
	thor::Animator<sf::Sprite, std::string> animation;
	animation.addAnimation("move", move, sf::seconds(0.5f));
	animation.playAnimation("move", true);

	//Gatherer Animations
	sf::Clock g_clock;
	sf::Texture gtexture;
	sf::Texture gtexture1;
	sf::Texture gtexture2;
	sf::Texture gtexture3;
	gtexture.loadFromFile("../assets/png/g_red.png", sf::IntRect(0, 0, 1532, 128));
	gtexture1.loadFromFile("../assets/png/g_blue.png", sf::IntRect(0, 0, 1532, 128));
	gtexture2.loadFromFile("../assets/png/g_yellow.png", sf::IntRect(0, 0, 1532, 128));
	gtexture3.loadFromFile("../assets/png/g_purple.png", sf::IntRect(0, 0, 1532, 128));
	sf::Sprite gred_ani(gtexture);
	sf::Sprite gblue_ani(gtexture1);
	sf::Sprite gyellow_ani(gtexture2);
	sf::Sprite gpurple_ani(gtexture3);

	std::vector<sf::Sprite> ganimatedSprites;
	ganimatedSprites.push_back(gred_ani);
	ganimatedSprites.push_back(gblue_ani);
	ganimatedSprites.push_back(gyellow_ani);
	ganimatedSprites.push_back(gpurple_ani);

	thor::FrameAnimation gmove;

	gmove.addFrame(0.04f, sf::IntRect(0, 0, 128, 128));
	gmove.addFrame(0.04f, sf::IntRect(129, 0, 128, 128));
	gmove.addFrame(0.04f, sf::IntRect(258, 0, 128, 128));
	gmove.addFrame(0.04f, sf::IntRect(387, 0, 128, 128));
	gmove.addFrame(0.04f, sf::IntRect(516, 0, 128, 128));
	gmove.addFrame(0.04f, sf::IntRect(645, 0, 128, 128));
	gmove.addFrame(0.04f, sf::IntRect(774, 0, 128, 128));
	gmove.addFrame(0.04f, sf::IntRect(903, 0, 128, 128));
	gmove.addFrame(0.04f, sf::IntRect(1032, 0, 128, 128));
	gmove.addFrame(0.04f, sf::IntRect(1161, 0, 128, 128));
	gmove.addFrame(0.04f, sf::IntRect(1290, 0, 128, 128));
	gmove.addFrame(0.04f, sf::IntRect(1419, 0, 128, 128));


	thor::Animator<sf::Sprite, std::string> ganimation;
	ganimation.addAnimation("gmove", gmove, sf::seconds(1.f));
	ganimation.playAnimation("gmove", true);


	//Particles
	sf::Texture particleTex;
	particleTex.loadFromFile("../assets/png/defender_collision.png", sf::IntRect(0, 0, 8, 8));
	thor::ParticleSystem partSys;
	partSys.setTexture(particleTex);

	thor::UniversalEmitter emitter;
	sf::Clock clockP;

	//shield
	Shield shield;
	shield.shieldPosition = sf::Vector2f(thor::random(200, 1800), thor::random(200, 900));
	shield.shieldRadius = 50.f;
	shield.shieldShape = sf::CircleShape(50);
	shield.shieldShape.setOrigin(shield.shieldRadius, shield.shieldRadius);
	shield.shieldShape.setFillColor(sf::Color::Cyan);
	shield.shieldShape.setPosition(shield.shieldPosition);

	//Lightning
	Lightning lightning;
	lightning.lightningPosition = sf::Vector2f(thor::random(200, 1800), thor::random(200, 900));
	lightning.lightningRadius = 50.f;
	lightning.lightningShape = sf::CircleShape(lightning.lightningRadius);
	lightning.lightningShape.setOrigin(lightning.lightningRadius, lightning.lightningRadius);
	lightning.lightningShape.setPosition(lightning.lightningPosition);
	lightning.lightningShape.setFillColor(sf::Color::Magenta);

	// Physics world
	b2Vec2 gravity(0.0f, 0.0f);
	b2World* world = new b2World(gravity);
	world->SetAllowSleeping(true); // Allow Box2D to exclude resting bodies from simulation


	WorldContactListener myContactListener;
	world->SetContactListener(&myContactListener);


	createWall(world, sf::Vector2f(0, 0), sf::Vector2f(1920, 0));
	createWall(world, sf::Vector2f(1920, 0), sf::Vector2f(1920, 1080));
	createWall(world, sf::Vector2f(1920, 1080), sf::Vector2f(0, 1080));
	createWall(world, sf::Vector2f(102, 1080), sf::Vector2f(102, 0));

	sf::CircleShape innerCircle;
	innerCircle.setRadius(200);
	innerCircle.setFillColor(sf::Color(255, 0, 0, 50));
	innerCircle.setOrigin(200, 200);
	innerCircle.setPosition(sf::Vector2f(1920 / 2, 1080 / 2));


	std::vector<sf::Vector2f> defenderPositions;
	std::vector<sf::Vector2f> gathererPositions;

	defenderPositions.emplace_back(102 + 100, 250 - 150);
	defenderPositions.emplace_back(1820, 250 - 150);
	defenderPositions.emplace_back(1820, 980);
	defenderPositions.emplace_back(102 + 100, 980);

	gathererPositions.emplace_back(102 + 50, 175 - 150);
	gathererPositions.emplace_back(1870, 175 - 150);
	gathererPositions.emplace_back(1870, 1030);
	gathererPositions.emplace_back(102 + 50, 1030);

	// create player
	std::vector<Player*> players;
	for (int i = 0; i < numDevices; i++)
	{
		Player* player = new Player();

		player->m_gatherer = new Gatherer();
		player->m_defender = new Defender();
		player->m_gatherer->SetLink(player->m_defender);
		player->m_defender->SetLink(player->m_gatherer);

		player->m_gatherer->m_textposition = textpositions[i];

		player->m_defender->m_audioSystem = audioSystem;
		player->m_gatherer->m_audioSystem = audioSystem;


		player->m_defender->defender.setFillColor(sf::Color::Green);
		player->m_defender->defender.setRadius(60.f);
		player->m_defender->defender.setOrigin(60.f, 60.f);
		player->m_defender->defender.setPosition(defenderPositions[i]);
		player->m_defender->m_respawn_pos = gameToPhysicsUnits(defenderPositions[i]);
		player->m_defender->m_sprite = d_sprites[i];
		player->m_defender->m_sprite.setOrigin(player->m_defender->m_sprite.getGlobalBounds().width / 2, player->m_defender->m_sprite.getGlobalBounds().height / 2);
		player->m_defender->m_animation = animatedSprites[i];
		player->m_defender->m_animation.setOrigin(128, 160);

		player->m_gatherer->gatherer.setFillColor(sf::Color::Green);
		player->m_gatherer->gatherer.setRadius(15.f);
		player->m_gatherer->gatherer.setOrigin(15.f, 15.f);
		player->m_gatherer->gatherer.setPosition(gathererPositions[i]);
		player->m_gatherer->m_respawn_pos = gameToPhysicsUnits(gathererPositions[i]);
		player->m_gatherer->m_sprite = g_sprites[i];
		player->m_gatherer->m_sprite.setOrigin(player->m_gatherer->m_sprite.getGlobalBounds().width / 2
			, player->m_gatherer->m_sprite.getGlobalBounds().top + player->m_gatherer->m_sprite.getGlobalBounds().height / 2);
		player->m_gatherer->m_animation = ganimatedSprites[i];
		player->m_gatherer->m_animation.setOrigin(128 / 2, 128 / 2);

		player->m_gatherer->type = "gatherer";
		//player->m_gatherer->gatherer = player->gatherer;

		player->m_defender->type = "defender";
		//player->m_defender->defender = player->defender;

		// Defender body
		{
			b2BodyDef bodyDef;
			bodyDef.position = gameToPhysicsUnits(player->m_defender->defender.getPosition());
			bodyDef.type = b2_dynamicBody;
			bodyDef.linearDamping = 0.3f;
			bodyDef.userData = player->m_defender;
			b2Body* body = world->CreateBody(&bodyDef);

			b2CircleShape shape;
			shape.m_radius = gameToPhysicsUnits(player->m_defender->defender.getRadius());

			b2FixtureDef fixtureDef;
			fixtureDef.density = 1;
			fixtureDef.shape = &shape;
			fixtureDef.friction = 0.3f;
			fixtureDef.restitution = 0.6;
			fixtureDef.userData = body->CreateFixture(&fixtureDef);
			player->m_defender->defender_body = body;
			player->m_defender->m_sprite.setPosition(player->m_defender->defender.getPosition());
		}

		// Gatherer body
		{
			b2BodyDef bodyDef;
			bodyDef.position = gameToPhysicsUnits(player->m_gatherer->gatherer.getPosition());
			bodyDef.type = b2_dynamicBody;
			bodyDef.linearDamping = 0.3f;
			bodyDef.userData = player->m_gatherer;
			b2Body* body = world->CreateBody(&bodyDef);

			b2CircleShape shape;
			shape.m_radius = gameToPhysicsUnits(player->m_gatherer->gatherer.getRadius());

			b2FixtureDef fixtureDef;
			fixtureDef.density = 1;
			fixtureDef.shape = &shape;
			fixtureDef.friction = 0.3f;
			fixtureDef.restitution = 0.6;
			body->CreateFixture(&fixtureDef);
			player->m_gatherer->gatherer_body = body;
			player->m_gatherer->m_sprite.setPosition(player->m_gatherer->gatherer.getPosition());
		}

		players.push_back(player);

	}
	//Setting the starting colors;
	SetStartingColors(players, numDevices);

	Stalker* stalker = CreateStalker(world);

	// Create render window
	sf::RenderWindow window(sf::VideoMode(1920, 1080), "Doodlemeat", sf::Style::None);
	window.setVerticalSyncEnabled(true);
	window.setMouseCursorVisible(false);
	window.setKeyRepeatEnabled(false);


	sf::Texture box;
	box.loadFromFile("../assets/png/box_32.png");
	box.setRepeated(true);

	thor::ActionMap<std::string> actionMap;
	actionMap["test"] = thor::Action(sf::Keyboard::X, thor::Action::PressOnce);
	actionMap["test2"] = thor::Action(sf::Mouse::Left, thor::Action::PressOnce);

	actionMap["p1_up"] = thor::Action(sf::Keyboard::/*N*/W, thor::Action::Hold);
	actionMap["p1_down"] = thor::Action(sf::Keyboard::/*Y*/S, thor::Action::Hold);
	actionMap["p1_left"] = thor::Action(sf::Keyboard::/*U*/A, thor::Action::Hold);
	actionMap["p1_right"] = thor::Action(sf::Keyboard::/*V*/D, thor::Action::Hold);

	actionMap["p2_up"] = thor::Action(sf::Keyboard::/*Down*/Up, thor::Action::Hold);
	actionMap["p2_down"] = thor::Action(sf::Keyboard::/*Up*/Down, thor::Action::Hold);
	actionMap["p2_left"] = thor::Action(sf::Keyboard::/*Right*/Left, thor::Action::Hold);
	actionMap["p2_right"] = thor::Action(sf::Keyboard::/*Left*/Right, thor::Action::Hold);

	/*actionMap["p3_up"] = thor::Action(sf::Keyboard::K, thor::Action::Hold);
	actionMap["p3_down"] = thor::Action(sf::Keyboard::I, thor::Action::Hold);
	actionMap["p3_left"] = thor::Action(sf::Keyboard::L, thor::Action::Hold);
	actionMap["p3_right"] = thor::Action(sf::Keyboard::J, thor::Action::Hold);*/

	actionMap["stalker"] = thor::Action(sf::Keyboard::Z, thor::Action::Hold);

	thor::ActionMap<std::string> actionMap2;
	actionMap2["test3"] = thor::Action(sf::Mouse::Left, thor::Action::PressOnce);
	bool win = false;
	float fDeltaTime;
	float CoolDown = 0;
	sf::Clock m_Clock;
	while (window.isOpen())
	{

		sf::Time m_TimeSinceLastUpdate = sf::Time::Zero;
		m_TimeSinceLastUpdate += m_Clock.restart();

		fDeltaTime = m_TimeSinceLastUpdate.asSeconds();

		if (fDeltaTime > 0.1f)
		{
			fDeltaTime = 0.1f;
		}
		//CoolDown += fDeltaTime;
		//std::cout << fDeltaTime << std::endl;

		

		for (auto player : players)
		{
			b2Vec2 velo(player->m_gatherer->gatherer_body->GetLinearVelocity().x *0.89f,
				player->m_gatherer->gatherer_body->GetLinearVelocity().y *0.89f);
			player->m_gatherer->gatherer_body->SetLinearVelocity(velo);
		}

		audioSystem->update();
		world->Step(1 / 60.f, 8, 3);
		actionMap.update(window);
		actionMap2.update(window);
		for (auto i : players)
		{
			i->m_defender->currentVel = i->m_defender->defender_body->GetLinearVelocity();
		}
		
		//Shield
		for (auto player : players)
		{
			float distance = std::sqrtf(
				(player->m_gatherer->gatherer.getPosition().x - shield.shieldShape.getPosition().x) *
				(player->m_gatherer->gatherer.getPosition().x - shield.shieldShape.getPosition().x) +
				(player->m_gatherer->gatherer.getPosition().y - shield.shieldShape.getPosition().y) *
				(player->m_gatherer->gatherer.getPosition().y - shield.shieldShape.getPosition().y));
			if (distance <= shield.shieldShape.getRadius())
			{
				shield.taken = true;
				player->m_gatherer->HasShield = true;
			}
		}

		//Lightning
		for (auto player : players)
		{
			float distance = std::sqrtf(
				(player->m_gatherer->gatherer.getPosition().x - lightning.lightningShape.getPosition().x) *
				(player->m_gatherer->gatherer.getPosition().x - lightning.lightningShape.getPosition().x) +
				(player->m_gatherer->gatherer.getPosition().y - lightning.lightningShape.getPosition().y) *
				(player->m_gatherer->gatherer.getPosition().y - lightning.lightningShape.getPosition().y));
			if (distance <= lightning.lightningShape.getRadius())
			{
				
				lightning.respawnTimer.restart();
				lightning.doDraw = false;
			}
		}
		ManyMouseEvent event;
		while (ManyMouse_PollEvent(&event))
		{
			Player* player = players[event.device];

			if (event.type == MANYMOUSE_EVENT_RELMOTION)
			{
				sf::Vector2f playerPosition = player->defender.getPosition();
				if (event.item == 0)
				{
					player->m_defender->defender_body->ApplyLinearImpulse(b2Vec2(6.0f * gameToPhysicsUnits(static_cast<float>(event.value)), 0.f), player->m_defender->defender_body->GetWorldCenter(), true);
				}
				if (event.item == 1)
				{
					player->m_defender->defender_body->ApplyLinearImpulse(b2Vec2(0.f, 6.0f * gameToPhysicsUnits(static_cast<float>(event.value))), player->m_defender->defender_body->GetWorldCenter(), true);
				}
			}
		}
		for (auto &player : players)
		{
			if (!player->m_gatherer->GathererIsHit)
			{
				if (actionMap.isActive("p1_up"))
				{
					//players[0]->m_gatherer->gatherer_body->SetLinearVelocity(b2Vec2(0.f, -8.f));
					players[0]->m_gatherer->gatherer_body->ApplyLinearImpulse(b2Vec2(0.f, gameToPhysicsUnits(-15.f)), players[0]->m_gatherer->gatherer_body->GetWorldCenter(), true);
				}
				if (actionMap.isActive("p1_down"))
				{
					players[0]->m_gatherer->gatherer_body->ApplyLinearImpulse(b2Vec2(0.f, gameToPhysicsUnits(15.f)), players[0]->m_gatherer->gatherer_body->GetWorldCenter(), true);
					//players[0]->m_gatherer->gatherer_body->SetLinearVelocity(b2Vec2(players[0]->m_gatherer->gatherer_body->GetLinearVelocity().x, 8.f));
				}
				if (actionMap.isActive("p1_left"))
				{
					players[0]->m_gatherer->gatherer_body->ApplyLinearImpulse(b2Vec2(gameToPhysicsUnits(-15.f), 0.f), players[0]->m_gatherer->gatherer_body->GetWorldCenter(), true);
					//players[0]->m_gatherer->gatherer_body->SetLinearVelocity(b2Vec2(-8.f, players[0]->m_gatherer->gatherer_body->GetLinearVelocity().y));
				}
				if (actionMap.isActive("p1_right"))
				{
					players[0]->m_gatherer->gatherer_body->ApplyLinearImpulse(b2Vec2(gameToPhysicsUnits(15.f), 0.f), players[0]->m_gatherer->gatherer_body->GetWorldCenter(), true);
					//players[0]->m_gatherer->gatherer_body->SetLinearVelocity(b2Vec2(8.f, players[0]->m_gatherer->gatherer_body->GetLinearVelocity().y));
				}

				if (actionMap.isActive("p2_up"))
				{
					players[1]->m_gatherer->gatherer_body->ApplyLinearImpulse(b2Vec2(0.f, gameToPhysicsUnits(-15.f)), players[1]->m_gatherer->gatherer_body->GetWorldCenter(), true);
					//players[1]->m_gatherer->gatherer_body->SetLinearVelocity(b2Vec2(players[1]->m_gatherer->gatherer_body->GetLinearVelocity().x, -8.f));
				}
				if (actionMap.isActive("p2_down"))
				{
					players[1]->m_gatherer->gatherer_body->ApplyLinearImpulse(b2Vec2(0.f, gameToPhysicsUnits(15.f)), players[1]->m_gatherer->gatherer_body->GetWorldCenter(), true);
					//players[1]->m_gatherer->gatherer_body->SetLinearVelocity(b2Vec2(players[1]->m_gatherer->gatherer_body->GetLinearVelocity().x, 8.f));
				}
				if (actionMap.isActive("p2_left"))
				{
					players[1]->m_gatherer->gatherer_body->ApplyLinearImpulse(b2Vec2(gameToPhysicsUnits(-15.f), 0.f), players[1]->m_gatherer->gatherer_body->GetWorldCenter(), true);
					//players[1]->m_gatherer->gatherer_body->SetLinearVelocity(b2Vec2(-8.f, players[1]->m_gatherer->gatherer_body->GetLinearVelocity().y));
				}
				if (actionMap.isActive("p2_right"))
				{
					players[1]->m_gatherer->gatherer_body->ApplyLinearImpulse(b2Vec2(gameToPhysicsUnits(15.f), 0.f), players[1]->m_gatherer->gatherer_body->GetWorldCenter(), true);
					//players[1]->m_gatherer->gatherer_body->SetLinearVelocity(b2Vec2(8.f, players[1]->m_gatherer->gatherer_body->GetLinearVelocity().y));
				}
				if (actionMap.isActive("p3_up"))
				{
					players[2]->m_gatherer->gatherer_body->ApplyLinearImpulse(b2Vec2(0.f, gameToPhysicsUnits(-15.f)), players[1]->m_gatherer->gatherer_body->GetWorldCenter(), true);
					//players[1]->m_gatherer->gatherer_body->SetLinearVelocity(b2Vec2(players[1]->m_gatherer->gatherer_body->GetLinearVelocity().x, -8.f));
				}
				if (actionMap.isActive("p3_down"))
				{
					players[2]->m_gatherer->gatherer_body->ApplyLinearImpulse(b2Vec2(0.f, gameToPhysicsUnits(15.f)), players[1]->m_gatherer->gatherer_body->GetWorldCenter(), true);
					//players[1]->m_gatherer->gatherer_body->SetLinearVelocity(b2Vec2(players[1]->m_gatherer->gatherer_body->GetLinearVelocity().x, 8.f));
				}
				if (actionMap.isActive("p3_left"))
				{
					players[2]->m_gatherer->gatherer_body->ApplyLinearImpulse(b2Vec2(gameToPhysicsUnits(-15.f), 0.f), players[1]->m_gatherer->gatherer_body->GetWorldCenter(), true);
					//players[1]->m_gatherer->gatherer_body->SetLinearVelocity(b2Vec2(-8.f, players[1]->m_gatherer->gatherer_body->GetLinearVelocity().y));
				}
				if (actionMap.isActive("p3_right"))
				{
					players[2]->m_gatherer->gatherer_body->ApplyLinearImpulse(b2Vec2(gameToPhysicsUnits(15.f), 0.f), players[1]->m_gatherer->gatherer_body->GetWorldCenter(), true);
					//players[1]->m_gatherer->gatherer_body->SetLinearVelocity(b2Vec2(8.f, players[1]->m_gatherer->gatherer_body->GetLinearVelocity().y));
				}
			}
		}



		// Cap the speed to max speed for all pla5yers
		for (auto &player : players)
		{
			b2Vec2 MAX_VELOCITY(30.f, 30.f);
			if (player->m_defender->defender_body->GetLinearVelocity().x >= MAX_VELOCITY.x)
			{
				player->m_defender->defender_body->SetLinearVelocity(b2Vec2(MAX_VELOCITY.x, player->m_defender->defender_body->GetLinearVelocity().y));
			}
			if (player->m_defender->defender_body->GetLinearVelocity().y >= MAX_VELOCITY.y)
			{
				player->m_defender->defender_body->SetLinearVelocity(b2Vec2(player->m_defender->defender_body->GetLinearVelocity().x, MAX_VELOCITY.y));
			}
			if (player->m_defender->defender_body->GetLinearVelocity().x <= -MAX_VELOCITY.x)
			{
				player->m_defender->defender_body->SetLinearVelocity(b2Vec2(-MAX_VELOCITY.x, player->m_defender->defender_body->GetLinearVelocity().y));
			}
			if (player->m_defender->defender_body->GetLinearVelocity().y <= -MAX_VELOCITY.y)
			{
				player->m_defender->defender_body->SetLinearVelocity(b2Vec2(player->m_defender->defender_body->GetLinearVelocity().x, -MAX_VELOCITY.y));
			}

			b2Vec2 MAX_VELOCITY_GATHERER(12.f, 12.f);
			if (player->m_gatherer->gatherer_body->GetLinearVelocity().x >= MAX_VELOCITY_GATHERER.x)
			{
				player->m_gatherer->gatherer_body->SetLinearVelocity(b2Vec2(MAX_VELOCITY_GATHERER.x, player->m_gatherer->gatherer_body->GetLinearVelocity().y));
			}
			if (player->m_gatherer->gatherer_body->GetLinearVelocity().y >= MAX_VELOCITY_GATHERER.y)
			{
				player->m_gatherer->gatherer_body->SetLinearVelocity(b2Vec2(player->m_gatherer->gatherer_body->GetLinearVelocity().x, MAX_VELOCITY_GATHERER.y));
			}
			if (player->m_gatherer->gatherer_body->GetLinearVelocity().x <= -MAX_VELOCITY_GATHERER.x)
			{
				player->m_gatherer->gatherer_body->SetLinearVelocity(b2Vec2(-MAX_VELOCITY_GATHERER.x, player->m_gatherer->gatherer_body->GetLinearVelocity().y));
			}
			if (player->m_gatherer->gatherer_body->GetLinearVelocity().y <= -MAX_VELOCITY_GATHERER.y)
			{
				player->m_gatherer->gatherer_body->SetLinearVelocity(b2Vec2(player->m_gatherer->gatherer_body->GetLinearVelocity().x, -MAX_VELOCITY_GATHERER.y));
			}

		}
		///////////
		//TESTING//
		///////////
		if (actionMap.isActive("stalker"))
		{
			stalker->body->SetTransform(b2Vec2(30.f, 10.f), 0);

		}
		stalker->shape.setPosition(physicsToGameUnits(stalker->body->GetPosition()));

		//update sprite positions
		for (int i = 0; i < numDevices; i++)
		{
			players[i]->m_defender->m_sprite.setPosition(physicsToGameUnits(players[i]->m_defender->defender_body->GetPosition()));
			players[i]->m_gatherer->m_sprite.setPosition(physicsToGameUnits(players[i]->m_gatherer->gatherer_body->GetPosition()));
		}


		for (auto &player : players)
		{

			player->m_defender->defender.setPosition(physicsToGameUnits(player->m_defender->defender_body->GetPosition()));
			player->m_gatherer->gatherer.setPosition(physicsToGameUnits(player->m_gatherer->gatherer_body->GetPosition()));


			if (!win)
			{
				float distance = std::sqrtf(
					(player->m_gatherer->gatherer.getPosition().x - innerCircle.getPosition().x) *
					(player->m_gatherer->gatherer.getPosition().x - innerCircle.getPosition().x) +
					(player->m_gatherer->gatherer.getPosition().y - innerCircle.getPosition().y) *
					(player->m_gatherer->gatherer.getPosition().y - innerCircle.getPosition().y));
				if (distance <= innerCircle.getRadius())
				{
					player->m_gatherer->gatherer.setFillColor(sf::Color::Yellow);
					player->m_gatherer->IsGatheringTime = true;
					int count = 0;
					for (auto it : players)
					{
						if (it->m_gatherer->IsGatheringTime == true)
						{
							++count;
						}
					}
					if (!player->timer.isRunning())
					{
						player->timer.start();
						if (count > 1)
						{
							for (auto it2 : players)
							{
								it2->timer.stop();
							}

						}
					}

				}
				else
				{
					if (player->timer.isRunning())
					{
						player->timer.stop();
					}
					if (player->GetStartColor() != nullptr)
					{
						player->m_gatherer->gatherer.setFillColor(*player->GetStartColor());
					}
					if (player->m_gatherer->IsGatheringTime)
					{
						player->m_gatherer->IsGatheringTime = false;
					}
				}

				if (player->timer.getElapsedTime().asSeconds() >= 15)
				{
					win = true;
					MessageBoxA(window.getSystemHandle(), "A player won", "WIN!!!", 0);
					player->m_gatherer->gatherer.setFillColor(sf::Color::Blue);
				}
			}
		}

		window.clear(sf::Color::Black);
		window.draw(background);

		for (b2Body* bodyIt = world->GetBodyList(); bodyIt != 0; bodyIt = bodyIt->GetNext())
		{
			if (bodyIt->GetType() == b2_staticBody)
			{
				b2Shape* shape = bodyIt->GetFixtureList()[0].GetShape();
				if (shape->GetType() == b2Shape::e_edge)
				{
					b2EdgeShape* edge_shape = static_cast<b2EdgeShape*>(shape);
					sf::Vertex line[] =
					{
						sf::Vertex(physicsToGameUnits(edge_shape->m_vertex1.x, edge_shape->m_vertex1.y)),
						sf::Vertex(physicsToGameUnits(edge_shape->m_vertex2.x, edge_shape->m_vertex2.y))
					};
					window.draw(line, 2, sf::Lines);
				}
			}
		}

		for (int i = 0; i < stones.size(); i++)
		{
			window.draw(stones[i]);
		}

		for (auto i : players)
		{
			if (i->m_gatherer->GathererIsHit)
			{

				i->m_gatherer->gatherer_body->SetTransform(i->m_gatherer->m_respawn_pos, 0);
				i->m_gatherer->gatherer_body->SetLinearVelocity(b2Vec2(0.f, 0.f));
				i->m_gatherer->GathererIsHit = false;
			}
		}

		//ShieldDeflection
		for (auto player : players)
		{
			if (player->m_defender->defenderDeflected)
			{
				player->m_defender->defender_body->ApplyLinearImpulse(500 * gameToPhysicsUnits(deflect_coll_dir), player->m_defender->defender_body->GetWorldCenter(), true);
				if (player->m_defender->deflectiontimer.getElapsedTime().asSeconds() > 1.5f)
				{
					player->m_defender->defenderDeflected = false;
				}

			}
		}

		//turning
		for (int i = 0; i < numDevices; ++i)
		{
			if (players[i]->m_gatherer->gatherer_body->GetLinearVelocity().x > 0)
			{
				players[i]->m_gatherer->m_animation.setScale(-1.f, 1.f);
			}
			else if (players[i]->m_gatherer->gatherer_body->GetLinearVelocity().x < 0)
			{
				players[i]->m_gatherer->m_animation.setScale(1.f, 1.f);
			}
			//b2Vec2 a = { players[i]->m_defender->DVel.x / fDeltaTime, players[i]->m_defender->DVel.y / fDeltaTime };

			/*b2Vec2 vector = gameToPhysicsUnits(sf::Mouse::getPosition()) - players[i]->m_defender->defender_body->GetPosition();
			float angle = atan2f(vector.y, vector.x);
			if (angle <0)
			{
			red_ani.setScale(-1.f, 1.f);
			}
			else
			{
			red_ani.setScale(1.f, 1.f);
			}*/

			/*if (players[i]->m_defender->currentVel.x >= 0 )
			{
			players[i]->m_defender->m_animation.setScale(-1.f, 1.f);
			if (a.x < -10 )
			{
			players[i]->m_defender->m_animation.setScale(1.f, 1.f);
			}
			}
			else if (players[i]->m_defender->currentVel.x <= 0 )
			{
			players[i]->m_defender->m_animation.setScale(1.f, 1.f);
			if (a.x > 10 )
			{
			players[i]->m_defender->m_animation.setScale(-1.f, 1.f);
			}*/
			//}
			/*if (a.x > 0 && players[i]->m_defender->currentVel.x >= 0)
			{
			players[i]->m_defender->m_animation.setScale(-1.f, 1.f);
			}
			else if (a.x < 0 && players[i]->m_defender->currentVel.x >= 0)
			{
			players[i]->m_defender->m_animation.setScale(-1.f, 1.f);
			}
			else if (a.x < 0 && players[i]->m_defender->currentVel.x <= 0)
			{
			players[i]->m_defender->m_animation.setScale(1.f, 1.f);
			}
			else if (a.x > 0 && players[i]->m_defender->currentVel.x <= 0)
			{
			players[i]->m_defender->m_animation.setScale(1.f, 1.f);
			}*/

			if (players[i]->m_defender->defender_body->GetLinearVelocity().x < 0)
			{
				players[i]->m_defender->m_animation.setScale(1, 1.f);
				/*if (a.Length() < players[i]->m_defender->defender_body->GetLinearVelocity().Length())
				{
				a = b2Vec2(0, 0);
				}
				else if (a.Length() / 10 > players[i]->m_defender->defender_body->GetLinearVelocity().Length())
				{
				players[i]->m_defender->m_animation.setScale(players[i]->m_defender->m_animation.getScale().x*-1, 1.f);

				}*/
			}
			else if (players[i]->m_defender->defender_body->GetLinearVelocity().x > 0)
			{
				players[i]->m_defender->m_animation.setScale(-1, 1.f);
				/*if (a.Length() < players[i]->m_defender->defender_body->GetLinearVelocity().Length())
				{
				a = b2Vec2(0, 0);
				}
				else if (a.Length() / 10 > players[i]->m_defender->defender_body->GetLinearVelocity().Length())
				{
				players[i]->m_defender->m_animation.setScale(players[i]->m_defender->m_animation.getScale().x*-1, 1.f);


				}*/
			}
		}
		window.draw(s_GUI);
		///////////
		//TESTING//
		///////////
		window.draw(stalker->shape);
		window.draw(innerCircle);
		for (auto player : players)
		{
			sf::Time time = player->timer.getElapsedTime();
			float ftime = time.asSeconds();
			std::ostringstream ss;
			ss << ftime;
			std::string s(ss.str());
			sf::String string(s);
			sf::Text text(sf::Text(string, font));
			text.setPosition(player->m_gatherer->m_textposition);
			window.draw(text);
			//window.draw(player->m_defender->defender);
			window.draw(player->m_gatherer->gatherer);
			//window.draw(player->m_gatherer->m_sprite);
			//window.draw(player->m_defender->m_sprite);
		}

		for (int i = 0; i < numDevices; i++)
		{
			players[i]->m_defender->m_animation.setPosition(physicsToGameUnits(players[i]->m_defender->defender_body->GetPosition()));
			players[i]->m_gatherer->m_animation.setPosition(physicsToGameUnits(players[i]->m_gatherer->gatherer_body->GetPosition()));
			for (int j = 0; j < 7; j++)
			{
				animation.update(clock.restart());
				animation.animate(players[i]->m_defender->m_animation);
				window.draw(players[i]->m_defender->m_animation);
				ganimation.update(g_clock.restart());
				ganimation.animate(players[i]->m_gatherer->m_animation);
				window.draw(players[i]->m_gatherer->m_animation);
			}
		}

		//PARTICLES
		partSys.update(clockP.restart());
		for (int i = 0; i < numDevices; i++)
		{
			for (int j = i + 1; j < numDevices; j++)
			{
				if (players[i]->m_defender->Particle || players[j]->m_defender->Particle)
				{

					emitter.setEmissionRate(DefenderVelocity(players[i]->m_defender->currentVel, players[j]->m_defender->currentVel) * 3);
					emitter.setParticleLifetime(sf::seconds(0.5f));
					emitter.setParticlePosition(def_coll_pos);
					emitter.setParticleVelocity(thor::Distributions::deflect(300.f*def_coll_dir, 45.f));
					partSys.addEmitter(emitter, sf::seconds(0.1));

					emitter.setParticlePosition(def_coll_pos);
					emitter.setParticleVelocity(thor::Distributions::deflect(-300.f*def_coll_dir, 45.f));
					partSys.addEmitter(emitter, sf::seconds(0.1));

					players[i]->m_defender->Particle = false;


				}
			}
		}
		if (shield.taken)
		{
			if (!shield.shieldTimer.isRunning())
			{
				shield.shieldTimer.restart();
			}
			for (int i = 0; i < numDevices; i++)
			{
				if (players[i]->m_gatherer->HasShield)
				{
					shield.shieldPosition = players[i]->m_gatherer->gatherer.getPosition();

				}
			}
			if (shield.shieldTimer.getElapsedTime().asSeconds() > 5)
			{
				shield.taken = false;
				shield.shieldTimer.stop();
				for (auto it : players)
				{
					if (it->m_gatherer->HasShield)
					{
						it->m_gatherer->HasShield = false;
					}
				}
				shield.shieldPosition = sf::Vector2f(thor::random(100, 1800), thor::random(100, 1000));

			}

		}


		shield.shieldShape.setPosition(shield.shieldPosition);
		window.draw(shield.shieldShape);
		if (lightning.respawnTimer.getElapsedTime().asSeconds() > 3.f)
		{
			lightning.respawnTimer.stop();
			lightning.doDraw = true;
			
		}
		if (lightning.doDraw)
		{
			window.draw(lightning.lightningShape);
		}
		
		window.draw(partSys);

		//Debugdraw
		Box2DWorldDraw debugDraw(&window);
		debugDraw.SetFlags(b2Draw::e_shapeBit);
		world->SetDebugDraw(&debugDraw);
		world->DrawDebugData();
		window.display();
	}
	for (int i = 0; i < players.size(); i++)
	{
		delete players[i]->m_Start_color;
		players[i]->m_Start_color = nullptr;
		delete players[i];
		players[i] = nullptr;
	}
	ManyMouse_Quit();
}

float DefenderVelocity(b2Vec2 vel1, b2Vec2 vel2)
{
	return vel1.Length() + vel2.Length();
}