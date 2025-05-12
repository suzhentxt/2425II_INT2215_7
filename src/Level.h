#pragma once
#include <queue>
#include <vector>
#include <string>
#include "SDL2/SDL.h"
#include "Vector2D.h"
#include "TextureLoader.h"



class Level
{
private:
	enum class TileType : char {
		empty,
		wall,
		enemySpawner
	};

	static const unsigned char flowDistanceMax = 255;

	struct Tile {
		TileType type = TileType::empty;
		int flowDirectionX = 0;
		int flowDirectionY = 0;
		unsigned char flowDistance = flowDistanceMax;
	};


public:
	// Modified constructor to accept background filename
	Level(SDL_Renderer* renderer, int tileCountX, int tileCountY, const std::string& backgroundFile);
	~Level();

	void draw(SDL_Renderer* renderer, int tileSize);
	void setTileWall(int x, int y, bool isWall);
	bool isTileWall(int x, int y) const;
	Vector2D getRandomEnemySpawnerLocation() const;
	void clearWalls();
	void loadBackground(SDL_Renderer* renderer, const std::string& backgroundFile);

	Vector2D getTargetPos() const;
	Vector2D getFlowNormal(int x, int y) const;


private:
	TileType getTileType(int x, int y) const;
	void setTileType(int x, int y, TileType tileType);
	void drawTile(SDL_Renderer* renderer, int x, int y, int tileSize);
	void calculateFlowField();
	void calculateDistances();
	void calculateFlowDirections();


	std::vector<Tile> listTiles;
	const int tileCountX, tileCountY;

	const int targetX = 0, targetY = 0;

	SDL_Texture* textureBackground = nullptr;
	SDL_Texture* textureTileWall = nullptr,
		*textureTileTarget = nullptr,
		*textureTileEnemySpawner = nullptr,
		*textureTileEmpty = nullptr,
		*textureTileArrowUp = nullptr,
		*textureTileArrowUpRight = nullptr,
		*textureTileArrowRight = nullptr,
		*textureTileArrowDownRight = nullptr,
		*textureTileArrowDown = nullptr,
		*textureTileArrowDownLeft = nullptr,
		*textureTileArrowLeft = nullptr,
		*textureTileArrowUpLeft = nullptr;
};