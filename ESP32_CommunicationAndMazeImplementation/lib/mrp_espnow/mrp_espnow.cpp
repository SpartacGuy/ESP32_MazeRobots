#include <WiFi.h>
#include <esp_now.h>
#include <mrp_espnow.hpp>
#include <ArduinoJson.h>

#define TIMEOUT_VALUE 10;

// heartbit timer configuration
// hw_timer_t *timer = NULL;
// portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
// volatile int interruptCounter;
//

int MRP_Mesh::add(const String &carID, const MacAddress &mac)
{
  AddressLine_t newEntry;
  for (size_t i = 0; i < addressBook.size(); i++)
  {
    if (addressBook[i].carID == carID)
    {
      return -1;
    }
  }
  newEntry.carID = carID;
  newEntry.mac = mac;
  addressBook.push_back(newEntry);
  return 0;
}

int MRP_Mesh::remove(const String &carID)
{
  for (size_t i = 0; i < addressBook.size(); i++)
  {
    if (addressBook[i].carID == carID)
    {
      addressBook.erase(addressBook.begin() + i);
      return 0;
    }
  }
  return -1;
}

int MRP_Mesh::getMac(String carID, MacAddress &mac)
{
  for (int i = 0; i < addressBook.size(); i++)
  {
    if (addressBook[i].carID == carID)
    {
      mac = addressBook[i].mac;
      return 0;
    }
  }
  return -1;
}
esp_err_t MRP_Mesh::sendPeer(const String &message, const uint8_t *peerAddress)
{
  esp_now_peer_info_t peerInfo = {};
  memcpy(&peerInfo.peer_addr, peerAddress, 6);
  if (!esp_now_is_peer_exist(peerAddress))
  {
    esp_now_add_peer(&peerInfo);
  }
  return esp_now_send(peerAddress, (const uint8_t *)message.c_str(), message.length());
}

esp_err_t MRP_Mesh::broadcast(const String &message)
{
  // this will broadcast a message to everyone in range
  uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  esp_now_peer_info_t peerInfo = {};
  memcpy(&peerInfo.peer_addr, broadcastAddress, 6);
  if (!esp_now_is_peer_exist(broadcastAddress))
  {
    esp_now_add_peer(&peerInfo);
  }
  return esp_now_send(broadcastAddress, (const uint8_t *)message.c_str(), message.length());
}

void MRP_Mesh::setCarID(String id)
{
  carID = id;
}
void MRP_Mesh::espnow_init()
{
  espnow_init(receiveCB);
}

void MRP_Mesh::espnow_init(esp_now_recv_cb_t receiveCallback)
{
  if (esp_now_init() == ESP_OK)
  {
    esp_now_register_recv_cb(receiveCallback);
  }
  else
  {
    ESP.restart();
  }
}

void MRP_Mesh::espnow_init(esp_now_recv_cb_t receiveCallback, esp_now_send_cb_t sentCallback)
{
  if (esp_now_init() == ESP_OK)
  {
    esp_now_register_recv_cb(receiveCallback);
    esp_now_register_send_cb(sentCallback);
  }
  else
  {
    ESP.restart();
  }
}

void formatMacAddress(const uint8_t *macAddr, char *buffer, int maxLength)
{
  snprintf(buffer, maxLength, "%02x:%02x:%02x:%02x:%02x:%02x", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
}

/**
 * Sends MsgID 0
 */
void MRP_Mesh::estamblishConnection()
{
  StaticJsonDocument<50> doc;
  doc["MsgID"] = "0";
  doc["CarID"] = carID;
  String output;
  serializeJson(doc, output);
  broadcast(output);
  doc.clear();
}

/**
 * Sends MsgID 1
 */
void MRP_Mesh::broadcastExitFound(const String &exitPath)
{
  StaticJsonDocument<100> doc;
  doc["MsgID"] = "1";
  doc["CarID"] = carID;
  doc["exitPath"] = exitPath;
  String output;
  serializeJson(doc, output);

  broadcast(output);
  doc.clear();
}

/**
 * Sends MsgID 2
 * Requires custom timer.
 */
void MRP_Mesh::heartBit()
{
  StaticJsonDocument<50> doc;
  doc["MsgID"] = "2";
  doc["CarID"] = carID;
  String output;
  serializeJson(doc, output);
  broadcast(output);
  doc.clear();
}

/**
 * Sends MsgID 3
 */
void MRP_Mesh::updateCommonList(String pointData)
{
  StaticJsonDocument<100> doc;
  doc["MsgID"] = "3";
  doc["CarID"] = carID;
  doc["newPointPath"] = pointData;

  String output;
  serializeJson(doc, output);

  broadcast(output);
  doc.clear();
}

/**
 * Sends MsgID 4
 */
void MRP_Mesh::requestMovePermission(String pointData)
{
  StaticJsonDocument<100> doc;
  doc["MsgID"] = "4";
  doc["CarID"] = carID;
  doc["pointData"] = pointData;
  String output;
  serializeJson(doc, output);
  broadcast(output);
  doc.clear();
  cleanResponses();
}

/**
 * Sends MsgID 5
 */
void MRP_Mesh::respondMovePermission(String response, String carIdDest)
{
  StaticJsonDocument<100> doc;
  doc["MsgID"] = "5";
  doc["CarID"] = carID;
  //doc["pointData"] = pointData;
  doc["Resp"] = response;
  String output;
  MacAddress destMacArray;
  getMac(carIdDest, destMacArray);
  uint8_t destMacPointer[6];
  memcpy(&destMacPointer, &destMacArray, 6);
  serializeJson(doc, output);
  sendPeer(output, destMacPointer);
  doc.clear();
}

/**
 * Sends MsgID 7
 */
void MRP_Mesh::askPointParameters(String direction)
{
  StaticJsonDocument<100> doc;
  doc["MsgID"] = "7";
  doc["CarID"] = carID;
  doc["direction"] = direction;
  //implement sendPeer
  doc.clear();
}

/**
 * Sets all responses to false.
 */
void MRP_Mesh::cleanResponses()
{
  for (size_t i = 0; i < addressBook.size(); i++)
  {
    addressBook[i].requestResponseStatus = 0;
  }
}

/**
 * Registers the response
 */
void MRP_Mesh::registerResponse(String respondedCar)
{
  for (size_t i = 0; i < addressBook.size(); i++)
  {
    if(addressBook[i].carID = respondedCar)
    {
      addressBook[i].requestResponseStatus = 1;
    }
  }
}
// void MRP_Mesh::enableTimeoutCounter()
// {
//   interruptCounter = 0;
//   timer = timerBegin(0, 80, true);
//   timerAttachInterrupt(timer, onTimer, true);
//   timerAlarmWrite(timer, 1000000, true);
//   timerAlarmEnable(timer);
// }

void receiveCB(const uint8_t *macAddr, const uint8_t *data, int dataLen)
{
  
}

/**
 * For detecting if the interrupt has been called, then use checkTimeouts() and 
 * use decInterruptCounter().
 */
// int MRP_Mesh::getInterruptCounter()
// {
//   return interruptCounter;
// }
// void MRP_Mesh::decInterruptCounter()
// {
//   interruptCounter--;
// }

/**
 * Decrements timeouts and removes a robot if detects a timeout
 */
void MRP_Mesh::checkTimeouts()
{
  for (size_t i = 0; i < addressBook.size(); i++)
  {
    addressBook[i].timeoutCounter -= 1;  
    if(addressBook[i].timeoutCounter == 0)
    {
      remove(addressBook[i].carID);
    }
  }
}

void MRP_Mesh::heartBitRegister(String carID)
{
  for (int i = 0; i < addressBook.size(); i++)
  {
    if (addressBook[i].carID == carID)
    {
      addressBook[i].timeoutCounter = TIMEOUT_VALUE; 
    }
  }
}

/**
 * Interrupt function
 */
// void IRAM_ATTR onTimer()
// {
//   portENTER_CRITICAL_ISR(&timerMux);
//   interruptCounter++;
//   portEXIT_CRITICAL_ISR(&timerMux);
// }

int MRP_Mesh::checkResponse()
{
  for (size_t i = 0; i < addressBook.size(); i++)
  {
      if(addressBook[i].requestResponseStatus == 0)
      {
          return 0;
      }
  }
  return 1;
}
