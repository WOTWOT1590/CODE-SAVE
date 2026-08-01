#include <iostream>
#include <cmath>

struct Vector2
{
    float x, y;
};

class SuperEasyAimSystem
{
private:
    float recoilFactor = 0.0f;               // 0% độ giật
    float aimAssistSnapStrength = 0.95f;     // Aimlock 95%
    float targetStickiness = 1.0f;           // Dính chặt đầu
    float lockOnDelay = 0.0f;                // Khóa tâm
    float dragSensitivityMultiplier = 50.0f; // Kéo nhẹ là lên đầu
    float virtualDPISetting = 3200.0f;       // DPI cảm ứng cao

public:
    Vector2 UpdateAimPosition(Vector2 currentAim, Vector2 touchInput, Vector2 enemyHeadPos, bool isEnemyVisible)
    {
        Vector2 calculatedAim = currentAim;
        float dpiScale = virtualDPISetting / 1000.0f;
        calculatedAim.x += touchInput.x * dragSensitivityMultiplier * dpiScale;
        calculatedAim.y += touchInput.y * dragSensitivityMultiplier * dpiScale;
        if (isEnemyVisible)
        {
            float distanceX = enemyHeadPos.x - calculatedAim.x;
            float distanceY = enemyHeadPos.y - calculatedAim.y;
            calculatedAim.x += distanceX * aimAssistSnapStrength * targetStickiness;
            calculatedAim.y += distanceY * aimAssistSnapStrength * targetStickiness;
        }
        return calculatedAim;
    }
    Vector2 ApplyRecoil(Vector2 rawRecoil)
    {
        Vector2 finalRecoil;
        finalRecoil.x = rawRecoil.x * recoilFactor;
        finalRecoil.y = rawRecoil.y * recoilFactor;
        return finalRecoil;
    }
    Vector2 ApplySpread(Vector2 rawSpread)
    {
        Vector2 finalSpread;
        finalSpread.x = rawSpread.x * (1.0f - recoilFactor);
        finalSpread.y = rawSpread.y * (1.0f - recoilFactor);
        return finalSpread;
    }
};
// --- Code của Bố NguyenTien ---
int main()
{
    SuperEasyAimSystem aimEngine;
    Vector2 currentAimAngle = {100.0f, 100.0f};   // Góc nhìn hiện tại
    Vector2 touchDragInput = {0.1f, 0.5f};        // Người chơi kéo tâm
    Vector2 enemyHeadPosition = {105.0f, 130.0f}; // Vị trí đầu kẻ địch
    bool enemyInView = true;
    Vector2 newAimAngle = aimEngine.UpdateAimPosition(currentAimAngle, touchDragInput, enemyHeadPosition, enemyInView);
    Vector2 simulatedRecoil = aimEngine.ApplyRecoil({5.0f, 12.0f});
    Vector2 simulatedSpread = aimEngine.ApplySpread({2.5f, 2.5f});
    return 0;
}