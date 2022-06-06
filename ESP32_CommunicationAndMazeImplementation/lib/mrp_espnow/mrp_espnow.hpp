#ifndef MRP_H
#define MRP_H
#include <WiFi.h>
#include <esp_now.h>
#include <ArduinoJson.h>
#include <iostream>
#include <array>
using MacAddress = std::array<uint8_t, 6>;

typedef struct
{
    String carID;
    MacAddress mac;
    int timeoutCounter;
    int requestResponseStatus = 0;
} AddressLine_t;

class MRP_Mesh
{
private:
    std::vector<AddressLine_t> addressBook;
    String carID;
public:
    
    int add(const String &carID, const MacAddress &mac);
    int remove(const String &carID);
    int getMac(String carID, MacAddress &mac);

    esp_err_t sendPeer(const String &message, const uint8_t *peerAddress);
    esp_err_t broadcast(const String &message);
    void setCarID(String id);
    void espnow_init();
    void espnow_init(esp_now_recv_cb_t receiveCallback);
    void espnow_init(esp_now_recv_cb_t receiveCallback, esp_now_send_cb_t sentCallback);
    void estamblishConnection();

    void receiveCallback(const uint8_t *macAddr, const uint8_t *data, int dataLen);
    void sentCallback(const uint8_t *macAddr, esp_now_send_status_t status);
    void broadcastExitFound(const String &exitPath);
    void heartBit();
    void updateCommonList(String pointData);
    void requestMovePermission(String pointData);
    void respondMovePermission(String respond, String CarIdDest);
    void askPointParameters(String direction);
    void cleanResponses();
    void registerResponse(String respondedCar);

    void enableTimeoutCounter();
    void checkTimeouts();
    int getInterruptCounter();
    void decInterruptCounter();
    void heartBitRegister(String carID);
    int checkResponse();
};

void receiveCB(const uint8_t *macAddr, const uint8_t *data, int dataLen);
void sentCB(const uint8_t *macAddr, esp_now_send_status_t status);
void formatMacAddress(const uint8_t *macAddr, char *buffer, int maxLength);
void IRAM_ATTR onTimer();

#endif