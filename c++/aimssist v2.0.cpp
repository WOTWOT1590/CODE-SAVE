#include <vector>
#include <cmath>
#include <limits>

struct Vec2
{
    float x, y;
};

struct Target
{
    int id;
    Vec2 screenAngleOffset;
    bool hasLineOfSight;
};

struct AimAssistConfig
{
    bool enabled;
    float magnetismRadius;    // "BÁM ĐẦU": bán kính hút crosshair về target
    float magnetismStrength;  // "BÁM ĐẦU": độ mạnh hút (0-1, KHÔNG BAO GIỜ = 1)
    float slowdownZone;       // vùng làm chậm chuột khi crosshair gần địch
    float slowdownFactor;     // hệ số giảm tốc độ xoay trong vùng slowdown
    bool onlyActiveWhileADS;  // chỉ hỗ trợ khi đang ngắm (ADS), không hỗ trợ hip-fire
    bool requiresLineOfSight; // không hỗ trợ xuyên tường
    float maxLockAngle;       // "NHẠY"/FOV: góc tối đa hệ thống còn "thấy" được target để hỗ trợ
};

class AimAssistSystem
{
public:
    explicit AimAssistSystem(const AimAssistConfig &cfg) : m_cfg(cfg) {}

    void ComputeAssist(const std::vector<Target> &targets,
                       bool isADS,
                       float deltaTime,
                       Vec2 &outCameraAdjust,
                       float &outSensitivityMultiplier)
    {
        outCameraAdjust = {0.0f, 0.0f};
        outSensitivityMultiplier = 1.0f;

        if (!m_cfg.enabled)
            return;
        if (m_cfg.onlyActiveWhileADS && !isADS)
            return;

        const Target *best = FindClosestValidTarget(targets);
        if (!best)
            return;

        float dist = AngleDistance(best->screenAngleOffset);

        if (dist <= m_cfg.magnetismRadius)
        {

            float pull = m_cfg.magnetismStrength * deltaTime * 10.0f;
            pull = std::min(pull, 1.0f);

            outCameraAdjust.x = best->screenAngleOffset.x * pull;
            outCameraAdjust.y = best->screenAngleOffset.y * pull;
        }
        else if (dist <= m_cfg.slowdownZone)
        {
            outSensitivityMultiplier = m_cfg.slowdownFactor;
        }
    }

private:
    AimAssistConfig m_cfg;

    float AngleDistance(const Vec2 &offset) const
    {
        return std::sqrt(offset.x * offset.x + offset.y * offset.y);
    }

    const Target *FindClosestValidTarget(const std::vector<Target> &targets) const
    {
        const Target *best = nullptr;
        float bestDist = std::numeric_limits<float>::max();

        for (const auto &t : targets)
        {
            if (m_cfg.requiresLineOfSight && !t.hasLineOfSight)
                continue;

            float dist = AngleDistance(t.screenAngleOffset);
            if (dist > m_cfg.maxLockAngle)
                continue;

            if (dist < bestDist)
            {
                bestDist = dist;
                best = &t;
            }
        }
        return best;
    }
};

// === CODE CỦA BỐ NGUYENTIEN ===
inline AimAssistConfig MakeLightAimAssistConfig()
{
    AimAssistConfig c;

    c.enabled = true;

    // --- BÁM ĐẦU
    c.magnetismRadius = 5.0f;

    // --- BÁM ĐẦU Buoi
    c.magnetismStrength = 0.7f;

    // --- VÙNG LÀM CHẬM CHUỘT ---
    c.slowdownZone = 9.0f;

    // --- HỆ SỐ GIẢM TỐC ---
    c.slowdownFactor = 0.5f;
    // --- SKIBIDI TOLET ---
    c.onlyActiveWhileADS = true;
    c.requiresLineOfSight = true;
    // NHÌN CÁI ĐB CODE ĐẲNG CẤP CHỨ GÌ
    // NHẠY PHI5
    c.maxLockAngle = 18.0f;
    // CHỈNH THAM SỐ BỊ BAND NGU THÌ NGUUUUU
    return c;
}