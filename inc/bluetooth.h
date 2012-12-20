#ifndef BLUETOOTH_H
#define BLUETOOTH_H


#include "ecg.h"


typedef enum {
    BT_DISCONNECTED,   // Sabemos que esta el modulo
    BT_CONNECTED,      // Estamos conectados al dispositivo (paired+connected)
    BT_MAKING_ECG,
    BT_REQUESTED_ECG
} bt_state;



//#define RTS_Pin GPIO_Pin_1
//#define CTS_Pin GPIO_Pin_0

void init_bluetooth();
bool btConnect();
void run();
void makeEcg();
void send_Message_Header(uint8_t message_type);
void finish();
bool btTest();
void btSetupModule();
void USART2_IRQHandler(void);
void send_number(int32_t n);
void putChar(char data);
void setInterruption(bool active);

#endif



