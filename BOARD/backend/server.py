from __future__ import annotations

import json
import math
import os
import threading
import html as html_lib
from datetime import datetime, timezone
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from urllib.parse import urlparse, parse_qs
from urllib.parse import urlencode
from urllib.request import Request, urlopen
import auth

ROOT = Path(__file__).resolve().parent
DATA = ROOT / "data"
DATA.mkdir(exist_ok=True)
TELEMETRY = DATA / "telemetry.jsonl"
ALERTS = DATA / "alerts.jsonl"
STOPS = DATA / "stops.jsonl"
PATIENTS = DATA / "patient_profiles.json"
LOCK = threading.Lock()
STOP_CANDIDATES = {}
LAST_ALERT_EVENT = {}


def get_local_ip() -> str:
    import socket
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.connect(("8.8.8.8", 80))
        ip = s.getsockname()[0]
        s.close()
        return ip
    except Exception:
        return "127.0.0.1"


def remove_diacritics(text: str) -> str:
    import unicodedata
    if not text:
        return ""
    text = text.replace("đ", "d").replace("Đ", "D")
    return "".join(c for c in unicodedata.normalize("NFD", text) if unicodedata.category(c) != "Mn").strip()


def get_patient_profile(device_id: str) -> dict:
    if not PATIENTS.exists():
        return {}
    try:
        with LOCK:
            data = json.loads(PATIENTS.read_text(encoding="utf-8"))
        return data.get(device_id, {})
    except Exception:
        return {}


def update_patient_profile(device_id: str, profile: dict) -> dict:
    with LOCK:
        data = {}
        if PATIENTS.exists():
            try:
                data = json.loads(PATIENTS.read_text(encoding="utf-8"))
            except Exception:
                data = {}
        current = data.get(device_id, {})
        current.update(profile)
        current["updated_at"] = datetime.now(timezone.utc).isoformat()
        data[device_id] = current
        PATIENTS.write_text(json.dumps(data, ensure_ascii=False, indent=2), encoding="utf-8")
        return current


def distance_m(a, b):
    if not a or not b:
        return 0.0
    r = 6371000.0
    p1, p2 = math.radians(a["lat"]), math.radians(b["lat"])
    dp = math.radians(b["lat"] - a["lat"])
    dl = math.radians(b["lon"] - a["lon"])
    h = math.sin(dp / 2) ** 2 + math.cos(p1) * math.cos(p2) * math.sin(dl / 2) ** 2
    return 2 * r * math.asin(math.sqrt(h))


def read_jsonl(path, limit=500):
    if not path.exists():
        return []
    with LOCK, path.open(encoding="utf-8") as f:
        rows = [json.loads(line) for line in f if line.strip()]
    return rows[-limit:]


def reverse_place(point):
    query = urlencode({"lat": point["lat"], "lon": point["lon"], "format": "jsonv2", "zoom": 18})
    try:
        request = Request("https://nominatim.openstreetmap.org/reverse?" + query,
                          headers={"User-Agent": "SmartbandIoT/1.0 (device stop log)"})
        with urlopen(request, timeout=3) as response:
            return json.load(response).get("display_name") or "Chưa xác định tên điểm dừng"
    except Exception:
        return "Chưa xác định tên điểm dừng"


class Handler(BaseHTTPRequestHandler):
    def log_message(self, *_):
        pass

    def send_json(self, status, value):
        body = json.dumps(value, ensure_ascii=False).encode()
        self.send_response(status)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("Access-Control-Allow-Methods", "GET,POST,OPTIONS")
        self.send_header("Access-Control-Allow-Headers", "Content-Type,Authorization")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def do_OPTIONS(self):
        self.send_json(204, {})

    def do_POST(self):
        path = urlparse(self.path).path
        if path.startswith("/api/v1/auth/") or path.startswith("/api/v1/admin/"):
            try:
                size = int(self.headers.get("Content-Length", 0))
                data = json.loads(self.rfile.read(size))
                if path == "/api/v1/auth/request-otp":
                    status, value = auth.request_otp(data.get("email", ""), data.get("purpose", "register"))
                elif path == "/api/v1/auth/register":
                    status, value = auth.register(data.get("email", ""), data.get("name", ""), data.get("password", ""), data.get("otp", ""))
                elif path == "/api/v1/auth/login":
                    status, value = auth.login(data.get("email", ""), data.get("password", ""))
                elif path == "/api/v1/auth/reset-password":
                    email = data.get("email", "").strip().lower()
                    password = data.get("password", "")
                    if len(password) < 6:
                        return self.send_json(400, {"error": "password_too_short"})
                    if not auth.verify_otp(email, "reset", data.get("otp", "")):
                        return self.send_json(403, {"error": "invalid_otp"})
                    auth.users.update_one({"email": email}, {"$set": {"password_hash": auth.hash_password(password)}})
                    auth.sessions.delete_many({"email": email})
                    status, value = 200, {"ok": True}
                elif path == "/api/v1/auth/change-password":
                    user = auth.authenticated(self.headers.get("Authorization", "").removeprefix("Bearer "))
                    if not user:
                        return self.send_json(401, {"error": "unauthorized"})
                    if not auth.verify_password(data.get("old_password", ""), user["password_hash"]):
                        return self.send_json(401, {"error": "invalid_password"})
                    if len(data.get("new_password", "")) < 6:
                        return self.send_json(400, {"error": "password_too_short"})
                    auth.users.update_one({"email": user["email"]}, {"$set": {"password_hash": auth.hash_password(data["new_password"])}})
                    status, value = 200, {"ok": True}
                elif path == "/api/v1/auth/profile":
                    user = auth.authenticated(self.headers.get("Authorization", "").removeprefix("Bearer "))
                    if not user:
                        return self.send_json(401, {"error": "unauthorized"})
                    prof = data.get("profile", {})
                    auth.users.update_one({"email": user["email"]}, {"$set": {"profile": prof}})
                    dev = user.get("device_id") or "smartband-001"
                    update_patient_profile(dev, prof)
                    status, value = 200, {"ok": True, "profile": prof}
                elif path == "/api/v1/admin/user":
                    if not auth.admin_authorized(self.headers.get("Authorization", "").removeprefix("Bearer ")):
                        return self.send_json(403, {"error": "forbidden"})
                    status, value = auth.admin_update(data.get("email", ""), data)
                else:
                    return self.send_json(404, {"error": "not_found"})
                return self.send_json(status, value)
            except Exception as exc:
                return self.send_json(503, {"error": str(exc)})
        if path.startswith("/api/v1/patient"):
            try:
                device_id = path.strip("/").split("/")[-1]
                if device_id in ("patient", "profile", ""):
                    device_id = "smartband-001"
                size = int(self.headers.get("Content-Length", 0))
                payload = json.loads(self.rfile.read(size)) if size else {}
                prof = payload.get("profile", payload)
                saved = update_patient_profile(device_id, prof)
                return self.send_json(200, {"ok": True, "device_id": device_id, "profile": saved})
            except Exception as exc:
                return self.send_json(400, {"ok": False, "error": str(exc)})
        if path != "/api/v1/telemetry":
            return self.send_json(404, {"error": "not_found"})
        try:
            size = int(self.headers.get("Content-Length", 0))
            item = json.loads(self.rfile.read(size))
            item["received_at"] = datetime.now(timezone.utc).isoformat()
            item.setdefault("device_id", "smartband-001")
            with LOCK, TELEMETRY.open("a", encoding="utf-8") as f:
                f.write(json.dumps(item, ensure_ascii=False) + "\n")
            gps = item.get("gps") or {}
            device = item["device_id"]
            if gps.get("valid") and gps.get("lat") is not None and gps.get("lon") is not None:
                point = {"lat": gps["lat"], "lon": gps["lon"]}
                candidate = STOP_CANDIDATES.get(device)
                now = datetime.now(timezone.utc)
                if candidate and (now - candidate["last_seen"]).total_seconds() > 10:
                    candidate = None
                    STOP_CANDIDATES.pop(device, None)
                moving = float(gps.get("speed_kmh") or 0) >= 2.0
                if candidate and distance_m(candidate["point"], point) >= 20:
                    moving = True
                if moving:
                    STOP_CANDIDATES.pop(device, None)
                else:
                    if candidate is None:
                        candidate = {"point": point, "since": now, "last_seen": now, "logged": False}
                        STOP_CANDIDATES[device] = candidate
                    candidate["last_seen"] = now
                    if not candidate["logged"] and (now - candidate["since"]).total_seconds() >= 180:
                        event = {"device_id": device, "gps": candidate["point"],
                                 "place_name": reverse_place(candidate["point"]),
                                 "stopped_at": candidate["since"].isoformat(),
                                 "received_at": item["received_at"], "duration_seconds": 180}
                        with LOCK, STOPS.open("a", encoding="utf-8") as f:
                            f.write(json.dumps(event, ensure_ascii=False) + "\n")
                        candidate["logged"] = True
            else:
                STOP_CANDIDATES.pop(device, None)
            alert_type = item.get("alert_type") or ("FALL" if item.get("state") == "EMERGENCY" else "")
            alert_id = (device, alert_type, item.get("alert_event_ms"))
            if alert_type and alert_id != LAST_ALERT_EVENT.get(device):
                with LOCK, ALERTS.open("a", encoding="utf-8") as f:
                    f.write(json.dumps(item, ensure_ascii=False) + "\n")
                LAST_ALERT_EVENT[device] = alert_id
            prof = get_patient_profile(device)
            resp = {"ok": True}
            host_header = self.headers.get("Host")
            if not host_header or "0.0.0.0" in host_header or "127.0.0.1" in host_header or "localhost" in host_header:
                current_host = f"{get_local_ip()}:8787"
            else:
                current_host = host_header
            if prof and prof.get("isConfigured"):
                name = remove_diacritics(prof.get("name", "Nguoi deo"))
                age = f" ({prof.get('age')}t)" if prof.get("age") else ""
                blood = remove_diacritics(prof.get("bloodGroup", "--")).upper()
                if "CHUA" in blood or not blood:
                    blood = "--"
                cond = remove_diacritics(prof.get("medicalNotes", "Khong co"))
                allergies = remove_diacritics(prof.get("allergies", "Khong co"))
                phone = "".join(filter(str.isdigit, prof.get("emergencyPhone", "115"))) or "115"
                contact = remove_diacritics(prof.get("emergencyContact", ""))
                contact_str = f" ({contact})" if contact else ""

                plain_qr = f"THONG TIN NAN NHAN\nHo ten: {name}{age}\nNhom mau: {blood}\nBenh ly: {cond}\nDi ung: {allergies}\nLH: {phone}{contact_str}"

                resp["medical"] = {
                    "name": name,
                    "blood": blood,
                    "condition": cond[:14],
                    "phone": phone[:12],
                    "url": plain_qr
                }
            self.send_json(201, resp)
        except Exception as exc:
            self.send_json(400, {"ok": False, "error": str(exc)})

    def do_GET(self):
        path = urlparse(self.path).path
        if path == "/admin":
            body = (ROOT / "admin.html").read_bytes()
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            return self.wfile.write(body)
        if path in {"/api/v1/auth/me", "/api/v1/admin/users", "/api/v1/admin/user-data"}:
            token = self.headers.get("Authorization", "").removeprefix("Bearer ")
            try:
                if path == "/api/v1/admin/users":
                    return self.send_json(200, auth.admin_users()) if auth.admin_authorized(token) else self.send_json(403, {"error": "forbidden"})
                if path == "/api/v1/admin/user-data":
                    if not auth.admin_authorized(token):
                        return self.send_json(403, {"error": "forbidden"})
                    email = parse_qs(urlparse(self.path).query).get("email", [""])[0]
                    user = auth.users.find_one({"email": email.lower()})
                    if not user:
                        return self.send_json(404, {"error": "user_not_found"})
                    device = user.get("device_id")
                    return self.send_json(200, {"user": auth.public_user(user),
                                                "telemetry": [r for r in read_jsonl(TELEMETRY, 100) if r.get("device_id") == device],
                                                "alerts": [r for r in read_jsonl(ALERTS, 100) if r.get("device_id") == device],
                                                "stops": [r for r in read_jsonl(STOPS, 100) if r.get("device_id") == device]})
                user = auth.authenticated(token)
                return self.send_json(200, {"user": auth.public_user(user)}) if user else self.send_json(401, {"error": "unauthorized"})
            except Exception as exc:
                return self.send_json(503, {"error": str(exc)})
        rows = read_jsonl(TELEMETRY)
        if path.startswith("/api/v1/devices/") and path.endswith("/snapshot"):
            device = path.split("/")[4]
            current = next((x for x in reversed(rows) if x.get("device_id") == device), None)
            received = datetime.fromisoformat(current["received_at"]) if current else None
            fresh = received is not None and (datetime.now(timezone.utc) - received).total_seconds() < 10
            return self.send_json(200, {"device_id": device, "connected": fresh, "latest": current})
        if path == "/api/v1/route":
            points = [x for x in rows if x.get("gps", {}).get("valid")]
            total = sum(distance_m(points[i - 1]["gps"], points[i]["gps"]) for i in range(1, len(points)))
            return self.send_json(200, {"points": points, "distance_m": round(total, 1)})
        if path == "/api/v1/alerts":
            return self.send_json(200, {"alerts": read_jsonl(ALERTS)})
        if path == "/api/v1/stops":
            return self.send_json(200, {"stops": read_jsonl(STOPS)})
        if path.startswith("/api/v1/patient/"):
            device_id = path.strip("/").split("/")[-1]
            if not device_id or device_id in ("patient", ""):
                device_id = "smartband-001"
            return self.send_json(200, {"ok": True, "device_id": device_id, "profile": get_patient_profile(device_id)})
        if path.startswith("/patient/") or path.startswith("/p/"):
            device_id = path.strip("/").split("/")[-1]
            if not device_id or device_id in ("patient", "p", ""):
                device_id = "smartband-001"
            prof = get_patient_profile(device_id)
            latest_t = next((x for x in reversed(rows) if x.get("device_id") == device_id), {})
            gps = latest_t.get("gps", {})
            lat = gps.get("lat") or 20.98108
            lon = gps.get("lon") or 105.78736
            has_gps = bool(gps.get("valid") and gps.get("lat") is not None and gps.get("lon") is not None)

            is_configured = prof.get("isConfigured", False) and bool(prof.get("name"))
            patient_name = prof.get("name", "Người đeo thiết bị")
            patient_age = str(prof.get("age", "--"))
            patient_gender = prof.get("gender", "Chưa rõ")
            blood_group = prof.get("bloodGroup") or "Chưa rõ"
            medical_notes = prof.get("medicalNotes") or "Chưa ghi nhận bệnh lý nền đặc biệt"
            allergies = prof.get("allergies") or "Không có dị ứng thuốc hoặc chưa ghi nhận"
            emergency_contact = prof.get("emergencyContact") or "Người thân"
            emergency_phone = (prof.get("emergencyPhone") or "").strip()
            current_addr = prof.get("currentAddress") or prof.get("hometown") or ""

            if is_configured:
                profile_card_html = f"""
      <div class="flex items-center space-x-4 border-b border-gray-100 pb-4">
        <div class="w-16 h-16 rounded-full bg-red-100 flex items-center justify-center text-red-600 text-2xl font-bold border-2 border-red-300 shrink-0">
          <i class="fa-solid fa-user-injured"></i>
        </div>
        <div class="min-w-0 flex-1">
          <h2 class="text-xl font-black text-gray-900 truncate">{html_lib.escape(patient_name)}</h2>
          <p class="text-xs text-gray-500 font-medium">Tuổi: <span class="font-bold text-gray-800">{html_lib.escape(patient_age)}</span> • Giới tính: <span class="font-bold text-gray-800">{html_lib.escape(patient_gender)}</span></p>
          <span class="inline-block px-3 py-0.5 bg-red-600 text-white font-extrabold text-xs rounded-full mt-1.5 shadow-sm">
            🩸 NHÓM MÁU: {html_lib.escape(blood_group)}
          </span>
        </div>
      </div>
      
      <div class="bg-amber-50 p-4 rounded-2xl border border-amber-200 shadow-sm">
        <h3 class="text-xs font-black uppercase text-amber-900 mb-1.5 flex items-center tracking-wide">
          <i class="fa-solid fa-heart-pulse mr-2 text-red-500 text-sm"></i> Bệnh lý nền (Đặc biệt lưu ý)
        </h3>
        <p class="text-sm font-semibold text-gray-800 leading-snug">{html_lib.escape(medical_notes)}</p>
      </div>

      <div class="bg-rose-50 p-4 rounded-2xl border border-rose-200 shadow-sm block clear-both">
        <h3 class="text-xs font-black uppercase text-rose-900 mb-1.5 flex items-center tracking-wide">
          <i class="fa-solid fa-triangle-exclamation mr-2 text-rose-600 text-sm"></i> Dị ứng thuốc & Chống chỉ định
        </h3>
        <p class="text-sm font-bold text-red-700 leading-snug break-words">{html_lib.escape(allergies)}</p>
      </div>"""
            else:
                profile_card_html = f"""
      <div class="bg-amber-50 border-2 border-amber-300 p-5 rounded-2xl text-center space-y-2">
        <div class="w-12 h-12 bg-amber-100 text-amber-600 rounded-full flex items-center justify-center mx-auto text-xl">
          <i class="fa-solid fa-triangle-exclamation"></i>
        </div>
        <h2 class="text-base font-black text-amber-900">HỒ SƠ CHƯA ĐƯỢC THIẾT LẬP</h2>
        <p class="text-xs text-amber-800 font-medium leading-relaxed">
          Người nhà chưa cấu hình thông tin bệnh nhân cho vòng tay này qua ứng dụng Smart Band AIoT.<br>
          Nếu gặp sự cố, xin vui lòng bấm nút gọi cấp cứu bên dưới ngay.
        </p>
      </div>"""

            call_contact_html = ""
            if emergency_phone:
                call_contact_html = f"""
        <a href="tel:{html_lib.escape(emergency_phone)}" class="flex items-center justify-center w-full py-3.5 px-4 bg-red-600 hover:bg-red-700 active:scale-98 text-white font-black rounded-2xl shadow-lg transition text-base tracking-wide">
          <i class="fa-solid fa-phone-volume mr-2 text-lg"></i> GỌI {html_lib.escape(emergency_contact.upper())} ({html_lib.escape(emergency_phone)})
        </a>"""
            else:
                call_contact_html = f"""
        <div class="p-3 bg-gray-50 border border-gray-200 rounded-xl text-center text-xs text-gray-500 font-medium">
          Chưa lưu số điện thoại người thân trong ứng dụng
        </div>"""

            gps_desc = f"{lat:.5f}, {lon:.5f}" if has_gps else "Đang cập nhật..."
            gps_html = f"""
        <a href="https://www.google.com/maps?q={lat},{lon}" target="_blank" class="text-blue-600 hover:underline font-bold mt-1.5 inline-flex items-center">
          <i class="fa-solid fa-map-location-dot mr-1 text-sm"></i> Xem vị trí GPS ({gps_desc})
        </a>""" if has_gps else """
        <p class="text-gray-400 font-medium mt-1">Vị trí GPS: Đang cập nhật từ thiết bị...</p>"""

            html = f"""<!DOCTYPE html>
<html lang="vi">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>HỒ SƠ CẤP CỨU Y TẾ KHẨN CẤP - {html_lib.escape(patient_name)}</title>
<link href="https://cdn.jsdelivr.net/npm/tailwindcss@2.2.19/dist/tailwind.min.css" rel="stylesheet">
<link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.4.0/css/all.min.css">
</head>
<body class="bg-red-50 text-gray-800 font-sans p-4 max-w-md mx-auto min-h-screen flex items-center justify-center">
  <div class="w-full bg-white rounded-3xl shadow-2xl overflow-hidden border-2 border-red-500">
    <div class="bg-gradient-to-r from-red-600 to-rose-700 text-white p-6 text-center">
      <div class="inline-block p-3.5 bg-red-800/80 rounded-full mb-2 animate-bounce shadow">
        <i class="fa-solid fa-truck-medical text-3xl"></i>
      </div>
      <h1 class="text-xl font-black uppercase tracking-wide">Hồ Sơ Y Tế Cấp Cứu</h1>
      <p class="text-xs text-red-100 mt-1 font-medium">Smart Band AIoT • Cứu hộ khẩn cấp người cao tuổi</p>
    </div>
    <div class="p-5 space-y-4">
      {profile_card_html}

      <div class="pt-2 space-y-2.5">
        {call_contact_html}
        <a href="tel:115" class="flex items-center justify-center w-full py-3 px-4 bg-gray-900 hover:bg-black active:scale-98 text-white font-bold rounded-2xl shadow transition text-sm">
          <i class="fa-solid fa-ambulance mr-2 text-red-400"></i> GỌI TRUNG TÂM CẤP CỨU 115
        </a>
      </div>

      <div class="border-t border-gray-100 pt-3 text-center text-xs text-gray-500">
        <p class="font-medium">Mã thiết bị: <span class="font-mono font-bold text-gray-800">{html_lib.escape(device_id)}</span></p>
        {gps_html}
      </div>
    </div>
  </div>
</body>
</html>"""
            body = html.encode("utf-8")
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            return self.wfile.write(body)
        if path == "/api/v1/server-info":
            local_ip = get_local_ip()
            return self.send_json(200, {
                "ok": True,
                "local_ip": local_ip,
                "port": 8787,
                "base_url": f"http://{local_ip}:8787",
                "qr_url": f"http://{local_ip}:8787/p/smartband-001"
            })
        if path == "/health":
            return self.send_json(200, {"ok": True, "ip": get_local_ip()})
        self.send_json(404, {"error": "not_found"})


if __name__ == "__main__":
    host = os.getenv("BACKEND_HOST", "0.0.0.0")
    port = int(os.getenv("BACKEND_PORT", "8787"))
    local_ip = get_local_ip()
    print("=" * 60)
    print(f"Smartband backend listening on http://{host}:{port}")
    print(f"-> Active Wi-Fi / LAN IP: http://{local_ip}:{port}")
    print(f"-> Emergency QR Webpage : http://{local_ip}:{port}/p/smartband-001")
    print("=" * 60)
    ThreadingHTTPServer((host, port), Handler).serve_forever()
