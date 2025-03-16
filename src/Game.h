#pragma once
#include <vector>
#include <chrono>
#include <memory>
#include "SDL2/SDL.h"
#include "Unit.h"
#include "Turret.h"
#include "Projectile.h"
#include "Level.h"
#include "Timer.h"



class Game
{
private:
	enum class PlacementMode {
		wall,
		turret
	} placementModeCurrent;


public:
	Game(SDL_Window* window, SDL_Renderer* renderer, int windowWidth, int windowHeight);
	~Game();


private:
	void processEvents(SDL_Renderer* renderer, bool& running);
	void update(SDL_Renderer* renderer, float dT);
	void updateUnits(float dT);
	void updateProjectiles(float dT);
	void updateSpawnUnitsIfRequired(SDL_Renderer* renderer, float dT);
	void draw(SDL_Renderer* renderer);
	void addUnit(SDL_Renderer* renderer, Vector2D posMouse);
	void addTurret(SDL_Renderer* renderer, Vector2D posMouse);
	void removeTurretsAtMousePosition(Vector2D posMouse);

	int mouseDownStatus = 0;

	const int tileSize = 64;
	Level level;

	std::vector<std::shared_ptr<Unit>> listUnits;
	std::vector<Turret> listTurrets;
	std::vector<Projectile> listProjectiles;

	SDL_Texture* textureOverlay = nullptr;
	bool overlayVisible = true;

	Timer spawnTimer, roundTimer;
	int spawnUnitCount = 0;

	Mix_Chunk* mix_ChunkSpawnUnit = nullptr;
};