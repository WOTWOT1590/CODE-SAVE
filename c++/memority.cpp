// === CODE CỦA BỐ NGUYỄN TIẾN ===
#include <iostream>
#include <cmath>
#include <vector>

struct Vector3
{
    float x, y, z;
    Vector3(float px = 0.f, float py = 0.f, float pz = 0.f) : x(px), y(py), z(pz) {}
};
struct WeaponMemoryLayout
{
    float recoilFactor;              // Fix rung
    float aimAssistSnapStrength;     // Bám đầu 95%
    float dragSensitivityMultiplier; // Độ nhạy kéo tâm
};
struct PlayerMemoryLayout
{
    int id;
    Vector3 position;
    Vector3 headPosition;
    float health;
    WeaponMemoryLayout *activeWeapon;
};
class InternalMemoryGameEngine
{
private:
    PlayerMemoryLayout *localPlayer;
    std::vector<PlayerMemoryLayout *> enemyList;

public:
    InternalMemoryGameEngine()
    {
        localPlayer = new PlayerMemoryLayout();
        localPlayer->id = 0;
        localPlayer->position = Vector3(0.f, 0.f, 0.f);
        localPlayer->headPosition = Vector3(0.f, 1.8f, 0.f);
        localPlayer->health = 100.f;
        localPlayer->activeWeapon = new WeaponMemoryLayout{0.0f, 0.95f, 50.0f};
    }

    ~InternalMemoryGameEngine()
    {
        delete localPlayer->activeWeapon;
        delete localPlayer;
        for (auto enemy : enemyList)
        {
            delete enemy;
        }
    }
    void AddEnemy(int id, Vector3 pos)
    {
        PlayerMemoryLayout *enemy = new PlayerMemoryLayout();
        enemy->id = id;
        enemy->position = pos;
        enemy->headPosition = Vector3(pos.x, pos.y + 1.8f, pos.z);
        enemy->health = 100.f;
        enemy->activeWeapon = nullptr;
        enemyList.push_back(enemy);
    }
    void WriteMemoryParameters()
    {
        if (localPlayer && localPlayer->activeWeapon)
        {
            localPlayer->activeWeapon->recoilFactor = 0.0f;               // Giật = 0
            localPlayer->activeWeapon->aimAssistSnapStrength = 0.95f;     // Khóa tâm 95%
            localPlayer->activeWeapon->dragSensitivityMultiplier = 50.0f; // Kéo nhẹ bay lên đầu
        }
    }
    void ProcessGameLoop()
    {
        for (const auto &enemy : enemyList)
        {
            float currentRecoil = localPlayer->activeWeapon->recoilFactor;
            float currentAimSnap = localPlayer->activeWeapon->aimAssistSnapStrength;
            float currentDragSensitivity = localPlayer->activeWeapon->dragSensitivityMultiplier;
        }
    }
};

int main()
{
    InternalMemoryGameEngine gameEngine;
    gameEngine.AddEnemy(101, Vector3(45.5f, 0.0f, 120.0f));
    gameEngine.WriteMemoryParameters();
    gameEngine.ProcessGameLoop();
    return 0;
}