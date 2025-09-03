#include <DHT11.h>

DHT11 dht11(22);
int temperature;
int humidity;

void setup() {
  Serial.begin(115200);

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


void loop() {
  int result = dht11.readTemperatureHumidity(temperature, humidity);
  if (result == 0) {
        Serial.print("Temperature: ");
        Serial.print(temperature);
        Serial.print(" °C\tHumidity: ");
        Serial.print(humidity);
        Serial.println(" %");
    } else {
        Serial.println(DHT11::getErrorString(result));
    }
  int a = combinedPredict(analogRead(32));
  Serial.println("Soil moisture : "+String(a)+" %");

}
