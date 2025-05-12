#pragma once
#include <vector>
#include <string>
#include "SDL2/SDL.h"
#include "TextureLoader.h"

class BackgroundSelection {
public:
    BackgroundSelection(SDL_Renderer* renderer, int windowWidth, int windowHeight);
    ~BackgroundSelection();
    
    // Returns selected background filename or empty string if still selecting
    std::string update(SDL_Event& event);
    void draw(SDL_Renderer* renderer);
    
    // Returns true if background was selected
    bool isSelectionMade() const { return selectionMade; }
    
    // Get the selected background filename
    std::string getSelectedBackground() const { return selectedBackground; }

private:
    struct BackgroundOption {
        std::string filename;
        std::string displayName;
        SDL_Texture* preview = nullptr;
        SDL_Rect rect;
        SDL_Texture* labelTexture = nullptr;
        SDL_Rect labelRect;
        bool hover = false;  // Track mouse hover state
    };
    
    std::vector<BackgroundOption> options;
    bool selectionMade = false;
    std::string selectedBackground;
    
    SDL_Texture* titleTexture = nullptr;
    SDL_Rect titleRect;
    
    int windowWidth;
    int windowHeight;
};