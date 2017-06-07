#include <Arduino.h>

#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN    9   //
#define SS_PIN    10    //

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance
#include "NDEF.h"

#define PAYLOAD_SIZE 236
uint8_t payload[PAYLOAD_SIZE] = {};

void setup(void)
{
  Serial.begin(9600);
  pinMode(SS_PIN, OUTPUT);
  pinMode(RST_PIN, OUTPUT);
  while (!Serial);    // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
  SPI.begin();      // Init SPI bus
  mfrc522.PCD_Init();   // Init MFRC522
}

void loop(void)
{

  memset(payload, 0, PAYLOAD_SIZE);

  /*memcpy(payload, "www.odopod.com", 14);
  uint8_t len = NDEF().encode_URI(0x3, payload);*/

  memcpy(payload, "test 1111", 9);
  uint8_t len = NDEF().encode_TEXT((uint8_t *)"fr", payload);

    memcpy(payload, "test 22222", 10);
  uint8_t len2 = NDEF().encode_TEXT((uint8_t *)"fr", payload);




  if ( ! mfrc522.PICC_IsNewCardPresent())
    return;

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial())
    return;

  boolean success = writePayload(payload, len);
  Serial.println(success ? "success" : "fail");

  mfrc522.PICC_DumpMifareUltralightToSerial();
  long block;
  byte buf[4];

  byte res[64];
  byte index = 0;

  for(int i=4; i<16; i++)
  {
      mfrc522.MIFARE_GetValue(i, &block);
      longToByteArray(block, buf);

      for(int j=0; j<4; j++)
      {
        Serial.print(buf[j], HEX);
        Serial.print("\t");
      }
      Serial.println();
      memcpy(res+index, buf, 4);
      index += 4;
  }

  for(int i=0; i<64; i++)
  {
    Serial.print(res[i], HEX);
    Serial.print("\t");
  }

  FOUND_MESSAGE msg = NDEF().decode_message((uint8_t*)res);

  Serial.println();
  String test((char*)msg.payload);
  Serial.println(test);

  delay(5000);
}

boolean writePayload (uint8_t *payload, uint8_t length)
{
  return ultralight_writePayload(payload, length);
}

void longToByteArray(long inLong, byte* outArray)
{
  outArray[0] = inLong;
  outArray[1] = (inLong >> 8);
  outArray[2] = (inLong >> 16);
  outArray[3] = (inLong >> 24);
}

boolean ultralight_writePayload (uint8_t *payload, uint8_t len)
{
  const uint8_t zero[4] = {0x00, 0x00, 0x00, 0x00};

  uint8_t block_buffer[4] = {};
  uint8_t start_block = 4;

  uint8_t position = 0;
  uint8_t block_count = start_block;
  uint8_t byte_count = 0;

  while (position < len) {
    memcpy (block_buffer + byte_count, payload + position, 1);

    byte_count ++;
    position ++;
    if (byte_count == 4 ) {
      //end of block
      byte_count = 0;

      // write block
      //if (!ultralight_writeMemoryBlock(block_count, block_buffer))//#####################################33
      //return false;
      mfrc522.MIFARE_Ultralight_Write(block_count, block_buffer, 4);

      //reset
      memcpy(block_buffer, zero, 16);
      block_count ++;
    }

  }
  //write any remaining buffer
  if (byte_count > 0) {
    //if (!ultralight_writeMemoryBlock(block_count, block_buffer))//#############################################3
    //return false;
    mfrc522.MIFARE_Ultralight_Write(block_count, block_buffer, 4);
  }

  return true;
}
