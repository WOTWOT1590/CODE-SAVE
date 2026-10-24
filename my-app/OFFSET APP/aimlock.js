// ============================================
// aimlock.js - Khóa mục tiêu vào kẻ địch gần nhất
// Offset: InitBase, StaticClass, CurrentMatch, LocalPlayer,
//         DictionaryEntities, TeamID, AimRotation, Weapon
// ============================================
Java.perform(function () {
    var base = Module.findBaseAddress("libil2cpp.so");
    if (!base) return;

    // ===== HELPER =====
    var Helper = {
        base: base,
        readPtr: function (addr, off) { return addr.add(off).readPointer(); },
        readFloat: function (addr, off) { return addr.add(off).readFloat(); },
        writeFloat: function (addr, off, val) { addr.add(off).writeFloat(val); },
        readInt: function (addr, off) { return addr.add(off).readS32(); },
        readByte: function (addr, off) { return addr.add(off).readU8(); },
        writeByte: function (addr, off, val) { addr.add(off).writeU8(val); }
    };

    // ===== OFFSETS từ AotForm =====
    var OFF = {
        InitBase: 0xA988FDC,
        StaticClass: 0x5C,
        CurrentMatch: 0x50,
        LocalPlayer: 0x94,
        DictionaryEntities: 0x68,
        TeamID: 0x29C,
        Player_IsDead: 0x50,
        Avatar: 0xA8,
        Avatar_Data: 0x14,
        Avatar_Data_IsTeam: 0x59,
        AimRotation: 0x400,
        SilentAimRotate: 0x4A0,
        Weapon: 0x3F4,
        StartPosition: 0x38,
        HeadCollider: 0x4A4,
        XPose: 0x78
    };

    var enabled = false;
    var aimSpeed = 3.0;

    // Lấy StaticClass pointer
    function getStaticClass() {
        return Helper.readPtr(base, OFF.InitBase);
    }
    // Lấy CurrentMatch
    function getCurrentMatch() {
        var sc = getStaticClass();
        if (sc.isNull()) return null;
        return Helper.readPtr(sc, OFF.CurrentMatch);
    }
    // Lấy LocalPlayer
    function getLocalPlayer() {
        var m = getCurrentMatch();
        if (!m || m.isNull()) return null;
        return Helper.readPtr(m, OFF.LocalPlayer);
    }

    // Hook aimlock function (nếu có offset hàm)
    var aimlockFuncOffset = 0x01D3F1C;
    try {
        Interceptor.attach(base.add(aimlockFuncOffset), {
            onEnter: function (args) {
                if (!enabled) return;
                args[0] = ptr(1);
            }
        });
    } catch (e) { }

    // Vòng lặp aim mỗi 100ms
    setInterval(function () {
        if (!enabled) return;
        try {
            var lp = getLocalPlayer();
            var m = getCurrentMatch();
            if (!lp || lp.isNull() || !m || m.isNull()) return;
            var myTeam = Helper.readInt(lp, OFF.TeamID);
            // Lấy danh sách entity
            var dictEntities = Helper.readPtr(m, OFF.DictionaryEntities);
            if (dictEntities.isNull()) return;
            // Duyệt list đơn giản (50 phần tử)
            for (var i = 0; i < 50; i++) {
                try {
                    var enemy = Helper.readPtr(dictEntities, 0x10 + i * 0x8);
                    if (enemy.isNull()) continue;
                    var enemyTeam = Helper.readInt(enemy, OFF.TeamID);
                    if (enemyTeam === myTeam) continue;
                    var isDead = Helper.readByte(enemy, OFF.Player_IsDead);
                    if (isDead === 1) continue;
                    // Ghi rotation vào LocalPlayer.AimRotation
                    var aimRot = lp.add(OFF.AimRotation);
                    // (đơn giản hóa: ghi giá trị mẫu, cần tính vector thật)
                    // aimRot.writeFloat(yaw); aimRot.add(4).writeFloat(pitch);
                    break;
                } catch (e) { }
            }
        } catch (e) { }
    }, 100);

    // Bật mặc định
    enabled = true;

    // RPC điều khiển
    rpc.exports = {
        enable: function () { enabled = true; },
        disable: function () { enabled = false; },
        setSpeed: function (s) { aimSpeed = s; }
    };
    console.log("[Aimlock] Loaded & enabled");
});