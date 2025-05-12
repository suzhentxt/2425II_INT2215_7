#include "BackgroundSelector.h"
#include "SDL2/SDL_ttf.h"
#include <iostream>

BackgroundSelection::BackgroundSelection(SDL_Renderer* renderer, int width, int height)
    : windowWidth(width), windowHeight(height) {
    
    // Load font - try multiple font paths for better compatibility
    TTF_Font* font = nullptr;
    const char* fontPaths[] = {
        "arial.ttf", 
        "./arial.ttf",
        "/usr/share/fonts/truetype/freefont/FreeSans.ttf",
        "/usr/share/fonts/TTF/arial.ttf",
        "/System/Library/Fonts/Supplemental/Arial.ttf", // macOS
        "C:\\Windows\\Fonts\\arial.ttf" // Windows
    };
    
    for (const char* path : fontPaths) {
        font = TTF_OpenFont(path, 36);
        if (font != nullptr) {
            std::cout << "Loaded font from: " << path << std::endl;
            break;
        }
    }
    
    if (font == nullptr) {
        std::cout << "Warning: Could not load any font: " << TTF_GetError() << std::endl;
    }
    
    // Create title texture
    if (font != nullptr) {
        SDL_Color textColor = {255, 255, 255, 255};
        SDL_Surface* surface = TTF_RenderText_Solid(font, "SELECT BACKGROUND", textColor);
        if (surface != nullptr) {
            titleTexture = SDL_CreateTextureFromSurface(renderer, surface);
            titleRect = {width/2 - surface->w/2, 50, surface->w, surface->h};
            SDL_FreeSurface(surface);
        }
    }
    
    // Define background options
    const int previewSize = 200;
    const int numOptions = 3;
    const int spacing = 40;
    const int startX = (width - ((previewSize * numOptions) + (spacing * (numOptions - 1)))) / 2;
    const int startY = height / 2 - previewSize / 2 - 30; // Move up a bit to make room for labels
    
    // Create background options
    std::vector<std::string> filenames = {"bg1.bmp", "bg2.bmp", "bg3.bmp"};
    std::vector<std::string> displayNames = {"Grassy Field", "Desert", "Light Desert"};
    
    // Create smaller font for labels
    TTF_Font* labelFont = nullptr;
    if (font != nullptr) {
        labelFont = TTF_OpenFont(fontPaths[0], 24);
        if (labelFont == nullptr) {
            // Try the same font paths as above but with smaller size
            for (const char* path : fontPaths) {
                labelFont = TTF_OpenFont(path, 24);
                if (labelFont != nullptr) break;
            }
        }
    }
    
    for (int i = 0; i < numOptions; i++) {
        BackgroundOption option;
        option.filename = filenames[i];
        option.displayName = displayNames[i];
        
        // Position the preview
        int posX = startX + (i * (previewSize + spacing));
        option.rect = {posX, startY, previewSize, previewSize};
        
        // Try multiple paths to find the background file
        const char* bgPaths[] = {
            option.filename.c_str(),
            ("./"+option.filename).c_str(),
            ("../"+option.filename).c_str(),
            ("assets/"+option.filename).c_str(),
            ("data/"+option.filename).c_str(),
            ("resources/"+option.filename).c_str()
        };
        
        // Load preview texture - try multiple paths
        for (const char* path : bgPaths) {
            option.preview = TextureLoader::loadTexture(renderer, path);
            if (option.preview != nullptr) {
                std::cout << "Loaded background preview from: " << path << std::endl;
                break;
            }
        }
            
        // If still failed, create a colored preview as fallback
        if (option.preview == nullptr) {
            std::cout << "Creating fallback colored preview for: " << option.filename << std::endl;
            SDL_Surface* surface = SDL_CreateRGBSurface(0, previewSize, previewSize, 32, 0, 0, 0, 0);
            if (surface != nullptr) {
                // Fill with different colors based on index
                Uint32 color = 0;
                switch (i) {
                    case 0: color = SDL_MapRGB(surface->format, 34, 139, 34); break; // Green
                    case 1: color = SDL_MapRGB(surface->format, 210, 180, 140); break; // Tan
                    case 2: color = SDL_MapRGB(surface->format, 230, 230, 250); break; // Light blue
                }
                SDL_FillRect(surface, NULL, color);
                option.preview = SDL_CreateTextureFromSurface(renderer, surface);
                SDL_FreeSurface(surface);
            }
        }
        
        // Create label texture
        if (labelFont != nullptr) {
            SDL_Color textColor = {255, 255, 255, 255};
            SDL_Surface* surface = TTF_RenderText_Solid(labelFont, option.displayName.c_str(), textColor);
            if (surface != nullptr) {
                option.labelTexture = SDL_CreateTextureFromSurface(renderer, surface);
                // Position label below preview
                option.labelRect = {
                    posX + (previewSize - surface->w) / 2, // Center horizontally
                    startY + previewSize + 10, // Place below the preview
                    surface->w, 
                    surface->h
                };
                SDL_FreeSurface(surface);
            }
        }
        
        options.push_back(option);
    }
    
    // Clean up fonts
    if (font != nullptr) {
        TTF_CloseFont(font);
    }
    
    if (labelFont != nullptr) {
        TTF_CloseFont(labelFont);
    }
}

BackgroundSelection::~BackgroundSelection() {
    // Clean up textures
    if (titleTexture != nullptr) {
        SDL_DestroyTexture(titleTexture);
    }
    
    for (auto& option : options) {
        if (option.preview != nullptr) {
            SDL_DestroyTexture(option.preview);
        }
        if (option.labelTexture != nullptr) {
            SDL_DestroyTexture(option.labelTexture);
        }
    }
}

std::string BackgroundSelection::update(SDL_Event& event) {
    // Handle selection
    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
        int mouseX = event.button.x;
        int mouseY = event.button.y;
        
        // Check if mouse is over any option
        for (const auto& option : options) {
            if (mouseX >= option.rect.x && mouseX < option.rect.x + option.rect.w &&
                mouseY >= option.rect.y && mouseY < option.rect.y + option.rect.h) {
                
                selectionMade = true;
                selectedBackground = option.filename;
                std::cout << "Selected background: " << selectedBackground << std::endl;
                return selectedBackground;
            }
        }
    }
    
    // Update hover state
    if (event.type == SDL_MOUSEMOTION) {
        int mouseX = event.motion.x;
        int mouseY = event.motion.y;
        
        // Reset all hover states first
        for (auto& option : options) {
            option.hover = false;
        }
        
        // Set hover state for option under mouse
        for (auto& option : options) {
            if (mouseX >= option.rect.x && mouseX < option.rect.x + option.rect.w &&
                mouseY >= option.rect.y && mouseY < option.rect.y + option.rect.h) {
                option.hover = true;
                break;
            }
        }
    }
    
    return ""; // No selection made yet
}

void BackgroundSelection::draw(SDL_Renderer* renderer) {
    // Clear screen with dark blue background
    SDL_SetRenderDrawColor(renderer, 25, 25, 50, 255);
    SDL_RenderClear(renderer);
    
    // Draw title
    if (titleTexture != nullptr) {
        SDL_RenderCopy(renderer, titleTexture, NULL, &titleRect);
    }
    
    // Draw instructions
    TTF_Font* font = TTF_OpenFont("arial.ttf", 20);
    if (font == nullptr) {
        // Try alternative paths
        const char* fontPaths[] = {
            "./arial.ttf",
            "/usr/share/fonts/truetype/freefont/FreeSans.ttf",
            "/usr/share/fonts/TTF/arial.ttf",
            "/System/Library/Fonts/Supplemental/Arial.ttf", // macOS
            "C:\\Windows\\Fonts\\arial.ttf" // Windows
        };
        
        for (const char* path : fontPaths) {
            font = TTF_OpenFont(path, 20);
            if (font != nullptr) break;
        }
    }
    
    if (font != nullptr) {
        SDL_Color textColor = {200, 200, 200, 255};
        SDL_Surface* surface = TTF_RenderText_Solid(font, "Click on a background to select it", textColor);
        if (surface != nullptr) {
            SDL_Texture* instructionsTexture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_Rect instructionsRect = {
                windowWidth/2 - surface->w/2,
                titleRect.y + titleRect.h + 20,
                surface->w,
                surface->h
            };
            SDL_RenderCopy(renderer, instructionsTexture, NULL, &instructionsRect);
            SDL_FreeSurface(surface);
            SDL_DestroyTexture(instructionsTexture);
        }
        TTF_CloseFont(font);
    }
    
    // Draw each option
    for (const auto& option : options) {
        // Draw preview
        if (option.preview != nullptr) {
            SDL_RenderCopy(renderer, option.preview, NULL, &option.rect);
        }
        
        // Draw border - highlight if hovered
        if (option.hover) {
            // Thicker, yellow border for hover
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
            
            // Draw outer border
            SDL_Rect outerRect = {
                option.rect.x - 4,
                option.rect.y - 4,
                option.rect.w + 8,
                option.rect.h + 8
            };
            SDL_RenderDrawRect(renderer, &outerRect);
            
            // Draw inner border
            SDL_Rect innerRect = {
                option.rect.x - 2,
                option.rect.y - 2,
                option.rect.w + 4,
                option.rect.h + 4
            };
            SDL_RenderDrawRect(renderer, &innerRect);
            
            // Regular border
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderDrawRect(renderer, &option.rect);
        } else {
            // Normal white border
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderDrawRect(renderer, &option.rect);
        }
        
        // Draw label
        if (option.labelTexture != nullptr) {
            SDL_RenderCopy(renderer, option.labelTexture, NULL, &option.labelRect);
        }
    }
    
    // Present the screen
    SDL_RenderPresent(renderer);
}