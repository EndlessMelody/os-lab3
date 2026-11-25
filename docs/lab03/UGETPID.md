# Bàn giao 4.1 — Shared PID page / ugetpid

Phụ trách: Đăng Khoa. Branch: `lab3-ugetpid`, kế thừa `db118dd`.
Commit code: `cf9756f`. Hoàn tất kiểm tra local ngày 26/11/2026 (Asia/Bangkok).
Người review theo phân công: Bảo Trân. Chưa xác nhận review thay thành viên.

## Cơ chế và thay đổi

`ugetpid()` đã có trong bộ khung user: đọc trường `pid` ở `USYSCALL`, không có
`ecall`. Phần kernel bổ sung một trang vật lý riêng cho mỗi `struct proc`:

1. `allocproc()` cấp phát `p->usyscall` sau trapframe, xóa sạch 4096 byte và ghi
   `p->pid`. Nếu cấp phát hoặc tạo page table lỗi, gọi `freeproc()` để rollback.
2. `proc_pagetable()` map `USYSCALL = TRAPFRAME - PGSIZE` với `PTE_U | PTE_R`;
   `mappages()` thêm `PTE_V`. Không cấp `PTE_W` hoặc `PTE_X`. Nhánh lỗi gỡ các
   mapping đã tạo và giải phóng cây page table, giữ quyền sở hữu trang ở process.
3. `proc_freepagetable()` gỡ mapping với `do_free=0`, sau đó hủy cây bảng trang.
4. `freeproc()` giải phóng trang vật lý đúng một lần và đặt con trỏ về 0 để tái
   sử dụng process slot an toàn, kể cả các nhánh cấp phát dở dang.

Không cần đổi `fork()` hay `exec()`. `fork()` gọi `allocproc()` cho child và
`uvmcopy()` chỉ sao chép vùng dưới `p->sz`, nên trang PID ở vùng địa chỉ cao không
bị ghi đè bằng trang của parent. `exec()` gọi `proc_pagetable(p)` cho ảnh mới nên
map lại chính trang PID hiện có. Bảng cũ và mới có thể cùng tham chiếu trang này
trong lúc chuyển ảnh; hủy một bảng không được giải phóng trang PID. Khi `exec`
thất bại, bảng cũ và PID vẫn còn nguyên.

Trang vẫn tồn tại khi process là zombie, và được giải phóng khi `wait()` thu hồi
process qua `freeproc()`, giống vòng đời tài nguyên process trong xv6.

Các file kernel thay đổi: `proc.c`, `proc.h`, chú thích layout trong `memlayout.h`.
`user/ulib.c` chỉ cập nhật chú thích của helper đã có. Không triển khai phần 4.2/4.3.

## Kiểm thử và bằng chứng

```sh
make clean
make -j4 kernel/kernel fs.img
./grade-lab-pgtbl ugetpid
make check-ugetpid
python3 tools/verify-lab03.py
./grade-lab02-kernel
```

`make check-ugetpid` dùng helper trong `tests/ugetpidcheck.c`, chỉ cài vào filesystem
kiểm thử, không thêm chương trình test vào `UPROGS` mặc định. Tám nhóm kiểm tra:

| Nhóm | Điều kiện kiểm tra |
|---|---|
| Scaffold | Test `pgtbltest ugetpid` có sẵn, 64 lượt fork/wait |
| Fast path | Bật trace riêng getpid; 128 lượt ugetpid đúng PID và không xuất hiện syscall getpid |
| Lifecycle | 96 lượt fork/wait; toàn bộ padding bằng 0; free bytes trước/sau bằng nhau |
| Concurrent | 8 child cùng tồn tại, báo 8 PID riêng; page parent không bị thay đổi |
| Exec | 32 ELF lỗi sau khi tạo bảng mới; 16 lần thay ảnh thành công; không mất trang |
| Read-only | User store gây store page fault; kernel copyout qua pipe không ghi được |
| NX | Gọi code ở USYSCALL gây instruction page fault, không phải illegal instruction |
| OOM | Fork khi chỉ còn khoảng 0–9 trang rảnh, kiểm tra rollback và bộ nhớ thu hồi đầy đủ |

Hai test quyền trang cố ý tạo user trap ở child. Grader kiểm tra chính xác
`scause=0xf` / `0xc`, `stval=USYSCALL` và exit status `-1`, thay vì bỏ qua mọi trap.
Ở xv6 này, `read(pipe, ...)` trả số byte đã copy, nên 0 cũng là kết quả từ chối
ghi hợp lệ; không tự sửa syscall để bắt nó trả `-1`.

Kết quả trên đúng runtime source của commit `cf9756f`:

| Bộ kiểm tra | Kết quả |
|---|---|
| Clean build kernel/filesystem | PASS |
| `./grade-lab-pgtbl ugetpid` | PASS |
| `make check-ugetpid` | 8/8 nhóm, 100/100 |
| Hồi quy Lab 1 + trace + Lab 2 | 33/33 nhóm |
| `usertests -q` qua `./grade-lab02-kernel` | 60 bài, ALL TESTS PASSED |

Trong test OOM local, fork lần lượt gặp 0, 4096, ... 36864 byte bộ nhớ rảnh và
không làm mất byte nào sau rollback. Test quyền trang ghi nhận đúng hai loại
page fault dự kiến. Không có số liệu benchmark thời gian để suy ra tỷ lệ tăng tốc.

[Log và context](evidence/lab3-ugetpid/context.json),
[manifest 101 file source](evidence/lab3-ugetpid/source-sha256.json) xác định bản
đã kiểm tra; các file này được đối chiếu với nội dung đã commit, không chỉ HEAD
trong một working tree đang sửa. CI trên nhánh này bổ sung `make check-ugetpid`
sau hồi quy Lab 1–2; [lượt chạy trên GitHub](https://github.com/EndlessMelody/os-lab3/actions/runs/45659671942).
Lượt CI này đã đạt trên commit code `cf9756f`, gồm cả hồi quy và tám nhóm test 4.1.
Bộ test là kiểm tra nội bộ theo PDF, không phải bộ chấm chính thức của môn.

## Nội dung chuẩn bị báo cáo

Project 3 tìm hiểu cơ chế bảng trang Sv39 trong xv6/RISC-V qua ba yêu cầu: tăng
tốc truy vấn PID bằng shared page, in cây bảng trang và phát hiện trang đã truy
cập. Source kế thừa các tiện ích Lab 1 và system call Lab 2. Nhánh này phụ trách
4.1, chạy trên Linux/WSL với RISC-V GCC/binutils, QEMU 128 MiB RAM, ba hart,
GNU Make, Perl và Python 3. Phiên bản công cụ thực tế được ghi cùng minh chứng.

Mục tiêu là tránh lượt chuyển user/kernel khi chỉ đọc PID. Mapping read-only cho
user vẫn cho kernel ghi qua địa chỉ vật lý trực tiếp để khởi tạo PID. Kết quả
kiểm tra trace chứng minh đường đọc không gọi syscall; chưa đo thời gian hay
khẳng định tỷ lệ tăng tốc. Chi phí là một trang 4 KiB cho mỗi process còn giữ slot.

Điểm dễ sai nhất là quyền sở hữu trang khi `exec`: nếu để `uvmunmap(..., 1)`
giải phóng trang PID theo từng bảng, ảnh mới sẽ giữ địa chỉ vật lý đã free hoặc
đường lỗi sẽ phá mapping đang sử dụng. Tách thao tác gỡ mapping khỏi giải phóng
trang vật lý giúp dùng lại đúng trang khi exec và rollback an toàn khi lỗi.
Xóa toàn bộ trang tránh để padding chứa dữ liệu cũ của allocator trong vùng user đọc được.

Câu trả lời về syscall khác có thể tăng tốc nằm trong
[answers-pgtbl.txt](../../answers-pgtbl.txt). Bảo Trân có thể dùng phần này cùng
log thật để gộp báo cáo sau khi tích hợp đủ ba nhiệm vụ.

Chưa review `pgaccess`: tại lúc bắt đầu công việc, `origin/lab3-pgaccess` vẫn ở
`db118dd` với stub trả `-1`. Source ZIP cuối và bài test toàn Lab 3 chờ tích hợp.
