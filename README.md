# Real-Time Systems Application: IoT-Based Smart Gas Leakage Monitoring with Edge AI

## Abstract
**[Assignee: Hiển] | [Status: To do]**
> **Hướng dẫn viết:** Tóm tắt ngắn gọn mục tiêu của dự án trong bối cảnh môn học Hệ thống thời gian thực (Real-Time Systems). Trình bày cách áp dụng RTOS trên nền tảng ESP32-S3 để giải quyết bài toán giám sát rò rỉ gas. Tóm lược các công nghệ cốt lõi được sử dụng: FreeRTOS, TinyML, và Cloud Integration.

---

## 1. Introduction and Objectives
**[Assignee: Hiển] | [Status: To do]**
> **Hướng dẫn viết:** Giải thích lý do thực hiện đề tài và các mục tiêu chính của dự án. Đề cập đến việc phát triển dựa trên mã nguồn gốc YoloUNO_PlatformIO và mục tiêu thay đổi ít nhất 30% logic hệ thống so với bản gốc. Nêu rõ phạm vi thiết bị được thiết lập tại phòng 301B9 và 812H6.

---

## 2. System Implementation & Innovations (Tasks 1-6)
> *(⚠️ Hướng dẫn chung: Đây là chương trọng tâm của báo cáo. Bắt đầu bằng kiến trúc hạ tầng RTOS, sau đó trình bày chi tiết từng Task theo hai phần: **Core Implementation** (đáp ứng đúng đặc tả kỹ thuật) và **Advanced/Innovation** (các tính năng sáng tạo để tạo sự khác biệt). Phân bổ dung lượng hợp lý, kèm ảnh chụp minh họa và code snippet).*

### 2.1. Overall RTOS Architecture & Infrastructure
**[Assignee: Hiển] | [Status: To do]**
* **Task Management:** Quy hoạch bảng mức độ ưu tiên (Priority) và phân bổ lõi xử lý (Core Affinity) cho các tác vụ.
* **Inter-task Communication:** Mô tả luồng dữ liệu giữa các thành phần thông qua cơ chế **Queue** thay vì sử dụng biến toàn cục.
* **Shared Resource Protection:** Giải thích cách khởi tạo và sử dụng **Mutex/Semaphore** toàn cục (như `i2c_semaphore`) để bảo vệ tài nguyên dùng chung, tránh xung đột phần cứng.

### 2.2. Task 1: Single LED Blink with Temperature Conditions
**[Assignee: Minh Huy] | [Status: To do]**
* **Core Implementation:** Định nghĩa ít nhất 3 hành vi chớp tắt của LED tương ứng với các điều kiện nhiệt độ khác nhau. Giải thích logic sử dụng **Semaphore** để đồng bộ hóa tác vụ hiển thị.
* **Advanced / Innovation:** [Thành viên tự đề xuất và triển khai nội dung sáng tạo tại đây].

### 2.3. Task 2: NeoPixel LED Control Based on Humidity
**[Assignee: Minh Huy] | [Status: To do]**
* **Core Implementation:** Thiết lập bảng mã màu NeoPixel (RGB) đại diện cho ít nhất 3 dải độ ẩm khác nhau. Sử dụng kỹ thuật **Semaphore** để đồng bộ hóa việc cập nhật và hiển thị màu sắc.
* **Advanced / Innovation:** [Thành viên tự đề xuất và triển khai nội dung sáng tạo tại đây].

### 2.4. Task 3: Temperature and Humidity Monitoring with LCD Display
**[Assignee: Minh Huy] | [Status: To do]**
* **Core Implementation:** Hiển thị dữ liệu cảm biến lên màn hình LCD với ít nhất 3 trạng thái môi trường (e.g., Normal, Warning, Critical). Định nghĩa logic tạo/giải phóng semaphore dựa trên kết quả đo lường.
* **BẮT BUỘC:** Chứng minh việc loại bỏ **HOÀN TOÀN** biến toàn cục (Global Variables) bằng cách sử dụng cơ chế truyền tin của RTOS.
* **Advanced / Innovation:** [Thành viên tự đề xuất và triển khai nội dung sáng tạo tại đây].

### 2.5. Task 4: Web Server in Access Point Mode
**[Assignee: Lê Huy] | [Status: To do]**
* **Core Implementation:** Thiết kế lại giao diện Web Server ở chế độ AP để tăng tính tiện dụng (User Experience). Tích hợp ít nhất 2 nút điều khiển có nhãn dán rõ ràng để quản lý 2 thiết bị đầu ra.
* **Advanced / Innovation:** [Thành viên tự đề xuất và triển khai nội dung sáng tạo tại đây].

### 2.6. Task 5: TinyML Deployment & Accuracy Evaluation
**[Assignee: Lê Huy] | [Status: To do]**
* **Core Implementation:** Mô tả quy trình thu thập dữ liệu (dataset), các bước tiền xử lý và gán nhãn. Triển khai mô hình TinyML trên ESP32-S3 và đánh giá độ chính xác thực tế trên phần cứng.
* **Advanced / Innovation:** [Thành viên tự đề xuất và triển khai nội dung sáng tạo tại đây].

### 2.7. Task 6: Data Publishing to CoreIOT Cloud Server
**[Assignee: Hiển] | [Status: To do]**
* **Core Implementation:** Cấu hình ESP32-S3 ở chế độ Station (STA) để kết nối WiFi và đẩy dữ liệu cảm biến lên server CoreIOT. Đảm bảo Authentication Token khớp với thiết bị đã đăng ký và sử dụng đúng Solution Template.
* **Advanced / Innovation:** [Thành viên tự đề xuất và triển khai nội dung sáng tạo tại đây].

---

## 3. Experimental Evaluation
**[Assignee: Team] | [Status: To do]**
> **Hướng dẫn viết:** Tổng hợp kết quả thực nghiệm về độ trễ hệ thống, hiệu năng của relay, và đánh giá chi tiết độ chính xác của mô hình TinyML. Sử dụng bảng biểu và biểu đồ để minh họa các thông số kỹ thuật thu được.

---

## 4. Group Organization & Git Contribution
**[Assignee: Hiển] | [Status: To do]**
* **GitHub Repository:** [Chèn link công khai của nhóm tại đây]
* **Bảng phân công nhiệm vụ:** Liệt kê chi tiết vai trò và tỷ lệ đóng góp (cam kết 100% cho mỗi thành viên).
* **Git Contribution:** Hình ảnh minh chứng lịch sử Commit, các Pull Request và cấu trúc quản lý nhánh (Branching strategy).

---

## 5. Group Discussion and Conclusions
**[Assignee: Team] | [Status: To do]**
> **Hướng dẫn viết:** Thảo luận về các khó khăn kỹ thuật đã gặp phải, các bài học rút ra về hệ thống thời gian thực. Kết luận về mức độ hoàn thành dự án so với mục tiêu ban đầu và đề xuất hướng phát triển tương lai.

---

### 📋 Thông tin dự án (Project Metadata)
* **Thời hạn nộp (Deadline):** 22/05/2026.
* **Hình thức nộp:** File PDF báo cáo và Link GitHub Repository, có thể thêm video demo nếu có.
* **Trọng số đánh giá:** 60% Chức năng, 25% Chất lượng báo cáo, 5% Code, 5% Sáng tạo, 5% Quản lý Git.