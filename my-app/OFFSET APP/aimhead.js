// ============================================
// aimhead.js - Tự động ngắm vào đầu kẻ địch
// Offset: HeadCollider, AimRotation, StartPosition, Weapon
// ============================================
Java.perform(function () {
    var base = Module.findBaseAddress("libil2cpp.so");
    if (!base) return;

    var Helper = {
        base: base,
        readPtr: function (a, o) { return a.add(o).readPointer(); },
        readFloat: function (a, o) { return a.add(o).readFloat(); },
        writeFloat: function (a, o, v) { a.add(o).writeFloat(v); },
        readInt: function (a, o) { return a.add(o).readS32(); }
    };

    var OFF = {
        InitBase: 0xA988FDC,
        StaticClass: 0x5C,
        CurrentMatch: 0x50,
        LocalPlayer: 0x94,
        DictionaryEntities: 0x68,
        TeamID: 0x29C,
        Player_IsDead: 0x50,
        AimRotation: 0x400,
        Weapon: 0x3F4,
        StartPosition: 0x38,
        HeadCollider: 0x4A4,
        LockedAimingCollider: 0x54,
        XPose: 0x78
    };

    var enabled = true;
    var FOV = 30.0;

    function getCurrentMatch() {
        var sc = Helper.readPtr(base, OFF.InitBase);
        if (sc.isNull()) return null;
        return Helper.readPtr(sc, OFF.CurrentMatch);
    }
    function getLocalPlayer() {
        var m = getCurrentMatch();
        if (!m || m.isNull()) return null;
        return Helper.readPtr(m, OFF.LocalPlayer);
    }

    // Hook hàm raycast aim
    try {
        var rayFuncAddr = base.add(0x02B4A10);
        Interceptor.attach(rayFuncAddr, {
            onEnter: function (args) {
                if (!enabled) return;
                // Ghi đè target collider thành HeadCollider
                args[1] = ptr(1);
            }
        });
    } catch (e) { }

    // Vòng lặp tự động tính góc ngắm
    setInterval(function () {
        if (!enabled) return;
        try {
            var lp = getLocalPlayer();
            var m = getCurrentMatch();
            if (!lp || lp.isNull() || !m || m.isNull()) return;

            var myTeam = Helper.readInt(lp, OFF.TeamID);
            var weapon = Helper.readPtr(lp, OFF.Weapon);
            if (weapon.isNull()) return;
            var startPos = weapon.add(OFF.StartPosition);

            var sx = startPos.readFloat();
            var sy = startPos.add(0x4).readFloat();
            var sz = startPos.add(0x8).readFloat();

            var dictEntities = Helper.readPtr(m, OFF.DictionaryEntities);
            if (dictEntities.isNull()) return;

            var closest = null;
            var closestDist = 99999;

            for (var i = 0; i < 50; i++) {
                try {
                    var enemy = Helper.readPtr(dictEntities, 0x10 + i * 0x8);
                    if (enemy.isNull()) continue;
                    if (Helper.readInt(enemy, OFF.TeamID) === myTeam) continue;
                    if (Helper.readByte(enemy, OFF.Player_IsDead) === 1) continue;

                    // Lấy vị trí XPose của enemy
                    var xpose = Helper.readPtr(enemy, OFF.XPose);
                    if (xpose.isNull()) continue;
                    var ex = Helper.readFloat(xpose, 0x38);
                    var ey = Helper.readFloat(xpose, 0x3C);
                    var ez = Helper.readFloat(xpose, 0x40);

                    var dx = ex - sx, dy = ey - sy, dz = ez - sz;
                    var dist = Math.sqrt(dx * dx + dy * dy + dz * dz);
                    if (dist < closestDist) {
                        closestDist = dist;
                        closest = { x: ex, y: ey, z: ez };
                    }
                } catch (e) { }
            }

            if (closest) {
                var dx = closest.x - sx;
                var dy = closest.y - sy;
                var dz = closest.z - sz;
                var yaw = Math.atan2(dx, dz) * 180 / Math.PI;
                var pitch = -Math.atan2(dy, Math.sqrt(dx * dx + dz * dz)) * 180 / Math.PI;

                Helper.writeFloat(lp, OFF.AimRotation, yaw);
                Helper.writeFloat(lp, OFF.AimRotation + 0x4, pitch);
            }
        } catch (e) { }
    }, 8);

    rpc.exports = {
        enable: function () { enabled = true; },
        disable: function () { enabled = false; },
        setFov: function (f) { FOV = f; }
    };
    console.log("[AimHead] Loaded & enabled");
});