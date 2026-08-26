# io7 IoT Thermostat

An IO7F32 (Arduino C++) thermostat device: it reads a DHT22 on GPIO17, takes the set temperature from a rotary encoder wired to GPIO44 (A) and GPIO43 (B), shows both on the TFT, and publishes/receives over the io7 MQTT topic spec. The sample targets the LilyGO T-Display-S3 with its built-in TFT — build and upload with PlatformIO (`git clone`, then Build/Upload), and configure Wi-Fi and io7 credentials through the ConfigPortal.

See Chapter 12, Section 12.4.2 of the book.

<img width="498" alt="Screenshot 2025-05-19 at 11 11 37 PM" src="https://github.com/user-attachments/assets/7c834287-78b9-455d-b4b3-c977a44162ab" />
