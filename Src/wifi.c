#include "wifi.h"

#include <stdio.h>

#include "string.h"
#include "usart.h"

#define BUFFER_SIZE 128

const char at[] = "AT\r\n";                  // AT
const char at_cwmode[] = "AT+CWMODE=1\r\n";  // ����ΪStationģʽ
const char at_cwjap[] = "AT+CWJAP=\"P40\",\"++++++++\"\r\n";
const char at_cipsta[] = "AT+CIPSTA?\r\n";  // ��ѯIP

const char at_restore[] = "AT+RESTORE\r\n";

// void show_init();
// extern uint8_t wifibuf[BUFFER_SIZE];

uint8_t Send_command(const char *Command, uint8_t num) {
  // HAL_UART_Receive_IT(&huart5,wifibuf,BUFFER_SIZE);//ʹ���жϷ�ʽ����
  HAL_UART_Transmit(&huart1, (uint8_t *)Command, num, 100);
  // HAL_UART_Receive(&huart3,wifibuf,BUFFER_SIZE,100);//ʹ��������ʽ����
  return num;
}

void set_wifi(void) {
  /*********************** �ָ��������� **********************ok*/
  // Send_command(at_restore, strlen(at_restore));
  // HAL_Delay(500);

  /*********************** AT **********************ok*/
  Send_command(at, strlen(at));
  HAL_Delay(500);

  /****************��wifiģ������ΪStationģʽ**************************ok*/
  Send_command(at_cwmode, strlen(at_cwmode));
  HAL_Delay(500);

  /***********************����wifi*****************************ok*/
  Send_command(at_cwjap, strlen(at_cwjap));
  HAL_Delay(5000);

  /***********************��ѯIP*****************************ok*/
  Send_command(at_cipsta, strlen(at_cipsta));
  HAL_Delay(500);
}