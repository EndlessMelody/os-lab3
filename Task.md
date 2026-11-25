# Lab 3 — Checklist triển khai và bàn giao

Đối chiếu [đề chính thức](Lab3_OS_PageTable.pdf),
[phân công](Lab3_PageTable_PhanCong.html) và [TEAM.md](docs/lab03/TEAM.md).
Ngày chuẩn bị: 25/11/2026. Chỉ đánh dấu kết quả code, test và review khi có minh chứng.

## DevOps

- [x] Giữ lịch sử Lab 1–2, baseline `1c503f0afc41a203de08b4cc66dadb1db506f7df`.
- [x] Dọn tài nguyên lab cũ khỏi cây làm việc; bảo toàn bằng tag `lab3-base`.
- [x] Đổi cấu hình và hướng dẫn sang repository `EndlessMelody/os-lab3`.
- [x] Chuẩn bị giao diện chung, test nội bộ, CI build + hồi quy và mẫu PR.
- [x] Build sạch và 33 nhóm test hồi quy đạt; Page Table 0/100 đúng trạng thái scaffold.
- [ ] Push `main`, `lab3-base`, `lab3-start` và ba branch cá nhân lên GitHub.

## Đăng Khoa — 4.1, branch `lab3-ugetpid`

- [ ] Cấp phát/khởi tạo shared page theo PID, map `USYSCALL` với `PTE_U | PTE_R`.
- [ ] Xử lý free/unmap, nhánh lỗi, fork/exec/exit và tái sử dụng process slot.
- [ ] `pgtbltest ugetpid` đạt; bổ sung test quyền user read-only và vòng đời.
- [ ] Commit code, log test và phần báo cáo 4.1; trả lời câu hỏi shared page.
- [ ] Review `pgaccess` của Nhựt Duy; chuẩn bị source ZIP sạch khi chốt.

## Bảo Trân — 4.2, branch `lab3-vmprint`

- [ ] Viết `vmprint` đệ quy, bỏ PTE invalid, phân biệt leaf, in đúng format `%p`.
- [ ] Khai báo `defs.h`, gọi trong `exec.c` khi PID = 1 trước `return argc`.
- [ ] Test `vmprint` đạt; phân tích từng leaf theo layout thực tế (hai stack pages).
- [ ] Review `ugetpid` của Đăng Khoa; tích hợp đủ ba phần sau review.
- [ ] Viết phần 4.2, tổng hợp báo cáo PDF và minh chứng tích hợp.

## Nhựt Duy — 4.3, branch `lab3-pgaccess`

- [ ] Thay stub `sys_pgaccess`, định nghĩa `PTE_A` bit 6, kiểm tra địa chỉ/số trang/buffer.
- [ ] Dùng mask `uint` 32 bit, `walk`, xóa accessed bit và `copyout` kết quả.
- [ ] `pgtbltest pgaccess` đạt; bổ sung test tham số lỗi, biên mask và gọi lặp lại.
- [ ] Review `vmprint` của Bảo Trân; viết phần 4.3 và tổng kết/hạn chế.
- [ ] Tạo patch từ `lab3-base`, tổng hợp `answers-*.txt`, kiểm tra gói nộp.

## Chốt chung — code 27/11, báo cáo 29/11/2026

- [ ] Chạy `pgtbltest`, `make grade` và hồi quy trên cùng commit tích hợp; lưu log.
- [ ] Review chéo đầy đủ; mỗi thành viên xác nhận nội dung báo cáo phần mình.
- [ ] Tạo PDF, patch `Realease/`, ZIP source đã `make clean`, đáp án và ZIP ngoài.
- [ ] Kiểm tra patch áp dụng được vào baseline, source ZIP build được và MSSV tăng dần.
- [ ] Xác nhận hạn/người nộp; nộp Moodle và kiểm tra kết quả upload.
