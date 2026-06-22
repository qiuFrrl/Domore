# ROBODESK Architecture

Project ini dibuat modular supaya `src/main.cpp` tetap kecil. Semua fitur utama dipisah menjadi manager kecil yang bisa dikembangkan tanpa membuat satu file besar.

## Pin Hardware

- TTP223 `OUT` -> GPIO4. Mode input aktif HIGH, jadi `BUTTON_ACTIVE_LOW = false`.
- OLED SH1106 I2C SDA -> GPIO8.
- OLED SH1106 I2C SCL -> GPIO9.
- Battery monitor: `OUT+`/battery positive -> resistor 220k -> GPIO3/ADC -> resistor 220k -> `OUT-`/GND. Rasio pembagi 2:1, jadi baterai 4.2V terbaca sekitar 2.1V di ADC. Kalau kabel ADC dipindah, ubah `ROBODESK_BATTERY_ADC_PIN`.

## Struktur Folder

- `src/main.cpp`: entry point Arduino. Hanya start Serial, panggil `RobodeskApp::begin()`, lalu `RobodeskApp::update()`.
- `include/config/AppConfig.h`: semua konfigurasi pin, WiFi, interval tombol, delay random animasi, NTP, lokasi fallback, dan weather refresh.
- `include/app` dan `src/app`: alur aplikasi utama, routing screen, startup intro, menu, birthday, dan koneksi antar manager.
- `include/animation` dan `src/animation`: daftar animasi yang boleh dipakai dan logic pemilihan animasi Domore.
- `include/display` dan `src/display`: OLED, player bitmap, katalog asset animasi, gambar weather icon, dan birthday scene.
- `include/input` dan `src/input`: pembacaan TTP223, debounce, tap sequence, reset tap setelah 5 detik idle, hold pendek, dan hold 5 detik untuk menu.
- `include/menu` dan `src/menu`: daftar menu dan selected index.
- `include/system` dan `src/system`: WiFi, Firebase (untuk sinkronisasi kredensial Wifi), NTP time, weather API, lokasi, dan monitor baterai.
- `include/assets/animation`: file bitmap animasi hasil export. Jangan diedit manual.

## File Utama

- `include/app/RobodeskApp.h`: deklarasi class aplikasi utama dan semua manager yang dipakai.
- `src/app/RobodeskApp.cpp`: mengatur boot sequence `Intro.h` -> tahan frame akhir 2 detik -> `Excited` sekali, cek birthday sekali setelah boot intro selesai dengan wait NTP maksimal 15 detik, update manager, pindah screen, input tombol, dan render display.
- `src/main.cpp`: loop Arduino yang bersih dan pendek.

## Animation System

- `include/display/AnimationTypes.h`: struktur `AnimationClip`, metadata ringan untuk bitmap frames.
- `include/display/AnimationCatalog.h`: daftar `AnimationId` yang boleh dipakai aplikasi.
- `src/display/AnimationCatalog.cpp`: satu-satunya tempat yang include file dari `include/assets/animation`. Kalau mau menambah animasi baru ke firmware, include header animasinya di sini lalu tambah entry clip.
- `include/display/AnimationPlayer.h`: player non-blocking untuk memainkan frame bitmap dengan `millis()`.
- `src/display/AnimationPlayer.cpp`: logic playback frame, loop count, stop, finished, dan draw ke U8g2.
- `include/animation/AnimationSets.h`: deklarasi daftar/rule animasi yang mudah diganti.
- `src/animation/AnimationSets.cpp`: tempat edit animasi default, animasi tap 1/3/5/7/9, hold, AFK, wake, birthday final, rule jam, dan rule cuaca.
- `include/animation/DomoreAnimationManager.h`: manager animasi Domore.
- `src/animation/DomoreAnimationManager.cpp`: memilih animasi default secara random dari pool, delay random 2.5-8 detik, menjalankan `Blank` sebagai animasi penunggu setelah setiap one-shot selesai, menambah animasi dari rule jam/cuaca, dan mode AFK 10 menit yang loop sampai tombol disentuh.

## Display

- `include/display/DisplayManager.h`: deklarasi screen `Boot`, `Domore`, `Menu`, `Time`, `Weather`, dan `Birthday`.
- `src/display/DisplayManager.cpp`: setup OLED SH1106, render setiap screen, boot animation dari `Intro.h`, menu, jam, weather icon, dan birthday scene.
- `include/display/BirthdayScene.h`: deklarasi scene ulang tahun khusus 7 Oktober.
- `src/display/BirthdayScene.cpp`: gambar kue, kembang api, bunga, teks birthday. Scene ini dipanggil sekali per boot setelah intro selesai jika tanggal sudah valid dan hari ini adalah 7 Oktober.

## Input dan Menu

- `include/input/InputManager.h`: event tombol `TapSequence`, `Hold`, dan `MenuHold`.
- `src/input/InputManager.cpp`: debounce TTP223, hitung tap berulang, emit tap sequence setelah gap pendek, reset counter tap setelah 5 detik tidak disentuh, hold pendek saat tombol dilepas, dan hold 5 detik untuk membuka menu.
- `include/menu/MenuManager.h`: deklarasi item menu dan action.
- `src/menu/MenuManager.cpp`: menu sementara: `Domore`, `Time`, `Weather`, `WIFI`.

## System

- `include/system/WifiTypes.h`: struct SSID dan password menggunakan `char[]` fixed-size untuk mencegah heap fragmentation pada ESP32.
- `include/system/WifiManager.h`: state machine WiFi non-blocking, dengan penampung kredensial gabungan antara `AppConfig.h` dan NVS `Preferences`.
- `src/system/WifiManager.cpp`: koneksi WiFi fallback dan dinamis, koneksi *retry* dengan interval tegas 10 detik agar tidak membanjiri loop, serta fungsi `forceConnect()` untuk dipanggil dari menu WIFI.
- `include/system/FirebaseManager.h` & `src/system/FirebaseManager.cpp`: HTTPS ke Firebase RTDB dengan `client.setInsecure()`. Mengambil JSON daftar Wifi dari (`/robot/wifi.json`). Sebelum menyimpannya ke `WifiManager` secara dinamis, sistem menghapus data Wifi dinamis lama di RAM dan NVS (`clearDynamicCredentials()`) agar sinkron.
- `include/system/TimeManager.h`: snapshot jam, tanggal, dan status valid.
- `src/system/TimeManager.cpp`: setup NTP dan update waktu real-time.
- `include/system/WeatherManager.h`: data cuaca, suhu, humidity, weather code, dan summary.
- `src/system/WeatherManager.cpp`: ambil weather gratis dari Open-Meteo memakai HTTPS, `WiFiClientSecure` dengan `client.setInsecure()`, interval request 5 menit, debug error HTTP ke Serial, lalu parse JSON Open-Meteo dengan ArduinoJson.
- `include/system/LocationTypes.h`: struct koordinat.
- `include/system/LocationManager.h`: deklarasi lokasi fallback dan lokasi dari phone.
- `src/system/LocationManager.cpp`: fallback Palembang dan override dari phone jika nanti dihubungkan.
- `include/system/BatteryManager.h` dan `src/system/BatteryManager.cpp`: baca tegangan baterai lewat pembagi 220k+220k ke ADC, averaging beberapa sampel, status low di 3.50V dan critical di 3.30V.

## Menambah Animasi

1. Jangan edit file di `include/assets/animation`.
2. Tambahkan include asset baru di `src/display/AnimationCatalog.cpp`.
3. Tambahkan enum baru di `include/display/AnimationCatalog.h`.
4. Tambahkan entry clip di array `CLIPS`.
5. Pakai enum itu di `src/animation/AnimationSets.cpp`.
6. Pastikan urutan enum `AnimationId` sama dengan urutan array `CLIPS`, karena `AnimationCatalog::get()` mengambil clip berdasarkan index enum (terdapat `static_assert` untuk validasi otomatis saat compile).

## Perilaku Domore

- Boot: OLED tidak menampilkan teks `ROBODESK`/`Starting...`; screen `Boot` langsung menggambar animasi `Intro.h` lewat `AnimationPlayer`, tahan frame terakhir 2 detik, menjalankan `Excited` sekali, lalu masuk fase birthday check sebelum Domore.
- Birthday check: setelah intro selesai, app loop `Blank` sambil menunggu waktu valid dari NTP sampai 15 detik. Kalau tanggal valid dan 7 Oktober, birthday scene jalan sekali per boot. Kalau tanggal bukan HBD atau waktu tetap tidak valid sampai timeout, app lanjut Domore.
- Domore normal: pilih random dari 5 animasi default, jalankan animasi sekali, lalu loop `Blank` selama menunggu delay random 2.5-8 detik selesai.
- `Blank`: animasi filler global. Setelah setiap one-shot selesai, `Blank` diputar agar layar tidak freeze di frame akhir animasi sebelumnya. Untuk random/home, `Blank` berjalan sampai delay random selesai. Untuk tap, `Blank` berjalan sampai ada input berikutnya.
- Tap: counter tap naik selama tap masih berdekatan, event diproses setelah gap pendek, lalu counter reset kalau tidak disentuh selama 5 detik.
- Rule jam: tiga slot tersedia di `AnimationSets.cpp`; aktifkan `enabled = true` dan atur jamnya.
- Rule cuaca/suhu: tiga slot tersedia di `AnimationSets.cpp`; aktifkan `enabled = true` dan atur range suhu/weather code.
- Menu WIFI: saat masuk ke menu WIFI, jika tidak terkoneksi, alat mencoba menghubungkan (maksimal 10 detik/kredensial). Jika terkoneksi, menampilkan animasi "Connecting..." selama minimal 5 detik sambil menarik kredensial baru dari Firebase. Kredensial dinamis lama akan dihapus bersih (dari RAM & NVS), lalu kredensial baru ditambahkan dan disimpan permanen, sehingga tidak memicu memori penuh akibat data lama.
- AFK: jika tombol tidak disentuh 10 menit, jalankan satu animasi loop terus sampai tombol disentuh.
- Saat AFK: tap memanggil animasi wake tap, hold memanggil animasi wake hold.
- Hold dari layar Domore: tahan kurang dari 5 detik untuk animasi biasa, tahan 5 detik atau lebih untuk membuka menu.
- Battery monitor: firmware membaca tegangan baterai dari pembagi 220k+220k ke GPIO3 (pin ADC bersih, menghindari pin strapping dan pin TX UART). Saat tegangan <= 3.50V layar Domore menampilkan `BAT`, dan saat <= 3.30V menampilkan tanda `!`. Proteksi TP4056 tetap menjadi cutoff hardware terakhir.
- 7 Oktober: setelah boot intro selesai, app cek tanggal sekali setelah waktu valid atau timeout birthday check. Kalau tanggal hari ini 7 Oktober dan belum pernah dimainkan di boot ini, tampilkan birthday scene, lalu lanjut satu animasi final dari file animation. Jika robot dimatikan lalu dinyalakan lagi pada hari yang sama, birthday scene akan muncul lagi sekali setelah intro.

## Algoritma Membuat Robot dari 0

1. Buat project PlatformIO ESP32-C3 dengan Arduino framework.
2. Buat `include/config/AppConfig.h` untuk semua konstanta awal: pin OLED, pin TTP223, WiFi credential, interval tombol, interval weather, NTP, lokasi fallback, delay animasi random, durasi tahan frame akhir intro, timeout birthday check, dan tanggal birthday.
3. Buat `src/main.cpp` sekecil mungkin: start `Serial`, panggil `RobodeskApp::begin()`, lalu panggil `RobodeskApp::update()` di `loop()`.
4. Buat tipe dasar asset animasi di `include/display/AnimationTypes.h`, minimal berisi nama clip, ukuran bitmap, jumlah frame, pointer frame, dan delay frame.
5. Masukkan semua bitmap animasi hasil export ke `include/assets/animation`, lalu buat aggregator asset seperti `allAnim.h`.
6. Buat `include/display/AnimationCatalog.h` dan `src/display/AnimationCatalog.cpp`; daftar semua `AnimationId` harus seurut dengan array `CLIPS`.
7. Buat `include/display/AnimationPlayer.h` dan `src/display/AnimationPlayer.cpp`; player harus non-blocking pakai `millis()`, bisa play sekali, loop terus, stop, dan draw frame ke U8g2.
8. Buat `include/display/DisplayManager.h` dan `src/display/DisplayManager.cpp`; setup OLED, render screen `Boot`, `Domore`, `Menu`, `Time`, `Weather`, dan `Birthday`.
9. Buat `include/input/InputManager.h` dan `src/input/InputManager.cpp`; baca TTP223 dengan debounce, bedakan tap sequence, hold pendek, dan hold 5 detik untuk menu.
10. Buat `include/menu/MenuManager.h` dan `src/menu/MenuManager.cpp`; simpan daftar item menu dan action seperti `Domore`, `Time`, `Weather`, dan `WIFI`.
11. Buat manager system satu per satu: `WifiManager`, `FirebaseManager`, `TimeManager`, `LocationManager`, dan `WeatherManager`.
12. Untuk weather, mulai dari WiFi connect dulu, lalu request Open-Meteo dengan `WiFiClientSecure`, `client.setInsecure()`, `HTTPClient`, dan parse ArduinoJson.
13. Buat `include/animation/AnimationSets.h` dan `src/animation/AnimationSets.cpp`; isi pool animasi default, mapping tap count, animasi hold, animasi AFK, birthday, rule jam, dan rule cuaca.
14. Buat `include/animation/DomoreAnimationManager.h` dan `src/animation/DomoreAnimationManager.cpp`; urus random idle, delay random, filler `Blank`, tap one-shot, hold one-shot, AFK loop, wake animation, dan pemilihan animasi dari rule jam/cuaca.
15. Buat `include/display/BirthdayScene.h` dan `src/display/BirthdayScene.cpp`; scene ini berdiri sendiri dan selesai berdasarkan durasi.
16. Buat `include/app/RobodeskApp.h` dan `src/app/RobodeskApp.cpp`; ini orkestrator utama yang memanggil semua manager, mengatur routing screen, input, menu action, boot sequence `Intro.h` -> hold 2 detik -> `Excited` -> birthday check dengan wait NTP maksimal 15 detik, Domore, weather, time, dan render.
17. Build dengan `pio run -j1`; perbaiki compile error dari header include, enum animasi, dan dependency library.
18. Flash ke ESP32-C3, buka Serial Monitor, validasi urutan: boot animation `Intro.h`, frame akhir intro tahan 2 detik, `Excited` sekali, birthday check menunggu NTP maksimal 15 detik dan menjalankan HBD sekali jika tanggal 7 Oktober, WiFi connect, time sync, weather update, menu tampil, tap animation, Blank filler, hold, menu hold, dan AFK.
19. Setelah setiap perubahan kode, update `ARCHITECTURE.md` supaya dokumentasi tetap sinkron dengan behavior firmware terbaru.
