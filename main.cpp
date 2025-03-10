#include <SDL2/SDL.h>
#include <vector>
#include <queue>
#include <climits>
#include <cmath>
#include <utility>
#include <iostream>

using namespace std;

// Cấu trúc cho ô trong lưới
struct Node {
    int x, y;         // Vị trí ô (cột, hàng)
    int distance;     // Khoảng cách từ điểm xuất phát
    int prevX, prevY; // Ô trước đó trong đường đi
    bool visited;     // Đã thăm chưa
    bool blocked;     // Bị chặn bởi tháp không
};

// Cấu trúc cho kẻ thù
struct Enemy {
    int gridX, gridY; // Vị trí trên lưới
    vector<pair<int, int>> path; // Đường đi từ Dijkstra
    int pathIndex;    // Vị trí hiện tại trong đường đi
    int hp;           // Máu
    float speed;      // Tốc độ
    float speedBoost; // Tăng tốc khi bị bắn
    int boostTimer;   // Thời gian tăng tốc
};

// Cấu trúc cho tháp
struct Tower {
    int gridX, gridY; // Vị trí trên lưới
    int range;        // Tầm bắn (pixel)
    int damage;       // Sát thương
};

const int GRID_WIDTH = 10;  // Số cột
const int GRID_HEIGHT = 10; // Số hàng
const int CELL_SIZE = 80;   // Kích thước ô (pixel)
const int WINDOW_WIDTH = GRID_WIDTH * CELL_SIZE;
const int WINDOW_HEIGHT = GRID_HEIGHT * CELL_SIZE;

// Hàm tìm đường bằng Dijkstra
vector<pair<int, int>> dijkstra(vector<vector<Node>>& grid, int startX, int startY, int endX, int endY) {
    // Khởi tạo tất cả các ô
    for (int i = 0; i < GRID_HEIGHT; i++) {
        for (int j = 0; j < GRID_WIDTH; j++) {
            grid[i][j].distance = INT_MAX;
            grid[i][j].visited = false;
            grid[i][j].prevX = -1;
            grid[i][j].prevY = -1;
        }
    }

    // Priority queue cho Dijkstra
    priority_queue<pair<int, pair<int, int>>, vector<pair<int, pair<int, int>>>, greater<pair<int, pair<int, int>>>> pq;
    grid[startY][startX].distance = 0;
    pq.push({0, {startX, startY}});

    int dx[] = {-1, 0, 1, 0}; // Các hướng: lên, phải, xuống, trái
    int dy[] = {0, 1, 0, -1};

    while (!pq.empty()) {
        int currX = pq.top().second.first;
        int currY = pq.top().second.second;
        pq.pop();

        if (grid[currY][currX].visited) continue;
        grid[currY][currX].visited = true;

        for (int i = 0; i < 4; i++) {
            int newX = currX + dx[i];
            int newY = currY + dy[i];

            if (newX >= 0 && newX < GRID_WIDTH && newY >= 0 && newY < GRID_HEIGHT &&
                !grid[newY][newX].blocked) {
                int newDist = grid[currY][currX].distance + 1;

                if (newDist < grid[newY][newX].distance) {
                    grid[newY][newX].distance = newDist;
                    grid[newY][newX].prevX = currX;
                    grid[newY][newX].prevY = currY;
                    pq.push({newDist, {newX, newY}});
                }
            }
        }
    }

    // Tái tạo đường đi
    vector<pair<int, int>> path;
    int x = endX, y = endY;
    while (x != -1 && y != -1) {
        path.push_back({x, y});
        int prevX = grid[y][x].prevX;
        int prevY = grid[y][x].prevY;
        x = prevX; y = prevY;
    }
    reverse(path.begin(), path.end());

    if (path.empty() || path[0] != make_pair(startX, startY)) {
        return vector<pair<int, int>>(); // Không tìm được đường
    }
    return path;
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("AI City Defense Simple (Dijkstra)", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Khởi tạo lưới
    vector<vector<Node>> grid(GRID_HEIGHT, vector<Node>(GRID_WIDTH));
    for (int i = 0; i < GRID_HEIGHT; i++) {
        for (int j = 0; j < GRID_WIDTH; j++) {
            grid[i][j] = {j, i, INT_MAX, -1, -1, false, false};
        }
    }

    vector<Enemy> enemies;
    vector<Tower> towers;
    int cityHP = 100;
    int wave = 1; // Đợt hiện tại
    int enemiesInWave = 3; // Số kẻ thù mỗi đợt
    int waveTimer = 0; // Đếm thời gian giữa các đợt
    bool running = true;
    SDL_Event event;

    // Đánh dấu thành phố tại (9, 5) là chướng ngại (không cho kẻ thù đi qua ô này)
    grid[5][9].blocked = true;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                int mx, my;
                SDL_GetMouseState(&mx, &my);
                int gridX = mx / CELL_SIZE;
                int gridY = my / CELL_SIZE;
                if (gridX < GRID_WIDTH && gridY < GRID_HEIGHT && !grid[gridY][gridX].blocked) {
                    towers.push_back({gridX, gridY, 2 * CELL_SIZE, 5});
                    grid[gridY][gridX].blocked = true;
                }
            }
        }

        // Spawn kẻ thù theo đợt
        waveTimer++;
        if (waveTimer >= 300 && enemies.size() < enemiesInWave) { // 5 giây với 60 FPS
            waveTimer = 0;
            enemies.push_back({0, 5, vector<pair<int, int>>(), 0, 20, 1.0f, 0, 0});
            vector<pair<int, int>> path = dijkstra(grid, 0, 5, 9, 5);
            if (!path.empty()) {
                enemies.back().path = path;
                enemies.back().pathIndex = 0;
            }
        }

        // Kiểm tra kết thúc đợt
        if (enemies.empty() && waveTimer >= 300 && wave < 5) {
            wave++;
            enemiesInWave += 2; // Tăng số kẻ thù mỗi đợt
            waveTimer = 0;
        } else if (wave == 5 && enemies.empty()) {
            running = false; // Thắng
        }

        // Di chuyển kẻ thù
        for (auto& enemy : enemies) {
            if (enemy.hp > 0 && !enemy.path.empty() && enemy.pathIndex < enemy.path.size()) {
                int targetX = enemy.path[enemy.pathIndex].first * CELL_SIZE + CELL_SIZE / 2;
                int targetY = enemy.path[enemy.pathIndex].second * CELL_SIZE + CELL_SIZE / 2;
                int currX = enemy.gridX * CELL_SIZE + CELL_SIZE / 2;
                int currY = enemy.gridY * CELL_SIZE + CELL_SIZE / 2;

                if (currX < targetX) enemy.gridX++;
                else if (currX > targetX) enemy.gridX--;
                if (currY < targetY) enemy.gridY++;
                else if (currY > targetY) enemy.gridY--;
                else enemy.pathIndex++;

                if (enemy.boostTimer > 0) {
                    enemy.boostTimer--;
                    if (enemy.boostTimer == 0) enemy.speedBoost = 0;
                }
            }
            if (enemy.gridX == 9 && enemy.gridY == 5) {
                cityHP -= 10;
                enemy.hp = 0;
            }
        }

        // Logic tháp bắn
        for (auto& tower : towers) {
            for (auto& enemy : enemies) {
                if (enemy.hp > 0) {
                    int dx = (enemy.gridX * CELL_SIZE + CELL_SIZE / 2) - (tower.gridX * CELL_SIZE + CELL_SIZE / 2);
                    int dy = (enemy.gridY * CELL_SIZE + CELL_SIZE / 2) - (tower.gridY * CELL_SIZE + CELL_SIZE / 2);
                    float dist = sqrt(dx * dx + dy * dy);
                    if (dist <= tower.range) {
                        enemy.hp -= tower.damage;
                        if (enemy.speedBoost == 0) {
                            enemy.speedBoost = 0.5f;
                            enemy.boostTimer = 120;
                        }
                    }
                }
            }
        }

        // Xóa kẻ thù chết
        enemies.erase(remove_if(enemies.begin(), enemies.end(), [](Enemy& e) { return e.hp <= 0; }), enemies.end());

        // Render
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Vẽ lưới
        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
        for (int i = 0; i <= GRID_HEIGHT; i++) {
            SDL_RenderDrawLine(renderer, 0, i * CELL_SIZE, WINDOW_WIDTH, i * CELL_SIZE);
        }
        for (int j = 0; j <= GRID_WIDTH; j++) {
            SDL_RenderDrawLine(renderer, j * CELL_SIZE, 0, j * CELL_SIZE, WINDOW_HEIGHT);
        }

        // Vẽ thành phố
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_Rect city = {9 * CELL_SIZE, 5 * CELL_SIZE, CELL_SIZE, CELL_SIZE};
        SDL_RenderFillRect(renderer, &city);

        // Vẽ tháp
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
        for (const auto& tower : towers) {
            SDL_Rect rect = {tower.gridX * CELL_SIZE, tower.gridY * CELL_SIZE, CELL_SIZE, CELL_SIZE};
            SDL_RenderFillRect(renderer, &rect);
        }

        // Vẽ kẻ thù
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        for (const auto& enemy : enemies) {
            if (enemy.hp > 0) {
                SDL_Rect rect = {enemy.gridX * CELL_SIZE, enemy.gridY * CELL_SIZE, CELL_SIZE, CELL_SIZE};
                SDL_RenderFillRect(renderer, &rect);
            }
        }

        SDL_RenderPresent(renderer);

        if (cityHP <= 0) running = false; // Thua
        SDL_Delay(16); // ~60 FPS
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}