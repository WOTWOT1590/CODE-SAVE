// ============================================
// noshake.js - Xóa rung camera/tâm
// Offset: FollowCamera, Camera, MainCameraTransform, ViewMatrix
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
        FollowCamera: 0x450,
        Camera: 0x18,
        MainCameraTransform: 0x24C,
        ViewMatrix: 0xE8
    };

    var enabled = true;

    // Hook hàm shake
    try {
        var shakeFuncAddr = base.add(0x02C8D50);
        Interceptor.attach(shakeFuncAddr, {
            onEnter: function (args) {
                if (!enabled) return;
                args[0] = ptr(0);
                args[1] = ptr(0);
                args[2] = ptr(0);
            }
        });
    } catch (e) { }

    // Hook hàm apply camera offset
    try {
        var applyCamFuncAddr = base.add(0x02C8F20);
        Interceptor.attach(applyCamFuncAddr, {
            onEnter: function (args) {
                if (!enabled) return;
                args[0] = ptr(0);
            },
            onLeave: function (retval) {
                if (!enabled) return;
                retval.replace(ptr(0));
            }
        });
    } catch (e) { }

    // Vòng lặp ổn định camera
    setInterval(function () {
        if (!enabled) return;
        try {
            var sc = Helper.readPtr(base, OFF.InitBase);
            if (sc.isNull()) return;
            var m = Helper.readPtr(sc, OFF.CurrentMatch);
            if (m.isNull()) return;
            var lp = Helper.readPtr(m, OFF.LocalPlayer);
            if (lp.isNull()) return;
            var followCam = Helper.readPtr(lp, OFF.FollowCamera);
            if (followCam.isNull()) return;
            var cam = Helper.readPtr(followCam, OFF.Camera);
            if (cam.isNull()) return;
            var camTransform = Helper.readPtr(cam, OFF.MainCameraTransform);
            if (camTransform.isNull()) return;
            // Reset local position của camera về 0
            camTransform.add(0x38).writeFloat(0.0);
            camTransform.add(0x3C).writeFloat(0.0);
            camTransform.add(0x40).writeFloat(0.0);
        } catch (e) { }
    }, 30);

    rpc.exports = {
        enable: function () { enabled = true; },
        disable: function () { enabled = false; }
    };
    console.log("[NoShake] Loaded & enabled");
});