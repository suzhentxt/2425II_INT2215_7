#pragma once
#include <memory>
#include <vector>
#include "SDL2/SDL.h"
#include "Vector2D.h"
#include "Level.h"
#include "TextureLoader.h"
#include "Timer.h"
class Game;



class Unit
{
public:
	Unit(SDL_Renderer* renderer, Vector2D setPos, int roundNumber = 0);
	void update(float dT, Level& level, std::vector<std::shared_ptr<Unit>>& listUnits);
	void draw(SDL_Renderer* renderer, int tileSize);
	bool checkOverlap(Vector2D posOther, float sizeOther);
	bool isAlive();
	Vector2D getPos();
	void removeHealth(int damage);
	bool reachedTarget() const { return hasReachedTarget; }


private:
	Vector2D pos;
	static const float baseSpeed;
	static const float size;
	float currentSpeed;

	SDL_Texture* texture = nullptr;

	Timer timerJustHurt;

	const int healthMax = 2;
	int healthCurrent = healthMax;
	bool hasReachedTarget = false;
};