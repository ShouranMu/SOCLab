#include "peripherals.h"

#include <uart.h>
#include <stdio.h>
#include <sleep.h>
#include <interrupt.h>

#include "i2c.h"

#include <spi.h>
#include "spi_local.h"
#include "oled_25664.h"

#define	I2C_ADDR_SEQ1	0xE0
#define	I2C_ADDR_SEQ2	0xE2
#define	I2C_ADDR_SEQ3	0xE4

#define STATUS_A 0
#define STATUS_B 1
#define STATUS_C 2

uint8_t currentStatus = STATUS_A;

unsigned int fms_app(){

	unsigned int wirte_A = 0;
	unsigned int wirte_B = 0;
	unsigned int wirte_C = 0;

	switch(currentStatus){
	    case STATUS_A:
	        i2c_peri_write(&I2C_MASTER_0, I2C_ADDR_SEQ1);
	        if( wirte_A == 0){ 
	        		currentStatus = STATUE_B; 
	        		wirte_A = 1;
	        }else{
        		while(i2c_peri_read(&I2C_MASTER_0, I2C_ADDR_SEQ1) > 254);
				//get the distance result		
				dis = i2c_peri_read2bytes(&I2C_MASTER_0, I2C_ADDR_SEQ1);
				wirte_A = 0;
        	}
	        break;
	    case STATUS_B: 
	        i2c_peri_write(&I2C_MASTER_0, I2C_ADDR_SEQ2);
	        if(wirte_B == 0 ){ 
	        	currentStatus = STATUE_C; 
	        	wirte_B = 1;
	        }else{
        		while(i2c_peri_read(&I2C_MASTER_0, I2C_ADDR_SEQ2) > 254);
				//get the distance result		
				dis = i2c_peri_read2bytes(&I2C_MASTER_0, I2C_ADDR_SEQ2);
				wirte_B = 0;
        	}
	        break;
	    case STATUS_C:  
	        i2c_peri_write(&I2C_MASTER_0, I2C_ADDR_SEQ2);
	        if(wirte_C == 0){ 
	        	currentStatus = STATUE_A; 
	        	wirte_C = 1;
	        }else{
	        	while(i2c_peri_read(&I2C_MASTER_0, I2C_ADDR_SEQ3) > 254);
				//get the distance result		
				dis = i2c_peri_read2bytes(&I2C_MASTER_0, I2C_ADDR_SEQ3);
				wirte_C = 0;
	        }
	        break;
	    default:
	        currentStatus = STATUE_A;
    }

}

unsigned int get_distance_SRF02(){
	
	unsigned int dis = 0;
	unsigned int disArray[20];
	unsigned int dis_result = 0;
	unsigned int k = 0;

	while(1){
	//for(int i=0; i<25; i++) {
		//start measurement
		i2c_peri_write(&I2C_MASTER_0, I2C_ADDR_SEQ1);
		//wait for at least 65ms		
		sleep(130000);
		//test software revision changed or not
		while(i2c_peri_read(&I2C_MASTER_0, I2C_ADDR_SEQ1) > 254);
		//get the distance result		
		dis = i2c_peri_read2bytes(&I2C_MASTER_0, I2C_ADDR_SEQ1);
		//printf("distance: %d\n", dis);
		//discard everytime first 5 results, because in the begining is not stable	
		disArray[(i+1)%20] = dis;
		k += 1;
		if ( k > 25 ) break;
	}

	// the smooth function is following
	unsigned int sum = 0;
	//the highest and lowest number will be discarded
	unsigned int highNum = 0;
	unsigned int lowNum = 600;

	//find the highest and lowest by a loop
	for (int i = 0; i < 20; ++i)
	{
		if (disArray[i]>highNum)
		{
			highNum = disArray[i];
		}else if (disArray[i]<lowNum)
		{
			lowNum = disArray[i];
		}
		sum = sum+disArray[i];
	}
	sum = sum-highNum;
	sum = sum-lowNum;
	//number of the array now is 18
	dis_result = sum/18;
	//if the different between the highest and lowest more than 5, we consider result as error result
	//reason 1: we find when too close to the sensor, the distance results fluctuate seriously
	//reason 2: the ultrasensor may be moved during the measurement  
	if(highNum-lowNum > 5){
		dis_result = 666;	
	}
	return dis_result;

}

// String "distance:"
int distance[]={69,74,84,85,66,79,68,70,27};
// String "error"
int error[]={70,83,83,80,83};

/**
 * 
 */
void show_distance_OLED() {

	unsigned int dis_result = 0;
	//to activate the oled, must firtly high signal, then low signal, then again high signal
	PORT_OUT_0.data = 1;
	PORT_OUT_0.oe=1;
	sleep(1000);
	PORT_OUT_0.data=0;
	sleep(1000);
	PORT_OUT_0.data=1;
	sleep(1000);

	OLED_Init_25664();
	Fill_RAM_25664(0x00);

	//shift from right side to first character
	int right_shift = 15;

	//show "distance"
	for (int i = 0; i < 9; i++)
	{
		Show_Font57_25664(1,distance[i],right_shift,30);
		right_shift=right_shift+2;
	}

	dis_result = get_distance_SRF02();
	
	//get wrong distance
	if(dis_result == 666){
		printf("result distance: error\n");
		for (int i = 0; i < 5; i++){
		Show_Font57_25664(1,error[i],right_shift,30);
		right_shift=right_shift+2;
		}
	}
	//get right distance
	else {
		printf("result distance: %d\n", dis_result);
		if(dis_result < 100){
			//caculate the number in different position
			//tens place number
			Show_Font57_25664(1,dis_result/10+17,right_shift,30);
			//unit place number
			right_shift=right_shift+2;
			Show_Font57_25664(1,dis_result%10+17,right_shift,30);
			right_shift=right_shift+2;	
		} else {
			// caculate the number in different position
			//hundreds place number
			Show_Font57_25664(1,dis_result/100+17,right_shift,30);
			right_shift=right_shift+2;
			//tens place number
			Show_Font57_25664(1,(dis_result%100)/10+17,right_shift,30);
			right_shift=right_shift+2;	
			//unit place number
			Show_Font57_25664(1,dis_result%10+17,right_shift,30);
			right_shift=right_shift+2;
		}
		// show "cm"
		Show_Font57_25664(1,68,right_shift,30);
		right_shift=right_shift+2;
		Show_Font57_25664(1,78,right_shift,30);
	}
}

/**
 * measure the distance with SRF02, display it in console
 */
void test_I2C_SRF02() {
	unsigned int dis = 0;
	for(int i=0; i<10; i++) {
		printf("start measurement...\n");	
		i2c_peri_write(&I2C_MASTER_0);
		//wait for at least 65 ms		
		sleep(130000);
		printf("read revision...\n");		
		while(i2c_peri_read(&I2C_MASTER_0) > 254)			
		printf("distance reading...\n");	
		dis = i2c_peri_read2bytes(&I2C_MASTER_0);		
		printf("distance: %d\n", dis);
	}
}

FILE * stdout = &UART_LIGHT_0_FILE;

void main(){

	printf("\n\n------------------- Hello SP601 ----------------------\n\n");
	
	interrupt_enable();
	while(1){
	/* ----------i2c_part---------- */
	//prescaler = (peripheral_clock / (5 * desired_SCL)) -1
	//SCL auf max. 40 kHz, wobei die peripheral_clock der sys_clk von 60 MHz
	//60MHz/(5*40KHz)-1 = 299
	i2c_peri_enable(&I2C_MASTER_0, 299); // put outside the while loop
	//test_I2C_SRF02();

	/* ----------spi_part---------- */
	spi_peri_enable(&SPI_MASTER_0);
	//select before using the oled display
	spi_peri_select(&SPI_MASTER_0);

	show_distance_OLED();
	spi_peri_deselect(&SPI_MASTER_0);
	printf("\n\n-------------------- next loop -----------------------\n\n");
	}

}

ISR(0)(){
	printf("i2c interrupt enable !!!\n");
	//sleep(13000);
}
