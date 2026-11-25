# Mốc kế thừa và dọn dẹp

Repository: https://github.com/EndlessMelody/os-lab3. Chuẩn bị ngày 25/11/2026.

| Mốc | Ý nghĩa |
|---|---|
| `lab3-base` | Bản Lab 2 tích hợp cuối, `1c503f0afc41a203de08b4cc66dadb1db506f7df`; baseline patch Lab 3. |
| `lab3-start` | Bộ khung DevOps đã kiểm tra; ba branch cùng bắt đầu ở đây. |
| `lab2/main` | Remote tham khảo của repo Lab 2; không dùng để push Lab 3. |
| `lab1` / `upstream` | Remote lịch sử trên checkout cũ; clone mới chỉ có `origin`. |

Giữ code kernel, tiện ích Lab 1, syscalls Lab 2, source test và grader hồi quy.
Bổ sung giao diện chung Lab 3; chưa triển khai ba nhiệm vụ kernel.
Giữ `Logo_Truong.png` làm tài nguyên cho báo cáo sắp tới.

Loại khỏi cây source hiện tại: đề/phân công Lab 1–2, PDF/LaTeX báo cáo cũ và bản PDF
trùng; log/figure/minh chứng cũ; scripts đóng gói/dựng báo cáo hardcode Lab 1–2.
Toàn bộ vẫn nằm trong lịch sử và tag `lab3-base`. Gói nộp `submissions/` bỏ tracking
nhưng giữ local; `.gitignore` chặn commit lại. Build output, filesystem image và
QEMU log dọn bằng `make clean`; bản sao checkout trước chuyển đổi nằm tại
`.git/lab3-migration-backup.tar.gz` (chỉ local, không upload GitHub).

Lấy tài nguyên cũ riêng để tham khảo:

```sh
git show lab3-base:README.md
git ls-tree -r --name-only lab3-base
git archive lab3-base OS2Lab_Report.pdf OS2Lab_Report.tex docs/lab02 > ../lab2-reference.tar
```

Chọn `lab3-base` để patch gồm mọi thay đổi từ code kế thừa, cả scaffold và file mới.
`lab3-start` chỉ dùng đối chiếu phần thành viên sau chuẩn bị. Nếu giảng viên cấp
source gốc khác, thống nhất lại baseline và kiểm tra patch trên đúng source trước
nộp. Không tự coi bộ khung MIT 2024 là đề Page Table của môn.

Hai PDF trong root là tài liệu đầu vào. Đề yêu cầu `ugetpid`, `vmprint`, `pgaccess`;
không thêm superpage hay nhiệm vụ thuộc phiên bản MIT khác.
