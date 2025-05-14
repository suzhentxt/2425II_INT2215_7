#pragma once

class ResourceManager {
private:
    int baseTurretsPerRound = 5;
    int baseWallsPerRound = 10;
    int turretIncreasePerRound = 1;
    int wallIncreasePerRound = 5;
    int maxTurretsPerRound;
    int maxWallsPerRound;
    int remainingTurrets;
    int remainingWalls;

public:
    ResourceManager();
    void updateForNewRound(int currentRound);
    void reset();
    
    // Getters
    int getMaxTurrets() const { return maxTurretsPerRound; }
    int getMaxWalls() const { return maxWallsPerRound; }
    int getRemainingTurrets() const { return remainingTurrets; }
    int getRemainingWalls() const { return remainingWalls; }
    
    // Setters
    void decrementTurrets() { remainingTurrets--; }
    void decrementWalls() { remainingWalls--; }
    void incrementWalls() { remainingWalls++; }
    
    // Checkers
    bool hasTurretsRemaining() const { return remainingTurrets > 0; }
    bool hasWallsRemaining() const { return remainingWalls > 0; }
}; 