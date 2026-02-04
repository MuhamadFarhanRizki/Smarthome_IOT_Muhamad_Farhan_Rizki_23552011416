Smart Home IoT ESP32

📌 Gambaran Sistem

Proyek ini merupakan Sistem Smart Home berbasis IoT yang menggunakan ESP32 sebagai mikrokontroler utama. Sistem memungkinkan pengguna mengontrol lampu rumah melalui Web Dashboard (desktop & mobile) serta komunikasi MQTT secara real-time.
Sistem dirancang non-blocking dan multitasking dengan memanfaatkan FreeRTOS, interrupt, dan PWM untuk kontrol LED.

🎥 Demo Video

📺 Link YouTube Demo Sistem:
https://youtu.be/UvEgn4nhbfk?si=5wGZxj86LK0gALg5

⚙️ Alur Singkat Sistem

ESP32 terhubung ke WiFi atau masuk ke mode konfigurasi (ESP32-Config/AP) jika WiFi tidak tersedia.

Pengguna mengontrol lampu melalui Web Dashboard.

Perintah dikirim menggunakan MQTT publish–subscribe.

ESP32 menerima payload ON / OFF, memprosesnya, lalu:

Mengatur status lampu

Mengontrol LED menggunakan PWM

Status lampu tersinkron secara real-time antara dashboard, ESP32, dan broker MQTT.

🖥️ Manajemen Dashboard (Desktop)

Web dashboard versi desktop digunakan untuk:

Menyalakan dan mematikan lampu

Monitoring status lampu secara real-time

Mengirim perintah MQTT ke ESP32

<img width="1784" height="979" alt="Screenshot 2026-02-04 184438" src="https://github.com/user-attachments/assets/751b0256-8700-4414-adfc-079dc8a0f24b" />
<img width="1775" height="981" alt="Screenshot 2026-02-04 184503" src="https://github.com/user-attachments/assets/59393a1f-328b-4481-aeed-059d368458e4" />

📱 Manajemen Dashboard (Mobile)
Dashboard dapat diakses melalui browser smartphone tanpa aplikasi tambahan.
Fungsinya sama dengan versi desktop, sehingga sistem tetap fleksibel dan mudah digunakan.

![WhatsApp Image 2026-02-04 at 18 47 58](https://github.com/user-attachments/assets/25729ffe-44f2-4802-ae42-916880793acb)
![WhatsApp Image 2026-02-04 at 18 47 58](https://github.com/user-attachments/assets/71f26fb5-3ad0-4006-b614-17d05c5d36d2)

🌐 MQTT Broker – public.cloud.shiftr.io

public.cloud.shiftr.io digunakan sebagai broker MQTT publik yang berfungsi sebagai:

Media komunikasi publish–subscribe

Penghubung antara ESP32 dan dashboard

Sarana pengujian IoT tanpa server lokal

Broker ini mempermudah implementasi karena tidak memerlukan setup broker sendiri.

<img width="1917" height="953" alt="Screenshot 2026-02-04 185054" src="https://github.com/user-attachments/assets/7a2962b3-eba8-4094-aba6-6feddc867829" />

🧪 HiveMQ
HiveMQ Web Client digunakan sebagai:

Alat monitoring dan pengujian MQTT

Subscriber dan publisher manual

Validasi bahwa topic dan payload bekerja dengan benar

HiveMQ membantu memastikan sistem MQTT berjalan sesuai konsep IoT.

<img width="1770" height="989" alt="Screenshot 2026-02-04 185243" src="https://github.com/user-attachments/assets/410c2cae-2391-4632-b98d-693c4372b8fa" />

