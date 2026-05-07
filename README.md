# Real-Time Systems Application: IoT-Based Smart Gas Leakage Monitoring with Edge AI

## Abstract
**[Assignee: Hiển] | [Status: To do]**
> **Hướng dẫn viết:** Tóm tắt ngắn gọn mục tiêu nhóm đề ra của bài tập lớn môn học IOT, bối cảnh áp dụng RTOS vào bài toán IoT thực tế trên nền tảng ESP32-S3. Trình bày tổng quan về các công nghệ cốt lõi được sử dụng và kết quả đạt được.

---

## 1. Introduction and Objectives
**[Assignee: Hiển] | [Status: To do]**
> **Hướng dẫn viết:** Nêu rõ lý do chọn đề tài và các mục tiêu chính của dự án. [cite_start]Đề cập đến phần cứng sử dụng và định hướng phát triển mã nguồn gốc[cite: 7].

---

## 2. System Implementation & Innovations (Tasks 1-6)
[cite_start]*(⚠️ Hướng dẫn chung: Chương này là phần quan trọng nhất của của báo cáo. Bắt đầu bằng kiến trúc tổng thể, sau đó mỗi Task phải được chia làm 2 phần rõ rệt: **Core Implementation** (đáp ứng đúng đặc tả) và **Advanced/Innovation** (sự khác biệt >30%sáng tạo từ đề tài)[cite: 8, 12]. Phân bổ dung lượng hợp lý, có ảnh/code snippet minh họa).*

### 2.1. Overall RTOS Architecture & Infrastructure
**[Assignee: Hiển] | [Status: To do]**
> **Hướng dẫn viết:** Đây là phần trình bày "Bức tranh toàn cảnh" (Big Picture) của hệ thống RTOS trước khi đi sâu vào chi tiết.
> - **Task Management:** Đưa ra bảng quy hoạch Task Priority và Core Affinity (Core 0 cho mạng/HMI, Core 1 cho AI/Cảm biến).
> - **Inter-task Communication:** Vẽ hoặc mô tả sơ đồ luồng dữ liệu, cách các task giao tiếp với nhau thông qua **Queue** (ví dụ: `qSensorLed`, `qSensorTinyML`).
> [cite_start]- **Shared Resource Protection:** Giải thích việc khởi tạo và sử dụng **Mutex/Semaphore** toàn cục (như `i2c_semaphore`) để bảo vệ bus I2C tránh xung đột phần cứng ngay từ khâu setup.

### 2.2. Task 1: Single LED Blink with Temperature Conditions
**[Assignee: Minh Huy] | [Status: To do]**
* [cite_start]**Core Implementation:** Trình bày cách định nghĩa lại hành vi chớp tắt của LED để phản hồi với ít nhất 3 điều kiện nhiệt độ[cite: 15, 16]. [cite_start]Giải thích logic xử lý và cách sử dụng **Semaphore** để đồng bộ hóa task[cite: 17, 18].
* **Advanced / Innovation:** (Người viết tự quyết định). *Gợi ý: Có thể là hiệu ứng chớp tắt phi tuyến tính (fading) thay vì blink cứng nhắc, hoặc ngưỡng nhiệt độ có thể thay đổi real-time (dynamic threshold) từ Webserver.*

### 2.3. Task 2: NeoPixel LED Control Based on Humidity
**[Assignee: Minh Huy] | [Status: To do]**
* [cite_start]**Core Implementation:** Hiển thị bản đồ (mapping) giữa các dải giá trị độ ẩm và ít nhất 3 mức độ/màu sắc của NeoPixel[cite: 19, 20, 22]. [cite_start]Trình bày kỹ thuật đồng bộ hóa bằng **Semaphore** để cập nhật màu sắc[cite: 21].
* **Advanced / Innovation:** (Người viết tự quyết định). *Gợi ý: Hiệu ứng chuyển màu mượt mà (breathing effect), hoặc nháy cảnh báo đỏ chớp nhoáng khi có rò rỉ gas chèn ngang độ ẩm.*

### 2.4. Task 3: Temperature and Humidity Monitoring with LCD Display
**[Assignee: Hiển] | [Status: To do]**
* [cite_start]**Core Implementation:** Chụp ảnh màn hình hiển thị ít nhất 3 trạng thái (e.g., normal, warning, critical)[cite: 25]. [cite_start]Định nghĩa điều kiện tạo/giải phóng semaphores dựa trên cảm biến[cite: 24]. [cite_start]**BẮT BUỘC:** Đưa code snippet chứng minh đã loại bỏ TOÀN BỘ biến toàn cục (global variables)[cite: 26].
* **Advanced / Innovation:** (Người viết tự quyết định). *Gợi ý: Sử dụng Mutex đã khai báo ở mục 2.1 để bảo vệ nghiêm ngặt bus I2C, hoặc tự vẽ các icon (custom characters) sinh động trên LCD.*

### 2.5. Task 4: Web Server in Access Point Mode
**[Assignee: Lê Huy] | [Status: To do]**
* [cite_start]**Core Implementation:** Chụp ảnh giao diện Web Server đã được thiết kế lại (better usability)[cite: 27, 28]. [cite_start]Chứng minh có ít nhất 2 nút nhấn (có nhãn dán rõ ràng) để điều khiển 2 thiết bị (VD: LED1/LED2)[cite: 29, 30].
* **Advanced / Innovation:** (Người viết tự quyết định). *Gợi ý: Dùng WebSockets/AJAX để cập nhật thông số real-time mà không cần tải lại trang, hoặc tính năng tự động chuyển từ AP mode sang Station mode khi cấu hình xong.*

### 2.6. Task 5: TinyML Deployment & Accuracy Evaluation
**[Assignee: Lê Huy] | [Status: To do]**
* [cite_start]**Core Implementation:** Mô tả tập dữ liệu (dataset), các bước thu thập và gán nhãn[cite: 31, 32]. [cite_start]Trình bày quá trình chạy mô hình TinyML trên ESP32-S3[cite: 33]. [cite_start]Đo lường và đánh giá độ chính xác (accuracy)[cite: 34].
* **Advanced / Innovation:** (Người viết tự quyết định). *Gợi ý: Áp dụng mô hình mạng nơ-ron phức tạp hơn (như CNN 1D cho dữ liệu chuỗi thời gian) hoặc tối ưu hóa Memory Arena để model chạy với RAM cực thấp.*

### 2.7. Task 6: Data Publishing to CoreIOT Cloud Server
**[Assignee: Hiển] | [Status: To do]**
* [cite_start]**Core Implementation:** Trình bày chức năng gửi dữ liệu cảm biến lên CoreIOT qua WiFi (Station Mode)[cite: 36, 37]. [cite_start]Chụp ảnh code minh chứng đã dùng đúng Authentication Token và sử dụng template giải pháp[cite: 38, 39, 40].
* **Advanced / Innovation:** (Người viết tự quyết định). *Gợi ý: Thiết lập Rule Chain trên CoreIOT để tự động gửi cảnh báo Telegram/Email khi có gas rò rỉ, hoặc dùng RPC để điều khiển Relay từ xa qua Cloud.*
---

## 3. Experimental Evaluation
**[Assignee: Team] | [Status: To do]**
> [cite_start]**Hướng dẫn viết:** Tổng hợp lại các kết quả thử nghiệm thực tế của toàn bộ hệ thống (latency của hệ thống, thời gian phản hồi của relay, đánh giá TinyML)[cite: 46]. Sử dụng bảng biểu và biểu đồ để minh họa.

---

## 4. Group Organization & Git Contribution
**[Assignee: Hiển] | [Status: To do]**
> [cite_start]- Chèn link GitHub công khai chứa toàn bộ mã nguồn của nhóm[cite: 49].
> [cite_start]- Đính kèm bảng phân công nhiệm vụ thể hiện rõ vai trò và đóng góp (100%) của các thành viên[cite: 48, 57].
> [cite_start]- Chụp ảnh lịch sử Commit và luồng Git để chứng minh 5% điểm tổ chức nhóm[cite: 53].

---

## 5. Group Discussion and Conclusions
**[Assignee: Hiển] | [Status: To do]**
> [cite_start]**Hướng dẫn viết:** Đưa ra thảo luận nhóm về các khó khăn gặp phải, bài học rút ra[cite: 47]. Kết luận lại những thành quả đạt được so với mục tiêu ban đầu.