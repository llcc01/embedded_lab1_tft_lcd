#include "wifi.h"

#include <stdio.h>

#include "string.h"
#include "usart.h"

#define BUFFER_SIZE 128

const char at[] = "AT\r\n";                  // AT
const char at_cwmode[] = "AT+CWMODE=1\r\n";  // 设置为Station模式
const char at_cwjap[] = "AT+CWJAP=\"P40\",\"++++++++\"\r\n";
const char at_cipsta[] = "AT+CIPSTA?\r\n";  // 查询IP

const char at_restore[] = "AT+RESTORE\r\n";

// void show_init();
// extern uint8_t wifibuf[BUFFER_SIZE];

uint8_t Send_command(const char *Command, uint8_t num) {
  // HAL_UART_Receive_IT(&huart5,wifibuf,BUFFER_SIZE);//使用中断方式接受
  HAL_UART_Transmit(&huart1, (uint8_t *)Command, num, 100);
  // HAL_UART_Receive(&huart3,wifibuf,BUFFER_SIZE,100);//使用阻塞方式接受
  return num;
}

void set_wifi(void) {
  /*********************** 恢复出厂设置 **********************ok*/
  // Send_command(at_restore, strlen(at_restore));
  // HAL_Delay(500);

  /*********************** AT **********************ok*/
  Send_command(at, strlen(at));
  HAL_Delay(500);

  /****************将wifi模块配置为Station模式**************************ok*/
  Send_command(at_cwmode, strlen(at_cwmode));
  HAL_Delay(500);

  /***********************连接wifi*****************************ok*/
  Send_command(at_cwjap, strlen(at_cwjap));
  HAL_Delay(5000);

  /***********************查询IP*****************************ok*/
  Send_command(at_cipsta, strlen(at_cipsta));
  HAL_Delay(500);
}