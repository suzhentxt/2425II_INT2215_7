#include "Game.h"



Game::Game(SDL_Window* window, SDL_Renderer* renderer, int windowWidth, int windowHeight, const std::string& backgroundFile) :
    placementModeCurrent(PlacementMode::wall), 
    level(renderer, windowWidth / tileSize, windowHeight / tileSize, backgroundFile),
    spawnTimer(0.25f), roundTimer(3.0f),
    windowWidth(windowWidth), windowHeight(windowHeight),
    currentBackground(backgroundFile) {

    // Initialize UI
    ui = new UI(window, renderer);

    // Initialize resource manager
    resourceManager.reset();

    //Load the font
    font = TTF_OpenFont("D:/Xius/Dev/Game_Project/Data/Fonts/arial.ttf", 24);
    if (font == nullptr) {
        std::cout << "Error: Couldn't load font = " << TTF_GetError() << std::endl;
        // Try loading a fallback font
        font = TTF_OpenFont("D:/Xius/Dev/Game_Project/Data/Fonts/arial.ttf", 24);
        if (font == nullptr) {
            std::cout << "Error: Couldn't load fallback font = " << TTF_GetError() << std::endl;
        }
    }

    //Run the game.
    if (window != nullptr && renderer != nullptr) {
        //Load the overlay texture.
        // textureOverlay = TextureLoader::loadTexture(renderer, "Overlay.bmp");
        textureWin = TextureLoader::loadTexture(renderer, "YouWin.bmp");
        textureGameOver = TextureLoader::loadTexture(renderer, "GameOver.bmp");
        textureInstructions = TextureLoader::loadTexture(renderer, "Instructions.bmp");

        // Create Try Again button
        createTryAgainButton(renderer);
        // Create Start Game button
        createStartButton(renderer);

        //Load the spawn unit sound.
        mix_ChunkSpawnUnit = SoundLoader::loadSound("Spawn Unit.ogg");

        //Store the current times for the clock.
        auto time1 = std::chrono::system_clock::now();
        auto time2 = std::chrono::system_clock::now();

        //The amount of time for each frame (60 fps).
        const float dT = 1.0f / 60.0f;


        //Start the game loop and run until it's time to stop.
        bool running = true;
        while (running) {
            //Determine how much time has elapsed since the last frame.
            time2 = std::chrono::system_clock::now();
            std::chrono::duration<float> timeDelta = time2 - time1;
            float timeDeltaFloat = timeDelta.count();

            //If enough time has passed then do everything required to generate the next frame.
            if (timeDeltaFloat >= dT) {
                //Store the new time for the next frame.
                time1 = time2;

                processEvents(renderer, running);
                update(renderer, dT);
                draw(renderer);
            }
        }
    }
}

Game::~Game() {
    delete ui;
    //Clean up.
    if (font != nullptr) {
        TTF_CloseFont(font);
        font = nullptr;
    }
    TextureLoader::deallocateTextures();
    SoundLoader::deallocateSounds();
}



void Game::processEvents(SDL_Renderer* renderer, bool& running) {
    bool mouseDownThisFrame = false;

    //Process events.
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            running = false;
            break;

        case SDL_MOUSEBUTTONDOWN:
            mouseDownThisFrame = (mouseDownStatus == 0);
            if (event.button.button == SDL_BUTTON_LEFT) {
                mouseDownStatus = SDL_BUTTON_LEFT;
                
                // Check if Try Again button was clicked
                if (gameState != GameState::playing) {
                    int mouseX, mouseY;
                    SDL_GetMouseState(&mouseX, &mouseY);
                    
                    if (gameState == GameState::instructions) {
                        // Check Start Game button
                        if (mouseX >= startButton.rect.x && 
                            mouseX <= startButton.rect.x + startButton.rect.w &&
                            mouseY >= startButton.rect.y && 
                            mouseY <= startButton.rect.y + startButton.rect.h) {
                            gameState = GameState::playing;
                            instructionsVisible = false;  // Hide instructions when starting game
                            mouseDownStatus = 0;  // Reset mouse status to prevent wall placement
                            break;  // Skip further processing
                        }
                    } else {
                        // Check Try Again button
                        if (mouseX >= tryAgainButton.rect.x && 
                            mouseX <= tryAgainButton.rect.x + tryAgainButton.rect.w &&
                            mouseY >= tryAgainButton.rect.y && 
                            mouseY <= tryAgainButton.rect.y + tryAgainButton.rect.h) {
                            resetGame(renderer);
                            mouseDownStatus = 0;  // Reset mouse status to prevent wall placement
                            break;  // Skip further processing
                        }
                    }
                }
            }
            else if (event.button.button == SDL_BUTTON_RIGHT)
                mouseDownStatus = SDL_BUTTON_RIGHT;
            break;

        case SDL_MOUSEBUTTONUP:
            mouseDownStatus = 0;
            break;

        case SDL_MOUSEMOTION:
            // Update button hover states
            if (gameState != GameState::playing) {
                int mouseX = event.motion.x;
                int mouseY = event.motion.y;
                
                if (gameState == GameState::instructions) {
                    startButton.hover = (mouseX >= startButton.rect.x && 
                                       mouseX <= startButton.rect.x + startButton.rect.w &&
                                       mouseY >= startButton.rect.y && 
                                       mouseY <= startButton.rect.y + startButton.rect.h);
                } else {
                    tryAgainButton.hover = (mouseX >= tryAgainButton.rect.x && 
                                          mouseX <= tryAgainButton.rect.x + tryAgainButton.rect.w &&
                                          mouseY >= tryAgainButton.rect.y && 
                                          mouseY <= tryAgainButton.rect.y + tryAgainButton.rect.h);
                }
            }
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.scancode) {
                //Quit the game.
            case SDL_SCANCODE_ESCAPE:
                running = false;
                break;

                //Set the current gamemode.
            case SDL_SCANCODE_1:
                placementModeCurrent = PlacementMode::wall;
                break;
            case SDL_SCANCODE_2:
                placementModeCurrent = PlacementMode::turret;
                break;

                //Show/hide the overlay
            case SDL_SCANCODE_H:
                ui->toggleGameState();
                break;
                //Show/hide the instructions
            case SDL_SCANCODE_I:
                instructionsVisible = !instructionsVisible;
                break;
            }
        }
    }


    //Process input from the mouse cursor.
    int mouseX = 0, mouseY = 0;
    SDL_GetMouseState(&mouseX, &mouseY);
    //Convert from the window's coordinate system to the game's coordinate system.
    Vector2D posMouse((float)mouseX / tileSize, (float)mouseY / tileSize);

    if (mouseDownStatus > 0 && gameState == GameState::playing) {  // Only process placement in playing state
        // Calculate center position and tile coordinates
        int centerX = windowWidth / (2 * tileSize);
        int centerY = windowHeight / (2 * tileSize);
        int tileX = (int)posMouse.x;
        int tileY = (int)posMouse.y;
        bool isAdjacentToCenter = (abs(tileX - centerX) <= 1 && abs(tileY - centerY) <= 1);
        int wallsAroundCenter = 0;

        switch (mouseDownStatus) {
        case SDL_BUTTON_LEFT:
            switch (placementModeCurrent) {
            case PlacementMode::wall:
                if (!resourceManager.hasWallsRemaining()) {
                    ui->showNotification("No walls remaining!");
                    break;
                }
                
                // Count existing walls around center
                for (int x = centerX - 1; x <= centerX + 1; x++) {
                    for (int y = centerY - 1; y <= centerY + 1; y++) {
                        if (level.isTileWall(x, y)) {
                            wallsAroundCenter++;
                        }
                    }
                }
                
                // Don't allow placement if it would create a complete barrier
                if (isAdjacentToCenter && wallsAroundCenter >= 7) {
                    ui->showNotification("Cannot block access to city!");
                    break;
                }
                
                //Add wall at the mouse position.
                if (!level.isTileWall(tileX, tileY)) {
                    level.setTileWall(tileX, tileY, true);
                    resourceManager.decrementWalls();
                }
                break;
            case PlacementMode::turret:
                //Add the selected turret at the mouse position.
                if (mouseDownThisFrame)
                    addTurret(renderer, posMouse);
                break;
            }
            break;

        case SDL_BUTTON_RIGHT:
            //Remove wall at the mouse position.
            if (level.isTileWall(tileX, tileY)) {
                level.setTileWall(tileX, tileY, false);
                resourceManager.incrementWalls();
            }
            //Remove turrets at the mouse position.
            removeTurretsAtMousePosition(posMouse);
            break;
        }
    }
}



void Game::update(SDL_Renderer* renderer, float dT) {
    // Update notification timer
    ui->updateNotification(dT);

    // Only update game if still playing
    if (gameState == GameState::playing) {
        //Update the units.
        updateUnits(dT);

        //Update the turrets.
        for (auto& turretSelected : listTurrets)
            turretSelected.update(renderer, dT, listUnits, listProjectiles);

        //Update the projectiles.
        updateProjectiles(dT);

        updateSpawnUnitsIfRequired(renderer, dT);

        // Check win condition
        if (currentRound >= maxRounds && listUnits.empty()) {
            gameState = GameState::victory;
        }
    }
}


void Game::updateUnits(float dT) {
    //Loop through the list of units and update all of them.
    auto it = listUnits.begin();
    while (it != listUnits.end()) {
        bool increment = true;

        if ((*it) != nullptr) {
            (*it)->update(dT, level, listUnits);

            // If unit reached target, reduce city health
            if ((*it)->reachedTarget()) {
                cityHealth -= 5; // Each enemy that reaches target reduces health by 5
                if (cityHealth <= 0) {
                    gameState = GameState::gameOver;
                }
            }

            //Check if the unit is still alive. If not then erase it and don't increment the iterator.
            if ((*it)->isAlive() == false) {
                it = listUnits.erase(it);
                increment = false;
            }
        }

        if (increment)
            it++;
    }
}


void Game::updateProjectiles(float dT) {
    //Loop through the list of projectiles and update all of them.
    auto it = listProjectiles.begin();
    while (it != listProjectiles.end()) {
        (*it).update(dT, listUnits);

        //Check if the projectile has collided or not, erase it if needed, and update the iterator.
        if ((*it).getCollisionOccurred())
            it = listProjectiles.erase(it);
        else
            it++;
    }
}


void Game::updateSpawnUnitsIfRequired(SDL_Renderer* renderer, float dT) {
    spawnTimer.countDown(dT);

    //Check if the round needs to start.
    if (listUnits.empty() && spawnUnitCount == 0) {
        roundTimer.countDown(dT);
        if (roundTimer.timeSIsZero()) {
            currentRound++;
            
            // Reset map for new round
            listTurrets.clear();
            level.clearWalls();
            level.loadBackground(renderer, currentBackground);
            
            // Update resource limits for new round
            resourceManager.updateForNewRound(currentRound);
            
            // Show round notification with new limits
            char buffer[128];
            sprintf_s(buffer, "Round %d Starting! (Turrets: %d, Walls: %d)", 
                     currentRound, resourceManager.getMaxTurrets(), resourceManager.getMaxWalls());
            ui->showNotification(buffer);
            
            spawnUnitCount = 15 + (currentRound * 5); // Increase enemies per round
            roundTimer.resetToMax();
        }
    }

    //Add a unit if needed.
    if (spawnUnitCount > 0 && spawnTimer.timeSIsZero()) {
        addUnit(renderer, level.getRandomEnemySpawnerLocation());

        //Play the spawn unit sound.
        if (mix_ChunkSpawnUnit != nullptr)
            Mix_PlayChannel(-1, mix_ChunkSpawnUnit, 0);

        spawnUnitCount--;
    }
}



void Game::draw(SDL_Renderer* renderer) {
    //Draw.
    //Set the draw color to white.
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    //Clear the screen.
    SDL_RenderClear(renderer);


    //Draw everything here.
    //Draw the level.
    level.draw(renderer, tileSize);

    //Draw the enemy units.
    for (auto& unitSelected : listUnits)
        if (unitSelected != nullptr)
            unitSelected->draw(renderer, tileSize);

    //Draw the turrets.
    for (auto& turretSelected : listTurrets)
        turretSelected.draw(renderer, tileSize);

    //Draw the projectiles.
    for (auto& projectileSelected : listProjectiles)
        projectileSelected.draw(renderer, tileSize);
    
    // Draw placement preview
    int mouseX = 0, mouseY = 0;
    SDL_GetMouseState(&mouseX, &mouseY);
    Vector2D mousePos((float)mouseX / tileSize, (float)mouseY / tileSize);
    drawPlacementPreview(renderer, mousePos);

    //Draw the overlay.
    if (textureOverlay != nullptr && overlayVisible) {
        int w = 0, h = 0;
        SDL_QueryTexture(textureOverlay, NULL, NULL, &w, &h);
        SDL_Rect rect = { 40, 40, w, h };
        SDL_RenderCopy(renderer, textureOverlay, NULL, &rect);
    }

    // Draw instructions overlay
    if (textureInstructions != nullptr && instructionsVisible) {
        int w = 0, h = 0;
        SDL_QueryTexture(textureInstructions, NULL, NULL, &w, &h);
        
        // Calculate scaling to fit the screen while maintaining aspect ratio
        float scale = std::min(
            (float)(windowWidth - 80) / w,  // Leave 40px padding on each side
            (float)(windowHeight - 120) / h  // Leave 40px padding top and 80px bottom for button
        );
        
        int scaledW = (int)(w * scale);
        int scaledH = (int)(h * scale);
        
        // Center the image on screen
        SDL_Rect rect = {
            (windowWidth - scaledW) / 2,
            (windowHeight - scaledH) / 2 - 20,  // Slightly above center to make room for button
            scaledW,
            scaledH
        };
        
        SDL_RenderCopy(renderer, textureInstructions, NULL, &rect);
    }

    // Draw game state using UI class
    ui->drawGameState(renderer, cityHealth, maxCityHealth, currentRound, maxRounds,
                     spawnUnitCount + listUnits.size(),
                     resourceManager.getRemainingTurrets(), resourceManager.getMaxTurrets(),
                     resourceManager.getRemainingWalls(), resourceManager.getMaxWalls());

    // Draw notification
    ui->drawNotification(renderer);

    // Draw win/lose screen
    if (gameState != GameState::playing) {
        if (gameState == GameState::instructions) {
            // Draw instructions
            if (textureInstructions != nullptr) {
                int w = 0, h = 0;
                SDL_QueryTexture(textureInstructions, NULL, NULL, &w, &h);
                
                // Calculate scaling to fit the screen while maintaining aspect ratio
                float scale = std::min(
                    (float)(windowWidth - 80) / w,  // Leave 40px padding on each side
                    (float)(windowHeight - 120) / h  // Leave 40px padding top and 80px bottom for button
                );
                
                int scaledW = (int)(w * scale);
                int scaledH = (int)(h * scale);
                
                // Center the image on screen
                SDL_Rect rect = {
                    (windowWidth - scaledW) / 2,
                    (windowHeight - scaledH) / 2 - 20,  // Slightly above center to make room for button
                    scaledW,
                    scaledH
                };
                
                SDL_RenderCopy(renderer, textureInstructions, NULL, &rect);
            }

            // Draw Start Game button
            if (startButton.texture != nullptr) {
                // Draw button background
                SDL_SetRenderDrawColor(renderer, startButton.hover ? 100 : 50, 50, 50, 255);
                SDL_Rect bgRect = {
                    startButton.rect.x - 20,
                    startButton.rect.y - 10,
                    startButton.rect.w + 40,
                    startButton.rect.h + 20
                };
                SDL_RenderFillRect(renderer, &bgRect);
                
                // Draw button text
                SDL_RenderCopy(renderer, startButton.texture, NULL, &startButton.rect);
            }
        } else {
            SDL_Texture* endScreenTexture = (gameState == GameState::victory) ? textureWin : textureGameOver;
            if (endScreenTexture != nullptr) {
                int w = 0, h = 0;
                SDL_QueryTexture(endScreenTexture, NULL, NULL, &w, &h);
                
                // Calculate scaling to fit the screen while maintaining aspect ratio
                float scale = std::min(
                    (float)windowWidth / w,
                    (float)windowHeight / h
                );
                
                int scaledW = (int)(w * scale);
                int scaledH = (int)(h * scale);
                
                // Center the image on screen
                SDL_Rect rect = {
                    (windowWidth - scaledW) / 2,
                    (windowHeight - scaledH) / 2,
                    scaledW,
                    scaledH
                };
                
                SDL_RenderCopy(renderer, endScreenTexture, NULL, &rect);
            }

            // Draw Try Again button
            if (tryAgainButton.texture != nullptr) {
                // Draw button background
                SDL_SetRenderDrawColor(renderer, tryAgainButton.hover ? 100 : 50, 50, 50, 255);
                SDL_Rect bgRect = {
                    tryAgainButton.rect.x - 20,
                    tryAgainButton.rect.y - 10,
                    tryAgainButton.rect.w + 40,
                    tryAgainButton.rect.h + 20
                };
                SDL_RenderFillRect(renderer, &bgRect);
                
                // Draw button text
                SDL_RenderCopy(renderer, tryAgainButton.texture, NULL, &tryAgainButton.rect);
            }
        }
    }

    //Send the image to the window.
    SDL_RenderPresent(renderer);
}



void Game::addUnit(SDL_Renderer* renderer, Vector2D posMouse) {
    listUnits.push_back(std::make_shared<Unit>(renderer, posMouse, currentRound));
}



void Game::addTurret(SDL_Renderer* renderer, Vector2D posMouse) {
    if (!resourceManager.hasTurretsRemaining()) {
        ui->showNotification("No turrets remaining!");
        return;
    }
    
    Vector2D pos((int)posMouse.x + 0.5f, (int)posMouse.y + 0.5f);
    listTurrets.push_back(Turret(renderer, pos));
    resourceManager.decrementTurrets();
}


void Game::removeTurretsAtMousePosition(Vector2D posMouse) {
    for (auto it = listTurrets.begin(); it != listTurrets.end();) {
        if ((*it).checkIfOnTile((int)posMouse.x, (int)posMouse.y))
            it = listTurrets.erase(it);
        else
            it++;
    }
}

void Game::drawPlacementPreview(SDL_Renderer* renderer, Vector2D mousePos) {
    if (gameState != GameState::playing) return;

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 128);

    int tileX = (int)mousePos.x;
    int tileY = (int)mousePos.y;

    SDL_Rect previewRect = {
        tileX * tileSize,
        tileY * tileSize,
        tileSize,
        tileSize
    };

    if (placementModeCurrent == PlacementMode::wall) {
        SDL_RenderFillRect(renderer, &previewRect);
    } else if (placementModeCurrent == PlacementMode::turret) {
        SDL_RenderDrawRect(renderer, &previewRect);
    }
}

void Game::createTryAgainButton(SDL_Renderer* renderer) {
    if (font == nullptr) return;

    SDL_Color textColor = {255, 255, 255, 255};
    SDL_Surface* surface = TTF_RenderText_Solid(font, "Try Again", textColor);
    if (surface != nullptr) {
        tryAgainButton.texture = SDL_CreateTextureFromSurface(renderer, surface);
        
        // Position button at bottom center of screen
        tryAgainButton.rect = {
            (windowWidth - surface->w) / 2,
            windowHeight - surface->h - 50,
            surface->w,
            surface->h
        };
        
        SDL_FreeSurface(surface);
    }
}

void Game::createStartButton(SDL_Renderer* renderer) {
    if (font == nullptr) return;

    SDL_Color textColor = {255, 255, 255, 255};
    SDL_Surface* surface = TTF_RenderText_Solid(font, "Start Game", textColor);
    if (surface != nullptr) {
        startButton.texture = SDL_CreateTextureFromSurface(renderer, surface);
        
        // Position button at bottom center of screen
        startButton.rect = {
            (windowWidth - surface->w) / 2,
            windowHeight - surface->h - 50,
            surface->w,
            surface->h
        };
        
        SDL_FreeSurface(surface);
    }
}

void Game::resetGame(SDL_Renderer* renderer) {
    // Reset game state
    gameState = GameState::playing;
    cityHealth = maxCityHealth;
    currentRound = 0;
    spawnUnitCount = 0;
    
    // Reset resource limits to base values
    resourceManager.reset();
    
    // Clear all game objects
    listUnits.clear();
    listTurrets.clear();
    listProjectiles.clear();
    
    // Reset timers
    spawnTimer.resetToMax();
    roundTimer.resetToMax();
    
    // Reset level by clearing walls and reloading background
    level.clearWalls();
    level.loadBackground(renderer, currentBackground);
}
