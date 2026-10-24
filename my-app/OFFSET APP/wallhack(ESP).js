// ============================================
// esp_draw.js - Vẽ ESP lên màn hình Android
// Dùng: WindowManager + View + Canvas (Java bridge từ Frida)
// Anti-ban: FLAG_SECURE + TYPE_APPLICATION_OVERLAY
// ============================================
Java.perform(function () {
    var base = Module.findBaseAddress("libil2cpp.so");
    if (!base) return;
    console.log("[ESP_DRAW] Bắt đầu khởi tạo overlay");

    // ===== HELPER OFFSET =====
    var H = {
        base: base,
        rp: function (a, o) { return a.add(o).readPointer(); },
        rf: function (a, o) { return a.add(o).readFloat(); },
        ri: function (a, o) { return a.add(o).readS32(); },
        rb: function (a, o) { return a.add(o).readU8(); }
    };

    var OFF = {
        InitBase: 0xA988FDC,
        StaticClass: 0x5C,
        CurrentMatch: 0x50,
        LocalPlayer: 0x94,
        DictionaryEntities: 0x68,
        TeamID: 0x29C,
        Player_IsDead: 0x50,
        Player_Name: 0x2DC,
        PlayerID: 0x268,
        XPose: 0x78,
        AvatarManager: 0x4C0,
        Avatar: 0xA8,
        Avatar_IsVisible: 0x95,
        FollowCamera: 0x450,
        Camera: 0x18,
        ViewMatrix: 0xE8
    };

    // ===== JAVA IMPORTS QUA FRIDA =====
    var J = {
        ActivityThread: Java.use("android.app.ActivityThread"),
        WindowManager: Java.use("android.view.WindowManager"),
        WindowManagerLayoutParams: Java.use("android.view.WindowManager$LayoutParams"),
        PixelFormat: Java.use("android.graphics.PixelFormat"),
        Gravity: Java.use("android.view.Gravity"),
        View: Java.use("android.view.View"),
        Canvas: Java.use("android.graphics.Canvas"),
        Paint: Java.use("android.graphics.Paint"),
        Color: Java.use("android.graphics.Color"),
        Point: Java.use("android.graphics.Point"),
        Typeface: Java.use("android.graphics.Typeface"),
        FrameLayout: Java.use("android.widget.FrameLayout"),
        Context: Java.use("android.content.Context")
    };

    // ===== LẤY CONTEXT + SCREEN SIZE =====
    var ctx = null;
    var wm = null;
    var screenW = 1080, screenH = 1920;

    try {
        ctx = J.ActivityThread.currentApplication().getApplicationContext();
        wm = ctx.getSystemService("window");
        var d = wm.getDefaultDisplay();
        var pt = J.Point.$new();
        d.getRealSize(pt);
        screenW = pt.x.value;
        screenH = pt.y.value;
        console.log("[ESP_DRAW] Screen " + screenW + "x" + screenH);
    } catch (e) {
        console.log("[ESP_DRAW] Lỗi lấy context: " + e);
        return;
    }

    // ===== TẠO VIEW OVERLAY VỚI CANVAS =====
    var EspView = Java.registerClass({
        name: "com.esp.OverlayView",
        superclass: J.View,
        fields: {
            data: "[F" // mảng float [x1,y1,x2,y2, dist, team, x1h,y1h] * N
        },
        methods: {
            onDraw: function (canvas) {
                this.onDraw(canvas); // gọi super
                try {
                    var arr = this.data.value;
                    if (!arr || arr.length === 0) return;

                    // Paint cho box
                    var paintBox = J.Paint.$new();
                    paintBox.setColor(J.Color.GREEN.value);
                    paintBox.setStrokeWidth(3.0);
                    paintBox.setStyle(Java.use("android.graphics.Paint$Style").STROKE.value);

                    // Paint cho line (kẻ từ đáy màn hình)
                    var paintLine = J.Paint.$new();
                    paintLine.setColor(J.Color.RED.value);
                    paintLine.setStrokeWidth(2.0);
                    paintLine.setStyle(Java.use("android.graphics.Paint$Style").STROKE.value);

                    // Paint cho text
                    var paintText = J.Paint.$new();
                    paintText.setColor(J.Color.YELLOW.value);
                    paintText.setTextSize(30.0);
                    paintText.setAntiAlias(true);
                    paintText.setTypeface(J.Typeface.DEFAULT_BOLD.value);

                    // Paint cho head circle
                    var paintHead = J.Paint.$new();
                    paintHead.setColor(J.Color.CYAN.value);
                    paintHead.setStrokeWidth(2.0);
                    paintHead.setStyle(Java.use("android.graphics.Paint$Style").STROKE.value);

                    // Vẽ từng player
                    // arr format: [x_head, y_head, x_feet, y_feet, dist, team, hp, visible] * N
                    var stride = 8;
                    var count = Math.floor(arr.length / stride);
                    for (var i = 0; i < count; i++) {
                        var o = i * stride;
                        var hx = arr[o + 0];
                        var hy = arr[o + 1];
                        var fx = arr[o + 2];
                        var fy = arr[o + 3];
                        var dist = arr[o + 4];
                        var team = arr[o + 5];
                        var hp = arr[o + 6];
                        var vis = arr[o + 7];

                        if (vis !== 1) continue;
                        if (hx <= 0 || hy <= 0 || fx <= 0 || fy <= 0) continue;

                        // Chiều cao box
                        var boxH = fy - hy;
                        if (boxH < 10) continue;
                        var boxW = boxH * 0.5;

                        // Vẽ box
                        var left = hx - boxW / 2;
                        var top = hy;
                        var right = hx + boxW / 2;
                        var bottom = fy;

                        // Đổi màu theo team
                        if (team === 0) {
                            // Enemy - đỏ
                            paintBox.setColor(J.Color.RED.value);
                        } else {
                            // Bot/khác - xanh
                            paintBox.setColor(J.Color.GREEN.value);
                        }

                        canvas.drawRect(left, top, right, bottom, paintBox);

                        // Vẽ đầu (hình tròn)
                        var headRadius = boxW * 0.25;
                        canvas.drawCircle(hx, hy - headRadius, headRadius, paintHead);

                        // Vẽ line từ đáy màn hình tới chân
                        canvas.drawLine(screenW / 2, screenH, hx, fy, paintLine);

                        // Vẽ HP bar
                        var hpBarW = boxW;
                        var hpBarH = 5;
                        var hpBarY = top - 12;
                        var hpPct = hp / 100.0;
                        var paintHpBg = J.Paint.$new();
                        paintHpBg.setColor(J.Color.BLACK.value);
                        paintHpBg.setStyle(Java.use("android.graphics.Paint$Style").FILL.value);
                        canvas.drawRect(left, hpBarY, right, hpBarY + hpBarH, paintHpBg);
                        var paintHpFg = J.Paint.$new();
                        paintHpFg.setColor(J.Color.GREEN.value);
                        paintHpFg.setStyle(Java.use("android.graphics.Paint$Style").FILL.value);
                        canvas.drawRect(left, hpBarY, left + hpBarW * hpPct, hpBarY + hpBarH, paintHpFg);

                        // Vẽ khoảng cách
                        var distText = Math.floor(dist) + "m";
                        canvas.drawText(distText, left, top - 20, paintText);

                        // Vẽ tên
                        var nameText = "Enemy_" + i;
                        canvas.drawText(nameText, left, bottom + 30, paintText);
                    }
                } catch (e) {
                    console.log("[ESP_DRAW] onDraw error: " + e);
                }
            }
        }
    });

    // ===== TẠO VÀ ADD VIEW VÀO WINDOW =====
    var overlayView = null;
    try {
        overlayView = EspView.$new(ctx);
        // Khởi tạo data rỗng
        overlayView.data.value = Java.array('float', []);

        var flags = J.WindowManagerLayoutParams.FLAG_NOT_FOCUSABLE.value |
            J.WindowManagerLayoutParams.FLAG_NOT_TOUCHABLE.value |
            J.WindowManagerLayoutParams.FLAG_LAYOUT_IN_SCREEN.value |
            J.WindowManagerLayoutParams.FLAG_LAYOUT_NO_LIMITS.value;

        // Anti-ban: FLAG_SECURE chống chụp màn hình
        try {
            flags = flags | J.WindowManagerLayoutParams.FLAG_SECURE.value;
        } catch (e) { }

        var params = J.WindowManagerLayoutParams.$new(
            J.WindowManagerLayoutParams.MATCH_PARENT.value,
            J.WindowManagerLayoutParams.MATCH_PARENT.value,
            2038, // TYPE_APPLICATION_OVERLAY
            flags,
            J.PixelFormat.TRANSLUCENT.value
        );
        params.gravity = J.Gravity.TOP.value | J.Gravity.LEFT.value;

        // Xóa view cũ nếu có
        try { wm.removeView(overlayView); } catch (e) { }

        wm.addView(overlayView, params);
        console.log("[ESP_DRAW] Overlay đã thêm vào WindowManager");
    } catch (e) {
        console.log("[ESP_DRAW] Lỗi addView: " + e);
        return;
    }

    // ===== HÀM LẤY VIEW MATRIX =====
    function getViewMatrix() {
        try {
            var sc = H.rp(base, OFF.InitBase);
            if (sc.isNull()) return null;
            var m = H.rp(sc, OFF.CurrentMatch);
            if (m.isNull()) return null;
            var lp = H.rp(m, OFF.LocalPlayer);
            if (lp.isNull()) return null;
            var followCam = H.rp(lp, OFF.FollowCamera);
            if (followCam.isNull()) return null;
            var cam = H.rp(followCam, OFF.Camera);
            if (cam.isNull()) return null;
            var matrixAddr = cam.add(OFF.ViewMatrix);
            var arr = [];
            for (var i = 0; i < 16; i++) arr.push(matrixAddr.add(i * 4).readFloat());
            return arr;
        } catch (e) { return null; }
    }

    // ===== WORLD TO SCREEN =====
    function w2s(matrix, wx, wy, wz) {
        if (!matrix || matrix.length < 16) return null;
        var m = matrix;
        var px = m[0] * wx + m[4] * wy + m[8] * wz + m[12];
        var py = m[1] * wx + m[5] * wy + m[9] * wz + m[13];
        var pw = m[3] * wx + m[7] * wy + m[11] * wz + m[15];
        if (pw < 0.001) return null;
        var ndcX = px / pw;
        var ndcY = py / pw;
        var sx = (1 + ndcX) * screenW / 2;
        var sy = (1 - ndcY) * screenH / 2;
        return { x: sx, y: sy };
    }

    // ===== VÒNG LẶP CẬP NHẬT DATA CHO VIEW =====
    var enabled = true;
    var lastDrawTime = 0;

    setInterval(function () {
        if (!enabled) return;
        if (!overlayView) return;
        try {
            var sc = H.rp(base, OFF.InitBase);
            if (sc.isNull()) return;
            var m = H.rp(sc, OFF.CurrentMatch);
            if (m.isNull()) return;
            var lp = H.rp(m, OFF.LocalPlayer);
            if (lp.isNull()) return;

            var myTeam = H.ri(lp, OFF.TeamID);
            var matrix = getViewMatrix();
            if (!matrix) return;

            var dict = H.rp(m, OFF.DictionaryEntities);
            if (dict.isNull()) return;

            var dataArr = [];

            for (var i = 0; i < 50; i++) {
                try {
                    var e = H.rp(dict, 0x10 + i * 0x8);
                    if (e.isNull()) continue;
                    if (H.ri(e, OFF.TeamID) === myTeam) continue;
                    if (H.rb(e, OFF.Player_IsDead) === 1) continue;

                    // Check avatar visible
                    var avMgr = H.rp(e, OFF.AvatarManager);
                    if (avMgr.isNull()) continue;
                    var avatar = H.rp(avMgr, OFF.Avatar);
                    if (avatar.isNull()) continue;
                    var isVisible = H.rb(avatar, OFF.Avatar_IsVisible);

                    // Lấy vị trí XPose
                    var xpose = H.rp(e, OFF.XPose);
                    if (xpose.isNull()) continue;
                    var ex = xpose.add(0x38).readFloat();
                    var ey = xpose.add(0x3C).readFloat();
                    var ez = xpose.add(0x40).readFloat();

                    // W2S cho đầu
                    var sHead = w2s(matrix, ex, ey + 0.75, ez); // đầu cao ~0.75m trên gốc
                    // W2S cho chân
                    var sFeet = w2s(matrix, ex, ey - 0.9, ez);

                    if (!sHead || !sFeet) continue;

                    // Lấy HP (giả định 200)
                    var hp = 200;
                    try {
                        var attrs = H.rp(lp, 0x4BC);
                        // fallback
                    } catch (er) { }

                    // Push vào mảng
                    dataArr.push(sHead.x, sHead.y, sFeet.x, sFeet.y, 0, H.ri(e, OFF.TeamID), hp, isVisible);
                } catch (er) { }
            }

            // Cập nhật view
            overlayView.data.value = Java.array('float', dataArr);
            // invalidate để vẽ lại
            overlayView.postInvalidate();

        } catch (e) {
            console.log("[ESP_DRAW] Loop error: " + e);
        }
    }, 30); // 30 FPS

    // ===== RPC CONTROL =====
    rpc.exports = {
        enable: function () { enabled = true; },
        disable: function () { enabled = false; },
        remove: function () {
            try { wm.removeView(overlayView); } catch (e) { }
        }
    };

    console.log("[ESP_DRAW] Đã khởi động - Overlay ESP đang vẽ");
});