import time
import sys
import os
import random
from datetime import datetime
from typing import Optional, Tuple, Set
import requests

try:
    from colorama import init as colorama_init
    colorama_init(autoreset=True)
except Exception:
    pass

den   = "\033[1;90m"
luc   = "\033[1;32m"
trang = "\033[1;37m"
do    = "\033[1;31m"
vang  = "\033[1;33m"
tim   = "\033[1;35m"
lamd  = "\033[1;34m"
lam   = "\033[1;36m"
hong  = "\033[1;95m"
reset = "\033[0m"

thanh_dep = trang + "~" + do + "[" + luc + "NTIEN" + do + "] " + trang + "➩ " + luc
API_URL = "https://keyherlyswar.x10.mx/Apidocs/reg/reglq.php"
TIMEOUT = 3
DELAY_BETWEEN = 0.1         # giây
MAX_RETRIES = 2
OUTPUT_FILE = "acclienquan.txt"

USER_AGENTS = [
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36",
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/119.0.0.0 Safari/537.36",
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:109.0) Gecko/20100101 Firefox/121.0",
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:109.0) Gecko/20100101 Firefox/120.0",
    "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36",
    "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/17.1 Safari/605.1.15",
    "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/119.0.0.0 Safari/537.36",
    "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36",
    "Mozilla/5.0 (X11; Linux x86_64; rv:109.0) Gecko/20100101 Firefox/121.0",
    "Mozilla/5.0 (iPhone; CPU iPhone OS 17_1 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/17.1 Mobile/15E148 Safari/604.1",
    "Mozilla/5.0 (iPhone; CPU iPhone OS 16_6 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/16.6 Mobile/15E148 Safari/604.1",
    "Mozilla/5.0 (iPad; CPU OS 17_1 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/17.1 Mobile/15E148 Safari/604.1",
    "Mozilla/5.0 (Linux; Android 13; SM-S908B) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Mobile Safari/537.36",
    "Mozilla/5.0 (Linux; Android 13; SM-S908B) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/119.0.0.0 Mobile Safari/537.36",
    "Mozilla/5.0 (Linux; Android 12; Pixel 6) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Mobile Safari/537.36",
    "Mozilla/5.0 (Linux; Android 11; Redmi Note 8) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/119.0.0.0 Mobile Safari/537.36",
    "Mozilla/5.0 (Windows NT 6.1; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36",
    "Mozilla/5.0 (Windows NT 6.3; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/119.0.0.0 Safari/537.36",
    "Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36",
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Edg/120.0.0.0 Safari/537.36",
    "Mozilla/5.0 (Windows NT 5.2; WOW64) AppleWebKit/535.1 (KHTML, like Gecko) Chrome/13.0.782.41 Safari/535.1",
    "Mozilla/5.0 (Windows NT 10.0; ARM; Lumia 635) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/46.0.2486.0 Safari/537.36 Edge/13.10586",
]


def ask_positive_int(prompt: str) -> int:
    while True:
        raw = input(luc + prompt + vang).strip()
        print(reset, end="")
        try:
            n = int(raw)
            if n <= 0:
                print(do + "Vui lòng nhập một số nguyên dương." + reset)
                continue
            return n
        except ValueError:
            print(do + "Không hợp lệ. Hãy nhập số." + reset)


def load_existing_accounts() -> Set[str]:
    """Đọc các username đã có trong file để tránh ghi trùng."""
    existing = set()
    if os.path.exists(OUTPUT_FILE):
        try:
            with open(OUTPUT_FILE, "r", encoding="utf-8") as f:
                for line in f:
                    line = line.strip()
                    if "|" in line:
                        username = line.split("|")[0].strip()
                        if username:
                            existing.add(username)
        except Exception:
            pass
    return existing


def create_garena_account(session: requests.Session) -> Tuple[bool, Optional[str], Optional[str], str]:
    try:
        # Đổi User-Agent ngẫu nhiên cho mỗi request
        session.headers.update({"User-Agent": random.choice(USER_AGENTS)})
        res = session.get(API_URL, timeout=TIMEOUT)
    except requests.RequestException as e:
        return False, None, None, f"Lỗi mạng: {e}"

    if res.status_code != 200:
        return False, None, None, f"HTTP {res.status_code}"

    try:
        data = res.json()
    except Exception:
        return False, None, None, "Phản hồi không phải JSON"

    status = data.get("status")
    result = data.get("result")

    if not status or not result or not isinstance(result, list) or not result:
        return False, None, None, "API trả về không hợp lệ"

    info = result[0] if isinstance(result[0], dict) else {}
    username = info.get("account") or info.get("username") or ""
    password = info.get("password") or ""

    if not username or not password:
        return False, None, None, "Thiếu username/password"

    return True, username, password, "OK"


def countdown(seconds: float):
    """Đếm ngược hỗ trợ số thực (0.5s)."""
    total = seconds
    while total > 0:
        print(f"{den}  {total:.1f}s...   {reset}", end="\r", flush=True)
        time.sleep(0.1)
        total = round(total - 0.1, 1)
    print(" " * 20, end="\r")


def main():
    os.system("cls" if os.name == "nt" else "clear")
    print(f"{thanh_dep}{trang}TOOL SCAN ACC LIÊN QUÂN VIP {den}\n")

    qty = ask_positive_int("Nhập số lượng acc scan: ")

    created = 0
    existing_accounts = load_existing_accounts()
    session = requests.Session()

    try:
        for i in range(1, qty + 1):
            print(f"\n{thanh_dep}{lam}Bắt đầu scan tài khoản {trang}[{vang}{i}{trang}/{vang}{qty}{trang}]{reset}")
            ok = False
            username = password = None

            for attempt in range(1, MAX_RETRIES + 1):
                print(f"{den}  → Thử lần {attempt}/{MAX_RETRIES}...{reset}")
                ok, username, password, _ = create_garena_account(session)
                if ok:
                    break
                else:
                    print(f"{do}  ✗ Thất bại, thử lại...{reset}")
                    if attempt < MAX_RETRIES:
                        time.sleep(1 * attempt)  # backoff nhẹ

            if ok and username and password:
                if username in existing_accounts:
                    print(f"{vang}  ⚠ Acc {username} đã có trong file, bỏ qua.{reset}")
                else:
                    created += 1
                    existing_accounts.add(username)
                    print(f"{luc}  ✓ Scan thành công!{reset}")
                    print(f"{trang}     👤 Username: {hong}{username}{reset}")
                    print(f"{trang}     🔑 Password: {hong}{password}{reset}")

                    with open(OUTPUT_FILE, "a", encoding="utf-8") as f:
                        f.write(f"{username} | {password}\n")
            else:
                print(f"{do}  ❌ Không thể scan tài khoản.{reset}")

            if i < qty:
                print(f"{vang}⏳ Đợi {DELAY_BETWEEN} giây trước khi scan acc tiếp theo...{reset}")
                countdown(DELAY_BETWEEN)

    except KeyboardInterrupt:
        print(f"\n{do}⛔ Đã dừng theo yêu cầu (Ctrl+C).{reset}")

    print(f"\n{thanh_dep}{trang}Hoàn tất. Scan thành công: {luc}{created}{trang}/{vang}{qty}{reset}")
    print(f"{thanh_dep}{trang}Danh sách đã lưu tại: {lam}{OUTPUT_FILE}{reset}")


if __name__ == "__main__":
    main()s