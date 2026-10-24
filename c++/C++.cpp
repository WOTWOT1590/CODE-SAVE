// ============================================================
// ULTIMATE AIM & PERFORMANCE ENGINE (V2.0)
// Gộp toàn bộ tính năng: Aimlock, Recoil, Sensitivity, Anti-Lag
// ============================================================

#include <iostream>
#include <variant>
#include <optional>
#include <map>
#include <string>
#include <algorithm>

class SettingsManager
{
public:
    using SettingValue = std::variant<int, double, bool, std::string>;

    void setSetting(const std::string &key, SettingValue value)
    {
        settings[key] = std::move(value);
    }

    std::optional<SettingValue> getSetting(const std::string &key) const
    {
        auto it = settings.find(key);
        if (it != settings.end())
        {
            return it->second;
        }
        return std::nullopt;
    }

    void applySettings()
    {
        // --- 1. NHÓM HỆ THỐNG / KẾT NỐI ---
        if (auto setting = getSetting("com.act_conf_seclect_sync_device"))
        {
            if (auto val = std::get_if<int>(&(*setting)))
            {
                if (*val == 100)
                    std::cout << "[System] Device sync: ENABLED (100)" << std::endl;
            }
        }
        if (auto setting = getSetting("Key_act_allow.file_code_unlock_connectInject"))
        {
            std::cout << "[System] Unlock connect inject: ENABLED" << std::endl;
        }
        if (auto setting = getSetting("RDR_aimLockBase64"))
        {
            if (auto val = std::get_if<std::string>(&(*setting)))
            {
                std::cout << "[System] RDR AimLockBase64 Address: " << *val << std::endl;
            }
        }

        // --- 2. NHÓM NHẸ TÂM (FEATHER LIGHT & SENSITIVITY) ---
        if (auto setting = getSetting("aim_smooth_factor"))
        {
            if (auto val = std::get_if<double>(&(*setting)))
            {
                // 0.01 - 0.1: Kéo siêu nhẹ tâm | 0.5 - 1.0: Tâm đầm, giữ nguyên vị trí
                std::cout << "[Nhe Tam] Smooth Factor: " << std::clamp(*val, 0.0, 1.0) << std::endl;
            }
        }
        if (auto setting = getSetting("feather_light_aim"))
        {
            if (auto val = std::get_if<double>(&(*setting)))
            {
                // 0.01: Tâm bay nhẹ như lông hồng | 1.0: Tâm đầm, ma sát cao
                std::cout << "[Nhe Tam] Feather Light Aim: " << std::clamp(*val, 0.01, 1.0) << std::endl;
            }
        }
        if (auto setting = getSetting("touch_drag_friction"))
        {
            if (auto val = std::get_if<double>(&(*setting)))
            {
                // 0.0: Triệt tiêu ma sát màn hình | 0.8 - 1.0: Trở lực cao, kéo đầm
                std::cout << "[Nhe Tam] Touch Drag Friction: " << std::clamp(*val, 0.0, 1.0) << std::endl;
            }
        }
        if (auto setting = getSetting("swipe_acceleration"))
        {
            if (auto val = std::get_if<double>(&(*setting)))
            {
                // > 5.0: Gia tốc cực mạnh | 1.0 - 2.0: Vuốt thanh thoát chuẩn xác
                std::cout << "[Nhe Tam] Swipe Acceleration: " << *val << std::endl;
            }
        }
        if (auto setting = getSetting("mouse_sensitivity"))
        {
            if (auto val = std::get_if<double>(&(*setting)))
            {
                std::cout << "[Nhe Tam] Overall Sensitivity: " << std::clamp(*val, 0.1, 5.0) << std::endl;
            }
        }

        // --- 3. NHÓM FIX RUNG & GIẬT (RECOIL & SHAKE) ---
        if (auto setting = getSetting("recoil_reduction_ratio"))
        {
            if (auto val = std::get_if<double>(&(*setting)))
            {
                // 1.0: Triệt tiêu 100% độ rung giật | 0.0: Rung mặc định
                std::cout << "[Xoa Rung] Recoil Reduction Ratio: " << std::clamp(*val, 0.0, 1.0) << std::endl;
            }
        }
        if (auto setting = getSetting("zero_recoil_shake"))
        {
            if (auto val = std::get_if<double>(&(*setting)))
            {
                // 1.0: Màn hình tĩnh lặng hoàn toàn | 0.0: Rung nguyên bản
                std::cout << "[Xoa Rung] Zero Recoil Shake: " << std::clamp(*val, 0.0, 1.0) << std::endl;
            }
        }
        if (auto setting = getSetting("anti_shake_xy"))
        {
            if (auto val = std::get_if<double>(&(*setting)))
            {
                // 1.0: Cố định trục XY tĩnh 100% | 0.5: Giảm 50% độ rung
                std::cout << "[Xoa Rung] Anti Shake XY: " << std::clamp(*val, 0.0, 1.0) << std::endl;
            }
        }
        if (auto setting = getSetting("bullet_spread_control"))
        {
            if (auto val = std::get_if<double>(&(*setting)))
            {
                // 1.0: Đạn gom 1 điểm, không xòe | 0.0: Tản đạn gốc
                std::cout << "[Xoa Rung] Bullet Spread Control: " << std::clamp(*val, 0.0, 1.0) << std::endl;
            }
        }
        if (auto setting = getSetting("recoil_vertical_kick"))
        {
            if (auto val = std::get_if<double>(&(*setting)))
            {
                // 0.4 - 0.9: Súng êm | 1.4 - 1.9: Giật mạnh
                std::cout << "[Xoa Rung] Vertical Kick: " << std::clamp(*val, 0.0, 3.0) << std::endl;
            }
        }

        // --- 4. NHÓM BÁM ĐẦU & AIMLOCK (AIM ASSIST) ---
        if (auto setting = getSetting("enable_head_lock"))
        {
            if (auto val = std::get_if<bool>(&(*setting)))
            {
                // True: Tự động khóa vùng đầu | False: Tắt
                std::cout << "[Bam Dau] Enable Head Lock: " << (*val ? "ON" : "OFF") << std::endl;
            }
        }
        if (auto setting = getSetting("aimlock_extreme"))
        {
            if (auto val = std::get_if<bool>(&(*setting)))
            {
                // True: Khóa cứng tâm, không văng khi địch lướt nhanh
                std::cout << "[Bam Dau] Extreme Aimlock: " << (*val ? "ON" : "OFF") << std::endl;
            }
        }
        if (auto setting = getSetting("head_magnet_radius"))
        {
            if (auto val = std::get_if<int>(&(*setting)))
            {
                // 30-50: Hút mượt 1 mục tiêu | >100: Quá to, dễ loạn tâm
                std::cout << "[Bam Dau] Magnet Radius: " << *val << std::endl;
            }
        }
        if (auto setting = getSetting("aimlock_target_priority"))
        {
            if (auto val = std::get_if<std::string>(&(*setting)))
            {
                // "head": Thẳng vào sọ | "body": Gom đạn ngực/thân
                std::cout << "[Bam Dau] Target Priority: " << *val << std::endl;
            }
        }
        if (auto setting = getSetting("aimlock_tracking_speed"))
        {
            if (auto val = std::get_if<double>(&(*setting)))
            {
                // 0.9-1.0: Bám cực nhanh | 0.2-0.4: Bám tự nhiên
                std::cout << "[Bam Dau] Tracking Speed: " << std::clamp(*val, 0.0, 1.0) << std::endl;
            }
        }
        if (auto setting = getSetting("head_snap_force"))
        {
            if (auto val = std::get_if<double>(&(*setting)))
            {
                // 1.0: Hút sập gắt, cấm văng | 0.3: Hút nhẹ
                std::cout << "[Bam Dau] Head Snap Force: " << std::clamp(*val, 0.0, 1.0) << std::endl;
            }
        }
        if (auto setting = getSetting("snap_sticky_strength"))
        {
            if (auto val = std::get_if<double>(&(*setting)))
            {
                // 1.0: Dính chặt không lố | 0.0: Dễ trượt
                std::cout << "[Bam Dau] Sticky Strength: " << std::clamp(*val, 0.0, 1.0) << std::endl;
            }
        }
        if (auto setting = getSetting("anti_overshoot_lock"))
        {
            if (auto val = std::get_if<bool>(&(*setting)))
            {
                // True: Đạp phanh khẩn cấp, chạm đỉnh đầu là khựng
                std::cout << "[Bam Dau] Anti-Overshoot Lock: " << (*val ? "ON" : "OFF") << std::endl;
            }
        }

        // --- 5. NHÓM TỐI ƯU & FIX LAG (PERFORMANCE) ---
        if (auto setting = getSetting("fps_stabilizer_level"))
        {
            if (auto val = std::get_if<int>(&(*setting)))
            {
                // 3 (Max): Ép xung CPU/GPU, FPS tối đa | 1 (Low): Tiết kiệm pin
                std::cout << "[Fix Lag] FPS Stabilizer Level: " << std::clamp(*val, 0, 3) << std::endl;
            }
        }
        if (auto setting = getSetting("render_skip_effects"))
        {
            if (auto val = std::get_if<bool>(&(*setting)))
            {
                // True: Xóa hiệu ứng thừa (khói, súng), triệt tiêu giật 100%
                std::cout << "[Fix Lag] Render Skip Effects: " << (*val ? "ON" : "OFF") << std::endl;
            }
        }
        if (auto setting = getSetting("camera_fov"))
        {
            if (auto val = std::get_if<int>(&(*setting)))
            {
                std::cout << "[Fix Lag/View] Camera FOV: " << std::clamp(*val, 60, 120) << std::endl;
            }
        }
    }

    void displaySettings() const
    {
        std::cout << "\n--- Current Loaded Settings ---" << std::endl;
        for (const auto &[key, value] : settings)
        {
            std::cout << key << " = ";
            std::visit([](auto &&val)
                       { std::cout << val; }, value);
            std::cout << std::endl;
        }
        std::cout << "-------------------------------\n"
                  << std::endl;
    }

    double smoothAim(double currentValue, double targetValue, double factor)
    {
        return currentValue + factor * (targetValue - currentValue);
    }

private:
    std::map<std::string, SettingValue> settings;
};

int main()
{
    SettingsManager settingsManager;

    // 1. System Set
    settingsManager.setSetting("com.act_conf_seclect_sync_device", 100);
    settingsManager.setSetting("Key_act_allow.file_code_unlock_connectInject", "enabled");
    settingsManager.setSetting("RDR_aimLockBase64", "0x7608F0");
    settingsManager.setSetting("set", 100);
    settingsManager.setSetting("on_auto_cws", 85);

    // 2. Nhe Tam (Feather / Sens)
    settingsManager.setSetting("aim_smooth_factor", 0.05);
    settingsManager.setSetting("feather_light_aim", 0.01);
    settingsManager.setSetting("touch_drag_friction", 0.0);
    settingsManager.setSetting("swipe_acceleration", 1.5);
    settingsManager.setSetting("mouse_sensitivity", 1.2);

    // 3. Xoa Rung (Recoil / Anti-shake)
    settingsManager.setSetting("recoil_reduction_ratio", 1.0);
    settingsManager.setSetting("zero_recoil_shake", 1.0);
    settingsManager.setSetting("anti_shake_xy", 1.0);
    settingsManager.setSetting("bullet_spread_control", 1.0);
    settingsManager.setSetting("recoil_vertical_kick", 0.4);

    // 4. Bam Dau (Aimlock / Assist)
    settingsManager.setSetting("enable_head_lock", true);
    settingsManager.setSetting("aimlock_extreme", true);
    settingsManager.setSetting("head_magnet_radius", 45);
    settingsManager.setSetting("aimlock_target_priority", "head");
    settingsManager.setSetting("aimlock_tracking_speed", 0.95);
    settingsManager.setSetting("head_snap_force", 1.0);
    settingsManager.setSetting("snap_sticky_strength", 0.95);
    settingsManager.setSetting("anti_overshoot_lock", true);

    // 5. Fix Lag (Performance / FOV)
    settingsManager.setSetting("fps_stabilizer_level", 3);
    settingsManager.setSetting("render_skip_effects", true);
    settingsManager.setSetting("camera_fov", 90);

    // Thuc thi & Hien thi
    settingsManager.applySettings();
    settingsManager.displaySettings();

    // Vong lap hieu chinh Tam (Lerp smooth)
    std::cout << "--- Smoothing Crosshair Execution ---" << std::endl;
    double currentValue = 20.0;
    double targetValue = 100.0; // Dinh dau
    double factor = 0.2;        // Toc do bám

    for (int i = 0; i < 50; ++i)
    {
        currentValue = settingsManager.smoothAim(currentValue, targetValue, factor);
        std::cout << "Smoothed value at step " << i + 1 << ": " << currentValue << std::endl;
    }

    return 0; // Tra ve 0 de hien thi qua trinh thuc thi an toan hoan tat
}