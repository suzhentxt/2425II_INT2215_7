#pragma once
#include "SDL2/SDL.h"
#include "SDL2/SDL_ttf.h"
#include <string>

class UI {
private:
    TTF_Font* font;
    int windowWidth;
    int windowHeight;
    bool gameStateVisible = true;
    
    struct Notification {
        std::string message;
        float displayTime = 4.0f;
        float currentTime = 0.0f;
        bool active = false;
    } notification;

public:
    UI(SDL_Window* window, SDL_Renderer* renderer);
    ~UI();
    
    void drawGameState(SDL_Renderer* renderer, int cityHealth, int maxCityHealth, 
                      int currentRound, int maxRounds, int enemiesRemaining,
                      int remainingTurrets, int maxTurrets, int remainingWalls, int maxWalls);
    void showNotification(const std::string& message);
    void updateNotification(float dT);
    void drawNotification(SDL_Renderer* renderer);
    void toggleGameState() { gameStateVisible = !gameStateVisible; }
    bool isGameStateVisible() const { return gameStateVisible; }
}; 