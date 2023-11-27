#include "string.h"
#include <stdio.h>
#include "usart.h"
#include "lcd.h"
#include "wifi.h"


#define BUFFER_SIZE 128
uint8_t rx_buffer[128];

const char  at[]             = "AT\r\n";           																			      //AT
const char  at_cipclose[]    = "AT+CIPCLOSE\r\n";            																	//关闭链接
const char  at_cwmode[]      = "AT+CWMODE=2\r\n";																				  		//设置为AT模式
const char  at_cipap[]       = "AT+CIPAP=\"192.168.0.15\"\r\n";               									//设置AP的ip地址
const char  at_cwsap[]       = "AT+CWSAP=\"farsight_1001\",\"1234567890123\",11,3\r\n";				//设置AP模式(SSID、密码)
const char  at_cipstart[]    = "AT+CIPSTART=\"UDP\",\"255.255.255.255\",20000,8080,0\r\n";			//设置UDP服务端模式 
const char  at_cipmux[]      = "AT+CIPMUX=0\r\n";																							//设置单链接
const char  at_cipmode[]     = "AT+CIPMODE=1\r\n";																							//设置透传
const char  at_cipsend[]     = "AT+CIPSEND\r\n";																			  		    //开启透传

const char  quit_cipsend[]   = "+++";                                                          //退出退出透传
const char  at_restore[]     = "AT+RESTORE\r\n";    		


const char  at_cwmode_ack[]     = "AT+CWMODE=2\r\r\n\r\nOK\r\n";		                              //设置透传
const char  at_cipap_ack[]      = "AT+CIPAP=\"192.168.0.15\"\r\r\n\r\nOK\r\n"; 
const char  at_cwsap_ack[]      = "AT+CWSAP=\"farsight_1001\",\"1234567890123\",11,3\r\r\n\r\nOK\r\n";	
const char  at_cipstart_ok[]    = "AT+CIPSTART=\"UDP\",\"255.255.255.255\",20000,8080,0\r\r\nCONNECT\r\n\r\nOK\r\n";
const char  at_cipstart_ok2[]   = "AT+CIPSTART=\"UDP\",\"255.255.255.255\",20000,8080,0\r\r\n0,CONNECT\r\n\r\nOK\r\n"; 
const char  at_cipstart_error[] = "AT+CIPSTART=\"UDP\",\"255.255.255.255\",5000,8080,0\r\r\nALREADY CONNECTED\r\n\r\nERROR\r\n"; 
const char  at_cipmux_ack[]     = "AT+CIPMUX=0\r\r\n\r\nOK\r\n";		


uint8_t crc8(uint8_t *data, uint8_t length);

//void show_init();
//extern uint8_t wifibuf[BUFFER_SIZE];

uint8_t Send_command(const char *Command, uint8_t num)
{
	//HAL_UART_Receive_IT(&huart5,wifibuf,BUFFER_SIZE);//使用中断方式接受
	HAL_UART_Transmit(&huart1,(uint8_t *)Command,num,100);
	//HAL_UART_Receive(&huart3,wifibuf,BUFFER_SIZE,100);//使用阻塞方式接受
	return num;
}
void set_wifi(void)
{

		//HAL_GPIO_WritePin(GPIOF,GPIO_PIN_8,GPIO_PIN_RESET);
	

		/***********************退出透传模式**********************ok*/
		Send_command(quit_cipsend, strlen(quit_cipsend));
		HAL_Delay(500);	

	
		/*********************** 恢复出厂设置 **********************ok*/	
		Send_command(at_restore, strlen(at_restore));
		HAL_Delay(500);	
	
			/***********************退出透传模式**********************ok*/
		Send_command(quit_cipsend, strlen(quit_cipsend));
		HAL_Delay(500);	

	
		/*********************** AT **********************ok*/
		Send_command(at, strlen(at));
		HAL_Delay(500);	
		
		/***********************关闭上次的链接**********************ok*/
		Send_command(at_cipclose, strlen(at_cipclose));	
		HAL_Delay(500);	


		/****************将wifi模块配置为AP模式**************************ok*/			
		Send_command(at_cwmode, strlen(at_cwmode));	
		HAL_Delay(500);	
		
		/***********************设置静态IP**********************ok*/
		Send_command(at_cipap, strlen(at_cipap));	
		HAL_Delay(500);	
		
		/*************************配置AP参数******************************ok*/
		Send_command(at_cwsap, strlen(at_cwsap));	
		HAL_Delay(500);	
			
		/***********************建立UDP链接*****************************ok*/
		Send_command(at_cipstart, strlen(at_cipstart));	
		HAL_Delay(500);	
		
		/**************************设置为单链接**************************ok*/
		Send_command(at_cipmux, strlen(at_cipmux));	
  	HAL_Delay(500);	
		
		/************************开启透传模式*******************************/
		Send_command(at_cipmode, strlen(at_cipmode));	
		HAL_Delay(500);	
			
		Send_command(at_cipsend, strlen(at_cipsend));	
		HAL_Delay(500);	
}


// uint8_t crc8(uint8_t *data, uint8_t length)
//{
//    uint8_t i;
//    uint8_t crc = 0;        // Initial value
//    while(length--)
//    {
//        crc ^= *data++;        // crc ^= *data; data++;
//        for ( i = 0; i < 8; i++ )
//        {
//            if ( crc & 0x80 )
//                crc = (crc << 1) ^ 0x07;
//            else
//                crc <<= 1;
//        }
//    }
//    return crc;
//}


