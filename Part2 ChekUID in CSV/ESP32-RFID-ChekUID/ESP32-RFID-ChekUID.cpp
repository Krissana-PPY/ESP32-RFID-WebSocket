#include <SPI.h>
#include <MFRC522.h>
#include <FS.h>
#include <SPIFFS.h>

#define SS_PIN  5 //SDA
#define RST_PIN 21 //RST

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

void setup() {
  Serial.begin(115200);  // Initialize serial communications
  SPI.begin();           // Init SPI bus
  mfrc522.PCD_Init();    // Init MFRC522
  Serial.println("Scan a RFID card");

  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS initialization failed!");
    return;
  }
  Serial.println("SPIFFS initialized.");
}

void loop() {
  // Look for new cards
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  // Show UID on serial monitor
  String content = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  content.toUpperCase();
  Serial.print("UID tag : ");
  Serial.println(content);

  // Check if the UID is known
  bool uidFound = false;
  File file = SPIFFS.open("/UIDs.csv");
  if (file) {
    while (file.available()) {
      String line = file.readStringUntil('\n');
      int commaIndex = line.indexOf(',');
      String uid = line.substring(0, commaIndex);
      String owner = line.substring(commaIndex + 1);

      if (content.equals(uid)) {
        Serial.print("Hello ");
        Serial.println(owner);
        Serial.println("");
        uidFound = true;
        break;
      }
    }
    file.close();
  } else {
    Serial.println("Failed to open UIDs.csv");
  }

  // If UID is not found, add it to the file
  if (!uidFound) {
    Serial.println("Unknown UID. Adding to UIDs.csv");
    file = SPIFFS.open("/UIDs.csv", FILE_APPEND);
    if (file) {
      file.print(content);
      file.println(",Unknown");
      file.close();
      Serial.println("UID added to UIDs.csv");
    } else {
      Serial.println("Failed to open UIDs.csv for appending");
    }
  }
}