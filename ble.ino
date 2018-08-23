
#ifdef USE_BLE_SERVER

// BLE server stuff (BLE Server example)
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#include <BLE2902.h>


#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"


//BLEServer* pServer = NULL;
//BLECharacteristic* pCharacteristic = NULL;

byte flags = 0b00111110;
byte bpm;
byte heart[8] = { 0b00001110, 60, 0, 0, 0 , 0, 0, 0};
byte hrmPos[1] = {2};
byte BLE_RW_Demo = 0;
bool _BLEClientConnected = false;

#define heartRateService BLEUUID((uint16_t)0x180D)
BLECharacteristic heartRateMeasurementCharacteristics(BLEUUID((uint16_t)0x2A37), BLECharacteristic::PROPERTY_NOTIFY);
BLECharacteristic sensorPositionCharacteristic(BLEUUID((uint16_t)0x2A38), BLECharacteristic::PROPERTY_READ);
BLECharacteristic variabele_demo_Characteristic(BLEUUID((uint16_t)0x2A39), BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);

BLEDescriptor heartRateDescriptor(BLEUUID((uint16_t)0x2901));
BLEDescriptor sensorPositionDescriptor(BLEUUID((uint16_t)0x2901));
BLEDescriptor variabele_demoDescriptior(BLEUUID((uint16_t)0x2901));

class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      _BLEClientConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      _BLEClientConnected = false;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();

      if (value.length() > 0) {
        Serial.println("*********");
        Serial.print("New value: ");
        for (int i = 0; i < value.length(); i++)
          Serial.print(value[i]);

        Serial.println();
        Serial.println("*********");
      }
    }
    void onRead(BLECharacteristic *pCharacteristic) {
      struct timeval tv;
      gettimeofday(&tv, NULL);
      std::ostringstream os;
      os << "Time: " << tv.tv_sec;
      Serial.println("Callback read: ");
      //Serial.println((String(os.str()));
      pCharacteristic->setValue(os.str());
    }
};









void InitBLE() {
  BLEDevice::init("FT7");
  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pHeart = pServer->createService(heartRateService);

  // Create a BLE Descriptor
  //heartRateMeasurementCharacteristics.addDescriptor(new BLE2902());


  pHeart->addCharacteristic(&heartRateMeasurementCharacteristics);
  heartRateDescriptor.setValue("Rate from 0 to 200");
  heartRateMeasurementCharacteristics.addDescriptor(&heartRateDescriptor);


  pHeart->addCharacteristic(&sensorPositionCharacteristic);
  sensorPositionDescriptor.setValue("Position 0 - 6");
  sensorPositionCharacteristic.addDescriptor(&sensorPositionDescriptor);

  pHeart->addCharacteristic(&variabele_demo_Characteristic);
  variabele_demoDescriptior.setValue("RW testing callback");
  //variabele_demo_Characteristic.setValue("Hello World");
  variabele_demo_Characteristic.addDescriptor(&variabele_demoDescriptior);
  // assign event handlers for characteristic
  variabele_demo_Characteristic.setCallbacks(new MyCallbacks());


  // The heartRate service is already advertized, now twice, disabled:
  pServer->getAdvertising()->addServiceUUID(heartRateService);
  //pServer->getAdvertising()->addServiceUUID(heartRateService);

  pHeart->start();
  // Start advertising
  pServer->getAdvertising()->start();


  bpm = 1;    // copied freom init loop as bpm is unknown there.... but what does it.
}


void  update_BLE_values() {
  static uint32_t previous_update_time = millis();
  uint32_t current_time = millis();

  current_time = millis();

  // TODO unlink from display update time
  // NOTE to frequent updates crashes device!!!! (for exampe each milli second)
  if ( ( current_time - previous_update_time ) >= 500 ) {
    previous_update_time = current_time;

    // BLE update stuff
    heart[1] = (byte)bpm;
    int energyUsed = 3000;
    heart[3] = energyUsed / 256;
    heart[2] = energyUsed - (heart[2] * 256);
    //Serial.println(bpm);

    heartRateMeasurementCharacteristics.setValue(heart, 8);
    heartRateMeasurementCharacteristics.notify();

    sensorPositionCharacteristic.setValue(hrmPos, 1);
    bpm++;
    // END BLE stuff

    variabele_demo_Characteristic.getValue();

  } else {
    // Serial.println("No time yet for BLE value updates");
  }
}



#endif

