# BTL_Game_3

## Yêu Cầu Phiên Bản
- **Visual Studio 17 2022**
- **Python 2.7**

## Cài Đặt Các Phụ Thuộc
1. **Tải và cài đặt Cocos2d-x 4.0**
   - Link tải: [Cocos2d-x 4.0](https://www.cocos.com/en/cocos2dx-download)
2. **Tạo dự án Cocos2d-x**
   - Mở terminal/cmd và chuyển đến thư mục chứa Cocos2d-x
   - Chạy lệnh sau để tạo dự án:
     ```sh
     cd .\cocos2d-x-4.0\cocos2d-x-4.0\
     cocos new BTL_Game_3 -l cpp -p com.btl3.epicgame
     ```
3. **Cấu hình CMake**
   - Chuyển vào thư mục `proj.win32`:
     ```sh
     cd .\BTL_Game_3\proj.win32\
     ```
   - Chạy lệnh để tạo project với Visual Studio 2022:
     ```sh
     cmake .. -G "Visual Studio 17 2022" -AWin32
     ```

## Cài Đặt Dự Án
1. **Clone dự án về máy**
   ```sh
   git clone https://github.com/Bunhiacopxki/scrolling_game.git
   ```
2. **Thay thế thư mục Classes và Resources**
   - Sao chép hai thư mục `Classes` và `Resources` từ dự án vừa clone.
   - Dán chúng vào thư mục `BTL_Game_3`, ghi đè lên thư mục cũ.

## Chạy Dự Án
1. Mở file **`BTL_GAME_3.sln`** trong thư mục `proj.win32`
2. Thêm lần lượt các file trong thư mục Classes vào Classes trong Visual Studio 2022 để chạy dự án

Chúc bạn lập trình vui vẻ! 🚀