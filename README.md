# City Defense

## Thông tin về game
- **Sinh viên:** Châu Nguyễn Tố Trinh - 24021647
- **Ngôn ngữ lập trình:** C++
- **Thư viện đồ hoạ:** SDL2

## Mô tả chi tiết
City Defense là một game chiến thuật phòng thủ, nơi người chơi sẽ đóng vai trò là người bảo vệ thành phố trung tâm. Trong game này:

- **Mục tiêu:** Bảo vệ thành phố trung tâm khỏi sự tấn công của kẻ thù
- **Cơ chế chơi:**
  - Xây dựng và đặt các công trình phòng thủ như tường và tháp pháo
  - Kẻ thù sẽ xuất hiện từ 4 góc màn hình và tìm đường tấn công vào thành phố
  - Sử dụng chiến thuật thông minh để chặn đường tiến và tiêu diệt kẻ thù
- **Tính năng:**
  - Hệ thống tìm đường tự động cho kẻ thù
  - Hiển thị sức khỏe còn lại của thành phố
  - Giới hạn số lượng tường và tháp pháo có thể xây dựng
  - Hiển thị số lượng kẻ thù còn lại trong mỗi đợt
  - Độ khó tăng dần theo cấp độ:
    - Số lượng kẻ thù tăng lên
    - Tốc độ di chuyển của kẻ thù nhanh hơn
  - Giao diện đồ họa được xây dựng bằng SDL2
