#include <Arduino.h>
#include "Maze.h"
#include "mrp_espnow.hpp"
#include "States.h"
#include "cstring"

MRP_Mesh* mesh;
Maze* maze;

bool responseToMoveNext = false;

Robot_State currentState = Connecting;


// callback when data is sent
void sentCallback(const uint8_t *macAddr, esp_now_send_status_t status)
{
  char macStr[18];
  formatMacAddress(macAddr, macStr, 18);
  Serial.print("Last Packet Sent to: ");
  Serial.println(macStr);
  Serial.print("Last Packet Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void receiveCallback(const uint8_t *macAddr, const uint8_t *data, int dataLen)
{
  // only allow a maximum of 250 characters in the message + a null terminating byte
  char buffer[ESP_NOW_MAX_DATA_LEN + 1];
  int msgLen = min(ESP_NOW_MAX_DATA_LEN, dataLen);
  strncpy(buffer, (const char *)data, msgLen);
  // make sure we are null terminated
  buffer[msgLen] = 0;
  // format the mac address
  char macStr[18];
  formatMacAddress(macAddr, macStr, 18);
  // debug log the message to the serial port
  Serial.printf("Received message from: %s - %s\n", macStr, buffer);

  StaticJsonDocument<250> doc;
  deserializeJson(doc, buffer);
  const char *msgID = doc["MsgID"];
  const char *carID = doc["CarID"];

  if (msgID[0] == '0')
  {
    MacAddress newMac;
    memcpy(newMac.data(), macAddr, 6);
    mesh->add(carID, newMac);

  }
  else if (msgID[0] == '1')
  {
    // not relevant
  }
  else if (msgID[0] == '2')
  {
    // not relevant
  }
  else if (msgID[0] == '3')
  {
    const char* pointData = doc["pointData"];
    char* arr[7];
    char* path = (char*)malloc(sizeof(pointData));
    std::memcpy(path, pointData, strlen(pointData));
    arr[0] = strtok(path, "/");
    arr[1] = strtok(NULL, "/");
    arr[2] = strtok(NULL, "/");
    arr[3] = strtok(NULL, "/");
    arr[4] = strtok(NULL, "/");
    arr[5] = strtok(NULL, "/");
    arr[6] = strtok(NULL, "/");
    maze->UpdateOtherRobotNode(arr[0], (bool)arr[1], (bool)arr[2], (bool)arr[3], (bool)arr[4],
                              (bool)arr[5], (bool)arr[6]);
  }
  else if (msgID[0] == '4')
  {
    String respondedCarID = carID;
    String response; 
    const char* pointData = doc["pointData"];
    char path[strlen(pointData)];
    std::memcpy(path, pointData, strlen(pointData));
    response = String(maze->GetCurrentPosition() == path);
    mesh->respondMovePermission(response,respondedCarID);
  }
  else if (msgID[0] == '5')
  {
    const char *requestResponse = doc["Resp"];
    if (strcmp(requestResponse, "0")==0)
    {
      responseToMoveNext = false;  
    }
    mesh->registerResponse(carID);
  }
  doc.clear();
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  //Set device in STA mode to begin with
  WiFi.mode(WIFI_STA);
  Serial.println("Robot #002");
  // Output my MAC address - useful for later
  Serial.print("My MAC Address is: ");
  Serial.println(WiFi.macAddress()); 
  // shut down wifi
  WiFi.disconnect();
  mesh = new MRP_Mesh();
  mesh->espnow_init(receiveCallback, sentCallback);
  mesh->setCarID("001");
  currentState = Connecting;
  maze = new Maze();
}

void loop() {

  switch (currentState)
  {
    case Connecting:
    {
      Serial.println("Connecting....");
      mesh->estamblishConnection();
      delay(3000);
      currentState = WaitingForNodeInfo;
      break;
    }
    case WaitingForNodeInfo:
    {
      Serial.println("Waiting For Node Info....");
      maze->UpdateNode(1, 1, 1, 0, 0, 0);
      currentState = ChoosingNextNode;
      
      delay(3000);
    }
    case ChoosingNextNode:
    {
      Serial.println("Choosing Next Node....");
      String decision = maze->GetDecision();
      responseToMoveNext = true;
      mesh->requestMovePermission(decision);
      currentState = WaitingForApproval;
      delay(3000);
    }
    case WaitingForApproval:
    {
      Serial.println("Waiting For Approval....");
      if (mesh->checkResponse())
      {
        currentState = MovingToNextNode;
      }
      //delay(3000);
    }
    case MovingToNextNode:
    {
      Serial.println("Moving To Next Node....");
      bool move = maze->MoveNext(responseToMoveNext);
      Serial.println("moving to next node move approved: " + String((int)move));
      if (!move) { currentState = ChoosingNextNode; }
      else { currentState = WaitingForNodeInfo; }
      delay(3000);
    }
    case UpdateCommonList:
    {
      
    }
  }


  
}