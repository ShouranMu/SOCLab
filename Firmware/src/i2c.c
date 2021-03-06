#include "peripherals.h"
#include <stdio.h>
#include <interrupt.h>
#include "i2c.h"

void i2c_peri_enable(i2c_master_regs_t *i2c, unsigned char num) {
	//enble the I2C master and set the prescaler to num
	i2c->ctrl = I2C_CTRL_EN | num;
	//i2c->ctrl = I2C_CTRL_IEN | I2C_CTRL_EN | num;
}

void i2c_peri_write(i2c_master_regs_t *i2c, uint16_t slave_address) {
	//send slave address, for SRF02 is 0xE0
	//i2c->data[0] = 0xE0;
	i2c->data[0] = slave_address;	
	//send the register nummber of SRF02, for register 0 is Software Firmware Revision
	i2c->data[1] = 0x00;	
	//send measurement command, to set the measurement unit to centimeter with 0x51
	i2c->data[2] = 0x51;
	//start to send 3 bytes, and then stop
	i2c->cmd = I2C_CMD_STA|I2C_CMD_STO|(3 << 3);

	//check if the current transfer has finished			
	while(i2c->stat & I2C_STA_TIP);	
	//check if a not acknowledge has been received			
	while(i2c->stat & I2C_STA_NO_ACK);	
}

//read 1 bytes from the slave
//used for read the revision
unsigned char i2c_peri_read(i2c_master_regs_t *i2c, uint16_t slave_address) {
	//default revision value
	unsigned char ret = 0xFF;
	//send slave address, for SRF02 is 0xE0
	//i2c->data[0] = 0xE0;
	i2c->data[0] = slave_address;
	//send the register nummber of SRF02, for register 0 is Software Firmware Revision		
	i2c->data[1] = 0x00;
	//start to send 2 bytes, and the stop
	i2c->cmd = I2C_CMD_STA|I2C_CMD_STO|(2 << 3);
	//check if the current transfer has finished	
	while(i2c->stat & I2C_STA_TIP);		
	//check if a not acknowledge has been received			
	if(i2c->stat & I2C_STA_NO_ACK) return -1;	

	//send slave read address, slaveid_read = slaveid + 1
	//i2c->data[0] = 0xE1;
	i2c->data[0] = slave_address + 0x01;	
	//read 1 byte					
	i2c->cmd = I2C_CMD_RD|I2C_CMD_STA|I2C_CMD_STO|(2 << 3);		
	//check if the current transfer has finished		
	while(i2c->stat & I2C_STA_TIP);		
	//check if a not acknowledge has been received						
	if(i2c->stat & I2C_STA_NO_ACK) return -1;				
	//read one byte
	ret = i2c->data[1];

	//return 0;
	return ret;
}

//read 2 bytes from the slave 
//used for calulation the distance result
unsigned char i2c_peri_read2bytes(i2c_master_regs_t *i2c, uint16_t slave_address) {
	unsigned int ret = 0;
	//send slave address, for SRF02 is 0xE0
	//i2c->data[0] = 0xE0;
	i2c->data[0] = slave_address;
	//send the register nummber of SRF02, for register 2 is distance result of high byte	
	i2c->data[1] = 0x02;
	//start to send 2 bytes, and the stop
	i2c->cmd = I2C_CMD_STA|(2 << 3);	
	//check if the current transfer has finished	
	while(i2c->stat & I2C_STA_TIP);	
	//check if a not acknowledge has been received				
	if(i2c->stat & I2C_STA_NO_ACK) return -1;	

	//send slave read address, slaveid_read = Slaveid + 1
	//i2c->data[0] = 0xE1;	
	i2c->data[0] = slave_address + 0x01;
	//read 2 byte			
	i2c->cmd = I2C_CMD_RD|I2C_CMD_STA|I2C_CMD_STO|(3 << 3);	
	//check if the current transfer has finished	
	while(i2c->stat & I2C_STA_TIP);		
	//check if a not acknowledge has been received			
	if(i2c->stat & I2C_STA_NO_ACK) return -1;		

	//high byte: i2c->data[1], low byte: i2c->data[2]
	ret = (i2c->data[1] << 8);		
	ret |= i2c->data[2];		
	//printf("high:%x low:%x\n", i2c->data[1], i2c->data[2]);	

	//return 0;
	return ret;
}


