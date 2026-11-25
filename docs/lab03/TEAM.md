# Lab 3 — Quy ước branch, giao diện và bàn giao

Repository để push: https://github.com/EndlessMelody/os-lab3.
Phân công chi tiết trong [HTML](../../Lab3_PageTable_PhanCong.html).

| Thành viên | Branch | Review phần của |
|---|---|---|
| Đăng Khoa | `lab3-ugetpid` | Nhựt Duy (`pgaccess`) |
| Bảo Trân | `lab3-vmprint` | Đăng Khoa (`ugetpid`) |
| Nhựt Duy | `lab3-pgaccess` | Bảo Trân (`vmprint`) |

Ba branch xuất phát từ `lab3-start`; mở PR vào `main`. Bảo Trân tích hợp code và
báo cáo. Đăng Khoa chuẩn bị DevOps/source ZIP; Nhựt Duy chuẩn bị patch và kiểm tra
gói nộp. Hạn nội bộ: 27/11/2026 chốt code, 29/11/2026 chốt báo cáo.
Chưa tạo PR khi branch còn giống hoàn toàn `main`.

## Bắt đầu và cập nhật

```sh
git clone https://github.com/EndlessMelody/os-lab3.git
cd os-lab3
git switch lab3-vmprint  # Bảo Trân; chọn branch cá nhân từ bảng.
make -j4 kernel/kernel fs.img
```

Nếu đã clone repo cũ trên máy thành viên, commit/stash thay đổi cá nhân trước,
rồi đổi remote và lấy branch mới:

```sh
git remote rename origin lab2
git remote add origin https://github.com/EndlessMelody/os-lab3.git
git fetch origin --tags
git switch --track origin/lab3-vmprint  # Đổi theo người thực hiện.
```

Checkout leader hiện tại đã đổi remote, không chạy lại nhóm lệnh đổi remote ở đó.
Cập nhật các phần đã tích hợp bằng merge, không ghi đè file chung:

```sh
git fetch origin
git merge origin/main
# Sửa code, kiểm tra diff, chạy test phần mình và hồi quy.
git add <cac-file-da-kiem-tra>
git commit -m "Implement page table printing"
git push -u origin HEAD
```

Giữ commit từng người khi tích hợp; không force-push branch dùng chung.
Tắt QEMU tương tác trước khi chạy grader; các suite chạy lần lượt.

## Giao diện chung đã chuẩn bị

- Giữ IDs Lab 2: trace 22, sysinfo 23, ptree 24; **pgaccess 25**. Dispatch và tên
  trace đã đăng ký; stub kernel hiện trả `-1`.
- `int pgaccess(void *base, int npages, void *mask)`: output **`uint` 32 bit**,
  tối đa 32 trang, bit i tương ứng trang i. Dịch `1U << i` sau kiểm tra giới hạn.
  Thống nhất `1 <= npages <= 32`; số trang khác trả `-1`.
- Đề không quy định mọi đầu vào lỗi: nhóm chọn base căn trang, vùng quét và buffer
  nằm trong user memory hợp lệ, không overflow, PTE hợp lệ có `PTE_U`.
  Tham số lỗi trả `-1` thay vì panic. Chủ phần 4.3 bổ sung test chính sách này.
- `USYSCALL = TRAPFRAME - PGSIZE`, `struct usyscall { int pid; }` nằm trong
  `kernel/memlayout.h`; helper `ugetpid()` đã đọc giao diện này. **Chưa có mapping**.
  Đăng Khoa thêm page vào `struct proc`, xử lý cấp phát, quyền, lỗi và unmap/free.
- `vmprint(pagetable_t)` chưa khai báo/triển khai. Bảo Trân thêm vào `vm.c`,
  `defs.h`, gọi trong `exec.c` khi PID = 1 sau cập nhật `p->pagetable`.
- `PTE_A` chưa định nghĩa; Nhựt Duy thêm bit 6 và xử lý accessed bits.
  Lưu ý hiệu lực TLB sau đổi bit và `copyout` lỗi.
- `LAB=pgtbl`; **hai user stack pages** như bản kế thừa. Output `vmprint` có thể
  thêm leaf so với hình mẫu; giải thích theo source thực tế.

File dễ trùng sửa: `kernel/proc.c`, `proc.h`, `vm.c`, `defs.h`, `riscv.h`,
`sysproc.c`, `syscall.c`, `user/user.h`, `Makefile`. Chỉ merge phần cần thiết.
Không sửa tay `user/usys.S`: file được sinh từ `user/usys.pl`.

## Test và review

| Phần | Lệnh grader riêng | Lệnh trong QEMU |
|---|---|---|
| 4.1 | `./grade-lab-pgtbl ugetpid` | `pgtbltest ugetpid` |
| 4.2 | `./grade-lab-pgtbl vmprint` | Kiểm tra tree lúc boot |
| 4.3 | `./grade-lab-pgtbl pgaccess` | `pgtbltest pgaccess` |

Test nội bộ theo PDF, không thay bộ chấm của môn. Bổ sung test quyền trang,
fork/exec/exit, biên và tham số lỗi theo phần phụ trách. Log thật và commit hash
đặt trong `docs/lab03/evidence/<branch>/`; chỉ commit minh chứng đã chọn.
Tác giả tự test chưa thay review của người được phân công.

CI mặc định kiểm tra clean build + hồi quy Lab 1–2. Bộ khung chưa làm ba phần
kernel nên job Page Table chỉ chạy khi bấm **Run workflow** trên GitHub Actions.
Trước code freeze, Bảo Trân chạy workflow trên `main`, yêu cầu cả `regression` và
`pgtbl` đạt, đồng thời chạy `pgtbltest` trên bản tích hợp. Khi đủ ba tính năng,
bỏ điều kiện `if` của job `pgtbl` để bật kiểm tra tự động cho push/PR.

Đây là quy ước nhóm trong tài liệu. Chưa cấu hình branch protection, quyền cộng tác
viên hay tự gán GitHub username vì chưa xác minh tài khoản tương ứng.

Xem [BASELINE.md](BASELINE.md) trước tạo patch và [Task.md](../../Task.md) trước nộp.
