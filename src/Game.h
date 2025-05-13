#pragma once
#include <vector>
#include <chrono>
#include <memory>
#include <iostream>
#include <string>
#include "SDL2/SDL.h"
#include "SDL2/SDL_ttf.h"
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

	enum class GameState {
		instructions,
		playing,
		gameOver,
		victory
	} gameState = GameState::instructions;

	// Try Again button
	struct Button {
		SDL_Rect rect;
		SDL_Texture* texture = nullptr;
		bool hover = false;
	} tryAgainButton;

	// Start Game button
	struct Button startButton;

public:
	// Modified constructor to accept background file name
	Game(SDL_Window* window, SDL_Renderer* renderer, int windowWidth, int windowHeight, const std::string& backgroundFile = "bg1.bmp");
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
	void drawGameState(SDL_Renderer* renderer);
	void drawPlacementPreview(SDL_Renderer* renderer, Vector2D mousePos);
	void resetGame(SDL_Renderer* renderer);
	void createTryAgainButton(SDL_Renderer* renderer);
	void createStartButton(SDL_Renderer* renderer);
	void showNotification(const std::string& message);
	void drawNotification(SDL_Renderer* renderer);

	int mouseDownStatus = 0;

	const int tileSize = 64;
	Level level;

	int windowWidth = 0;
	int windowHeight = 0;

	std::vector<std::shared_ptr<Unit>> listUnits;
	std::vector<Turret> listTurrets;
	std::vector<Projectile> listProjectiles;

	SDL_Texture* textureOverlay = nullptr;
	bool overlayVisible = true;

	SDL_Texture* textureWin = nullptr;
	SDL_Texture* textureGameOver = nullptr;
	SDL_Texture* textureInstructions = nullptr;
	bool instructionsVisible = true;

	Timer spawnTimer, roundTimer;
	int spawnUnitCount = 0;
	int currentRound = 0;
	const int maxRounds = 5;
	const int maxCityHealth = 100;
	int cityHealth = maxCityHealth;

	// Notification system
	struct Notification {
		std::string message;
		float displayTime = 3.0f;  // How long to show the notification
		float currentTime = 0.0f;  // Current time remaining
		bool active = false;
	} notification;

	Mix_Chunk* mix_ChunkSpawnUnit = nullptr;

	TTF_Font* font = nullptr;
	std::string currentBackground;
};