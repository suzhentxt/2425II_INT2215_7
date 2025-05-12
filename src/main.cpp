#include <iostream>
#include <string>
#include "SDL2/SDL.h"
#include "SDL2/SDL_mixer.h"
#include "SDL2/SDL_ttf.h"
#include "Game.h"
#include "BackgroundSelector.h"

int main(int argc, char* args[]) {
	// Seed the random number generator
	srand((unsigned)time(NULL));

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
		std::cout << "Error: Couldn't initialize SDL Video or Audio = " << SDL_GetError() << std::endl;
		return 1;
	}
	else {
		// Initialize SDL_ttf
		if (TTF_Init() < 0) {
			std::cout << "Error: Couldn't initialize SDL_ttf = " << TTF_GetError() << std::endl;
			return 1;
		}

		// Setup the audio mixer
		bool isSDLMixerLoaded = (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) == 0);
		if (isSDLMixerLoaded == false) {
			std::cout << "Error: Couldn't initialize Mix_OpenAudio = " << Mix_GetError() << std::endl;
		}
		else {
			Mix_AllocateChannels(32);
			std::cout << "Audio driver = " << SDL_GetCurrentAudioDriver() << std::endl;
		}

		// Create the window
		SDL_Window* window = SDL_CreateWindow("City Defense", 
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 960, 576, 0);
			
		if (window == nullptr) {
			std::cout << "Error: Couldn't create window = " << SDL_GetError() << std::endl;
			return 1;
		}
		else {
			// Create a renderer
			SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 
                               SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
                               
			if (renderer == nullptr) {
				std::cout << "Error: Couldn't create renderer = " << SDL_GetError() << std::endl;
				return 1;
			}
			else {
				// Ensure transparent graphics are drawn correctly
				SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

				// Output renderer information
				SDL_RendererInfo rendererInfo;
				SDL_GetRendererInfo(renderer, &rendererInfo);
				std::cout << "Renderer = " << rendererInfo.name << std::endl;

				// Get window dimensions
				int windowWidth = 0, windowHeight = 0;
				SDL_GetWindowSize(window, &windowWidth, &windowHeight);

                // Visual background selection menu
                std::string selectedBackground = "bg1.bmp"; // Default in case selection fails
                
                {
                    BackgroundSelection bgSelection(renderer, windowWidth, windowHeight);
                    bool selectionRunning = true;
                    
                    while (selectionRunning) {
                        SDL_Event event;
                        while (SDL_PollEvent(&event)) {
                            if (event.type == SDL_QUIT) {
                                selectionRunning = false;
                                // Exit entire application
                                SDL_DestroyRenderer(renderer);
                                SDL_DestroyWindow(window);
                                TTF_Quit();
                                SDL_Quit();
                                return 0;
                            }
                            
                            std::string selected = bgSelection.update(event);
                            if (!selected.empty()) {
                                selectedBackground = selected;
                                selectionRunning = false;
                                break;
                            }
                        }
                        
                        if (selectionRunning) {
                            bgSelection.draw(renderer);
                            SDL_Delay(16); // ~60fps
                        }
                    }
                }
                
                std::cout << "Starting game with background: " << selectedBackground << std::endl;
                
				// Start the game with selected background
				Game game(window, renderer, windowWidth, windowHeight, selectedBackground);

				// Clean up renderer
				SDL_DestroyRenderer(renderer);
			}

			// Clean up window
			SDL_DestroyWindow(window);
		}

		// Clean up audio
		if (isSDLMixerLoaded) {
			Mix_CloseAudio();
			Mix_Quit();
		}

		TTF_Quit();
		SDL_Quit();
	}
	return 0;
}