// DL900.h
// Emulation of NX-590E TCP/IP module for DL-900 encrypted connection (uses NX584 protocol)

#ifndef _DL900_h
#define _DL900_h

#include "NX584.h"
extern "C" {
    #include "des.h"
}

class DL900 : public NX584
{
 private:
     //DES des;
     uint8_t encSchedule[3][16][6];
     uint8_t decSchedule[3][16][6];     
     bool hasKey = false;
     void decodeKey(uint8_t *encodedKey, uint8_t masterKeySelector = 1);
     void encrypt(uint8_t *buf, size_t len);
     void decrypt(uint8_t *buf, size_t len);
 protected:
     virtual size_t preProcessReceive(uint8_t *buf, size_t len) override;
	 virtual size_t postProcessSend(uint8_t *buf, size_t len) override;
     virtual void onClientDisconnection() override;
 public:
 	 DL900() {
		tcpServer = WiFiServer(DL900_PORT);
	 }

};

extern DL900 dl900;

#endif