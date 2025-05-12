#include <iostream>
#include <string>
#include "SDL2/SDL.h"
#include "SDL2/SDL_mixer.h"
#include "SDL2/SDL_ttf.h"
#include "Game.h"


// Function for background selection menu
std::string selectBackground() {
    int choice = 0;
    std::string backgroundOptions[] = { "bg1.bmp", "bg2.bmp", "bg3.bmp" };
    
    std::cout << "===============================" << std::endl;
    std::cout << "      CITY DEFENSE GAME        " << std::endl;
    std::cout << "===============================" << std::endl;
    std::cout << "Select a background image:" << std::endl;
    std::cout << "1. Background 1" << std::endl;
    std::cout << "2. Background 2" << std::endl;
    std::cout << "3. Background 3" << std::endl;
    std::cout << "Enter your choice (1-3): ";
    
    std::cin >> choice;
    
    // Validate input
    while (choice < 1 || choice > 3 || std::cin.fail()) {
        std::cin.clear(); // Clear error flags
        std::cin.ignore(100, '\n'); // Ignore incorrect input
        std::cout << "Invalid choice. Please enter a number between 1 and 3: ";
        std::cin >> choice;
    }
    
    return backgroundOptions[choice - 1];
}


int main(int argc, char* args[]) {
	//Seed the random number generator with the current time so that it will generate different 
	//numbers every time the game is run.
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

		//Setup the audio mixer.
		bool isSDLMixerLoaded = (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) == 0);
		if (isSDLMixerLoaded == false) {
			std::cout << "Error: Couldn't initialize Mix_OpenAudio = " << Mix_GetError() << std::endl;
		}
		else {
			Mix_AllocateChannels(32);

			//Output the name of the audio driver.
			std::cout << "Audio driver = " << SDL_GetCurrentAudioDriver() << std::endl;
		}

        // Choose background before creating the window
        std::string selectedBackground = selectBackground();
        std::cout << "Starting game with background: " << selectedBackground << std::endl;

		//Create the window.
		SDL_Window* window = SDL_CreateWindow("City Defense", 
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 960, 576, 0);
		if (window == nullptr) {
			std::cout << "Error: Couldn't create window = " << SDL_GetError() << std::endl;
			return 1;
		}
		else {
			//Create a renderer for GPU accelerated drawing.
			SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | 
				SDL_RENDERER_PRESENTVSYNC);
			if (renderer == nullptr) {
				std::cout << "Error: Couldn't create renderer = " << SDL_GetError() << std::endl;
				return 1;
			}
			else {
				//Ensure transparent graphics are drawn correctly.
				SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

				//Output the name of the render driver.
				SDL_RendererInfo rendererInfo;
				SDL_GetRendererInfo(renderer, &rendererInfo);
				std::cout << "Renderer = " << rendererInfo.name << std::endl;

				//Get the dimensions of the window.
				int windowWidth = 0, windowHeight = 0;
				SDL_GetWindowSize(window, &windowWidth, &windowHeight);

				//Start the game with the selected background
				Game game(window, renderer, windowWidth, windowHeight, selectedBackground);

				//Clean up.
				SDL_DestroyRenderer(renderer);
			}

			//Clean up.
			SDL_DestroyWindow(window);
		}

		//Clean up.
		if (isSDLMixerLoaded) {
			Mix_CloseAudio();
			Mix_Quit();
		}

		TTF_Quit();
		SDL_Quit();
	}
	return 0;
}