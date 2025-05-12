#include "TextureLoader.h"
#include <vector>
#include <iostream>

std::unordered_map<std::string, SDL_Texture*> TextureLoader::umapTexturesLoaded;

SDL_Texture* TextureLoader::loadTexture(SDL_Renderer* renderer, std::string filename) {
    if (filename != "") {
        auto found = umapTexturesLoaded.find(filename);

        if (found != umapTexturesLoaded.end()) {
            std::cout << "Found cached texture for: " << filename << std::endl;
            // Verify the cached texture is still valid
            int w, h;
            if (SDL_QueryTexture(found->second, NULL, NULL, &w, &h) == 0) {
                std::cout << "Cached texture is valid, dimensions: " << w << "x" << h << std::endl;
                return found->second;
            } else {
                std::cout << "Cached texture is invalid, reloading..." << std::endl;
                // Remove invalid texture from cache
                SDL_DestroyTexture(found->second);
                umapTexturesLoaded.erase(found);
            }
        }

        // Try multiple paths for the image
        std::vector<std::string> pathsToTry = {
            filename,                        // Original path
            "./" + filename,                 // Current directory
            "Data/Images/" + filename       // Data/Images directory
        };

        for (const auto& filepath : pathsToTry) {
            std::cout << "Trying to load texture from: " << filepath << std::endl;
            //Try to create a surface using the filepath.
            SDL_Surface* surfaceTemp = SDL_LoadBMP(filepath.c_str());
            if (surfaceTemp != nullptr) {
                std::cout << "Successfully loaded surface from: " << filepath << std::endl;
                //The surface was created successfully so attempt to create a texture with it.
                SDL_Texture* textureOutput = SDL_CreateTextureFromSurface(renderer, surfaceTemp);
                //Free the surface because it's no longer needed. 
                SDL_FreeSurface(surfaceTemp);

                if (textureOutput != nullptr) {
                    std::cout << "Successfully created texture from: " << filepath << std::endl;
                    //Enable transparency for the texture.
                    SDL_SetTextureBlendMode(textureOutput, SDL_BLENDMODE_BLEND);

                    //Add the texture to the map of loaded textures to keep track of it and for clean-up purposes.
                    umapTexturesLoaded[filename] = textureOutput;

                    // Verify texture is valid
                    int w, h;
                    if (SDL_QueryTexture(textureOutput, NULL, NULL, &w, &h) == 0) {
                        std::cout << "Texture is valid, dimensions: " << w << "x" << h << std::endl;
                    } else {
                        std::cout << "Warning: Texture validation failed: " << SDL_GetError() << std::endl;
                        SDL_DestroyTexture(textureOutput);
                        return nullptr;
                    }

                    return textureOutput;
                } else {
                    std::cout << "Failed to create texture from surface: " << SDL_GetError() << std::endl;
                }
            } else {
                std::cout << "Failed to load surface from: " << filepath << " - " << SDL_GetError() << std::endl;
            }
        }
    }

    return nullptr;
}

void TextureLoader::deallocateTextures() {
    //Destroy all the textures
    while (umapTexturesLoaded.empty() == false) {
        auto it = umapTexturesLoaded.begin();
        if (it->second != nullptr)
            SDL_DestroyTexture(it->second);

        umapTexturesLoaded.erase(it);
    }
}