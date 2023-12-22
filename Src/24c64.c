#include "24c64.h"

#include "i2c.h"

/* Maximum Timeout values for flags and events waiting loops. These timeouts are
not based on accurate values, they just guarantee that the application will 
not remain stuck if the I2C communication is corrupted.
You may modify these timeout values depending on CPU frequency and application
conditions (interrupts routines ...). */   
#define I2C_Open_FLAG_TIMEOUT         ((uint32_t)0x1000)

#define I2C_Open_LONG_TIMEOUT         ((uint32_t)0xffff)

__IO uint32_t  I2CTimeout = I2C_Open_FLAG_TIMEOUT;

#define EEPROM_DEVICE_ADDRESS 0xA0


/*******************************************************************************
* Function Name  : I2C_24C64_Read
* Description    : 
* Input          : 
* Output         : 
* Return         : 
* Attention      : None
*******************************************************************************/

void I2C_24C64_Read(I2C_HandleTypeDef *I2Cx,uint8_t I2C_Addr,uint16_t addr,uint8_t *buf,uint16_t num)
{
    while(HAL_I2C_Mem_Read (I2Cx ,I2C_Addr,addr,I2C_MEMADD_SIZE_16BIT,buf,num,I2CTimeout) != HAL_OK ){};
}

/*******************************************************************************
* Function Name  : I2C_24C64_WriteOneByte
* Description    : 
* Input          : 
* Output         : None
* Return         : 
* Attention      : None
*******************************************************************************/

void I2C_24C64_WriteOneByte(I2C_HandleTypeDef *I2Cx,uint8_t I2C_Addr,uint16_t addr,uint8_t value)
{   
	while( HAL_I2C_Mem_Write(I2Cx, I2C_Addr, addr, I2C_MEMADD_SIZE_16BIT, &value, 0x01, I2CTimeout) != HAL_OK ){};
}

/*******************************************************************************
* Function Name  : I2C_24C64_Write
* Description    : 
* Input          : 
* Output         : None
* Return         : 
* Attention      : None
*******************************************************************************/

void I2C_24C64_Write(I2C_HandleTypeDef *I2Cx,uint8_t I2C_Addr,uint16_t addr,uint8_t *buf,uint16_t num)
{
	while(num--)
	{
    I2C_24C64_WriteOneByte(I2Cx, I2C_Addr,addr++,*buf++);
	}
}

void floatToBytes(float floatValue, uint8_t *byteArray) {
    uint8_t *floatBytes = (uint8_t *)&floatValue;
    for (int i = 0; i < sizeof(float); i++) {
        byteArray[i] = floatBytes[i];
    }
}

float bytesToFloat(uint8_t *byteArray) {
    float floatValue;
    uint8_t *floatBytes = (uint8_t *)&floatValue;
    for (int i = 0; i < sizeof(float); i++) {
        floatBytes[i] = byteArray[i];
    }
    return floatValue;
}


void writeFloatToEEPROM(float floatValue, uint16_t address) {
    uint8_t byteArray[sizeof(float)];
    floatToBytes(floatValue, byteArray);
    I2C_24C64_Write(&hi2c1, EEPROM_DEVICE_ADDRESS, address, byteArray, sizeof(float));
}


float readFloatFromEEPROM(uint16_t address) {
    uint8_t byteArray[sizeof(float)];
    I2C_24C64_Read(&hi2c1, EEPROM_DEVICE_ADDRESS, address, byteArray, sizeof(float));
    return bytesToFloat(byteArray);
}

/**
* @}
*/

/**
* @}
*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
