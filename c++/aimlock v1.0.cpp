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
    bool enabled = true;
    float magnetismRadius = 4.0f;
    float magnetismStrength = 0.35f;
    float slowdownZone = 9.0f;
    float slowdownFactor = 0.55f;
    bool onlyActiveWhileADS = true;
    bool requiresLineOfSight = true;
    float maxLockAngle = 14.0f;
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

// === Code của Bố NGUYENTIEN ===
inline AimAssistConfig MakeLightConfig()
{
    AimAssistConfig c;
    c.magnetismRadius = 4.0f;
    c.magnetismStrength = 0.35f; // kéo rõ hơn, cảm giác "dính" nhẹ
    c.slowdownZone = 9.0f;
    c.slowdownFactor = 0.5f; // giảm sens nhiều hơn khi gần địch -> dễ giữ ngắm
    c.maxLockAngle = 14.0f;  // FOV hỗ trợ rộng hơn
    return c;
}