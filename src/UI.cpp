#include "UI.h"
#include <iostream>

UI::UI(SDL_Window* window, SDL_Renderer* renderer) {
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);
    font = TTF_OpenFont("D:/Xius/Dev/Game_Project/Data/Fonts/arial.ttf", 24);
    if (font == nullptr) {
        std::cout << "Error: Couldn't load font = " << TTF_GetError() << std::endl;
    }
}

UI::~UI() {
    if (font != nullptr) {
        TTF_CloseFont(font);
        font = nullptr;
    }
}

void UI::drawGameState(SDL_Renderer* renderer, int cityHealth, int maxCityHealth, 
                      int currentRound, int maxRounds, int enemiesRemaining,
                      int remainingTurrets, int maxTurrets, int remainingWalls, int maxWalls) {
    if (font == nullptr || !gameStateVisible) return;

    SDL_Color textColor = { 255, 255, 255, 255 };
    char buffer[128];

    // Draw semi-transparent background
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    SDL_Rect bgRect = { 10, 10, 300, 170 };
    SDL_RenderFillRect(renderer, &bgRect);

    // Draw city health
    sprintf_s(buffer, "City Health: %d/%d", cityHealth, maxCityHealth);
    SDL_Surface* surface = TTF_RenderText_Solid(font, buffer, textColor);
    if (surface != nullptr) {
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (texture != nullptr) {
            SDL_Rect rect = { 20, 20, surface->w, surface->h };
            SDL_RenderCopy(renderer, texture, NULL, &rect);
            SDL_DestroyTexture(texture);
        }
        SDL_FreeSurface(surface);
    }

    // Draw round information
    sprintf_s(buffer, "Round: %d/%d", currentRound, maxRounds);
    surface = TTF_RenderText_Solid(font, buffer, textColor);
    if (surface != nullptr) {
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (texture != nullptr) {
            SDL_Rect rect = { 20, 50, surface->w, surface->h };
            SDL_RenderCopy(renderer, texture, NULL, &rect);
            SDL_DestroyTexture(texture);
        }
        SDL_FreeSurface(surface);
    }

    // Draw remaining enemies
    sprintf_s(buffer, "Enemies Remaining: %d", enemiesRemaining);
    surface = TTF_RenderText_Solid(font, buffer, textColor);
    if (surface != nullptr) {
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (texture != nullptr) {
            SDL_Rect rect = { 20, 80, surface->w, surface->h };
            SDL_RenderCopy(renderer, texture, NULL, &rect);
            SDL_DestroyTexture(texture);
        }
        SDL_FreeSurface(surface);
    }

    // Draw remaining turrets
    sprintf_s(buffer, "Turrets: %d/%d", remainingTurrets, maxTurrets);
    surface = TTF_RenderText_Solid(font, buffer, textColor);
    if (surface != nullptr) {
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (texture != nullptr) {
            SDL_Rect rect = { 20, 110, surface->w, surface->h };
            SDL_RenderCopy(renderer, texture, NULL, &rect);
            SDL_DestroyTexture(texture);
        }
        SDL_FreeSurface(surface);
    }

    // Draw remaining walls
    sprintf_s(buffer, "Walls: %d/%d", remainingWalls, maxWalls);
    surface = TTF_RenderText_Solid(font, buffer, textColor);
    if (surface != nullptr) {
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (texture != nullptr) {
            SDL_Rect rect = { 20, 140, surface->w, surface->h };
            SDL_RenderCopy(renderer, texture, NULL, &rect);
            SDL_DestroyTexture(texture);
        }
        SDL_FreeSurface(surface);
    }
}

void UI::showNotification(const std::string& message) {
    notification.message = message;
    notification.currentTime = notification.displayTime;
    notification.active = true;
}

void UI::updateNotification(float dT) {
    if (notification.active) {
        notification.currentTime -= dT;
        if (notification.currentTime <= 0) {
            notification.active = false;
        }
    }
}

void UI::drawNotification(SDL_Renderer* renderer) {
    if (!notification.active || font == nullptr) return;

    SDL_Color textColor = { 255, 255, 255, 255 };
    SDL_Surface* surface = TTF_RenderText_Solid(font, notification.message.c_str(), textColor);
    if (surface != nullptr) {
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (texture != nullptr) {
            SDL_Rect rect = {
                (windowWidth - surface->w) / 2,
                windowHeight / 4,
                surface->w,
                surface->h
            };
            
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
            SDL_Rect bgRect = {
                rect.x - 20,
                rect.y - 10,
                rect.w + 40,
                rect.h + 20
            };
            SDL_RenderFillRect(renderer, &bgRect);
            
            SDL_RenderCopy(renderer, texture, NULL, &rect);
            SDL_DestroyTexture(texture);
        }
        SDL_FreeSurface(surface);
    }
} 