// Code của Bố NGUYENTIEN

#include <vector>
#include <random>
#include <cmath>
#include <algorithm>
#include <limits>
// độ rung tâm / độ nhẹ tâm / độ mượt
struct RecoilPattern
{
    std::vector<float> verticalKick;
    std::vector<float> horizontalKick;
    float firstShotMultiplier;
    float recoveryDelay;
    float recoverySpeed;
    float spreadIncreasePerShot;
    float maxSpread;
    float spreadDecayRate;
};

class RecoilSystem
{
public:
    RecoilSystem(const RecoilPattern &pattern)
        : m_pattern(pattern), m_rng(std::random_device{}()) {}

    void OnShotFired()
    {
        int idx = std::min(m_shotIndex, (int)m_pattern.verticalKick.size() - 1);
        float vKick = m_pattern.verticalKick[idx];
        if (m_shotIndex == 0)
            vKick *= m_pattern.firstShotMultiplier;

        float hRange = m_pattern.horizontalKick[idx];
        std::uniform_real_distribution<float> dist(-hRange, hRange);
        float hKick = dist(m_rng);

        m_pendingPitchOffset += vKick;
        m_pendingYawOffset += hKick;
        m_currentSpread = std::min(m_currentSpread + m_pattern.spreadIncreasePerShot,
                                   m_pattern.maxSpread);
        m_timeSinceLastShot = 0.0f;
        m_shotIndex++;
    }

    void OnStopFiring() { m_shotIndex = 0; }

    void Tick(float deltaTime, float &outCameraPitch, float &outCameraYaw)
    {
        m_timeSinceLastShot += deltaTime;

        outCameraPitch -= m_pendingPitchOffset;
        outCameraYaw += m_pendingYawOffset;
        m_recoilAccumPitch += m_pendingPitchOffset;
        m_recoilAccumYaw += m_pendingYawOffset;
        m_pendingPitchOffset = 0.0f;
        m_pendingYawOffset = 0.0f;

        if (m_timeSinceLastShot >= m_pattern.recoveryDelay)
        {
            float recoverStep = m_pattern.recoverySpeed * deltaTime;
            float pitchRecover = std::min(recoverStep, std::abs(m_recoilAccumPitch));
            float yawRecover = std::min(recoverStep, std::abs(m_recoilAccumYaw));
            float pitchSign = m_recoilAccumPitch > 0 ? 1.0f : -1.0f;
            float yawSign = m_recoilAccumYaw > 0 ? 1.0f : -1.0f;

            outCameraPitch += pitchSign * pitchRecover;
            outCameraYaw -= yawSign * yawRecover;
            m_recoilAccumPitch -= pitchSign * pitchRecover;
            m_recoilAccumYaw -= yawSign * yawRecover;
        }

        if (m_timeSinceLastShot > m_pattern.recoveryDelay)
        {
            m_currentSpread = std::max(0.0f,
                                       m_currentSpread - m_pattern.spreadDecayRate * deltaTime);
        }
    }

    float GetCurrentSpread() const { return m_currentSpread; }

private:
    RecoilPattern m_pattern;
    std::mt19937 m_rng;
    int m_shotIndex = 0;
    float m_timeSinceLastShot = 0.0f;
    float m_pendingPitchOffset = 0.0f;
    float m_pendingYawOffset = 0.0f;
    float m_recoilAccumPitch = 0.0f;
    float m_recoilAccumYaw = 0.0f;
    float m_currentSpread = 0.0f;
};

RecoilPattern MakeEasyPattern()
{
    RecoilPattern p;
    p.verticalKick = {0.25f, 0.3f, 0.35f, 0.32f, 0.3f, 0.27f, 0.25f, 0.22f, 0.2f, 0.2f};
    p.horizontalKick = {0.05f, 0.07f, 0.1f, 0.12f, 0.15f, 0.15f, 0.17f, 0.17f, 0.2f, 0.2f};
    p.firstShotMultiplier = 1.05f; // viên đầu gần như không giật thêm
    p.recoverySpeed = 50.0f;       // NHẸ TÂM
    p.maxSpread = 1.0f;
    return p;
}
// bám đầu / nhạy / độ lố đầu
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
    float magnetismRadius;
    float magnetismStrength;
    float slowdownZone;
    float slowdownFactor;
    bool onlyActiveWhileADS;
    bool requiresLineOfSight;
    float maxLockAngle;
    float overshootDamping; // fix lố đầu
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

            // Fix lố đầu
            float proximityFactor = std::max(0.3f, dist / m_cfg.magnetismRadius);
            pull *= (1.0f - m_cfg.overshootDamping * (1.0f - proximityFactor));

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
AimAssistConfig MakeEasyAimAssistConfig()
{
    AimAssistConfig c;
    c.enabled = true;
    c.magnetismStrength = 0.6f; // bám đầu mạnh nhưng vẫn < 1.0 (không auto-lock cứng)
    c.slowdownZone = 9.0f;
    c.slowdownFactor = 0.6f; // ghì chậm hơn nhiều khi gần địch
    c.onlyActiveWhileADS = true;
    c.requiresLineOfSight = true;
    c.maxLockAngle = 20.0f;
    c.overshootDamping = 0.7f; // fix lố đầu
    return c;
}

enum class HitZone
{
    Head,
    Body,
    Limb
};

struct DamageConfig
{
    float baseDamage;
    float headshotMultiplier; // headshot
    float limbMultiplier;
};

class DamageSystem
{
public:
    explicit DamageSystem(const DamageConfig &cfg) : m_cfg(cfg) {}

    float CalculateDamage(HitZone zone) const
    {
        switch (zone)
        {
        case HitZone::Head:
            return m_cfg.baseDamage * m_cfg.headshotMultiplier;
        case HitZone::Limb:
            return m_cfg.baseDamage * m_cfg.limbMultiplier;
        default:
            return m_cfg.baseDamage;
        }
    }

private:
    DamageConfig m_cfg;
};
DamageConfig MakeEasyDamageConfig()
{
    DamageConfig d;
    d.headshotMultiplier = 2.0f; // headshot
    d.limbMultiplier = 0.8f;
    return d;
}
int main()
{
    RecoilSystem recoil(MakeEasyPattern());
    AimAssistSystem aimAssist(MakeEasyAimAssistConfig());
    DamageSystem damage(MakeEasyDamageConfig());

    recoil.OnShotFired();

    float pitch = 0.0f, yaw = 0.0f;
    float deltaTime = 0.016f;
    recoil.Tick(deltaTime, pitch, yaw);

    std::vector<Target> enemies = {
        {1, {1.5f, 0.8f}, true}};
    Vec2 camAdjust;
    float sensMult;
    aimAssist.ComputeAssist(enemies, true, deltaTime, camAdjust, sensMult);

    float headDamage = damage.CalculateDamage(HitZone::Head);

    return 0;
}