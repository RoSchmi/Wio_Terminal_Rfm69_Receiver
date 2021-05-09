// Copyright Roschmi 2021, License Apache V2
// Please obey the License agreements of the libraries included like Rfm69 and others


//File: Wio_Terminal_Rfm69_Receiver

#include <Arduino.h>
#include "RFM69.h"
#include "RFM69registers.h"
#include "Reform_uint16_2_float32.h"
#include "config_secret.h"
#include "config.h"

//*********************************************************************************************
// *********** IMPORTANT SETTINGS - YOU MUST CHANGE/CONFIGURE TO FIT YOUR HARDWARE *************
//*********************************************************************************************

#define NETWORKID                   100  //the same on all nodes that talk to each other
#define NODEID                      1    // The unique identifier of this node
#define SOLARPUMP_CURRENT_SENDER_ID 2    // The node which sends current and temperature states
#define SOLAR_TEMP_SENDER_ID        3    // The node which sends state changes of the solar pump
//#define RECEIVER      3    // The recipient of packets (of the test-node)

//Match frequency to the hardware version of the radio on your Feather
#define FREQUENCY     RF69_433MHZ
//#define FREQUENCY     RF69_868MHZ
//#define FREQUENCY      RF69_915MHZ

// Changed by RoSchmi
//#define ENCRYPTKEY     "sampleEncryptKey" //exactly the same 16 characters/bytes on all nodes!
// see in config_secret.h

#define IS_RFM69HCW       true // set to 'true' if you are using an RFM69HCW module

#define RF69_SPI_CS           SS // SS is the SPI slave select pin, 
#define RF69_IRQ_PIN          4  // Rfm69 Interrupt pin
#define RFM69_RST             3  // Rfm69 reset pin


uint8_t receivedData[62] {0};
volatile int actPacketNum = 0;
volatile int lastPacketNum = 0;
volatile int lastPacketNum_3 = 0;


RFM69 rfm69(RF69_SPI_CS, RF69_IRQ_PIN, IS_RFM69HCW);

//RFM69::RFM69(uint8_t slaveSelectPin, uint8_t interruptPin, bool isRFM69HW, SPIClass *spi)

// forward declarations

//float Convert(uint16_t pU1, uint16_t pU2);


int16_2_float_function_result reform_uint16_2_float32(uint16_t u1, uint16_t u2);


void setup() {
  Serial.begin(115200);
  while(!Serial);
  Serial.println("Starting...");

// Hard Reset the RFM module
  pinMode(RFM69_RST, OUTPUT);   // Wio Terminal: 3
  digitalWrite(RFM69_RST, HIGH);  
  delay(100);
  digitalWrite(RFM69_RST, LOW);
  delay(100);

  uint32_t startTime = millis();     // wait 3000 ms
  while(millis() - startTime < 3000)
  {
    delay(10);
  }
  Serial.println("Initializing Rfm69...");

  bool initResult = rfm69.initialize(RF69_433MHZ, NODEID, NETWORKID);

  Serial.printf("Rfm69 initialization %s\r\n", initResult == true ? "was successful" : "failed");

  rfm69.setPowerLevel(31); // power output ranges from 0 (5dBm) to 31 (20dBm), default = 31

  rfm69.encrypt(ENCRYPTKEY);
  //encrypt(0);

  Serial.printf("Working at %i MHz", FREQUENCY==RF69_433MHZ ? 433 : FREQUENCY==RF69_868MHZ ? 868 : 915);
}
  

 // uint8_t initialize(uint8_t freqBand, uint8_t nodeID, uint8_t networkID);

void loop() {
  //check if something was received (could be an interrupt from the radio)
  if (rfm69.receiveDone())
  {
    // Save received data
    memcpy(receivedData, rfm69.DATA, rfm69.PAYLOADLEN);
    uint16_t senderID = rfm69.SENDERID;
    char cmdChar = (char)receivedData[4];

    uint32_t sensor_1 = 0;
    uint32_t sensor_2 = 0;
    uint32_t sensor_3 = 0;
    uint16_t sensor_3_Lower = 0;
    uint16_t sensor_3_Higher = 0;



    //print message received to serial

    Serial.print('[');Serial.print(senderID);
    Serial.println("] ");
    Serial.println(rfm69.DATALEN);
    uint8_t payLoadLen = rfm69.PAYLOADLEN;
    Serial.println(payLoadLen);
    
    

    Serial.print((char*)receivedData);
    Serial.println("");
    for (int i = 0; i < payLoadLen; i++)
    {
      Serial.printf("0x%02x ", receivedData[i]);
    }
    Serial.println("");
    
    char workBuffer[10] {0};
    memcpy(workBuffer, receivedData, 3);
    workBuffer[3] = '\0';
    actPacketNum = atoi(workBuffer);

    memcpy(workBuffer, receivedData + 8, 4);
    workBuffer[4] = '\0';
    int sendInfo = atoi(workBuffer);

    

    int selectedLastPacketNum = (cmdChar == '3') ? lastPacketNum_3 : lastPacketNum;
                
                if (actPacketNum != selectedLastPacketNum)
                {
                    if (cmdChar == '3')
                    {
                        lastPacketNum_3 = actPacketNum;
                    }
                    else
                    {
                        lastPacketNum = actPacketNum;
                    }
                    
                    char oldChar = receivedData[6];

                    sensor_1 = (uint32_t)((uint32_t)receivedData[16] | (uint32_t)receivedData[15] << 8 | (uint32_t)receivedData[14] << 16 | (uint32_t)receivedData[13] << 24);                   
                    sensor_2 = (uint32_t)((uint32_t)receivedData[21] | (uint32_t)receivedData[20] << 8 | (uint32_t)receivedData[19] << 16 | (uint32_t)receivedData[18] << 24);                                        
                    sensor_3 = (uint32_t)((uint32_t)receivedData[26] | (uint32_t)receivedData[25] << 8 | (uint32_t)receivedData[24] << 16 | (uint32_t)receivedData[23] << 24);
                    sensor_3_Lower = (uint16_t)((uint32_t)receivedData[26] | (uint32_t)receivedData[25] << 8);
                    sensor_3_Higher = (uint16_t)((uint32_t)receivedData[24] | (uint32_t)receivedData[23] << 8);
                   
                    switch (senderID)
                    {
                        case SOLARPUMP_CURRENT_SENDER_ID :    
                        {
                          
                          switch (cmdChar)
                          {
                            case '0':             // comes from solar pump
                            case '1':
                            {
                                Serial.println("SolarPump event Sendr Id 2");
                                break;
                            }

                            case '2':             // comes from current sensor
                            {

                              float currentInFloat = ((float)sensor_1 / 100);
                              String currentInString = String(currentInFloat, 2);
                              Serial.print("Current: ");
                              Serial.println(currentInString);

                              //Serial.printf("Current: %d \r\n", sensor_1);
                              float powerInFloat = ((float)sensor_2 / 100);
                              String powerInString = String(powerInFloat, 2);
                              Serial.print("Power: ");
                              Serial.println(powerInString);
                            
                               // convert to float
                              int16_2_float_function_result workInFloat = reform_uint16_2_float32(sensor_3_Higher, sensor_3_Lower);
                              Serial.print("Work: ");       
                              Serial.println(workInFloat.value, 2);

                              break;
                            }                                                      
                          }
                          break;
                        }
                        case SOLAR_TEMP_SENDER_ID :    
                        {
                          switch (cmdChar)
                          {                          
                            case '3':           // came from temp sensors
                            {
                              float collector_float = (float)(((float)sensor_1 / 10) - 70);
                              String collectorInString = String(collector_float, 2);
                              Serial.print("Collector: ");
                              Serial.println(collectorInString);
                              //Serial.printf("Collector: %d \r\n", (int)collector_float);

                              float storage_float = (float)(((float)sensor_2 / 10) - 70);
                              Serial.printf("Storage: %d \r\n", (int)storage_float);

                              float water_float = (float)(((float)sensor_3 / 10) - 70);
                              Serial.printf("Water: %d \r\n", (int)water_float);
                                                           
                              break;
                            }
                            
                          }

                        }
                        break;
                    }
                                                      
                    if (cmdChar == '0' || cmdChar == '1')      // Comes from Pump on/off sensor
                    {
                        //actState = cmdChar == '1' ? InputSensorState.High : InputSensorState.Low;
                        //oldState = oldChar == '1' ? InputSensorState.High : InputSensorState.Low;
                       
                        //OnRfm69OnOffSensorSend(this, new OnOffSensorEventArgs(actState, oldState, repeatSend, DateTime.Now.AddMinutes(RoSchmi.DayLihtSavingTime.DayLihtSavingTime.DayLightTimeOffset(dstStart, dstEnd, dstOffset, DateTime.Now, true)), SensorLabel, SensorLocation, MeasuredQuantity, DestinationTable, Channel, false, current, power, work));
                    }
                    if (cmdChar == '2')                          // Comes from current sensor    
                    {
                        
                      //  OnRfm69DataSensorSend(this, new DataSensorEventArgs(DateTime.Now.AddMinutes(RoSchmi.DayLihtSavingTime.DayLihtSavingTime.DayLightTimeOffset(dstStart, dstEnd, dstOffset, DateTime.Now, true)), repeatSend, current, power, work, SensorLabel, SensorLocation, MeasuredQuantityContinuous, DestinationTableContinuous, Channel, false));
                    }
                    if (cmdChar == '3')                          // Comes from solar temperature sensor (Collector, Storage, Water)
                    {
                      //  OnRfm69SolarTempsDataSensorSend(this, new DataSensorEventArgs(DateTime.Now.AddMinutes(RoSchmi.DayLihtSavingTime.DayLihtSavingTime.DayLightTimeOffset(dstStart, dstEnd, dstOffset, DateTime.Now, true)), repeatSend, current, power, work, SensorLabel, SensorLocation, MeasuredQuantityContinuous, "EscapeTableLocation_03", Channel, false));
                        // Destination Table is changed to "EscapeTableLocation_03" (Magic String)
                       
                    }
                    
                }
            


   //Char cmdChar = Encoding.UTF8.GetChars(e.receivedData, 4, 1)[0];
    
    //UInt16 sendInfo = UInt16.Parse(new string(Encoding.UTF8.GetChars(e.receivedData, 8, 4)));
  
    //actPacketNum = int.Parse(new string(Encoding.UTF8.GetChars(e.receivedData, 0, 3)));

    Serial.print("   [RX_RSSI:");Serial.print(rfm69.RSSI);Serial.print("]\r\n");

    //check if received message contains Hello World
    if (strstr((char *)rfm69.DATA, "Hello World"))
    {
      //check if sender wanted an ACK
      if (rfm69.ACKRequested())
      {
          
          unsigned long Start = millis();
          while(millis() - Start < 50);    // A delay is needed for NETMF (50 ms ?)
                                           // if ACK comes to early it is missed by NETMF
                           
          rfm69.sendACK();
          Serial.println(" - ACK sent");
      }
      //Blink(LED, 40, 3); //blink LED 3 times, 40ms between blinks
    }  
  }
  rfm69.receiveDone(); //put radio in RX mode
  Serial.flush(); //make sure all serial data is clocked out before sleeping the MCU

  delay (100);
  // put your main code here, to run repeatedly:
}

