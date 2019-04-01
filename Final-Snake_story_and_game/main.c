/*	Name & E-mail: 
 *  1. Chi Chiu Tsang, ctsan005@ucr.edu
 *	Lab Section: 23
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include "nokia5110.c"//Library is from https://github.com/LittleBuster/avr-nokia5110, Thank you for such a simple and useful library
#include "bit.h"//from ucr library, use for getbit and setbit function
#include "io.c"//from ucr library, use for basic control for the LCD1602 
//#include "timer.h" // already include in the main.c directly
//#include "scheduler.h" // already include in the main.c directly

unsigned char Pattern1[]= {0x0E,0x0E,0x04,0x0E,0x15,0x15,0x0A,0x11}; //normal standing human
unsigned char Pattern2[]= {0x0E,0x0E,0x04,0x15,0x0E,0x04,0x0A,0x11};//jump human
unsigned char Pattern3[]= {0x00,0x00,0x00,0x00,0x00,0x04,0x0E,0x1F};//spike
unsigned char Pattern4[]= {0x00,0x00,0x00,0x00,0x00,0x01,0x02,0x1C};//snake (right)
unsigned char Pattern5[]= {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07};//snake (left)
unsigned char Pattern6[]= {0x1C,0x1C,0x0E,0x07,0x07,0x03,0x03,0x03};//big snake (left)
unsigned char Pattern7[]= {0x00,0x00,0x00,0x00,0x00,0x1F,0x1F,0x1F};//big snake (middle)
unsigned char Pattern8[]= {0x00,0x00,0x00,0x00,0x00,0x10,0x1C,0x1E};//big snake (right)
unsigned char Pattern9[]= {0x00,0x0A,0x15,0x11,0x0A,0x04,0x00,0x00};//empty heart
unsigned char Pattern10[]= {0x00,0x1B,0x1F,0x1F,0x0E,0x04,0x00,0x00};// full heart
unsigned char Pattern11[]= {0x00,0x00,0x00,0x0E,0x1F,0x1F,0x1F,0x1F};// big snake (middle ver full)
unsigned char Pattern12[]= {0x00,0x00,0x0C,0x08,0x08,0x08,0x08,0x18};// snake (jump left)
unsigned char Pattern13[]= {0x00,0x00,0x06,0x02,0x02,0x02,0x02,0x03};// snake (jump right)
unsigned char Pattern14[]= {0x10,0x08,0x04,0x1A,0x1A,0x02,0x02,0x02};// house (right)
unsigned char Pattern15[]= {0x01,0x02,0x04,0x08,0x08,0x0B,0x0B,0x0B};// house (left)


typedef struct task {
	int state; // Current state of the task
	unsigned long period; // Rate at which the task should tick
	unsigned long elapsedTime; // Time since task's previous tick
	int (*TickFct)(int); // Function to call for task's tick
} task;

typedef struct location {//use to store the target location
	unsigned char x_loc;
	unsigned char y_loc;
}location;

typedef struct snake_loc{//use to store the snake location
	unsigned char x_loc;
	unsigned char y_loc;
} snake_loc;

unsigned char storyFlag = 0;
unsigned char storyEnd = 0;
unsigned char gameFlag = 0;
unsigned char gameEnd = 0;

unsigned char snake_direction;
unsigned char snake_size;
unsigned char snake_target;

location level1[20];//need a bigger number? increase the size and initial it in the level1_init!!
void level1_init(){
	level1[0].x_loc = 20;
	level1[0].y_loc = 30;
	level1[1].x_loc = 78;
	level1[1].y_loc = 26;
	level1[2].x_loc = 54;
	level1[2].y_loc = 2;
	level1[3].x_loc = 4;
	level1[3].y_loc = 35;
	level1[4].x_loc = 2;
	level1[4].y_loc = 9;
	level1[5].x_loc = 22;
	level1[5].y_loc = 25;
	level1[6].x_loc = 35;
	level1[6].y_loc = 24;
	level1[7].x_loc = 37;
	level1[7].y_loc = 37;
	level1[8].x_loc = 80;
	level1[8].y_loc = 8;
	level1[9].x_loc = 34;
	level1[9].y_loc = 21;
	level1[10].x_loc = 47;
	level1[10].y_loc = 11;	
	level1[11].x_loc = 24;
	level1[11].y_loc = 22;
	level1[12].x_loc = 5;
	level1[12].y_loc = 30;
	level1[13].x_loc = 4;
	level1[13].y_loc = 11;
	level1[14].x_loc = 25;
	level1[14].y_loc = 22;
	level1[15].x_loc = 39;
	level1[15].y_loc = 11;
	level1[16].x_loc = 78;
	level1[16].y_loc = 22;
	level1[17].x_loc = 37;
	level1[17].y_loc = 37;
	level1[18].x_loc = 80;
	level1[18].y_loc = 8;
	level1[19].x_loc = 34;
	level1[19].y_loc = 21;
};

snake_loc snake[50] = {};
void snake_init(){//init the size of the snake so that it start with longer length, but remember to update the condition for score and decider for win!!
	snake[0].x_loc = 10;
	snake[0].y_loc = 30;
	snake[1].x_loc = 9;
	snake[1].y_loc = 30;
	snake[2].x_loc = 8;
	snake[2].y_loc = 30;
	snake[3].x_loc = 7;
	snake[3].y_loc = 30;
	snake[4].x_loc = 6;
	snake[4].y_loc = 30;
	snake[5].x_loc = 5;
	snake[5].y_loc = 30;
	snake[6].x_loc = 4;
	snake[6].y_loc = 30;
	snake[7].x_loc = 3;
	snake[7].y_loc = 30;
	snake[8].x_loc = 3;
	snake[8].y_loc = 29;
	snake[9].x_loc = 3;
	snake[9].y_loc = 28;
	snake[10].x_loc = 3;
	snake[10].y_loc = 27;
	for(unsigned i = 0; i < 11; ++i){//after listing the dot location of the snake, need to write it to the lcd
		nokia_lcd_set_pixel(snake[i].x_loc, snake[i].y_loc, 1);
	}
	snake_direction = 1;
	snake_size =11;
	snake_target = 0;
	nokia_lcd_set_pixel(level1[0].x_loc, level1[0].y_loc, 1);// also write the target to the lcd 
}

void snake_target_update(){
	nokia_lcd_set_pixel(level1[snake_target].x_loc, level1[snake_target].y_loc, 1);//write the next dot to the lcd
}


uint16_t adc_read(uint8_t ch);
void adc_init();

unsigned char checkJoyStick(){//use to determine which direction is the joystick right now, include a table for reference
	unsigned short adc_result1 = adc_read(2);
	unsigned short adc_result2 = adc_read(3);
	if(adc_result1 > 700){
		if(adc_result2 > 700){
			return 0;
		}
		else if(adc_result2 < 300){
			return 1;
		}
		else{
			return 2;
		}
	}
	else if(adc_result1 < 250){
		if(adc_result2 > 700){
			return 3;
		}
		else if(adc_result2 < 300){
			return 4;
		}
		else{
			return 5;
		}
	}
	else{
		if(adc_result2 > 700){
			return 6;
		}
		else if(adc_result2 < 300){
			return 7;
		}
		else{
			return 8;
		}
	}
	
	/************************************************************************/
	/* reference table
	
	case0: Right Up
	case1: Right down
	case2: Right
	case3: Left Up
	case4: Left Down
	case5: Left
	case6: Up
	case7: Down
	case8: None                                                             */
	/************************************************************************/
	
}

unsigned char snake_check(unsigned char s_size, unsigned char direction){//return does the snake hit the target
	
	unsigned char tempX = 0;//first find the next location where the snake head goes
	unsigned char tempY = 0;
	switch(direction){//find the temp x and y location
		case 0://up
		if(snake[0].y_loc == 0){
			tempX = snake[0].x_loc;
			tempY = 47;
		}
		else{
			tempX = snake[0].x_loc;
			tempY = (snake[0].y_loc - 1);
		}
		break;
		
		
		case 1://right
		if(snake[0].x_loc == 83){
			tempX = 0;
			tempY = snake[0].y_loc;
		}
		else{
			tempX = (snake[0].x_loc + 1);
			tempY = snake[0].y_loc;
		}
		break;
		
		
		case 2://down
		if(snake[0].y_loc == 47){
			tempX = snake[0].x_loc;
			tempY = 0;
		}
		else{
			tempX = snake[0].x_loc;
			tempY = (snake[0].y_loc + 1);
		}
		break;
		
		
		case 3://left
		if(snake[0].x_loc == 0){
			tempX = 83;
			tempY = snake[0].y_loc;
		}
		else{
			tempX = (snake[0].x_loc - 1);
			tempY = snake[0].y_loc;
		}
		break;
		
	}
	
	if(level1[snake_target].x_loc == tempX && level1[snake_target].y_loc == tempY){//check does it hit the target
		return 1;
	}
	for(unsigned char i = 2; i < s_size - 1; ++i){//check does it hit itself
		if(tempX == snake[i].x_loc && tempY == snake[i].y_loc){
			return 2;
		}
	}
	return 0;//snake does not hit itself nor hit target
}

unsigned char snake_update(unsigned char s_size, unsigned char direction){ // return the size for the snake
	unsigned char tempCheck;
	tempCheck = snake_check(s_size, direction);//find the condition of the snake after moving once
	switch(direction){
		case 0: // move up
		if(tempCheck == 0){//did not die nor hit the target
			nokia_lcd_set_pixel(snake[s_size - 1].x_loc , snake[s_size - 1].y_loc, 0);//clear the tail
			for(unsigned char a = s_size - 1; a > 0; --a ){
				snake[a].x_loc = snake[a-1].x_loc;
				snake[a].y_loc = snake[a-1].y_loc;
			}
			if(snake[0].y_loc == 0){
				snake[0].y_loc = 47;
			}
			else{
				snake[0].y_loc = snake[0].y_loc - 1;
			}
			nokia_lcd_set_pixel(snake[0].x_loc, snake[0].y_loc, 1);//set the head to 1
			return s_size;
			
		}
		else if(tempCheck == 1){//hit the target
			if(s_size == 25){
				return 100;//indicate player win
			}
			for(unsigned char a = s_size; a > 0; --a){
				snake[a].x_loc = snake[a-1].x_loc;
				snake[a].y_loc = snake[a-1].y_loc;
			}
			if(snake[0].y_loc == 0){
				snake[0].y_loc = 47;
			}
			else{
				snake[0].y_loc = snake[0].y_loc - 1;
			}
			nokia_lcd_set_pixel(snake[0].x_loc, snake[0].y_loc, 1);//set the head to 1
			snake_target++;
			snake_target_update();
			return (s_size + 1);
			
		}
		else if(tempCheck == 2){//hit itself
			return 200;//indicate the player lose			
		}
		break;
		
		
		case 1: // move right
		if(tempCheck == 0){//did not die nor hit the target
			nokia_lcd_set_pixel(snake[s_size - 1].x_loc , snake[s_size - 1].y_loc, 0);//clear the tail
			for(unsigned char a = s_size - 1; a > 0; --a ){
				snake[a].x_loc = snake[a-1].x_loc;
				snake[a].y_loc = snake[a-1].y_loc;
			}
			if(snake[0].x_loc == 83){
				snake[0].x_loc = 0;
			}
			else{
				snake[0].x_loc = snake[0].x_loc + 1;
			}
			nokia_lcd_set_pixel(snake[0].x_loc, snake[0].y_loc, 1);//set the head to 1
			return s_size;
			
		}
		else if(tempCheck == 1){//hit the target
			if(s_size == 25){
				return 100;//indicate player win
			}
			
			for(unsigned char a = s_size; a > 0; --a){
				snake[a].x_loc = snake[a-1].x_loc;
				snake[a].y_loc = snake[a-1].y_loc;
			}
			if(snake[0].x_loc == 83){
				snake[0].x_loc = 0;
			}
			else{
				snake[0].x_loc = snake[0].x_loc + 1;
			}
			nokia_lcd_set_pixel(snake[0].x_loc, snake[0].y_loc, 1);//set the head to 1
			snake_target++;
			snake_target_update();
			return s_size + 1;
			
		}
		else if(tempCheck == 2){//hit itself
			return 200;//indicate the player lose
		}
		break;
		
		case 2: // move down
		if(tempCheck == 0){//did not die nor hit the target
			nokia_lcd_set_pixel(snake[s_size - 1].x_loc , snake[s_size - 1].y_loc, 0);//clear the tail
			for(unsigned char a = s_size - 1; a > 0; --a ){
				snake[a].x_loc = snake[a-1].x_loc;
				snake[a].y_loc = snake[a-1].y_loc;
			}
			if(snake[0].y_loc == 47){
				snake[0].y_loc = 0;
			}
			else{
				snake[0].y_loc = snake[0].y_loc + 1;
			}
			nokia_lcd_set_pixel(snake[0].x_loc, snake[0].y_loc, 1);//set the head to 1
			return s_size;
			
		}
		else if(tempCheck == 1){//hit the target
			if(s_size == 25){
				return 100;//indicate player win
			}
			for(unsigned char a = s_size; a > 0; --a){
				snake[a].x_loc = snake[a-1].x_loc;
				snake[a].y_loc = snake[a-1].y_loc;
			}
			if(snake[0].y_loc == 47){
				snake[0].y_loc = 0;
			}
			else{
				snake[0].y_loc = snake[0].y_loc + 1;
			}
			nokia_lcd_set_pixel(snake[0].x_loc, snake[0].y_loc, 1);//set the head to 1
			snake_target++;
			snake_target_update();
			return s_size + 1;
			
		}
		else if(tempCheck == 2){//hit itself
			return 200;//indicate the player lose
		}
		break;
		
		
		case 3: // move left
		if(tempCheck == 0){//did not die nor hit the target
			nokia_lcd_set_pixel(snake[s_size - 1].x_loc , snake[s_size - 1].y_loc, 0);//clear the tail
			for(unsigned char a = s_size - 1; a > 0; --a ){
				snake[a].x_loc = snake[a-1].x_loc;
				snake[a].y_loc = snake[a-1].y_loc;
			}
			if(snake[0].x_loc == 0){
				snake[0].x_loc = 83;
			}
			else{
				snake[0].x_loc = snake[0].x_loc - 1;
			}
			nokia_lcd_set_pixel(snake[0].x_loc, snake[0].y_loc, 1);//set the head to 1
			return s_size;
			
		}
		else if(tempCheck == 1){//hit the target
			if(s_size == 25){
				return 100;//indicate player win
			}
			for(unsigned char a = s_size; a > 0; --a){
				snake[a].x_loc = snake[a-1].x_loc;
				snake[a].y_loc = snake[a-1].y_loc;
			}
			if(snake[0].x_loc == 0){
				snake[0].x_loc = 83;
			}
			else{
				snake[0].x_loc = snake[0].x_loc - 1;
			}
			nokia_lcd_set_pixel(snake[0].x_loc, snake[0].y_loc, 1);//set the head to 1
			snake_target++;
			snake_target_update();
			return s_size + 1;
			
		}
		else if(tempCheck == 2){//hit itself
			return 200;//indicate the player lose
		}
		break;
		
		default:
		break;
	}
	return 255; // error happen
}






task tasks[3];

const unsigned char tasksNum = 3;
const unsigned long tasksPeriodGCD = 200;

const unsigned long periodLCD_Display = 200;
const unsigned long periodDisplay = 1000;
const unsigned long periodSelect = 200;


void CreateCustomCharacter (unsigned char *Pattern, const char Location)//use to load the custom character into the Nokia 5110 ram
{
	int i=0;
	LCD_WriteCommand( 0x40 + (Location * 8));     //Send the Address of CGRAM
	for (i=0; i<8; i++){
		LCD_WriteData(Pattern [ i ] );         //Pass the bytes of pattern on LCD
	}
}

enum SE_States { SE_SMStart, SE_s1, SE_s2, SE_s3, SE_story, SE_game };
int TickFct_Select(int state){
	unsigned char tempDir;
	static unsigned char tempChoice = 1;
	switch(state){
		case SE_SMStart://init the display with the starting phase
		LCD_DisplayString(1,"select mode");
		state = SE_s1;
		
		tempChoice = 0;
		break;
		
		case SE_s1://Print the two choices into the screen
		if(!GetBit(PINA, 4)){
			state = SE_s2;
			LCD_DisplayString(1,"  Story           game");
		}
		break;
		
		case SE_s2://Require the user to release the press from the joystick
		if(GetBit(PINA, 4)){
			state = SE_s3;
		}
		break;
		
		case SE_s3://show the user choices by arrow, also raise the flag for the corresponding mode to start
		tempDir = checkJoyStick();
		if( tempDir == 6 || tempDir == 7){//up or down
			tempChoice = tempChoice + 1;
		}
		if((tempChoice+1) % 2){
			LCD_WriteCommand(0xC0);
			LCD_WriteData(' ');
			LCD_WriteCommand(0x80);
			LCD_WriteData(0x7E);
		}
		else{
			LCD_WriteCommand(0x80);
			LCD_WriteData(' ');
			LCD_WriteCommand(0xC0);
			LCD_WriteData(0x7E);
		}
		
		if(!GetBit(PINA, 4)){
			if((tempChoice+1) % 2){
				storyFlag = 1;
				state = SE_story;
			}
			else{
				gameFlag = 1;
				state = SE_game;
			}
		}
		break;
		
		case SE_story://stay until story end, drop the story flag, handshake 
		if(storyEnd){
			storyFlag = 0;
			state = SE_SMStart;
		}
		break;
		
		case SE_game://stay until game end, drop the game flag, handshake
		if(gameEnd){
			gameFlag = 0;
			state = SE_SMStart;
		}
		break;
		
		default:
		break;
		
		
		
		
		
	}
	return state;
};

enum DY_States { DY_SMStart, DY_wait, DY_s1, DY_s2,  DY_s3,  DY_s4,  DY_s5,  DY_s6,  DY_s7,  DY_s8, DY_s9, DY_s10,  DY_Null };
int TickFct_Display(int state){
	static unsigned char count = 0;
	switch(state){
		case DY_SMStart://Get the LCD1602 ready
		state = DY_wait;
		LCD_ClearScreen();
		LCD_WriteCommand(0x88);
		break;
		
		
		case DY_wait://wait for a flag to show the story
		if(storyFlag){
			state = DY_s1;
		}
		break;
		
		
		case DY_s1://init the story
		LCD_DisplayString(1, " Snake's story");
		CreateCustomCharacter(Pattern1,1);
		CreateCustomCharacter(Pattern2,2);
		CreateCustomCharacter(Pattern3,3);
		CreateCustomCharacter(Pattern4,4);
		CreateCustomCharacter(Pattern5,5);
		CreateCustomCharacter(Pattern12,6);
		CreateCustomCharacter(Pattern13,7);
		state = DY_s2;
		break;
		
		
		case DY_s2://6th and 10th space insert spike, write string, spike, and human to the lcd screen
		LCD_ClearScreen();
		LCD_DisplayString(1, " Catch snake");
		LCD_WriteCommand(0xC0);
		LCD_WriteCommand(0xC5);
		LCD_WriteData(3);
		LCD_WriteCommand(0xC9);
		LCD_WriteData(3);
		LCD_WriteCommand(0xC0);
		LCD_WriteData(1);
		LCD_WriteCommand(0xC2);
		LCD_WriteData(5);
		LCD_WriteData(4);
		state = DY_s3;
		
		nokia_lcd_clear();
		nokia_lcd_write_string("Little snake  is scared,    running to itsmother for    help" , 1);
		nokia_lcd_render();
		break;
		
		case DY_s3://clear only the top row
		LCD_WriteCommand(0x80);
		for(unsigned char i = 0; i < 16; ++i){
			LCD_WriteData(' ');
		}
		state = DY_s4;
		count = 0;
		break;
		
		
		case DY_s4://update human and small snake action moving from left to right
		if(count <16){
			LCD_WriteCommand(0xC0 + count);
			
			if(count == 15){
				LCD_WriteData(' ');
			}
			
			else if(count < 15){
				if(count == 4 || count == 8){
					LCD_WriteData(' ');
					LCD_WriteCommand(0x80 + count + 1);
					LCD_WriteData(2);
				}
				else if(count == 5 || count == 9){
					LCD_WriteCommand(0x80 + count);
					LCD_WriteData(' ');
					LCD_WriteCommand(0xC0 + count + 1);
					LCD_WriteData(1);
				}
				else{
					LCD_WriteData(' ');
					LCD_WriteData(1);
				}
			}
			//Top is update human
			
			if(count == 12){
				LCD_WriteCommand(0xC0 + count + 2);
				LCD_WriteData(' ');
				LCD_WriteData(5);
			}
			else if(count == 13){
				LCD_WriteCommand(0xC0 + count + 2);
				LCD_WriteData(' ');
			}
			else if(count == 1 || count == 5){
				LCD_WriteCommand(0xC0 + count + 2);
				LCD_WriteData(' ');
				LCD_WriteData(' ');
				LCD_WriteCommand(0x80 + count + 4);
				LCD_WriteData(6);
				
			}
			else if(count == 2 || count == 6){
				LCD_WriteCommand(0x80 + count + 3);
				LCD_WriteData(7);
			}
			else if(count == 3 || count == 7){
				LCD_WriteCommand(0x80 + count + 2);
				LCD_WriteData(' ');
				LCD_WriteData(' ');
				LCD_WriteCommand(0xC0 + count + 3);
				LCD_WriteData(5);
				LCD_WriteData(4);
			}
			
			else{
				LCD_WriteCommand(0xC0 + count + 2);
				LCD_WriteData(' ');
				LCD_WriteData(5);
				LCD_WriteData(4);
			}
			//Above is update snake
			
			++count;
		}
		else{
			state = DY_s5;
			count = 0;
			
		}
		break;
		
		case DY_s5://display the string for the screen change
		LCD_DisplayString(1," Mom, help!");
		CreateCustomCharacter(Pattern6,6);
		CreateCustomCharacter(Pattern7,7);
		CreateCustomCharacter(Pattern8,0);
		state = DY_s6;
		
		nokia_lcd_clear();
		nokia_lcd_write_string("Mother snake  is angry,     chasing back  the human" , 1);
		nokia_lcd_render();
		break;
		
		
		
		case DY_s6://clear the string, add the spike back to the screen
		LCD_WriteCommand(0x80);
		for(unsigned char i = 0; i < 16; ++i){
			LCD_WriteData(' ');
		}
		LCD_WriteCommand(0xC5);
		LCD_WriteData(3);
		LCD_WriteCommand(0xC9);
		LCD_WriteData(3);
		state = DY_s7;
		break;
		
		
		case DY_s7://update the human and big snake
		if(count < 21){
			if(count == 6 || count == 10){
				LCD_WriteCommand(0x8F - count);
				LCD_WriteData(2);
				LCD_WriteCommand(0xCF - count + 1);
				LCD_WriteData(' ');
			}
			else if(count == 7 || count == 11){
				LCD_WriteCommand(0xCF - count);
				LCD_WriteData(1);
				LCD_WriteCommand(0x8F - count + 1);
				LCD_WriteData(' ');
			}
			else if(count == 0){
				LCD_WriteCommand(0xCF);
				LCD_WriteData(1);
			}
			else if(count == 16){
				LCD_WriteCommand(0xC0);
				LCD_WriteData(' ');
			}
			else if(count >15 && count < 20){
				//do nothing
			}
			else{
				LCD_WriteCommand(0xCF - count);
				LCD_WriteData(1);
				LCD_WriteData(' ');
			}
			//top is update human
			
			if(count < 2){
				//do nothing
			}
			else if(count == 2){
				LCD_WriteCommand(0xCF);//0xCF - count + 2
				LCD_WriteData(6);
			}
			else if(count == 3){
				LCD_WriteCommand(0xCE);//0xCF - count + 2
				LCD_WriteData(6);
				LCD_WriteData(7);
			}
			else if(count == 4){
				LCD_WriteCommand(0xCD);
				LCD_WriteData(6);
				LCD_WriteData(7);
				LCD_WriteData(0);
			}
			else if(count == 18){
				LCD_WriteCommand(0xC0);
				LCD_WriteData(7);
				LCD_WriteData(0);
				LCD_WriteData(' ');
			}
			else if(count == 19){
				LCD_WriteCommand(0xC0);
				LCD_WriteData(0);
				LCD_WriteData(' ');
			}
			else if(count == 20){
				LCD_WriteCommand(0xC0);
				LCD_WriteData(' ');
			}
			else{
				LCD_WriteCommand(0xCF - count + 2);
				LCD_WriteData(6);
				LCD_WriteData(7);
				LCD_WriteData(0);
				LCD_WriteData(' ');
			}
			++count;
		}
		else{
			state = DY_s8;
			count = 0;
			CreateCustomCharacter(Pattern11, 7);
			CreateCustomCharacter(Pattern14, 1);
			CreateCustomCharacter(Pattern15, 2);
			nokia_lcd_clear();
			nokia_lcd_write_string("Mother snake  catch and eat the human     before he run back to his   house" , 1);
			nokia_lcd_render();
		}
		break;
		
		
		case DY_s8://display snake eat human
		if(count >=  0 && count < 5){}
		else if(count == 5){
			LCD_WriteCommand(0xC0);
			LCD_WriteData(0);
		}
		else if(count == 6){
			LCD_WriteCommand(0xC0);
			LCD_WriteData(7);
			LCD_WriteData(0);
		}
		else if(count == 7){
			LCD_WriteCommand(0xC0);
			LCD_WriteData(6);
			LCD_WriteData(7);
			LCD_WriteData(0);
		}
		else if(count == 8){
			LCD_WriteCommand(0xC0);
			LCD_WriteData(1);
			LCD_WriteData(6);
			LCD_WriteData(7);
			LCD_WriteData(0);
		}
		else if(count == 9){
			LCD_WriteCommand(0xC0);
			LCD_WriteData(2);
			LCD_WriteData(1);
			LCD_WriteData(6);
			LCD_WriteData(7);
			LCD_WriteData(0);
		}
		else if(count == 16){
			state = DY_s9;
			count = 0;
		}
		else{
			LCD_WriteCommand(0xC0 + count - 10);
			LCD_WriteData(' ');
			LCD_WriteData(2);
			LCD_WriteData(1);
			LCD_WriteData(6);
			LCD_WriteData(7);
			LCD_WriteData(0);
		}
		++ count;
		break;
		
		
		case DY_s9://display another message
		LCD_DisplayString(1, "I want to get   stronger");
		nokia_lcd_clear();
		nokia_lcd_write_string("After watching all, little  snake wish to get as strong as mother" , 1);
		nokia_lcd_render();
		state = DY_s10;
		
		break;
		
		
		case DY_s10://need the player to click the joystick to continue
		if(!GetBit(PINA, 4)){
			storyEnd = 1;
			state = DY_Null;
		}
		
		break;
		
		
		case DY_Null://handshake perform to drop the story end flag
		if(!storyFlag){
			state = DY_wait;
			storyEnd = 0;
		}
		
		default:
		break;
	}
	return state;
};

enum LCD_Display_States { LCD_SMStart, LCD_wait, LCD_Level1_init, LCD_Level1_begin,  LCD_Level1, LCD_Lost, LCD_Win, LCD_wait2,  LCD_Null};
int TickFct_LCD_Display(int state){
	unsigned char dir;
	switch(state){
		case LCD_SMStart://Start the nokia LCD
		nokia_lcd_init();
		nokia_lcd_clear();
		adc_init();
		state = LCD_wait;
		break;
		
		
		case LCD_wait://wait for the game flag from the selection to raise
		if(gameFlag){
			state = LCD_Level1_init;
			LCD_DisplayString(1, "Look at other   screen");
			LCD_WriteCommand(0xC8);
			LCD_WriteData(0x5E);
			
		}
		break;
		
		
		case LCD_Level1_init://init the dot in the game
		nokia_lcd_clear();
		nokia_lcd_write_string("LEVEL1",1);
		nokia_lcd_set_cursor(0, 10);
		nokia_lcd_write_string("READY", 3);
		nokia_lcd_render();
		level1_init();
		state = LCD_Level1_begin;		
		break;
		
		
		case LCD_Level1_begin://also put the snake in 
		nokia_lcd_clear();
		snake_init();
		nokia_lcd_render();
		state = LCD_Level1;
		break;
		
		
		case LCD_Level1://update the direction, update the snake, and also decide win or lose
		// update the direction
		dir = checkJoyStick();
		if(dir == 2){//right
			if(snake_direction != 3){//if the current direction is not left
				snake_direction = 1;
			}
		}
		else if(dir == 5){//left
			if(snake_direction != 1){// if the current direction is not right
				snake_direction = 3;
			}
		}
		else if( dir == 6){//up
			if(snake_direction != 2){// if the current direction is not down
				snake_direction = 0;
			}
		}
		else if(dir == 7){//down
			if(snake_direction != 0){// if the current direction is not up
				snake_direction = 2;
			}
		}
		unsigned char temp_size = snake_update(snake_size, snake_direction);//update the snake and the size
		if(temp_size > snake_size){
			LCD_DisplayString(1, "Current score:  ");
			if(temp_size < 21 ){
				LCD_WriteCommand(0xC0);
				LCD_WriteData('0' + temp_size - 11);
			}
			else if(temp_size < 31){
				LCD_WriteCommand(0xC0);
				LCD_WriteData('1');
				LCD_WriteData('0' + (temp_size - 10) - 11);
			}
			else{//should not go to this condition because the game end when reach length 25
				//LCD_WriteCommand(0xC0);
				//LCD_WriteData('2');
				//LCD_WriteData('0' + (temp_size - 20) - 11);
			}
		}
		if(temp_size == 100){
			state = LCD_Win;
			if(snake_size < 21 ){
				LCD_WriteCommand(0xC0);
				LCD_WriteData('0' + snake_size - 11);
			}
			else if(snake_size < 31){
				LCD_WriteCommand(0xC0);
				LCD_WriteData('1');
				LCD_WriteData('0' + (snake_size - 10) - 10);
			}
		}
		else if(temp_size == 200){
			state = LCD_Lost;
			if(snake_size < 21 ){
				LCD_WriteCommand(0xC0);
				LCD_WriteData('0' + snake_size - 11);
			}
			else if(snake_size < 31){
				LCD_WriteCommand(0xC0);
				LCD_WriteData('1');
				LCD_WriteData('0' + (snake_size - 10) - 11);
			}
		}
		else{
			snake_size = temp_size;
		}
		nokia_lcd_render();
		break;
		
		case LCD_Win://output the winning string
		nokia_lcd_clear();
		nokia_lcd_write_string("YOU WIN       But now snake needs to do   some exercise because eating too much...",1);
		nokia_lcd_render();
		state = LCD_wait2;
		break;
		
		case LCD_Lost://output the losing string
		nokia_lcd_clear();
		nokia_lcd_write_string("YOU LOSE      Snake's dream did not come  true......",1);
		nokia_lcd_render();
		state = LCD_wait2;
		break;
		
		case LCD_wait2://wait for the player to click the joystick 
		if(!GetBit(PINA, 4)){
			gameEnd = 1;
			state = LCD_Null;
		}
		
		case LCD_Null://handshake to turn off the gameend flag
		if(!gameFlag){
			state = LCD_wait;
			gameEnd = 0;
		}
		break;
		
		
		
		
		
		
		default:
			break;			
	}
	return state;
};



// Internal variables for mapping AVR's ISR to our cleaner TimerISR model.
unsigned long _avr_timer_M = 1; // Start count from here, down to 0. Default 1 ms.
unsigned long _avr_timer_cntcurr = 0; // Current internal count of 1ms ticks

void TimerOn() {
	// AVR timer/counter controller register TCCR1
	TCCR1B = 0x0B;// bit3 = 0: CTC mode (clear timer on compare)
	// bit2bit1bit0=011: pre-scaler /64
	// 00001011: 0x0B
	// SO, 8 MHz clock or 8,000,000 /64 = 125,000 ticks/s
	// Thus, TCNT1 register will count at 125,000 ticks/s

	// AVR output compare register OCR1A.
	OCR1A = 125;	// Timer interrupt will be generated when TCNT1==OCR1A
	// We want a 1 ms tick. 0.001 s * 125,000 ticks/s = 125
	// So when TCNT1 register equals 125,
	// 1 ms has passed. Thus, we compare to 125.
	// AVR timer interrupt mask register
	TIMSK1 = 0x02; // bit1: OCIE1A -- enables compare match interrupt

	//Initialize avr counter
	TCNT1=0;

	_avr_timer_cntcurr = _avr_timer_M;
	// TimerISR will be called every _avr_timer_cntcurr milliseconds

	//Enable global interrupts
	SREG |= 0x80; // 0x80: 1000000
}

void TimerOff() {
	TCCR1B = 0x00; // bit3bit1bit0=000: timer off
}

void TimerISR() {
	unsigned char i;
	for (i = 0; i < tasksNum; ++i) { // Heart of the scheduler code
		if ( tasks[i].elapsedTime >= tasks[i].period ) { // Ready
			tasks[i].state = tasks[i].TickFct(tasks[i].state);
			tasks[i].elapsedTime = 0;
		}
		tasks[i].elapsedTime += tasksPeriodGCD;
	}
}

// In our approach, the C programmer does not touch this ISR, but rather TimerISR()
ISR(TIMER1_COMPA_vect) {
	// CPU automatically calls when TCNT1 == OCR1 (every 1 ms per TimerOn settings)
	_avr_timer_cntcurr--; // Count down to 0 rather than up to TOP
	if (_avr_timer_cntcurr == 0) { // results in a more efficient compare
		TimerISR(); // Call the ISR that the user uses
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

// Set TimerISR() to tick every M ms
void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}




int main(void)
{
	DDRA = 0x03; PORTA = 0xFC;
	DDRC = 0xF0; PORTC = 0x0F;
	DDRD = 0xFF; PORTD = 0x00;
	unsigned char i = 0;
	adc_init();
	LCD_init();
	LCD_ClearScreen();          //Clear the LCD
	LCD_WriteCommand(0x88);
	
	
	
	tasks[i].state = LCD_SMStart;
	tasks[i].period = periodLCD_Display;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickFct_LCD_Display;
	++i;	
	tasks[i].state = DY_SMStart;
	tasks[i].period = periodDisplay;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickFct_Display;
	++i;
	tasks[i].state = SE_SMStart;
	tasks[i].period = periodSelect;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickFct_Select;

    TimerSet(tasksPeriodGCD);
    TimerOn();

	
	while (1)
    {
		   
		   
		   


    }
}

void adc_init()
{
	// AREF = AVcc
	ADMUX = (1<<REFS0);
	
	// ADC Enable and prescaler of 128
	// 16000000/128 = 125000
	ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
}

uint16_t adc_read(uint8_t ch)
{
	// select the corresponding channel 0~7
	// ANDing with f7Œ will always keep the value
	// of echf between 0 and 7
	ch &= 0b00000111;  // AND operation with 7
	ADMUX = (ADMUX & 0xF8)|ch; // clears the bottom 3 bits before ORing
	
	// start single convertion
	// write f1Œ to ADSC
	ADCSRA |= (1<<ADSC);
	
	// wait for conversion to complete
	// ADSC becomes f0Œ again
	// till then, run loop continuously
	while(ADCSRA & (1<<ADSC));
	
	return (ADC);
}
















