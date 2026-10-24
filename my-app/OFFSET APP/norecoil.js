// ============================================
// norecoil.js - Xóa độ giật súng
// Offset: Weapon, WeaponData, WeaponRecoil, UnkPlayerWeaponInfoClass
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
        Weapon: 0x3F4,
        WeaponData: 0x58,
        WeaponRecoil: 0xC,
        UnkPlayerWeaponInfoClass: 0x4A8,
        WeaponInfo: 0x64
    };

    var enabled = true;

    function getLocalPlayer() {
        var sc = Helper.readPtr(base, OFF.InitBase);
        if (sc.isNull()) return null;
        var m = Helper.readPtr(sc, OFF.CurrentMatch);
        if (m.isNull()) return null;
        return Helper.readPtr(m, OFF.LocalPlayer);
    }

    // Hook hàm recoil
    try {
        var recoilFuncAddr = base.add(0x02A1B9C);
        Interceptor.attach(recoilFuncAddr, {
            onEnter: function (args) {
                if (!enabled) return;
                args[0] = ptr(0);
                args[1] = ptr(0);
            },
            onLeave: function (retval) {
                if (!enabled) return;
                retval.replace(ptr(0));
            }
        });
    } catch (e) { }

    // Vòng lặp ghi 0 recoil vào weapon
    setInterval(function () {
        if (!enabled) return;
        try {
            var lp = getLocalPlayer();
            if (!lp || lp.isNull()) return;
            var weapon = Helper.readPtr(lp, OFF.Weapon);
            if (weapon.isNull()) return;
            var wData = Helper.readPtr(weapon, OFF.WeaponData);
            if (wData.isNull()) return;
            // Ghi recoil X, Y = 0
            wData.add(OFF.WeaponRecoil).writeFloat(0.0);
            wData.add(OFF.WeaponRecoil + 0x4).writeFloat(0.0);
        } catch (e) { }
    }, 30);

    rpc.exports = {
        enable: function () { enabled = true; },
        disable: function () { enabled = false; }
    };
    console.log("[NoRecoil] Loaded & enabled");
});