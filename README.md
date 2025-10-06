# ⚡ ESP32 RFID Attendance System (Telegram + Google Form + LCD)

A Smart IoT Attendance System using **ESP32**, **RFID (MFRC522)**, **16x2 I2C LCD**, **Telegram Bot**, and **Google Form** integration.  
It automatically records attendance, sends instant Telegram notifications, and logs data to Google Sheets — all in **real-time**.

---

### 🎥 Demo Video  
📺 **[Watch on YouTube](https://www.youtube.com/)** *(Add your YouTube link here)*  

---

### ✨ Features  
✅ Real-time attendance logging via RFID  
✅ Automatic Google Form submission (Google Sheets logging)  
✅ Instant Telegram notification to parent/admin  
✅ Custom LCD WiFi status icon  
✅ Auto WiFi reconnect system  
✅ Works even in offline mode (LCD + buzzer feedback)  
✅ Anti-duplicate punch (within 5 seconds)  
✅ Smart LCD update with flicker-free display  

---

### 🧰 Hardware Required  

| Component | Quantity | Description |
|------------|-----------|-------------|
| **ESP32 Dev Board** | 1 | Main Microcontroller |
| **MFRC522 RFID Reader** | 1 | To scan RFID cards/tags |
| **RFID Card/Tag** | Multiple | For user identification |
| **16x2 I2C LCD** | 1 | Status display |
| **Active Buzzer** | 1 | Audio feedback |
| **LED (Red & Green)** | 2 | Visual indication |
| **Jumper Wires** | - | Connection setup |
| **Breadboard (Optional)** | 1 | For prototyping |
| **Power Supply (5V)** | 1 | To power the ESP32 & sensors |

---

### ⚡ Circuit Diagram  

| ESP32 Pin | Connection |
|------------|-------------|
| **3.3V / 5V** | Power to RFID + LCD |
| **GND** | Common Ground |
| **GPIO 27 (MOSI)** | RFID MOSI |
| **GPIO 23 (MISO)** | RFID MISO |
| **GPIO 19 (SCK)** | RFID SCK |
| **GPIO 18 (RST)** | RFID RST |
| **GPIO 21 (SDA)** | LCD SDA |
| **GPIO 22 (SCL)** | LCD SCL |
| **GPIO 14** | RED LED |
| **GPIO 12** | BUZZER |
| **GPIO 13** | GREEN LED |

---

### 🔌 Power Configuration  
- **RFID & LCD** → 3.3V / 5V (as supported)  
- **Common GND** for all components  

---

### 💻 Software / Libraries  

#### Required Tools  
- Arduino IDE  
- ESP32 Board Package  
- Stable Internet Connection  

#### Libraries Used  
```cpp
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "time.h"
```

---

### 🌐 How It Works  

1. The system connects to your WiFi network.  
2. When an RFID card is scanned → ESP32 reads the UID.  
3. Data (Name, UID, Time, Date, Status) is displayed on LCD.  
4. The same data is sent to **Google Sheets** via Google Form.  
5. A **Telegram notification** is instantly sent to the admin/parent.  
6. If WiFi fails, system still works in offline mode and stores data locally (LCD + buzzer feedback).  

---

### 🧩 Project Workflow  
RFID Scan → ESP32 → LCD Display → Google Form Submission → Telegram Notification  

---

### 🧠 Future Improvements  
🚀 Add SD Card backup logging  
📶 MQTT or Firebase integration  
📱 Android/iOS app for admin view  
💬 Voice assistant or dashboard control  

🧩 Get Telegram Chat ID

To find your Telegram Chat ID, open this link in your browser (replace <YOUR_BOT_TOKEN> with your actual token):

🔗 https://api.telegram.org/bot<YOUR_BOT_TOKEN>/getUpdates

Then send any message to your bot on Telegram — your chat ID will appear in the JSON response under "chat":{"id": ... }.

---

### 📜 License  
This project is licensed under the **MIT License** — you’re free to use, modify, and distribute with proper credit.

---

### ✨ Author  
👨‍💻 **ElectriTrend**  
🎥 **[YouTube Channel](https://www.youtube.com/)**  
🌍 **Website:** *coming soon*  
