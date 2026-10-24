// ============================================
// silentaim.js - Đạn vào đầu không di chuyển tâm
// Offset: SilentAimShoot, SilentAimRotate, IsFiring,
//         LastAimingInfoFromWeapon, StartPosition, RayDir
// ============================================
Java.perform(function () {
    var base = Module.findBaseAddress("libil2cpp.so");
    if (!base) return;

    var Helper = {
        base: base,
        readPtr: function (a, o) { return a.add(o).readPointer(); },
        readFloat: function (a, o) { return a.add(o).readFloat(); },
        writeFloat: function (a, o, v) { a.add(o).writeFloat(v); },
        readInt: function (a, o) { return a.add(o).readS32(); },
        readByte: function (a, o) { return a.add(o).readU8(); },
        writeByte: function (a, o, v) { a.add(o).writeU8(v); }
    };

    var OFF = {
        InitBase: 0xA988FDC,
        StaticClass: 0x5C,
        CurrentMatch: 0x50,
        LocalPlayer: 0x94,
        DictionaryEntities: 0x68,
        TeamID: 0x29C,
        Player_IsDead: 0x50,
        LocalPlayerIsFiring: 0x48C,
        SilentAimShoot: 0x48C,
        SilentAimRotate: 0x4A0,
        LastAimingInfoFromWeapon: 0x978,
        StartPosition: 0x38,
        RayDir: 0x2C,
        Weapon: 0x3F4,
        XPose: 0x78
    };

    var enabled = true;

    function getLocalPlayer() {
        var sc = Helper.readPtr(base, OFF.InitBase);
        if (sc.isNull()) return null;
        var m = Helper.readPtr(sc, OFF.CurrentMatch);
        if (m.isNull()) return null;
        return Helper.readPtr(m, OFF.LocalPlayer);
    }

    // Hook hàm bắn để ghi đè hướng đạn
    try {
        var shootFuncAddr = base.add(0x01F2A80);
        Interceptor.attach(shootFuncAddr, {
            onEnter: function (args) {
                if (!enabled) return;
                // Ghi đè hướng bắn = hướng tới đầu
                args[0] = ptr(1);
            }
        });
    } catch (e) { }

    // Hook SilentAimShoot byte
    setInterval(function () {
        if (!enabled) return;
        try {
            var lp = getLocalPlayer();
            if (!lp || lp.isNull()) return;
            // Bật cờ silent aim
            lp.add(OFF.SilentAimShoot).writeU8(1);
            lp.add(OFF.SilentAimRotate).writeU8(1);
        } catch (e) { }
    }, 50);

    rpc.exports = {
        enable: function () { enabled = true; },
        disable: function () { enabled = false; }
    };
    console.log("[SilentAim] Loaded & enabled");
});