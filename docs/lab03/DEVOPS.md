# Bàn giao bộ khung DevOps — 25/11/2026

Đã chuyển `origin` sang `EndlessMelody/os-lab3`, giữ remote cũ tên `lab2` và lịch
sử Lab 1–2. Đã push thành công `main`, `lab3-ugetpid`, `lab3-vmprint`,
`lab3-pgaccess` và hai tag `lab3-base` / `lab3-start`; commit bộ khung là `082f7c0`. Phạm vi dọn dẹp/khôi phục nằm trong [BASELINE.md](BASELINE.md).

Kết quả local trên source chuẩn bị, trước khi tạo commit/tag bàn giao:

| Kiểm tra | Kết quả |
|---|---|
| `make clean`; `make -j4 kernel/kernel fs.img` | PASS |
| `make check-lab01` | PASS — 17 nhóm (MIT subset 5, TV1 5, Lab 1 mở rộng 7) |
| `make check-trace` | PASS — 7 nhóm, 70/70 |
| `make check-lab02` | PASS — 9 nhóm, 90/90 |
| `make grade` | FAIL dự kiến — 0/100 vì chưa triển khai ba phần kernel |
| Python syntax, Git whitespace, link tài liệu local | PASS |

[Log và manifest](evidence/devops/context.json) lưu lệnh/kết quả build và test;
[source hashes](evidence/devops/source-sha256.json) xác định đúng source đã kiểm
tra. Runtime source không đổi trong suốt các suite. Chưa có kết quả implementation,
review chéo, báo cáo hay gói nộp Lab 3. Log Page Table xác nhận shared page chưa
được map, `vmprint` chưa có và stub `pgaccess` trả lỗi.

CI mặc định kiểm tra build + hồi quy trên source commit ở GitHub. Trạng thái thực
tế xem ở [GitHub Actions](https://github.com/EndlessMelody/os-lab3/actions).
Job Page Table bật thủ công trong giai đoạn scaffold; trước chốt bài phải đạt
đầy đủ theo [TEAM.md](TEAM.md). Quy tắc review hiện được ghi trong tài liệu và
mẫu PR; chưa cấu hình branch protection/quyền thành viên trên GitHub.
