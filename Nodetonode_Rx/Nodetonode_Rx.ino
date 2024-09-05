#include <Rak3172_Canopus.h>
#define ESP32_ADDRESS 0x06

long startTime;
bool rx_done = false;
double myFreq = 868000000;
uint16_t sf = 12, bw = 0, cr = 0, preamble = 8, txPower = 22;

void recv_cb(rui_lora_p2p_recv_t data)
{
  rx_done = true;
  if (data.BufferSize == 0)
  {
    Serial.println("Empty buffer.");
    return;
  }
  digitalWrite(LED_RECV, HIGH);

  // Kiểm tra kích thước payload để đảm bảo nó chứa dữ liệu nhiệt độ và độ ẩm
  if (data.BufferSize == 8) // Mỗi giá trị float chiếm 4 byte, tổng cộng 8 byte cho 2 giá trị
  {
    int temperature, humidity;

    // Chuyển đổi các byte nhận được thành giá trị nhiệt độ và độ ẩm
    memcpy(&temperature, &data.Buffer[0], sizeof(float));
    memcpy(&humidity, &data.Buffer[4], sizeof(float));

    // In ra giá trị nhiệt độ và độ ẩm
    Serial.printf("Received Temperature: %.2f °C\r\n", temperature);
    Serial.printf("Received Humidity: %.2f %%\r\n", humidity);

    Wire.beginTransmission(ESP32_ADDRESS);
    Wire.write(1); // Mã định danh cho nhiệt độ
    Wire.write(temperature);
    Wire.endTransmission();


    Wire.beginTransmission(ESP32_ADDRESS);
    Wire.write(2); // Mã định danh cho độ ẩm
    Wire.write(humidity);
    Wire.endTransmission();

  }
  else
  {
    Serial.println("Unexpected payload size.");
  }

  digitalWrite(LED_RECV, LOW);
}

void send_cb(void)
{
  api.lora.precv(65534); // Đặt lại chế độ Rx sau khi gửi
}

void setup()
{
  Serial.begin(115200);
  // Khởi tạo và cài đặt các thông số cho P2P mode
  init_io();
  startTime = millis();

  if (api.lora.nwm.get() != 0)
  {
    api.lora.nwm.set();
    api.system.reboot();
  }

  api.lora.pfreq.set(myFreq);
  api.lora.psf.set(sf);
  api.lora.pbw.set(bw);
  api.lora.pcr.set(cr);
  api.lora.ppl.set(preamble);
  api.lora.ptp.set(txPower);

  api.lora.registerPRecvCallback(recv_cb);
  api.lora.registerPSendCallback(send_cb);
  api.lora.precv(65534);
}

void loop()
{
  if (rx_done)
  {
    rx_done = false;
  }
  delay(500);
}
