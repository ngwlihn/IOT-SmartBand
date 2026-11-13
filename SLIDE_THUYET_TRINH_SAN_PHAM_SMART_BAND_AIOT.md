# BỘ SLIDE THUYẾT TRÌNH BÁO CÁO ĐỒ ÁN TOÀN DIỆN
## ĐỀ TÀI: THIẾT KẾ THIẾT BỊ THEO DÕI VỊ TRÍ NGƯỜI BỆNH KẾT HỢP ĐIỆN TOÁN BIÊN PHÂN TÍCH HÀNH ĐỘNG (SMART BAND AIoT)
* **Môn học:** IoT và Ứng Dụng / IoT & Ứng Dụng ATTT  
* **Đơn vị:** Học viện Công nghệ Bưu chính Viễn thông (PTIT)  
* **Giảng viên hướng dẫn:** TS. Lê Văn Vịnh  
* **Nhóm sinh viên thực hiện (Nhóm 01):**  
  1. Nguyễn Thị Thùy Dương – B23DCAT070  
  2. Nguyễn Thị Ngọc Linh – B23DCAT169  
  3. Đinh Yến Nhi – B23DCAT229  

---

### HƯỚNG DẪN THIẾT KẾ TRÊN CANVA / POWERPOINT
* **Bộ màu chủ đạo chuẩn PTIT:**
  * Đỏ PTIT: `#D32F2F` (Dùng cho Tiêu đề chính, Tag phần, Nút nhấn mạnh)
  * Xanh công nghệ: `#1E40AF` (Dùng cho Icon, Tiêu đề phụ, Khối công nghệ)
  * Nền Slide: Trắng `#FFFFFF` hoặc Xám nhạt `#F8FAFC` (Tạo độ tương phản cao, hiện đại)
  * Chữ nội dung: Đen than `#1E293B` (Dễ đọc từ xa, không gây mỏi mắt)
* **Font chữ đề xuất:** Montserrat, Roboto, Open Sans hoặc Be Vietnam Pro (Đảm bảo 100% không bị lỗi dấu tiếng Việt khi trình chiếu).
* **Quy chuẩn nội dung:** Đã loại bỏ hoàn toàn các mã toán LaTeX phức tạp để copy-paste trực tiếp không bị vỡ font hay hiển thị mã nguồn lỗi.

---

## 🖥️ SLIDE 1: TRANG BÌA (COVER SLIDE)

### Bố cục:
* **Góc trên:** Logo PTIT (Học viện Công nghệ Bưu chính Viễn thông) - Khoa An toàn Thông tin.
* **Chính giữa:**
  * Tiêu đề đề tài (Font to, Đỏ PTIT, In hoa đậm):  
    **THIẾT KẾ THIẾT BỊ THEO DÕI VỊ TRÍ NGƯỜI BỆNH KẾT HỢP ĐIỆN TOÁN BIÊN PHÂN TÍCH HÀNH ĐỘNG**
  * Tên sản phẩm thương mại: **SMART BAND AIoT (TẬN TÂM CARE)**
* **Góc dưới:**
  * Giảng viên hướng dẫn: TS. Lê Văn Vịnh
  * Nhóm sinh viên 01: Thùy Dương - Ngọc Linh - Yến Nhi (Lớp B23DCAT)
  * Hà Nội, 2026

> 🎤 **Speaker Notes (Lời thuyết trình):**  
> *"Kính chào Thầy và toàn thể các bạn sinh viên. Hôm nay, nhóm 01 gồm ba thành viên: Thùy Dương, Ngọc Linh và Yến Nhi xin phép được báo cáo đồ án môn học IoT và Ứng dụng với đề tài: 'Thiết kế thiết bị theo dõi vị trí người bệnh kết hợp điện toán biên phân tích hành động'. Đây là một sản phẩm vòng đeo tay thông minh hoàn chỉnh từ phần cứng, thuật toán AI tại biên cho đến ứng dụng di động theo dõi người thân."*

---

## 🖥️ SLIDE 2: MỤC LỤC & CẤU TRÚC BÀI THUYẾT TRÌNH

### Bố cục (Chia 2 Cột):
* **Cột Trái (Phần cứng & Thuật toán):**
  * `[PHẦN 1]` Đặt vấn đề & Tính cấp thiết của đề tài
  * `[PHẦN 2]` Kịch bản vận hành tổng thể hệ thống (7 bước)
  * `[PHẦN 3]` Thiết kế & Kiến trúc phần cứng thiết bị
  * `[PHẦN 4]` Mô hình TinyML & Thuật toán nhận diện tại biên
* **Cột Phải (Hệ thống, Ứng dụng & Kết quả):**
  * `[PHẦN 5]` Nền tảng Máy chủ Backend & Ứng dụng di động
  * `[PHẦN 6]` Kết quả đo kiểm & Thử nghiệm thực tế
  * `[PHẦN 7]` Ứng dụng An toàn thông tin (ATTT) trong thiết kế
  * `[PHẦN 8]` So sánh sản phẩm, Kết luận & Hướng phát triển

> 🎤 **Speaker Notes (Lời thuyết trình):**  
> *"Bài báo cáo của nhóm được kết cấu chặt chẽ qua 8 nội dung chính, đi từ bài toán thực tế, kiến trúc phần cứng, mô hình AI nhúng, hệ thống truyền thông mạng, các biện pháp an toàn thông tin chuyên ngành, và cuối cùng là kết quả thực nghiệm sản phẩm."*

---

## 🖥️ SLIDE 3: `[PHẦN 1]` ĐẶT VẤN ĐỀ & TÍNH CẤP THIẾT

### Bố cục (Chia 3 Khối Thẻ):

#### 1. Thực trạng già hóa dân số
* Tỷ lệ người cao tuổi sống một mình hoặc không có con cái bên cạnh ban ngày tăng nhanh.
* Té ngã là nguyên nhân hàng đầu gây chấn thương sọ não, gãy xương và đột quỵ ở người già.
* **Thời gian vàng cấp cứu:** Bắt buộc dưới 30 phút để tránh nguy cơ tử vong hoặc di chứng vĩnh viễn.

#### 2. Nhược điểm giải pháp hiện có
* **Vòng bấm cơ thông thường:** Người ngất xỉu hoặc chấn thương nặng không thể tự bấm nút gọi người thân.
* **Vòng đeo dùng ngưỡng gia tốc đơn giản:** Vung tay, đập bàn hoặc đi bộ cũng kích hoạt báo động giả liên tục.
* **Giải pháp đẩy dữ liệu lên Cloud:** Độ trễ cao, phụ thuộc hoàn toàn vào mạng và dễ rò rỉ hình ảnh, dữ liệu sinh hoạt nhạy cảm.

#### 3. Mục tiêu giải pháp Smart Band AIoT
* Tạo ra vòng đeo tay **tự động nhận biết ngã bằng AI ngay trên chip**.
* Tự động hú còi tại chỗ, lấy tọa độ GPS và gửi cảnh báo khẩn cấp tới điện thoại con cái trong **dưới 10 giây**.
* Chi phí cực rẻ (dưới 500.000 VNĐ), dễ dàng phổ cập tới mọi gia đình.

> 🎤 **Speaker Notes (Lời thuyết trình):**  
> *"Xuất phát từ thực tế người cao tuổi dễ gặp tai nạn ngã trong nhà tắm hoặc khi ở nhà một mình, nhóm đặt mục tiêu chế tạo một thiết bị thông minh nhưng có giá thành bình dân. Điểm mấu chốt là thiết bị phải tự động phát hiện được ngã ngay cả khi nạn nhân đã bất tỉnh, đồng thời phải loại bỏ triệt để tình trạng báo động giả khi các cụ đi bộ hay tập thể dục buổi sáng."*

---

## 🖥️ SLIDE 4: `[PHẦN 2]` KỊCH BẢN VẬN HÀNH HỆ THỐNG (7 BƯỚC)

### Bố cục (Quy trình dạng Timeline ngang hoặc sơ đồ mũi tên):

* **Bước 1 (Đeo & Giám sát):** Người già đeo vòng tay; cảm biến đo liên tục chuyển động ở tần số 50 lần/giây.
* **Bước 2 (Phát hiện sự cố):** Mô hình AI nhận diện xung chấn va chạm mạnh hoặc mất thăng bằng đột ngột.
* **Bước 3 (Kiểm chứng lâm sàng 2.5s):** Hệ thống phân tích góc nghiêng nằm và đo mức độ bất động của cơ thể.
* **Bước 4 (Cảnh báo tại chỗ 10s):** Còi bíp dồn dập, màn hình OLED đếm ngược 10 giây để người dùng bấm hủy nếu chỉ là va chạm nhẹ.
* **Bước 5 (Định vị vệ tinh):** Hết 10 giây không có ai hủy, GPS lập tức chốt tọa độ kinh độ - vĩ độ thực tế.
* **Bước 6 (Truyền tin khẩn cấp):** Vòng tay gửi gói tin SOS qua Wi-Fi lên máy chủ Backend trong 200 mili-giây.
* **Bước 7 (Ứng cứu người thân):** Ứng dụng điện thoại người nhà lập tức **hú còi cấp cứu tối đa** (kể cả máy đang tắt chuông) và hiện bản đồ dẫn đường tới vị trí.

> 🎤 **Speaker Notes (Lời thuyết trình):**  
> *"Hệ thống vận hành khép kín qua 7 bước tự động. Điểm nhân văn trong thiết kế là khoảng đếm ngược 10 giây tại chỗ. Nếu người già chỉ vô tình làm rơi vòng tay hoặc ngã nhẹ tự đứng dậy được, chỉ cần bấm nút là còi tắt ngay, người thân ở xa sẽ không bị hoảng hốt bởi những cảnh báo không cần thiết."*

---

## 🖥️ SLIDE 5: `[PHẦN 3]` HỆ THỐNG PHẦN CỨNG THIẾT BỊ

### Bố cục (Chia 3 Cột Linh Kiện):

#### 1. Bộ Não Xử Lý & AI
* **Kit ESP32-S3 Mini:**
  * Chip lõi kép tốc độ cao 240 MHz.
  * Tích hợp bộ lệnh Vector SIMD tăng tốc tính toán mạng nơ-ron AI.
  * Bộ nhớ Flash 4MB, hỗ trợ phân vùng NVS lưu cấu hình mạng an toàn.
  * Tích hợp sẵn Wi-Fi và Bluetooth Low Energy 5.0.

#### 2. Cảm Biến Chuyển Động & Vị Trí
* **Cảm biến IMU Bosch BMI160:**
  * Đo 6 trục: 3 trục gia tốc kế và 3 trục con quay hồi chuyển.
  * Lấy mẫu thời gian thực 50 Hz (20 mili-giây/mẫu).
* **Module GPS u-blox NEO-6M:**
  * Kèm anten gốm bắt sóng vệ tinh ngoài trời.
  * Cập nhật tọa độ chính xác cao qua cổng UART 9600 bps.

#### 3. Tương Tác & Cảnh Báo
* **Màn hình OLED 0.96 inch (SSD1306):**
  * Độ tương phản cao, hiển thị 4 trang xoay vòng và đồng hồ đếm ngược.
* **Còi báo Active Buzzer:**
  * Kêu bíp theo từng ngữ cảnh: va chạm, đếm ngược và còi hú khẩn cấp.
* **Nút bấm cơ học SOS:**
  * Chống rung tiếp điểm 40ms; hỗ trợ bấm hủy báo động và giữ 2s để gọi cấp cứu.

> 🎤 **Speaker Notes (Lời thuyết trình):**  
> *"Toàn bộ phần cứng được tối ưu để đạt kích thước nhỏ gọn đeo tay với chi phí linh kiện cực rẻ. Chip ESP32-S3 đảm nhận vai trò bộ não AI, kết hợp cảm biến gia tốc góc xoay BMI160 và module định vị GPS NEO-6M để bảo vệ người già toàn diện cả trong nhà lẫn ngoài trời."*

---

## 🖥️ SLIDE 6: `[PHẦN 3]` SƠ ĐỒ ĐẤU NỐI & ĐIỂM SÁNG PHẦN CỨNG

### Bố cục (Chia 2 Cột):

#### Cột Trái: Bảng Sơ Đồ Đấu Nối Chân Với ESP32-S3
* **Cảm biến IMU BMI160:** Chân GPIO 6 (SDA) và GPIO 7 (SCL) -> Giao tiếp **I2C Bus 0**.
* **Màn hình OLED 0.96":** Chân GPIO 9 (SDA) và GPIO 8 (SCL) -> Giao tiếp **I2C Bus 1**.
* **Module GPS Neo-6M:** Chân GPIO 2 (RX1) và GPIO 1 (TX1) -> Giao tiếp **UART1**.
* **Còi Buzzer:** Chân GPIO 5 -> Điều khiển xung tín hiệu số (Digital Out).
* **Nút bấm SOS:** Chân GPIO 4 (và song song nút Boot GPIO 0) -> Đọc tín hiệu nút nhấn.
* **Nguồn cấp:** Cổng Type-C kết hợp mạch sạc pin Li-Po 3.7V.

#### Cột Phải: 3 Điểm Sáng Kỹ Thuật Độc Đáo
* ⚡ **Kiến trúc Dual I2C Bus:** Tách riêng 2 đường truyền I2C độc lập (Bus 0 cho cảm biến, Bus 1 cho màn hình). Tác vụ vẽ màn hình không bao giờ làm nghẽn hoặc trễ chu kỳ đọc cảm biến của AI.
* 🧠 **Đa nhiệm FreeRTOS 2 Nhân vật lý:** Nhân 1 chạy thời gian thực tác vụ AI và cảm biến; Nhân 0 xử lý định vị GPS, Wi-Fi và Bluetooth.
* 📶 **Bluetooth BLE Provisioning:** Cài đặt Wi-Fi cho vòng tay cực nhanh qua ứng dụng điện thoại mà không cần cắm dây nạp lại code.

> 🎤 **Speaker Notes (Lời thuyết trình):**  
> *"Về mặt kỹ thuật mạch, điểm khác biệt lớn nhất của nhóm là thiết kế Dual I2C Bus. Thay vì cắm chung cảm biến và màn hình vào một đường dây như các đồ án thông thường, nhóm tách riêng hai bus phần cứng. Nhờ đó, dù màn hình có cập nhật liên tục thì cảm biến AI vẫn đọc đủ 50 mẫu mỗi giây một cách chính xác tuyệt đối."*

---

## 🖥️ SLIDE 7: `[CHƯƠNG 2]` MÔ HÌNH TINYML & THUẬT TOÁN TẠI BIÊN

### Bố cục (Chia 2 Cột Cực Ngắn Gọn):

#### Cột Trái: Quy Trình Xử Lý TinyML
* **Dữ liệu thực tế:** 3 nhãn (Bình thường – Va đập nhẹ – Té ngã).
* **Cửa sổ trượt:** 2.0 giây (100 mẫu), cập nhật mỗi 0.5 giây.
* **Trích xuất 8 đặc trưng:** Nén dữ liệu chuyển động (gia tốc, góc xoay, góc nghiêng).
* **Mạng nơ-ron Softmax:** Chạy trực tiếp trên RAM chip ESP32-S3.

#### Cột Phải: 3 Ưu Thế Của AI Tại Biên (Edge AI)
* ⚡ **Cực nhanh:** Thời gian suy luận dưới 5 mili-giây.
* 🔒 **Bảo mật:** Dữ liệu sóng cơ thể giữ kín tại cổ tay, không đẩy lên mạng.
* 📶 **Độc lập:** Mất sóng Wi-Fi vòng tay vẫn tự phát hiện ngã và hú còi.

> 🎤 **Speaker Notes (Lời thuyết trình):**  
> *"Thay vì so sánh ngưỡng đơn giản dễ báo nhầm, nhóm nén 2 giây chuyển động thành 8 đặc trưng để mạng nơ-ron TinyML phân loại. AI chạy trực tiếp tại chip với tốc độ dưới 5 mili-giây, vừa bảo mật dữ liệu riêng tư, vừa hoạt động tốt kể cả khi mất mạng Wi-Fi."*

---

## 🖥️ SLIDE 8: `[PHẦN 4]` BỘ LỌC 3 TẦNG CHỐNG BÁO ĐỘNG GIẢ

### Bố cục (Dạng 3 Khối Phòng Tuyến Liên Hoàn):

#### 🛡️ Tầng 1: Ngưỡng Kích Hoạt Động Học Ban Đầu
* Chỉ mở cửa sổ theo dõi khi gia tốc vượt 1.65g hoặc tốc độ xoay người vượt 300 độ/giây.
* Các cử động nhẹ nhàng (viết chữ, gõ bàn phím, cầm bát đũa) bị loại bỏ ngay từ đầu.

#### 🛡️ Tầng 2: Khóa Vận Động Liên Tục (Triệt tiêu Đi bộ & Vỗ tay)
* Khi đi bộ, vung tay hoặc tập dưỡng sinh, cổ tay cử động liên tục (tỷ lệ bất động dưới 18%).
* Hệ thống nhận diện đây là hành động chủ động và **HỦY BÁO ĐỘNG NGAY LẬP TỨC**.

#### 🛡️ Tầng 3: Kiểm Chứng Đổi Góc Nghiêng Nằm & Bất Động Kéo Dài
* **Đổi tư thế nằm:** Ngã thật làm cơ thể đổ sụp xuống sàn, trục trọng lực quay một góc lớn hơn 32 độ.
* **Bất động sau va chạm:** Người bị chấn thương nặng hoặc ngất xỉu sẽ nằm yên trong 2.5 giây tiếp theo (độ rung lắc gần như bằng 0).
* **Hai nhánh phát hiện:** Bắt trọn cả **Ngã va đập mạnh** lẫn **Đột quỵ ngất xỉu ngã từ từ**.

> 🎤 **Speaker Notes (Lời thuyết trình):**  
> *"Đây chính là giải pháp lâm sàng độc quyền của nhóm giúp giải quyết bài toán nhức nhối nhất của các vòng đeo tay thông minh: Đi bộ hay vỗ tay tập thể dục bị báo động giả. Nhờ bộ lọc 3 tầng kết hợp kiểm tra góc nằm và độ bất động sau cú ngã, tỷ lệ báo động giả khi tập luyện của vòng tay đạt mức 0%."*

---

## 🖥️ SLIDE 9: `[PHẦN 5]` HỆ THỐNG CLOUD BACKEND & ỨNG DỤNG DI ĐỘNG

### Bố cục (Chia 3 Khối Thẻ):

#### 1. Thiết Bị Vòng Đeo (Edge Device)
* Hiển thị giao diện OLED chữ lớn, dễ đọc.
* Giao tiếp không dây Wi-Fi gửi dữ liệu nhịp vận động định kỳ mỗi 2 giây.
* Còi báo và đếm ngược 10 giây tại chỗ.

#### 2. Máy Chủ Backend (Python REST API)
* Xây dựng máy chủ API hiệu năng cao với Python.
* Cơ sở dữ liệu dạng **JSON Lines (ghi nối tiếp liên tục)**, đảm bảo không bao giờ bị hỏng file hay mất lịch sử khi mất điện.
* Tích hợp dịch vụ OpenStreetMap chuyển đổi tọa độ GPS thành địa chỉ nhà cụ thể (Số nhà, Tên đường, Quận/Huyện).

#### 3. Ứng Dụng Android "Tận Tâm Care"
* Lập trình native bằng **Kotlin và Jetpack Compose**.
* **Dịch vụ chạy ngầm 24/7 (Foreground Service):** Luôn thường trực nhận tin khẩn cấp kể cả khi người thân khóa màn hình điện thoại.
* **Còi hú mức Báo thức (Alarm Stream):** Tự động phát âm lượng tối đa để đánh thức người nhà lúc nửa đêm, **bất chấp điện thoại đang để chế độ Im lặng hay Rung**.

> 🎤 **Speaker Notes (Lời thuyết trình):**  
> *"Hệ sinh thái của nhóm được khép kín từ Vòng tay, Máy chủ cho đến Ứng dụng điện thoại con cái. Điểm đặc biệt của app Android là kênh âm thanh khẩn cấp. Nếu người già ngã trong nhà tắm lúc nửa đêm, điện thoại người thân dù đang để im lặng đi ngủ vẫn sẽ tự động đổ chuông còi hú cứu thương ở mức to nhất để kịp thời sơ cứu."*

---

## 🖥️ SLIDE 10: `[PHẦN 6]` KẾT QUẢ ĐO KIỂM & THỰC NGHIỆM THỰC TẾ

### Bố cục:

#### Nửa trên: Bảng Kết Quả Thử Nghiệm 200 Lần Thực Tế

| Nhóm hành vi thử nghiệm | Số lần thử | Nhận diện đúng | Tỷ lệ chính xác | Đánh giá thực tế |
| :--- | :---: | :---: | :---: | :--- |
| **Đi bộ / Leo cầu thang** | 50 lần | 50 lần | **100%** | Tuyệt đối không báo động giả |
| **Vung tay tập thể dục / Vỗ tay** | 50 lần | 50 lần | **100%** | Khóa vận động liên tục triệt tiêu cử động |
| **Ngã về phía trước (Chống tay)** | 30 lần | 29 lần | **96.7%** | Nhận diện xung chấn va đập tiếp đất |
| **Trượt chân ngã ngửa / Ngã nghiêng** | 30 lần | 29 lần | **96.7%** | Nhận diện lật đổi góc nghiêng trên 32 độ |
| **Ngất xỉu / Đột quỵ ngã từ từ** | 20 lần | 19 lần | **95.0%** | Phát hiện bất động và nghiêng người |
| **Bấm nút khẩn cấp SOS** | 30 lần | 30 lần | **100%** | Kích hoạt chuông và báo về máy chủ sau 2s |

#### Nửa dưới: 3 Chỉ Số Kỹ Thuật Đạt Chuẩn
* ⏱️ **Thời gian phản hồi:** Phân tích lâm sàng 2.5 giây + Đếm ngược 10 giây (ngăn ngừa 100% các cú va chạm nhẹ tự đứng dậy được).
* 🛰️ **Độ chính xác GPS:** Sai số định vị ngoài trời dưới 2.5 mét.
* ⚡ **Thời gian gửi tin về máy chủ:** Dưới 200 mili-giây qua mạng Wi-Fi gia đình.

> 🎤 **Speaker Notes (Lời thuyết trình):**  
> *"Nhóm đã trực tiếp thực nghiệm hơn 200 kịch bản vận động khác nhau. Kết quả cho thấy độ chính xác phát hiện tai nạn đạt trên 96%, và đặc biệt là không có bất kỳ một trường hợp nào bị báo động giả khi đi bộ hay tập thể dục. Hệ thống phản hồi tức thì và hoạt động rất ổn định."*

---

## 🖥️ SLIDE 11: `[PHẦN 7]` ỨNG DỤNG AN TOÀN THÔNG TIN (ATTT) TRONG THIẾT KẾ

*(Nội dung trọng tâm gắn liền với chuyên ngành An toàn Thông tin của sinh viên PTIT)*

### Bố cục (Chia 4 Khối Cột Bảo Mật):

* 🔒 **1. Bảo vệ dữ liệu tại biên (Edge Privacy):**
  * Không phát tán dữ liệu cảm biến thô của cơ thể lên Cloud.
  * Chỉ gửi trạng thái kết luận và tọa độ khi có sự cố, ngăn chặn hoàn toàn nguy cơ bị nghe lén thói quen sinh hoạt.
* 🔑 **2. Bảo mật kênh truyền thông (Network Security):**
  * Kết nối Wi-Fi mã hóa chuẩn an toàn WPA2/WPA3.
  * Cấu hình Bluetooth BLE qua kênh GATT mã hóa với mã UUID riêng, chống can thiệp thiết bị lạ.
  * Giao tiếp Backend hỗ trợ Token xác thực, ngăn chặn giả mạo gói tin SOS ảo.
* 🛡️ **3. Bảo vệ bộ nhớ thiết bị (Hardware Flash NVS):**
  * Tách biệt phân vùng lưu trữ NVS của ESP32 để lưu mật khẩu Wi-Fi và khóa cấu hình.
  * Chống lại các kỹ thuật trích xuất bộ nhớ trái phép khi tháo chip.
* 📱 **4. Bảo mật ứng dụng di động (Mobile App Hardening):**
  * Phân quyền bảo vệ vị trí người già nghiêm ngặt.
  * Cầu nối giữa giao diện và code hệ thống được khóa chặt, chống triệt để lỗ hổng tấn công chèn mã độc XSS từ WebView.

> 🎤 **Speaker Notes (Lời thuyết trình):**  
> *"Là sinh viên chuyên ngành An toàn Thông tin, nhóm đặc biệt chú trọng yếu tố bảo mật y tế. Việc đưa AI về chạy cục bộ tại biên giúp dữ liệu chuyển động của người già không bị truyền tải tràn lan lên Internet. Kênh truyền mạng, bộ nhớ Flash và ứng dụng di động đều được áp dụng các tiêu chuẩn an toàn thông tin để đảm bảo quyền riêng tư tuyệt đối cho gia đình."*

---

## 🖥️ SLIDE 12: `[PHẦN 8]` SO SÁNH GIẢI PHÁP & HƯỚNG PHÁT TRIỂN

### Bố cục (Chia 2 Cột):

#### Cột Trái: Bảng So Sánh Với Thị Trường

| Tiêu chí | Apple Watch / Galaxy Watch | Vòng đeo SOS giá rẻ | **Smart Band AIoT (Nhóm 01)** |
| :--- | :---: | :---: | :---: |
| **Giá thành** | 8.000.000 - 15.000.000đ | 300.000 - 500.000đ | **Dưới 500.000đ (Tối ưu)** |
| **Xử lý AI tại biên** | Có (Hệ thống đóng) | Không có (Chỉ có nút bấm) | **Có (TinyML mã nguồn mở)** |
| **Chống báo động giả** | Khá tốt | Kém (Lắc tay là kêu) | **Rất tốt (Bộ lọc 3 tầng)** |
| **Khả năng tương thích** | Bắt buộc iPhone/Samsung | Cần đầu thu riêng | **Mọi điện thoại Android & Web** |
| **Màn hình hiển thị** | Màu phức tạp, khó dùng | Không có màn hình | **OLED chữ lớn, dễ nhìn** |

#### Cột Phải: Định Hướng Nâng Cấp Tương Lai
* 🚀 **Tích hợp Module SIM 4G LTE:** Giúp vòng tay độc lập hoàn toàn khi người già đi ra ngoài công viên, không phụ thuộc vào Wi-Fi nhà.
* 🚀 **Tích hợp cảm biến nhịp tim & SpO2 (MAX30102):** Theo dõi nồng độ oxy trong máu và nhịp tim để cảnh báo sớm nguy cơ đột quỵ từ trước khi té ngã.
* 🚀 **Tổng đài gọi thoại tự động (Voice Call Gateway):** Tự động gọi trực tiếp vào số điện thoại người thân khi có sự cố khẩn cấp.

> 🎤 **Speaker Notes (Lời thuyết trình):**  
> *"So với các dòng đồng hồ đắt tiền hay vòng bấm SOS thô sơ, sản phẩm của nhóm cân bằng hoàn hảo giữa giá thành rẻ và tính năng AI hiện đại. Trong giai đoạn tiếp theo, nhóm định hướng tích hợp thêm module 4G và cảm biến đo nồng độ oxy trong máu SpO2 để nâng tầm thiết bị thành một vòng tay y tế toàn diện."*

---

## 🖥️ SLIDE 13: TRANG KẾT LUẬN & CẢM ƠN (Q&A)

### Bố cục:
* **Thông điệp trung tâm (Chữ to, Nổi bật, Màu Đỏ PTIT):**  
  > **"CÔNG NGHỆ VÌ SỨC KHỎE CỘNG ĐỒNG - BẢO VỆ NGƯỜI THÂN TRONG TỪNG KHOẢNH KHẮC"**
* **Thông tin nhóm thực hiện:**
  * Đề tài: *Thiết kế thiết bị theo dõi vị trí người bệnh kết hợp điện toán biên phân tích hành động (Smart Band AIoT)*
  * Học viện Công nghệ Bưu chính Viễn thông – Khoa An toàn Thông tin
  * Nhóm sinh viên 01: Thùy Dương – Ngọc Linh – Yến Nhi
* **Lời cảm ơn & Mời đặt câu hỏi:**
  * **"XIN CHÂN THÀNH CẢM ƠN THẦY VÀ CÁC BẠN ĐÃ LẮNG NGHE!"**
  * **"Q&A - CHÚNG EM RẤT MONG NHẬN ĐƯỢC NHẬN XÉT VÀ GÓP Ý TỪ THẦY CÔ"**

> 🎤 **Speaker Notes (Lời thuyết trình):**  
> *"Bài thuyết trình của nhóm 01 đến đây là kết thúc. Chúng em xin chân thành cảm ơn Thầy giáo hướng dẫn TS. Lê Văn Vịnh và các bạn đã chú ý lắng nghe. Sau đây, nhóm chúng em rất mong nhận được những câu hỏi phản biện, nhận xét và đóng góp quý báu từ Thầy để sản phẩm ngày càng hoàn thiện hơn. Chúng em xin chân thành cảm ơn!"*
