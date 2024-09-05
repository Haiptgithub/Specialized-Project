#include <Rak3172_Canopus.h>
#include <Arduino.h>
#include <Wire.h>
#include <ArtronShop_SHT3x.h>

long startTime;
double myFreq = 868000000;
uint16_t sf = 12, bw = 0, cr = 0, preamble = 8, txPower = 22;

ArtronShop_SHT3x sht3x(0x44, &Wire);

void send_cb(void)
{
  api.lora.precv(65534); // Đặt lại chế độ Rx sau khi gửi
}

void setup()
{
  Serial.begin(115200);
  init_io();
  enable_Vss3();
  delay(100);
  Wire.begin();

  while (!sht3x.begin()) {
    Serial.println("SHT3x not found !");
    delay(1000);
  }
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

  api.lora.registerPSendCallback(send_cb);
  api.lora.precv(65534);
}

void loop()
{
  if (sht3x.measure()) {
    int temperature = sht3x.temperature();
    int humidity = sht3x.humidity();

    Serial.print("Temperature: ");
    Serial.print(temperature, 1);
    Serial.print(" *C\tHumidity: ");
    Serial.print(humidity, 1);
    Serial.print(" %RH");
    Serial.println();

    // Tạo payload chứa giá trị nhiệt độ và độ ẩm
    uint8_t payload[8];
    memcpy(&payload[0], &temperature, sizeof(int));
    memcpy(&payload[4], &humidity, sizeof(int));

    bool send_result = false;
    while (!send_result)
    {
      send_result = api.lora.psend(sizeof(payload), payload);
      if (!send_result)
      {
        api.lora.precv(0); // Đặt lại chế độ Rx nếu gửi thất bại
        delay(1000);
      }
    }
  }
  delay(1000); // Giảm tần suất gửi dữ liệu
}
