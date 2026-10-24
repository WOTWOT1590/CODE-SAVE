// ============================================
// lightaim.js - Giảm độ nhạy aim, làm mượt tâm
// Offset: sensitivity hook, AimRotation, ViewMatrix
// ============================================
Java.perform(function () {
    var base = Module.findBaseAddress("libil2cpp.so");
    if (!base) return;

    var Helper = {
        base: base,
        readPtr: function (a, o) { return a.add(o).readPointer(); },
        readFloat: function (a, o) { return a.add(o).readFloat(); },
        writeFloat: function (a, o, v) { a.add(o).writeFloat(v); }
    };

    var OFF = {
        InitBase: 0xA988FDC,
        StaticClass: 0x5C,
        CurrentMatch: 0x50,
        LocalPlayer: 0x94,
        AimRotation: 0x400,
        ViewMatrix: 0xE8
    };

    var enabled = true;
    var multiplier = 0.35;
    var lastYaw = 0, lastPitch = 0;

    // Hook sensitivity
    try {
        var sensFuncAddr = base.add(0x019FE70);
        Interceptor.attach(sensFuncAddr, {
            onEnter: function (args) {
                if (!enabled) return;
                args[0] = ptr(multiplier);
            }
        });
    } catch (e) { }

    // Làm mượt aim rotation
    setInterval(function () {
        if (!enabled) return;
        try {
            var sc = Helper.readPtr(base, OFF.InitBase);
            if (sc.isNull()) return;
            var m = Helper.readPtr(sc, OFF.CurrentMatch);
            if (m.isNull()) return;
            var lp = Helper.readPtr(m, OFF.LocalPlayer);
            if (lp.isNull()) return;
            var aimRot = lp.add(OFF.AimRotation);
            var yaw = aimRot.readFloat();
            var pitch = aimRot.add(0x4).readFloat();
            // Nội suy 30% giữa giá trị cũ và mới
            var smoothYaw = lastYaw * 0.3 + yaw * 0.7;
            var smoothPitch = lastPitch * 0.3 + pitch * 0.7;
            aimRot.writeFloat(smoothYaw);
            aimRot.add(0x4).writeFloat(smoothPitch);
            lastYaw = smoothYaw;
            lastPitch = smoothPitch;
        } catch (e) { }
    }, 16);

    rpc.exports = {
        enable: function () { enabled = true; },
        disable: function () { enabled = false; },
        setMultiplier: function (v) { multiplier = v; }
    };
    console.log("[LightAim] Loaded & enabled, multiplier=" + multiplier);
});