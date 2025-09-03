#include <DHT11.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <esp_sleep.h>

int no = 3;
unsigned long currentMills,Time,TimeLED,TimeNode;
DHT11 dht11(22);
int LED = 16;
int nfLED;
int Soil_moisture;
int res;
int temperature;
int humidity;
const char *mqtt_broker = "broker.emqx.io";
const int mqtt_Port = 1883;
const int sleepDuration = 180 * 1000000;

const char *topic_temp = "HiGrow_st";
const char *topic_hum = "HiGrow_sh";
const char *topic_moisture = "HiGrow_sm";  

WiFiClient espClient;
PubSubClient client(espClient);

#define WiFi_SSID "TP-Link_DAE7"
#define WiFi_PASSWORD "50696382"

void setup() {
  Serial.begin(115200);
  pinMode(LED,OUTPUT);
  WiFi.begin(WiFi_SSID,WiFi_PASSWORD);//ทำการเซ็ตค่า WiFi
  client.setServer(mqtt_broker, mqtt_Port);
}

float predictTree1(float sensor_value) {
  if (sensor_value <= 1524.76) {
    if (sensor_value <= 1446.0) {
      if (sensor_value <= 1352.34) {
        return 81.5006;
      } else {
        return 74.5023;
      }
    } else {
      if (sensor_value <= 1509.26) {
        return 49.2241;
      } else {
        return 40.0;
      }
    }
  } else {
    if (sensor_value <= 2783.14) {
      if (sensor_value <= 2055.5) {
        return 34.9195;
      } else {
        return 24.5245;
      }
    } else {
      if (sensor_value <= 2949.06) {
        return 11.3068;
      } else {
        return 1.5033;
      }
    }
  }
}

float predictTree2(float sensor_value) {
  if (sensor_value <= 1529.26) {
    if (sensor_value <= 1437.5) {
      if (sensor_value <= 1352.34) {
        return 81.7073;
      } else {
        return 76.0693;
      }
    } else {
      if (sensor_value <= 1508.76) {
        return 50.9582;
      } else {
        return 40.0;
      }
    }
  } else {
    if (sensor_value <= 2783.14) {
      if (sensor_value <= 2055.5) {

        return 34.6555;
      } else {
        return 24.4377;
      }
    } else {
      if (sensor_value <= 2949.06) {
        return 9.5853;
      } else {
        return 1.5797;
      }
    }
  }
}

float predictTree3(float sensor_value) {
  if (sensor_value <= 1525.26) {
    if (sensor_value <= 1410.5) {
      if (sensor_value <= 1392.32) {
        return 81.2642;
      } else {
        return 76.8768;
      }
    } else {
      if (sensor_value <= 1447.5) {
        return 60.6818;
      } else {
        return 49.7;
      }
    }
  } else {
    if (sensor_value <= 2784.14) {
      if (sensor_value <= 2055.5) {
        return 34.8724;
      } else {
        return 24.5015;
      }
    } else {
      if (sensor_value <= 2949.06) {
        return 9.4156;
      } else {
        return 1.5869;
      }
    }
  }
}

// Random Forest Prediction
float randomForestPredict(float sensor_value) {
  float tree1 = predictTree1(sensor_value);
  float tree2 = predictTree2(sensor_value);
  float tree3 = predictTree3(sensor_value);
  return (tree1 + tree2 + tree3) / 3.0;
}

// SVR Prediction
float svrPredict(float sensor_value) {
  if (sensor_value >= 1000 && sensor_value <= 1500) {
    return -0.0758 * sensor_value + 157.4269;
  } else if (sensor_value >= 1501 && sensor_value <= 2500) {
    return -0.0219 * sensor_value + 75.4338;
  } else if (sensor_value >= 2501 && sensor_value <= 3500) {
    return -0.0322 * sensor_value + 99.3774;
  } else {
    return 0.0; // Out of range
  }
}

// Combined Prediction
float combinedPredict(float sensor_value) {
  float rfPrediction = randomForestPredict(sensor_value);
  float svrPrediction = svrPredict(sensor_value);
  return (rfPrediction + svrPrediction) / 2.0;
}

void checkWiFi(){
while(WiFi.status() != WL_CONNECTED)//ตรวจสอบ WiFi
  {
delay(1000);//ทำการดีเรย์ 1 วินาที
  Serial.print(".");//ทำการแสดง . ใน Serial moniter เมื่อเชื่อมต่อ WiFi ไม่ได้
  
  res=res+1;//ทำการบวกค่า 1 ทุกๆ 1 วินาทีเพื่อใช้ในการเข้าเงื่อนไขรีเซตบอร์ด
  Serial.println(res);//แสดงค่าวินาทีในการรีเซตบอร์ดเมื่อค่าเป็น 10 จะทำการรีเซตบอร์ด
  //เมื่อไม่สามารถเชื่อมต่อ WiFi ได้จะทำการรีเซ็ตบอร์ดเมื่อเชื่อมต่อไม่ได้เป็นเวลา 10 วิ
  if(res>=10){//เงื่อนไขในการ รีเซตบอรืด
    ESP.restart(); //ทำการรีเซ็ตบอร์ด
    }
  res=0;//เมื่อสามารถเชื่อมต่อ WiFi ได้ จะทำการเปลี่ยนให้เงื่อนไขในการรีเซตบอร์ดมีค่าเป็น 0
  Serial.println("WiFi connected");//แสดงข้อความบน Serial moniter เมื่อสามารถเชื่อมต่อ WiFi ได้
  Serial.print("IP address: ");//แสดงข้อความบน Serial moniter
  Serial.println(WiFi.localIP());//แสดง IP ของ WiFi บน Serial moniter
  /*ความรู้เพิ่มเติม
    ถึงแม้จะเขียนฟังชั้นในการเชื่อมต่อ WiFi ได้ถูกต้อง และทำการเปิด WiFi จากอุปกรณ์ต่างๆแล้ว แต่ในบางครั้งบอร์ดไมโครคอนโทรเลอร์ก็ไม่สามารถเชื่อมต่อกับ WiFi ได้
    จึงจำเป็นต้องทำการรีเซ็ตบอร์ดเพื่อที่จะให้ตัวบอร์ดสามารถเชื่อมต่อ WiFi ได้ การใช้ฟังชั่นในการรีเซ็ตบอร์ดจึงจะทำให้ไม่จำเป็นต้องกดรีเซ็ตที่บอร์ดด้วยตนเอง*/
  }}

void check_mqtt() {
  //สร้างเงื่อนไขเมื่อเชื่อมต่อ mqtt ไม่ได้
  if (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(("npru_test_mqtt_HiGrow"+String(no)).c_str())) //ทำการเชื่อมต่อ mqtt โดยที่ในวงเล็บคือชื่อที่สามารถตั้งได้ตามต้องการโดยที่ชื่อที่ตั้งไม่ควรซ้ำกันในจำนวนมาก
    {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state()); //เมื่อไม่สามารถเชื่อมต่อ mqtt ได้จะทำการแสดง state เพื่อบงบอกสาเหตุที่ไม่สามารถเชื่อมต่อ mqtt ได้
      delay(5000);
   }}}

void Google_Sheet(){
  HTTPClient http;//สร้างตัวแปรในการส่งข้อมูลไปยัง Google Sheet
        //ทำการสร้างตัวแปรในการเก็บ url ของ App script
        String url = "https://script.google.com/macros/s/AKfycbxrU488bGlw8tScNlAdd7VVgOguxskGMteLnq7_N7b1UQF6NcAiQj1I1y3oHyr-wODqBA/exec?hum"+String(no)+"="+String(humidity)+"&temp"+String(no)+"="+String(temperature)+"&soil"+String(no)+"="+String(Soil_moisture);
        //ตัวอย่าง
        //"url?data1=1&data2=2";
        Serial.println("Making a request");//ทำการแสดงข้อความเมื่อจะทำการส่งข้อมูลไปยัง google sheet
        http.begin(url.c_str()); //ทำการส่งข้อมูลไปยัง google sheet  
        http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);//ทำการรับสเตต้สของข้อมูลที่ทำการติดตาม
        int httpCode = http.GET();//สร้างตัวแปรเก็บข้อความที่ถูกส่งกลับมา
        String payload;//สร้างตัวแปรเก็บข้อความที่ถูกส่งกลับมา
        if (httpCode > 0) { //ตรวจสอบสเตตัสการส่งข้อมูล
          payload = http.getString();//รับข้อมูลที่ได้ทำการติดตาม
          Serial.println(httpCode);//แสดงเสตัสบน Serial moniter
          Serial.println(payload);//แสดงข้อมูลที่ได้ทำการติดตามบน Serial moniter
        }
        else {//เงื่อนไขเมื่อไม่สามารถส่งข้อมูลไปยัง Google Sheet ได้
          Serial.println("Error on HTTP request");//แสดงข้อความบน Serial moniter เมื่อไม่สามารถส่วข้อมูลไปยัง Google Sheet ได้
        }
        http.end();//สิ้นสุดการใช้งานการส่งข้อมูลขึ้น Google Sheet (ควรจะสิ้นสุดการใช้งานทุกครั้งเมื่อการส่งเสร็จสิ้นเพื่อที่จะสามารถส่งข้อมูลไปยัง Google Sheet ได้ในครั้งถัดไป)
}

void sleep(){
  esp_sleep_enable_timer_wakeup(sleepDuration);//ทำการตั้งค่าเวลาเวลาในการ sleep
  esp_deep_sleep_start(); //ทำการเริ่มเข้าสู่โหมด deep sleep
}

void loop() {
  digitalWrite(LED,LOW);
  checkWiFi();
  check_mqtt();
  int result = dht11.readTemperatureHumidity(temperature, humidity);
  if (result == 0) {
        /*Serial.print("Temperature: ");
        Serial.print(temperature);
        Serial.print(" °C\tHumidity: ");
        Serial.print(humidity);
        Serial.println(" %");*/
    } else {
        //Serial.println(DHT11::getErrorString(result));
    }
  Soil_moisture = combinedPredict(analogRead(32));
  if(Soil_moisture<=0){Soil_moisture=0;}
  if(Soil_moisture>=100){Soil_moisture=100;}
  //Serial.println("Soil moisture : "+String(Soil_moisture)+" %");
  client.publish((topic_temp+String(no)).c_str(),String(temperature).c_str());
  client.publish((topic_hum+String(no)).c_str(),String(humidity).c_str());
  client.publish((topic_moisture+String(no)).c_str(),String(Soil_moisture).c_str());
  //Serial.println("time="+String(currentMills-Time));
  Google_Sheet();
  digitalWrite(LED,HIGH);
  sleep();
  
}
