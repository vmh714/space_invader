## 1. Tổng quan hệ thống (System Overview)

Dự án phát triển trò chơi "Space Invaders" thời gian thực trên nền tảng kít phát triển **STM32F429I-DISCO**. Hệ thống được xây dựng dựa trên sự kết hợp giữa hiệu năng phần cứng (DMA2D, LTDC, FMC) và kiến trúc phần mềm hiện đại (TouchGFX, FreeRTOS, MVP).

- **MCU:** STM32F429ZIT6 (ARM Cortex-M4, 180MHz).
    
- **Display:** LCD 2.4" QVGA (240x320) giao tiếp RGB565.
    
- **Framework:** TouchGFX (Graphics), FreeRTOS (CMSIS V2 - Multitasking).
    
- **Architecture:** Model-View-Presenter (MVP).
    

---

## 2. Cấu hình Phần cứng & Ngoại vi (Hardware Configuration)

Hệ thống tận dụng tối đa các ngoại vi (Peripherals) để giảm tải cho CPU, đảm bảo Game Loop đạt FPS cao nhất.

### 2.1. Sơ đồ chân & GPIO (Pin Mapping)

Các nút nhấn điều khiển được cấu hình ở chế độ Ngắt ngoài (EXTI) để đảm bảo độ nhạy cao nhất, không bị trễ do polling.

| **Thành phần**   | **Chức năng**      | **Chân GPIO**                      | **Chế độ (Mode)**   | **Ghi chú**      |
| ---------------- | ------------------ | ---------------------------------- | ------------------- | ---------------- |
| **Button UP**    | Di chuyển Lên      | `PC12`                             | EXTI (Falling Edge) | Internal Pull-up |
| **Button DOWN**  | Di chuyển Xuống    | `PC10`                             | EXTI (Falling Edge) | Internal Pull-up |
| **Button LEFT**  | Di chuyển Trái     | `PA15`                             | EXTI (Falling Edge) | Internal Pull-up |
| **Button RIGHT** | Di chuyển Phải     | `PC11`                             | EXTI (Falling Edge) | Internal Pull-up |
| **Buzzer**       | Âm thanh           | `PB4`                              | TIM3_CH1 (AF2)      | PWM Output       |
| **UART Log**     | Debug Console      | `USART1`: `PA9` (TX) / `PA10` (RX) | Alternate Function  | Baudrate 115200  |
| **LCD Control**  | Giao tiếp màn hình | `SPI5` pins                        | Alternate Function  | ILI9341 Driver   |

### 2.2. Hệ thống Ngắt & Input (Interrupts)

- **EXTI Handler:** Các chân nút nhấn kích hoạt ngắt `EXTI15_10_IRQHandler`.
    
- **Debounce (Chống rung):** Xử lý mềm trong `HAL_GPIO_EXTI_Callback`. Hệ thống sử dụng `osKernelGetTickCount()` để kiểm tra thời gian giữa hai lần nhấn. Chỉ chấp nhận tín hiệu nếu $\Delta t > 200ms$ (`DEBOUNCE_DELAY`).
    

### 2.3. Hệ thống Âm thanh (Audio PWM)

Sử dụng **Timer 3 (TIM3)** để điều chế độ rộng xung (PWM) tạo âm thanh.

- **Clock Source:** APB1 = 45MHz $\rightarrow$ TIM3 CLK = 90MHz.
    
- **Prescaler:** 89 $\rightarrow$ Timer chạy ở tần số 1 MHz ($1\mu s$/tick).
    
- **Cơ chế phát:**
    
    - **Win Sound (3kHz):** $ARR = \frac{1,000,000}{3000} - 1 \approx 332$.
        
    - **Lose Sound (1kHz):** $ARR = \frac{1,000,000}{1000} - 1 = 999$.
        
    - **Duty Cycle:** 50% (`Pulse = (ARR+1)/2`) để đạt âm lượng tối đa.
        

### 2.4. Đồ họa & Bộ nhớ (Graphics & Memory)

- **FMC (SDRAM):** Sử dụng SDRAM ngoài (Bank 2) để chứa Framebuffer (do RAM nội không đủ cho 2 buffer màu 240x320).
    
- **DMA2D (Chrom-ART):** Tăng tốc phần cứng cho các tác vụ vẽ, sao chép vùng nhớ và pha trộn màu, giúp CPU rảnh tay xử lý logic game.
    
- **LTDC:** Quét dữ liệu từ SDRAM đẩy ra màn hình liên tục mà không cần CPU can thiệp.
    

---

## 3. Kiến trúc Phần mềm & Tương tác (Software Architecture)

Hệ thống được chia thành các Tác vụ (Tasks) chạy song song, giao tiếp qua Hàng đợi (Message Queues).

### 3.1. Phân chia Tác vụ (FreeRTOS Tasks)

1. **GUI Task (`TouchGFX_Task`):** Mức ưu tiên Normal. Chịu trách nhiệm render đồ họa và thực thi logic game (Game Loop).
    
2. **Button Task (`StartButtonTask`):** Mức ưu tiên Normal. Nhận tín hiệu từ ngắt, giải mã nút nhấn và gửi lệnh điều khiển.
    
3. **Sound Task (`StartDefaultTask`):** Mức ưu tiên Normal. Lắng nghe yêu cầu phát âm thanh và điều khiển PWM.
    

### 3.2. Luồng xử lý tín hiệu (Input Flow)

**Mục tiêu:** Chuyển đổi tín hiệu vật lý thành hành động trong game.

- `Hardware Interrupt` $\rightarrow$ `buttonQueue` $\rightarrow$ `ButtonTask` $\rightarrow$ `controlQueue` $\rightarrow$ `Model` $\rightarrow$ `Presenter` $\rightarrow$ `View`.
    
- **Chi tiết:**
    
    1. Khi nhấn nút, ISR gửi mã pin vào `buttonQueue`.
        
    2. `ButtonTask` đọc queue, map mã pin sang ký tự lệnh (`U`, `D`, `L`, `R`) và đẩy vào `controlQueue`.
        
    3. `Model::tick()` kiểm tra `controlQueue` mỗi khung hình và cập nhật vị trí người chơi.
        

### 3.3. Luồng xử lý Âm thanh (Sound Flow)

**Mục tiêu:** Phát âm thanh bất đồng bộ (Non-blocking).

- `View` $\rightarrow$ `buzzerQueue` $\rightarrow$ `SoundTask` $\rightarrow$ `TIM3 PWM`.
    
- **Chi tiết:**
    
    1. `PlayScreenView` phát hiện va chạm, gửi ký tự `'W'` (Win) hoặc `'L'` (Lose) vào `buzzerQueue` với timeout = 0.
        
    2. `SoundTask` (đang ngủ chờ lệnh) tỉnh dậy, cấu hình PWM tần số tương ứng, delay (osDelay), rồi tắt PWM.
        

### 3.4. Lưu trữ Điểm cao (Persistence)

**Mục tiêu:** Lưu High Score vào Flash nội ngay cả khi mất điện.

- **Địa chỉ:** Sector 23 (Cuối Flash 2MB - `0x081E0000`).
    
- **Quy trình:**
    
    1. Khi Game Over, `Model` so sánh điểm hiện tại với điểm trong Flash.
        
    2. Nếu cao hơn: `Unlock Flash` $\rightarrow$ `Erase Sector 23` $\rightarrow$ `Program Word` $\rightarrow$ `Lock Flash`.
        
    3. Thư viện sử dụng: `stm32f4xx_hal_flash.h`.
        

---

## 4. Logic Game & Thuật toán (Game Mechanics)

### 4.1. Máy trạng thái kẻ thù (Enemy AI State Machine)

Hành vi của quái vật không ngẫu nhiên mà tuân theo quy tắc:

1. **Chu trình:** `MOVE_DOWN` $\rightarrow$ `MOVE_LEFT` $\rightarrow$ `MOVE_UP` $\rightarrow$ `MOVE_RIGHT`.
    
2. **Smart Wall Detection (Phát hiện tường thông minh):**
    
    - Hàm `canMove(dx, dy)` quét toàn bộ quái vật còn sống.
        
    - Nếu bước đi tiếp theo khiến bất kỳ con quái nào chạm biên màn hình:
        
        - Hủy di chuyển hướng đó.
            
        - Lập tức chuyển sang trạng thái tiếp theo trong chu trình (Ví dụ: Đang sang Trái đụng tường $\rightarrow$ Chuyển sang Lên ngay).
            
    - **Kết quả:** Quái vật di chuyển mượt mà bám theo viền màn hình, không bao giờ đi xuyên tường.
        

### 4.2. Hệ thống độ khó (Level Progression)

Mỗi khi tiêu diệt hết quái vật, hàm `respawnEnemies()` được gọi:

- **Level 1:** Quái đứng yên.
    
- **Level 2:** Quái bắt đầu di chuyển theo hình chữ nhật.
    
- **Level 3+:** Tăng tốc độ bắn (`currentEnemyFireRate` giảm) và giữ nguyên tốc độ di chuyển.
    

### 4.3. Xử lý va chạm (Collision Detection)

Sử dụng thuật toán **AABB (Axis-Aligned Bounding Box)**:

- Kiểm tra giao nhau giữa hình chữ nhật của Đạn ($x_b, y_b, w_b, h_b$) và Quái vật ($x_e, y_e, w_e, h_e$).
    
- Tối ưu hóa: Sử dụng **Object Pooling** cho đạn (mảng tĩnh `bulletStates`) thay vì cấp phát động (`new/delete`), tránh phân mảnh bộ nhớ và lỗi Heap.

## 5. Biểu đồ tuần tự (Sequence Diagram)
``` mermaid
sequenceDiagram
    participant User
    participant HW_Button as Hardware (ISR)
    participant ButtonTask
    participant GUI_Task as GUI Task (View/Model)
    participant SoundTask
    participant Flash

    Note over User, Flash: Kịch bản: Di chuyển, Tự động bắn và Xử lý va chạm

    %% 1. Luồng Di chuyển (Input Flow)
    User->>HW_Button: Nhấn nút (Ví dụ: RIGHT)
    HW_Button->>ButtonTask: Queue Put (Pin ID)
    ButtonTask->>GUI_Task: Queue Put ('R')
    GUI_Task->>GUI_Task: Model::tick() -> Move Player

    %% 2. Vòng lặp Game (Game Loop) - Auto Fire & Physics
    loop Game Loop (60Hz)
        Note right of GUI_Task: Tự động bắn (Timer)
        GUI_Task->>GUI_Task: Spawn Bullet (Auto)
        
        GUI_Task->>GUI_Task: Update Pos (Bullets, Enemies)
        
        %% Xử lý va chạm (Collision)
        GUI_Task->>GUI_Task: Check Collision (AABB)
        
        opt Bullet hits Enemy
            GUI_Task->>SoundTask: Queue Put ('W')
            SoundTask-->>SoundTask: PWM Beep (3kHz)
        end
    end

    %% 3. Luồng Game Over (Game Over Flow)
    GUI_Task->>GUI_Task: Player Hit (Collision)
    GUI_Task->>SoundTask: Queue Put ('L')
    SoundTask-->>SoundTask: PWM Beep (1kHz)
    
    GUI_Task->>GUI_Task: Transition to Game Over
    GUI_Task->>Flash: Read HighScore (Sec 23)
    
    alt Score > HighScore
        GUI_Task->>Flash: Erase & Write New Score
    end
    
    GUI_Task->>User: Display Game Over & High Score
```
