#include "Level.h"
#include <iostream>


Level::Level(SDL_Renderer* renderer, int setTileCountX, int setTileCountY, const std::string& backgroundFile) :
    tileCountX(setTileCountX), tileCountY(setTileCountY),
    targetX(setTileCountX / 2), targetY(setTileCountY / 2) {
    
    // Try multiple approaches to load the background texture
    std::vector<std::string> pathsToTry = {
        backgroundFile,                        // Original path
        "./" + backgroundFile,                 // With explicit current directory
        "../" + backgroundFile,                // Parent directory
        "assets/" + backgroundFile,            // Assets folder
        "data/" + backgroundFile,              // Data folder
        "resources/" + backgroundFile          // Resources folder
    };
    
    // Also try alternative extensions if loading fails
    std::vector<std::string> extensions = {".bmp", ".png", ".jpg"};
    
    std::cout << "Attempting to load background: " << backgroundFile << std::endl;
    
    // First try direct paths with different prefixes
    for (const auto& path : pathsToTry) {
        textureBackground = TextureLoader::loadTexture(renderer, path.c_str());
        if (textureBackground != nullptr) {
            std::cout << "Successfully loaded background from: " << path << std::endl;
            break;
        } else {
            std::cout << "Failed to load from: " << path << " - " << SDL_GetError() << std::endl;
        }
    }
    
    // If still not loaded, try different extensions
    if (textureBackground == nullptr) {
        std::string baseName = backgroundFile;
        
        // Remove extension if present
        size_t dotPos = baseName.find_last_of('.');
        if (dotPos != std::string::npos) {
            baseName = baseName.substr(0, dotPos);
        }
        
        // Try each extension
        for (const auto& ext : extensions) {
            for (const auto& path : pathsToTry) {
                std::string pathWithExt = path;
                
                // Replace extension if path already has one
                dotPos = pathWithExt.find_last_of('.');
                if (dotPos != std::string::npos) {
                    pathWithExt = pathWithExt.substr(0, dotPos) + ext;
                }
                
                std::cout << "Trying with alternate extension: " << pathWithExt << std::endl;
                textureBackground = TextureLoader::loadTexture(renderer, pathWithExt.c_str());
                if (textureBackground != nullptr) {
                    std::cout << "Successfully loaded background from: " << pathWithExt << std::endl;
                    break;
                }
            }
            if (textureBackground != nullptr) break;
        }
    }
    
    // Last resort: Create a colored background if texture loading failed
    if (textureBackground == nullptr) {
        std::cout << "Failed to load background texture, creating a fallback surface..." << std::endl;
        
        // Choose color based on filename to somewhat match the intended bg
        Uint8 r = 100, g = 150, b = 100; // Default green-ish
        
        if (backgroundFile.find("bg2") != std::string::npos) {
            // Desert-like color for bg2
            r = 210; g = 180; b = 140;
        } 
        else if (backgroundFile.find("bg3") != std::string::npos) {
            // Bluish color for bg3
            r = 180; g = 200; b = 220;
        }
        
        // Create a surface with the chosen color
        SDL_Surface* surface = SDL_CreateRGBSurface(0, tileCountX * 64, tileCountY * 64, 32, 0, 0, 0, 0);
        if (surface != nullptr) {
            SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, r, g, b));
            textureBackground = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);
            
            if (textureBackground != nullptr) {
                std::cout << "Created fallback background texture" << std::endl;
            }
        }
    }
    
    // Load other textures
    textureTileWall = TextureLoader::loadTexture(renderer, "Tile Wall.bmp");
    textureTileTarget = TextureLoader::loadTexture(renderer, "City.bmp");
    textureTileEnemySpawner = TextureLoader::loadTexture(renderer, "Tile Enemy Spawner.bmp");

    textureTileEmpty = TextureLoader::loadTexture(renderer, "Tile Empty.bmp");
    textureTileArrowUp = TextureLoader::loadTexture(renderer, "Tile Arrow Up.bmp");
    textureTileArrowUpRight = TextureLoader::loadTexture(renderer, "Tile Arrow Up Right.bmp");
    textureTileArrowRight = TextureLoader::loadTexture(renderer, "Tile Arrow Right.bmp");
    textureTileArrowDownRight = TextureLoader::loadTexture(renderer, "Tile Arrow Down Right.bmp");
    textureTileArrowDown = TextureLoader::loadTexture(renderer, "Tile Arrow Down.bmp");
    textureTileArrowDownLeft = TextureLoader::loadTexture(renderer, "Tile Arrow Down Left.bmp");
    textureTileArrowLeft = TextureLoader::loadTexture(renderer, "Tile Arrow Left.bmp");
    textureTileArrowUpLeft = TextureLoader::loadTexture(renderer, "Tile Arrow Up Left.bmp");

    size_t listTilesSize = (size_t)tileCountX * tileCountY;
    listTiles.assign(listTilesSize, Tile{});

    //Add an enemy spawner at each corner.
    int xMax = tileCountX - 1;
    int yMax = tileCountY - 1;
    setTileType(0, 0, TileType::enemySpawner);
    setTileType(xMax, 0, TileType::enemySpawner);
    setTileType(0, yMax, TileType::enemySpawner);
    setTileType(xMax, yMax, TileType::enemySpawner);

    calculateFlowField();
}


void Level::draw(SDL_Renderer* renderer, int tileSize) {
    // Clear the renderer first
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    // Calculate level dimensions
    int levelWidth = tileCountX * tileSize;
    int levelHeight = tileCountY * tileSize;
    
    // Draw background if available
    bool backgroundDrawn = false;
    if (textureBackground != nullptr) {
        std::cout << "Attempting to render background texture" << std::endl;
        // Create a rect covering the entire level area
        SDL_Rect bgRect = { 0, 0, levelWidth, levelHeight };
        
        // Verify texture is still valid
        int w, h;
        if (SDL_QueryTexture(textureBackground, NULL, NULL, &w, &h) == 0) {
            std::cout << "Background texture is valid, dimensions: " << w << "x" << h << std::endl;
            // Draw the background texture
            if (SDL_RenderCopy(renderer, textureBackground, NULL, &bgRect) == 0) {
                backgroundDrawn = true;
                std::cout << "Successfully rendered background" << std::endl;
            } else {
                std::cout << "Failed to render background texture: " << SDL_GetError() << std::endl;
            }
        } else {
            std::cout << "Background texture is invalid: " << SDL_GetError() << std::endl;
        }
    } else {
        std::cout << "Background texture is null" << std::endl;
    }
    
    // Draw the checkerboard background only if no background image was drawn
    if (!backgroundDrawn) {
        std::cout << "Drawing fallback checkerboard background" << std::endl;
        for (int y = 0; y < tileCountY; y++) {
            for (int x = 0; x < tileCountX; x++) {
                if ((x + y) % 2 == 0)
                    SDL_SetRenderDrawColor(renderer, 240, 240, 240, 255);
                else
                    SDL_SetRenderDrawColor(renderer, 225, 225, 225, 255);

                SDL_Rect rect = { x * tileSize, y * tileSize, tileSize, tileSize };
                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }

    //Uncomment to draw the flow field.
    /*for (int count = 0; count < listTiles.size(); count++)
        drawTile(renderer, (count % tileCountX), (count / tileCountX), tileSize);*/
    
    //Draw the enemy spawner tiles.
    for (int y = 0; y < tileCountY; y++) {
        for (int x = 0; x < tileCountX; x++) {
            if (getTileType(x, y) == TileType::enemySpawner) {
                SDL_Rect rect = { x * tileSize, y * tileSize, tileSize, tileSize };
                SDL_RenderCopy(renderer, textureTileEnemySpawner, NULL, &rect);
            }
        }
    }

    //Draw the target tile.
    if (textureTileTarget != nullptr) {
        SDL_Rect rect = { targetX * tileSize, targetY * tileSize, tileSize, tileSize };
        SDL_RenderCopy(renderer, textureTileTarget, NULL, &rect);
    }
    
    //Draw the wall tiles.
    for (int y = 0; y < tileCountY; y++) {
        for (int x = 0; x < tileCountX; x++) {
            if (isTileWall(x, y)) {
                int w, h;
                SDL_QueryTexture(textureTileWall, NULL, NULL, &w, &h);
                SDL_Rect rect = {
                    x * tileSize + tileSize / 2 - w / 2,
                    y * tileSize + tileSize / 2 - h / 2,
                    w,
                    h };
                SDL_RenderCopy(renderer, textureTileWall, NULL, &rect);
            }
        }
    }
}


void Level::drawTile(SDL_Renderer* renderer, int x, int y, int tileSize) {
    //Set the default texture image to be empty.
    SDL_Texture* textureSelected = textureTileEmpty;

    //Ensure that the input tile exists.
    int index = x + y * tileCountX;
    if (index > -1 && index < listTiles.size() &&
        x > -1 && x < tileCountX &&
        y > -1 && y < tileCountY) {
        Tile& tileSelected = listTiles[index];

        //Select the correct tile texture based on the flow direction.
        if (tileSelected.flowDirectionX == 0 && tileSelected.flowDirectionY == -1)
            textureSelected = textureTileArrowUp;
        else if (tileSelected.flowDirectionX == 1 && tileSelected.flowDirectionY == -1)
            textureSelected = textureTileArrowUpRight;
        else if (tileSelected.flowDirectionX == 1 && tileSelected.flowDirectionY == 0)
            textureSelected = textureTileArrowRight;
        else if (tileSelected.flowDirectionX == 1 && tileSelected.flowDirectionY == 1)
            textureSelected = textureTileArrowDownRight;
        else if (tileSelected.flowDirectionX == 0 && tileSelected.flowDirectionY == 1)
            textureSelected = textureTileArrowDown;
        else if (tileSelected.flowDirectionX == -1 && tileSelected.flowDirectionY == 1)
            textureSelected = textureTileArrowDownLeft;
        else if (tileSelected.flowDirectionX == -1 && tileSelected.flowDirectionY == 0)
            textureSelected = textureTileArrowLeft;
        else if (tileSelected.flowDirectionX == -1 && tileSelected.flowDirectionY == -1)
            textureSelected = textureTileArrowUpLeft;

    }

    //Draw the tile.
    if (textureSelected != nullptr) {
        SDL_Rect rect = { x * tileSize, y * tileSize, tileSize, tileSize };
        SDL_RenderCopy(renderer, textureSelected, NULL, &rect);
    }
}



Vector2D Level::getRandomEnemySpawnerLocation() {
    //Create a list of all tiles that are enemy spawners.
    std::vector<int> listSpawnerIndices;
    for (int count = 0; count < listTiles.size(); count++) {
        auto& tileSelected = listTiles[count];
        if (tileSelected.type == TileType::enemySpawner)
            listSpawnerIndices.push_back(count);
    }

    //If one or more spawners are found, pick one at random and output it's center position.
    if (listSpawnerIndices.empty() == false) {
        int index = listSpawnerIndices[rand() % listSpawnerIndices.size()];
        return Vector2D((float)(index % tileCountX) + 0.5f, (float)(index / tileCountX) + 0.5f);
    }

    return Vector2D(0.5f, 0.5f);
}



bool Level::isTileWall(int x, int y) {
    return (getTileType(x, y) == TileType::wall);
}


void Level::setTileWall(int x, int y, bool setWall) {
    if (getTileType(x, y) != TileType::enemySpawner)
        setTileType(x, y, (setWall ? TileType::wall : TileType::empty));
}


Level::TileType Level::getTileType(int x, int y) {
    int index = x + y * tileCountX;
    if (index > -1 && index < listTiles.size() &&
        x > -1 && x < tileCountX &&
        y > -1 && y < tileCountY)
        return listTiles[index].type;

    return TileType::empty;
}


void Level::setTileType(int x, int y, TileType tileType) {
    int index = x + y * tileCountX;
    if (index > -1 && index < listTiles.size() &&
        x > -1 && x < tileCountX &&
        y > -1 && y < tileCountY) {
        listTiles[index].type = tileType;
        calculateFlowField();
    }
}



Vector2D Level::getTargetPos() {
    return Vector2D((float)targetX + 0.5f, (float)targetY + 0.5f);
}


void Level::calculateFlowField() {
    //Ensure the target is in bounds.
    int indexTarget = targetX + targetY * tileCountX;
    if (indexTarget > -1 && indexTarget < listTiles.size() &&
        targetX > -1 && targetX < tileCountX &&
        targetY > -1 && targetY < tileCountY) {

        //Reset the tile flow data.
        for (auto& tileSelected : listTiles) {
            tileSelected.flowDirectionX = 0;
            tileSelected.flowDirectionY = 0;
            tileSelected.flowDistance = flowDistanceMax;
        }

        //Calculate the flow field.
        calculateDistances();
        calculateFlowDirections();
    }
}


void Level::calculateDistances() {
    int indexTarget = targetX + targetY * tileCountX;

    //Create a queue that will contain the indices to be checked.
    std::queue<int> listIndicesToCheck;
    //Set the target tile's flow value to 0 and add it to the queue.
    listTiles[indexTarget].flowDistance = 0;
    listIndicesToCheck.push(indexTarget);

    //The offset of the neighboring tiles to be checked.
    const int listNeighbors[][2] = { { -1, 0}, {1, 0}, {0, -1}, {0, 1} };

    //Loop through the queue and assign distance to each tile.
    while (listIndicesToCheck.empty() == false) {
        int indexCurrent = listIndicesToCheck.front();
        listIndicesToCheck.pop();

        //Check each of the neighbors;
        for (int count = 0; count < 4; count++) {
            int neighborX = listNeighbors[count][0] + indexCurrent % tileCountX;
            int neighborY = listNeighbors[count][1] + indexCurrent / tileCountX;
            int indexNeighbor = neighborX + neighborY * tileCountX;

            //Ensure that the neighbor exists and isn't a wall.
            if (indexNeighbor > -1 && indexNeighbor < listTiles.size() && 
                neighborX > -1 && neighborX < tileCountX &&
                neighborY > -1 && neighborY < tileCountY &&
                listTiles[indexNeighbor].type != TileType::wall) {

                //Check if the tile has been assigned a distance yet or not.
                if (listTiles[indexNeighbor].flowDistance == flowDistanceMax) {
                    //If not the set it's distance and add it to the queue.
                    listTiles[indexNeighbor].flowDistance = listTiles[indexCurrent].flowDistance + 1;
                    listIndicesToCheck.push(indexNeighbor);
                }
            }
        }
    }
}


void Level::calculateFlowDirections() {
    //The offset of the neighboring tiles to be checked.
    const int listNeighbors[][2] = {
        {-1, 0}, {-1, 1}, {0, 1}, {1, 1},
        {1, 0}, {1, -1}, {0, -1}, {-1, -1} };

    for (int indexCurrent = 0; indexCurrent < listTiles.size(); indexCurrent++) {
        //Ensure that the tile has been assigned a distance value.
        if (listTiles[indexCurrent].flowDistance != flowDistanceMax) {
            //Set the best distance to the current tile's distance.
            unsigned char flowFieldBest = listTiles[indexCurrent].flowDistance;

            //Check each of the neighbors;
            for (int count = 0; count < 8; count++) {
                int offsetX = listNeighbors[count][0];
                int offsetY = listNeighbors[count][1];

                int neighborX = offsetX + indexCurrent % tileCountX;
                int neighborY = offsetY + indexCurrent / tileCountX;
                int indexNeighbor = neighborX + neighborY * tileCountX;

                //Ensure that the neighbor exists.
                if (indexNeighbor > -1 && indexNeighbor < listTiles.size() &&
                    neighborX > -1 && neighborX < tileCountX &&
                    neighborY > -1 && neighborY < tileCountY) {
                    //If the current neighbor's distance is lower than the best then use it.
                    if (listTiles[indexNeighbor].flowDistance < flowFieldBest) {
                        flowFieldBest = listTiles[indexNeighbor].flowDistance;
                        listTiles[indexCurrent].flowDirectionX = offsetX;
                        listTiles[indexCurrent].flowDirectionY = offsetY;
                    }
                }
            }
        }
    }
}



Vector2D Level::getFlowNormal(int x, int y) {
    int index = x + y * tileCountX;
    if (index > -1 && index < listTiles.size() &&
        x > -1 && x < tileCountX &&
        y > -1 && y < tileCountY)
        return Vector2D((float)listTiles[index].flowDirectionX, (float)listTiles[index].flowDirectionY).normalize();

    return Vector2D();
}