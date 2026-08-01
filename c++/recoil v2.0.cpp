#include <vector>
#include <random>
#include <algorithm>
#include <cmath>

struct RecoilPattern
{
    std::vector<float> verticalKick;   // Độ giật LÊN mỗi viên (độ/degree)
    std::vector<float> horizontalKick; // Biên độ lệch TRÁI/PHẢI mỗi viên
    float firstShotMultiplier;         // Viên đầu giật mạnh hơn bao nhiêu lần
    float recoveryDelay;               // Thời gian chờ trước khi camera tự hồi (giây)
    float recoverySpeed;               // Tốc độ camera trả về gốc (số càng lớn = hồi càng nhanh)
    float spreadIncreasePerShot;       // Độ tỏa nòng tăng mỗi viên (bắn càng lâu càng kém chính xác)
    float maxSpread;                   // Độ tỏa nòng tối đa
    float spreadDecayRate;             // Tốc độ độ tỏa nòng giảm khi ngừng bắn
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

    void OnStopFiring()
    {
        m_shotIndex = 0;
    }

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
// === CODE CỦA BỐ NGUYENTIEN ===
inline RecoilPattern MakeLightARPattern()
{
    RecoilPattern p;

    // --- FIX RUNG TÂM ---
    p.verticalKick = {0.5f, 0.6f, 0.7f, 0.65f, 0.6f, 0.55f, 0.5f, 0.45f, 0.4f, 0.4f};

    // --- FIX LỆCH ---
    p.horizontalKick = {0.1f, 0.15f, 0.2f, 0.25f, 0.3f, 0.3f, 0.35f, 0.35f, 0.4f, 0.4f}; // Để im là ngon cmnr
    p.firstShotMultiplier = 1.0f;

    // --- Deocanbiet
    p.recoveryDelay = 0.03f;

    // --- NHẸ TÂM ---
    p.recoverySpeed = 15.0f;

    // --- FIX LẠC ĐẠN ---
    p.spreadIncreasePerShot = 0.08f;

    // Deocangbiet
    p.spreadDecayRate = 3.0f;
    p.maxSpread = 2.5f;

    // spreadDecayRate: độ lệch đạn
    p.spreadDecayRate = 3.5f;
    // Giữ tham số nếu chỉnh có thể bị gà rán cho bay acc ngu hết người
    return p;
}