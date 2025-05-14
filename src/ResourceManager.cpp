#include "ResourceManager.h"

ResourceManager::ResourceManager() {
    reset();
}

void ResourceManager::updateForNewRound(int currentRound) {
    maxTurretsPerRound = baseTurretsPerRound + (currentRound - 1) * turretIncreasePerRound;
    maxWallsPerRound = baseWallsPerRound + (currentRound - 1) * wallIncreasePerRound;
    remainingTurrets = maxTurretsPerRound;
    remainingWalls = maxWallsPerRound;
}

void ResourceManager::reset() {
    maxTurretsPerRound = baseTurretsPerRound;
    maxWallsPerRound = baseWallsPerRound;
    remainingTurrets = maxTurretsPerRound;
    remainingWalls = maxWallsPerRound;
} 