Đồ án 2 - Hệ điều hành - HCMUS
1712856 - Huỳnh Văn Tú
1712210 - Nguyễn Xuân Vỹ 

I/ LINUX KERNEL MODULE 
// Viết một module dùng để tạo số ngẫu nhiên. Module này sẽ tạo một character device để cho phép các tiền trình ở user space có thể open và read các số ngẫu nhiên. 

0. Di chuyển đến thư mục LinuxKernelModule.
1. Cài đặt 
make
sudo insmod my_mod_rand.ko

2. Kiểm tra
make test
sudo ./test.o

3. Gỡ cài đặt
sudo rmmod my_mod_rand.ko
make clean

II/ SYSTEM CALL HOOKING 
// syscall open =>ghi vào dmesg tên tiến trình mở file và tên file được mở.
// syscall write => ghi vào dmesg tên tiến trình, tên file bị ghi và số byte được ghi.

0. Di chuyển đến thư mục SystemCallHook 
1. Test hook trước khi tải vào kernel
make 
1. Mở terminal thứ 2 để quan sát kernel messages 
sudo dmesg -C
dmesg -wH 

2. Chạy trên terminal thứ 1. Quan sát terminal thứ 2 để thấy được kết quả.
sudo insmod hook.ko 
sudo rmmod hook.ko 



