# OS Lab 3 — Page Table

Repository làm việc: **https://github.com/EndlessMelody/os-lab3**.
Kế thừa nguyên lịch sử và code đã tích hợp của [Lab 2](https://github.com/EndlessMelody/os-lab2),
commit `1c503f0afc41a203de08b4cc66dadb1db506f7df`, bao gồm tiện ích
[Lab 1](https://github.com/EndlessMelody/os-lab1), `trace`, `sysinfo` và `ptree`/`pstree`.

- [Đề chính thức](Lab3_OS_PageTable.pdf) và [hướng dẫn](<Hướng dẫn project 3 - PageTable.pdf>).
- [Phân công nhóm](Lab3_PageTable_PhanCong.html), [quy ước bàn giao](docs/lab03/TEAM.md), [checklist](Task.md).
- [Baseline, phạm vi dọn dẹp và cách lấy tài nguyên cũ](docs/lab03/BASELINE.md).

## Lấy source và build

Linux/WSL, RISC-V GCC/binutils, QEMU, GDB, Make, Perl và Python 3. Trên Ubuntu:

```sh
sudo apt update
sudo apt install build-essential gcc-riscv64-linux-gnu binutils-riscv64-linux-gnu qemu-system-misc gdb-multiarch perl python3
git clone https://github.com/EndlessMelody/os-lab3.git
cd os-lab3
git switch lab3-ugetpid  # Đổi theo bảng phân công.
make -j4 kernel/kernel fs.img
make qemu
```

Thoát QEMU bằng `Ctrl-a`, rồi `x`. Checkout đang mở trong IDE vẫn có tên thư mục
`os-lab2`; remote `origin` đã trỏ sang `os-lab3`, không cần đổi thư mục để làm tiếp.

| Thành viên | Branch | Phần chính | Người review |
|---|---|---|---|
| Đăng Khoa | [`lab3-ugetpid`](https://github.com/EndlessMelody/os-lab3/tree/lab3-ugetpid) | 4.1 · shared page / ugetpid | Bảo Trân |
| Bảo Trân | [`lab3-vmprint`](https://github.com/EndlessMelody/os-lab3/tree/lab3-vmprint) | 4.2 · vmprint; tích hợp code và báo cáo | Nhựt Duy |
| Nhựt Duy | [`lab3-pgaccess`](https://github.com/EndlessMelody/os-lab3/tree/lab3-pgaccess) | 4.3 · pgaccess / PTE_A | Đăng Khoa |

Ba branch cùng bắt đầu từ `lab3-start`. `main` ban đầu là bộ khung DevOps build được,
chưa phải bài Lab 3 hoàn thành. Hạn nội bộ: code **27/11/2026**, báo cáo và đóng gói
**29/11/2026**; hạn Moodle theo thông báo môn học.

## Bộ khung và kiểm thử

`conf/lab.mk` dùng `LAB=pgtbl`. Giữ hai user stack pages như Lab 1–2 để các tiện ích
đệ quy tiếp tục hoạt động. Đã có `USYSCALL`/`struct usyscall`, helper user `ugetpid`,
`SYS_pgaccess=25`, prototype, stub và dispatch. `sys_pgaccess()` hiện trả `-1`;
chưa cấp phát/map shared page, chưa có `vmprint` và chưa xử lý `PTE_A`.

```sh
python3 tools/verify-lab03.py         # Clean build + hồi quy Lab 1, trace, Lab 2.
./grade-lab-pgtbl ugetpid             # Test riêng 4.1.
./grade-lab-pgtbl vmprint             # Test riêng 4.2.
./grade-lab-pgtbl pgaccess            # Test riêng 4.3.
make grade                          # Cả ba phần Lab 3, sau khi triển khai.
python3 tools/verify-lab03.py --pgtbl  # Kiểm tra đầy đủ trước chốt tích hợp.
```

Trong QEMU: `pgtbltest`, hoặc `pgtbltest ugetpid` / `pgtbltest pgaccess` để tự test
từng phần. `make check-lab03` cũng chạy bộ chấm Lab 3. `grade-lab-pgtbl` và
`user/pgtbltest.c` là bộ test nội bộ viết theo PDF, **không phải bộ chấm chính thức
của giảng viên/MIT**. Cần bổ sung test quyền trang và tham số lỗi. Ở bộ khung ban
đầu, ba test Lab 3 dự kiến FAIL vì chưa triển khai nhiệm vụ kernel.

CI tự chạy build + hồi quy khi push ba branch hoặc mở PR vào `main`; log lưu trong
GitHub Actions artifacts. Chạy workflow thủ công để bật thêm job chấm Page Table;
job này phải đạt trước code freeze. CI hồi quy xanh chưa xác nhận Lab 3 hoàn thành.
Log local ở `artifacts/`, không tự commit. Chỉ commit minh chứng đã chọn vào
`docs/lab03/evidence/`. Grader dùng chung `fs.img` và cổng GDB, chạy tuần tự và tắt
QEMU tương tác trước khi test. Các suite tạo lại filesystem image.

## Bàn giao và nộp bài

Push branch cá nhân, mở PR vào `main`, kèm lệnh test, output thực tế và người review.
Bảo Trân tích hợp sau review, chạy toàn bộ test trên cùng commit và cập nhật checklist.
Xem lệnh Git và quy ước trong [TEAM.md](docs/lab03/TEAM.md).

Tag `lab3-base` là source Lab 2 kế thừa trước mọi thay đổi Lab 3; `lab3-start` là
bộ khung chung sau dọn dẹp. Patch cuối lấy từ **`lab3-base`** để chứa cả thay đổi
chuẩn bị và file mới, sau khi commit đầy đủ:

```sh
git diff --binary lab3-base HEAD > ../StudentID1_StudentID2_StudentID3.patch
```

Nộp Moodle file `StudentID1_StudentID2_StudentID3.zip`, MSSV tăng dần, gồm PDF báo
cáo, patch trong **`Realease/`** (đúng cách viết trong đề) và ZIP source đã
`make clean` trong `Source/`; bổ sung `answers-*.txt` theo câu hỏi đề bài.
Chưa tạo báo cáo, đáp án hay bộ nộp Lab 3 trong giai đoạn DevOps.

README xv6 gốc và giấy phép: [README](README), [LICENSE](LICENSE).
