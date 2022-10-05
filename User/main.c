/*
IERG3810 project
Group 11
Welcome to Sokoban!!!
This game is made by GROUP 11
SID: 1155136168 Lai Chak Yan, Edward
SID: 1155144577 Chow Pak Ho, Ivan
*/

#include "IERG3810_LED.h"
#include "IERG3810_KEY.h"
#include "IERG3810_Buzzer.h"
#include "IERG3810_Clock.h"
#include "IERG3810_USART.h"
#include "FONT.h"
#include "CFONT.h"
#include "stm32f10x.h"
#include "global.h"

typedef struct
{
		u16 LCD_REG;
		u16 LCD_RAM;
}	LCD_TypeDef;


#define LCD_BASE      ((u32) (0x6c000000 | 0x000007FE))
#define LCD 					((LCD_TypeDef	*) LCD_BASE)
#define LCD_LIGHT_ON GPIOB->ODR=1<<0
#define DS0_ON GPIOB->BRR |= 1<<5;
#define DS0_OFF GPIOB->BSRR |= 1<<5;
#define DS1_ON GPIOE->BRR |= 1<<5;
#define DS1_OFF GPIOE->BSRR |= 1<<5;
#define DS1_TOGGLE GPIOE->ODR ^= 1<<5;
#define DS0_TOGGLE GPIOB->ODR ^= 1<<5;
#define LED0_PWM_VAL TIM3->CCR2

#define EMPTY 0
#define WALL 1
#define BOX 2
#define TARGET 3

#define GAME_ON 1
#define GAME_LOSE 2
#define GAME_WIN 3
#define GAME_MENU 4
#define GAME_INT 5
#define GAME_LOAD 6

#define ON 1
#define OFF 0

#define UP 8
#define DOWN 2
#define LEFT 4
#define RIGHT 6

#define MAP_HEIGHT 12
#define MAP_WEIGHT 12

void IERG3810_TFTLCD_Init(void);
void IERG3810_TFTLCD_SetParameter(void);
void IERG3810_TFTLCD_WrReg(u16);
void IERG3810_TFTLCD_WrData(u16);
void IERG3810_TFTLCD_DrawDot(u16, u16, u16);
void IERG3810_TFTLCD_FillRectangle(u16, u16, u16, u16, u16);
void IERG3810_TFTLCD_SevenSegment(u16, u16, u16, u8);
void IERG3810_TFTLCD_ShowChar(u16, u16, u8, u16, u16);
void IERG3810_TFTLCD_ShowChinChar(u16, u16, u8, u16, u16);
void IERG3810_TFTLCD_ShowChar_Overlay(u16, u16, u8, u16);
void IERG3810_TFTLCD_ShowChinChar_Overlay(u16, u16, u8, u16);
void IERG3810_NVIC_SetPriorityGroup(u8);
void IERG3810_key2_ExtiInit(void);
void EXTI2_IRQHandler(void);
void IERG3810_keyUP_ExtiInit(void);
void EXTI0_IRQHandler(void);
void Delay(u32);
void IERG3810_PS2key_ExtiInit(void);
void IERG3810_TIM3_Init(u16, u16);
void TIM3_IRQHandler(void);
void IERG3810_TIM4_Init(u16, u16);
void TIM4_IRQHandler(void);
void IERG3810_SYSTICK_Init10ms(void);
void IERG3810_TIM3_PwmInit(u16, u16);

void init_game(void);
void init_map(void);
void update_map(void);
void clear_screen(u16);
void draw_block(u8, u8, u8);
void PS2key_control(u8);
void handle_player_control(u8, u8, u8);
void update_timer(void);
void update_score(void);
void update_banner(void);
void check_point(void);
void update_low_banner(void);
void IERG3810_PrintInstrut(void);
void IERG3810_PrintMenu(void);
void start_game(u8);

u32 sheep=0;
u32 pa2count = 0;
u32 timeout = 20000;
u32 pa2key = 0;
u8 task1HeartBeat = 0;
u8 task2HeartBeat = 0;
u8 i;
u8 j;
u8 u;

u8 timer_d1 = 0;
u8 timer_d2 = 0;

u8 game_timer = 0;
u8 curr_game_timer = 0;
u8 prev_game_timer = 0;

char *words_banner_ON = "PRESS 0 to PAUSE";
char *words_banner_OFF = "PRESS 0 to REUME";
char *words_banner_END = "PRESS 0 to MENU";
char *words_banner_WIN = "CONGRATULATIONS!!!";
char *words_banner_LOSE = "GAME OVER!!!";
char *words_score = "SCORE:";
char *words_time = "TIME: ";
char *words_nums = "0123456789";
char* Gp = "Gp11";

char* ins0 = "Instrustion Page       ";
char* ins1 = "1.It is a box push game";
char* ins2 = "2.Player need to push  ";
char* ins2_1 = "  boxes to targets   ";
char* ins3 = "3.Once all boxes are   ";
char* ins3_1 = "  pushed to targets  ";
char* ins3_2 = "  the game finished  ";
char* ins4 = "4.There are 3 levels   ";
char* ins4_1 = "  in the game to let ";
char* ins4_2 = "  player choose      ";
char* ins5 = "5.Player can press key2";
char* ins5_1 = "  to return menu     ";
char* ins6 = "6.After player finished";	
char* ins6_1 = "  the easy level,they";
char* ins6_2 = "  can leave the game ";
char* ins6_3 = "  or go to next level";
char* sid1 = "1155136168";
char* sid2 = "1155144577";
char* wel = "Welcome to Sokoban!";
char* ins = "Press 5 to instruction!";
char* play = "Press 123 to select lv!";

u8 init_player_p[2];
u8 prev_player_p[2];
u8 curr_player_p[2];
u8 GAME_STATUS = 0;
u8 TIMER_STATUS = 0;
u8 FULL_POINT = 0;
u8 curr_point = 0;
u8 tmp_point = 0;
u8 prev_point = 0;
u8 select_map = 0;

u8 first_map[12][12] = {{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
												{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
												{1, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0, 1},
												{1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1},
												{1, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 1},
												{1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1},
												{1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1},
												{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
												{1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1},
												{1, 0, 0, 0, 1, 0, 0, 2, 0, 0, 0, 1},
												{1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1},
												{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};												
												
u8 first_target_map[12][12] = 	
												{{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
												{0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
												{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
												{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
												{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
												{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
												{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
												{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
												{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
												{0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0},
												{0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0},
												{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};
												

u8 second_map[12][12] = {{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
												{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
												{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
												{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1},
												{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1},
												{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1},
												{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1},
												{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1},
												{1, 0, 0, 2, 0, 1, 0, 2, 0, 0, 0, 1},
												{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
												{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
												{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};												
												
u8 second_target_map[12][12] = 	
												{{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
												{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
												{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
												{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
												{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
												{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
												{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
												{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
												{0, 0, 0, 0, 3, 0, 3, 0, 0, 0, 0, 0},
												{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
												{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
												{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};
												
u8 third_map[12][12] = {{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
												{1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1},
												{1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1},
												{1, 0, 2, 2, 0, 1, 0, 0, 0, 1, 0, 1},
												{1, 0, 2, 0, 0, 1, 0, 0, 0, 1, 0, 1},
												{1, 1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 1},
												{1, 1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 1},
												{1, 1, 1, 0, 0, 1, 0, 1, 1, 1, 0, 1},
												{1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
												{1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 1},
												{1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 1},
												{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};												
												
u8 third_target_map[12][12] = 	
												{{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
												{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0},
												{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0},
												{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0},
												{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
												{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
												{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
												{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
												{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
												{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
												{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
												{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};

u8 target_p1[10][2] = {{1, 1}, {10, 2}, {9, 5}};
u8 target_p2[10][2] = {{8, 4}, {8, 6}};
u8 target_p3[10][2] = {{1, 10}, {2, 10}, {3, 10}};
												
u8 prev_map[12][12];
u8 curr_map[12][12];
u8 curr_target[12][12];

int main(void)
{
		IERG3810_clock_tree_init();
		IERG3810_USART2_init(36, 9600);
    IERG3810_USART1_init(72, 9600);

		IERG3810_TFTLCD_Init();
    IERG3810_LED_INIT();
		IERG3810_NVIC_SetPriorityGroup(5);
		
		IERG3810_PS2key_ExtiInit();
		IERG3810_key2_ExtiInit();
	
		IERG3810_TIM3_Init(1249, 7199); //1hz (72M/7200/5000/2)
		//IERG3810_TIM4_Init(9999, 7199); //4hz (72M/7200/1250/2)
		IERG3810_SYSTICK_Init10ms();
		
		USART_print(2,"Welcome to Sokoban!!!\n");
		USART_print(2,"This game is made by GROUP 11\n");
		USART_print(2,"SID: 1155136168 Lai Chak Yan, Edward\n");
		USART_print(2,"SID: 1155144577 Chow Pak Ho, Ivan\n");
	
		clear_screen(0x0000);
		Delay(50000);
		//IERG3810_PrintMenu();
		init_game();
	
		while(1)
		{
			
			if (GAME_STATUS == GAME_ON&&TIMER_STATUS == ON) {
				if (task1HeartBeat >= 85) {
					task1HeartBeat = 0;
					if (curr_game_timer != 0) {
						curr_game_timer--;
					}
					if (curr_game_timer == 0) {
						update_timer();
						GAME_STATUS = GAME_LOSE;
						update_banner();
						update_low_banner();
						USART_print(2,"Player lose!!!\n");
					}
				}
			}
			
			if (pa2count >= 11) {
				EXTI->PR = 1<<11;
			}
			timeout--;
			if (timeout == 0) {
				timeout = 2000;
				pa2key = 0;
				pa2count = 0;
			}
		}
}


void clear_screen(u16 color) {
	IERG3810_TFTLCD_FillRectangle(color, 0, 240, 0, 320);
}


void init_game() {
	GAME_STATUS = GAME_MENU;
	IERG3810_PrintMenu();
	DS1_OFF;
	DS0_OFF;
}

void start_game(u8 map){
	USART_print(2,"GAME START\n");
	clear_screen(0x0000);
	select_map = map;
	GAME_STATUS = GAME_ON;
	TIMER_STATUS = ON;
	
	switch (map) {
		case 1:
			FULL_POINT = 2;
			game_timer = 40;
			// prepare player data
			// player x position
			init_player_p[0] = 5;
			// player y position
			init_player_p[1] = 2;
			break;
		case 2:
			FULL_POINT = 3;
			game_timer = 45;
			init_player_p[0] = 2;
			init_player_p[1] = 4;
			break;
		case 3:
			FULL_POINT = 3;
			game_timer = 99;
			init_player_p[0] = 1;
			init_player_p[1] = 3;
			break;
	}
	
	curr_game_timer = game_timer;
	prev_game_timer = game_timer;
	
	curr_point = 0;
	prev_point = 0;
	
	prev_player_p[0] = init_player_p[0];
	prev_player_p[1] = init_player_p[1];
	curr_player_p[0] = init_player_p[0];
	curr_player_p[1] = init_player_p[1];
	
	// prepare map data
	for (i = 0; i < MAP_HEIGHT; i++) {
		for (j = 0; j < MAP_WEIGHT; j++) {
			switch (map) {
				case 1:
					prev_map[i][j] = second_map[i][j];
					curr_map[i][j] = second_map[i][j];
					curr_target[i][j] = second_target_map[i][j];
					break;
				case 2:
					prev_map[i][j] = first_map[i][j];
					curr_map[i][j] = first_map[i][j];
					curr_target[i][j] = first_target_map[i][j];
					break;
				case 3:
					prev_map[i][j] = third_map[i][j];
					curr_map[i][j] = third_map[i][j];
					curr_target[i][j] = third_target_map[i][j];
					break;
			}
		}
	}
	
	init_map();
}

void init_map() {
	//clear screen
	for (i = 0; i < MAP_HEIGHT; i++) {
		for (j = 0; j < MAP_WEIGHT; j++) {
			if (curr_map[i][j] == WALL) {
				IERG3810_TFTLCD_FillRectangle(0xBCCB, 0+(20*j), 20, 280-(20*(i+1)), 20);
				IERG3810_TFTLCD_FillRectangle(0x07FF, 0+(20*j), 20, 285-(20*(i+1)), 2);
				IERG3810_TFTLCD_FillRectangle(0x07FF, 0+(20*j), 20, 295-(20*(i+1)), 2);
				IERG3810_TFTLCD_FillRectangle(0x07FF, 0+(20*j)+5, 2, 280-(20*(i+1)), 20);
				IERG3810_TFTLCD_FillRectangle(0x07FF, 0+(20*j)+15, 2, 280-(20*(i+1)), 20);
			} else if (curr_map[i][j] == BOX) {
				IERG3810_TFTLCD_FillRectangle(0x7F1F, 0+(20*j), 20, 280-(20*(i+1)), 20);
			} else if (j == init_player_p[0]&&i == init_player_p[1]) {
				IERG3810_TFTLCD_FillRectangle(0xFF46, 0+(20*j), 20, 280-(20*(i+1)), 20);
			} else if (curr_target[i][j] == TARGET) {
				IERG3810_TFTLCD_FillRectangle(0xFC10, 0+(20*j), 20, 280-(20*(i+1)), 20);
				for (u = 0; u < 20; u++) {
					IERG3810_TFTLCD_DrawDot(0+(20*j)+u, 280-(20*(i+1))+u, 0x0000);
					IERG3810_TFTLCD_DrawDot(0+(20*j)+20-u, 280-(20*(i+1))+u, 0x0000);
				}
			} else {
				IERG3810_TFTLCD_FillRectangle(0xFFFF, 0+(20*j), 20, 280-(20*(i+1)), 20);
			}
		}
	}
	
	IERG3810_TFTLCD_FillRectangle(0x0000, 0, 240, 280, 40);
	IERG3810_TFTLCD_FillRectangle(0x0000, 0, 240, 0, 40);
	
	//banner
	for (i = 0; i < 16; i++) {
		IERG3810_TFTLCD_ShowChar_Overlay(50+10*i, 290, words_banner_ON[i], 0xFFFF);
	}
	
	for (i = 0; i < 6; i++) {
		IERG3810_TFTLCD_ShowChar_Overlay(10+10*i, 10, words_score[i], 0xFFFF);
		IERG3810_TFTLCD_ShowChar_Overlay(30+120+10*i, 10, words_time[i], 0xFFFF);
	}
	
	//score
	IERG3810_TFTLCD_ShowChar_Overlay(10+10*7, 10, words_nums[0], 0xFFFF);
	
	//time
	timer_d1 = curr_game_timer/10;
	timer_d2 = curr_game_timer%10;
	IERG3810_TFTLCD_ShowChar_Overlay(30+120+10*6, 10, words_nums[timer_d1], 0xFFFF);
	IERG3810_TFTLCD_ShowChar_Overlay(30+120+10*7, 10, words_nums[timer_d2], 0xFFFF);
}


void update_map() {
	//update map object
	for (i = 0; i < MAP_HEIGHT; i++) {
		for (j = 0; j < MAP_WEIGHT; j++) {
			if (curr_map[i][j] != prev_map[i][j]) {
				
				draw_block(curr_map[i][j], i, j);
				
				//update prev_map
				prev_map[i][j] = curr_map[i][j];
			}
		}
	}
	
	//update player
	if (curr_player_p[0] != prev_player_p[0] || curr_player_p[1] != prev_player_p[1]) {
		draw_block(curr_map[prev_player_p[1]][prev_player_p[0]], prev_player_p[1], prev_player_p[0]);
		IERG3810_TFTLCD_FillRectangle(0xFF46, 0+(20*curr_player_p[0]), 20, 280-(20*(curr_player_p[1]+1)), 20);
		
		// update prev_player_p
		prev_player_p[0] = curr_player_p[0];
		prev_player_p[1] = curr_player_p[1];
		

		GPIOE->ODR ^= 1<<5;
	}
}


void update_timer() {
	if (curr_game_timer != prev_game_timer) {
		IERG3810_TFTLCD_FillRectangle(0x0000, 206, 36, 0, 40);
		timer_d1 = curr_game_timer/10;
		timer_d2 = curr_game_timer%10;
		IERG3810_TFTLCD_ShowChar_Overlay(30+120+10*6, 10, words_nums[timer_d1], 0xFFFF);
		IERG3810_TFTLCD_ShowChar_Overlay(30+120+10*7, 10, words_nums[timer_d2], 0xFFFF);
		prev_game_timer = curr_game_timer;
	}
}


void update_score() {
	if (curr_point != prev_point) {
		IERG3810_TFTLCD_FillRectangle(0x0000, 80, 36, 0, 40);
		IERG3810_TFTLCD_ShowChar_Overlay(10+10*7, 10, words_nums[curr_point], 0xFFFF);
		prev_point = curr_point;
	}
}


void update_banner() {
	IERG3810_TFTLCD_FillRectangle(0x0000, 0, 240, 280, 40);
	if (GAME_STATUS == GAME_ON) {
		if (TIMER_STATUS == ON) {
			for (i = 0; i < 16; i++) {
				IERG3810_TFTLCD_ShowChar_Overlay(50+10*i, 290, words_banner_ON[i], 0xFFFF);
			}
		} else if (TIMER_STATUS == OFF) {
			for (i = 0; i < 17; i++) {
				IERG3810_TFTLCD_ShowChar_Overlay(50+10*i, 290, words_banner_OFF[i], 0xFFFF);
			}
		}
	}
	if (GAME_STATUS == GAME_WIN||GAME_STATUS == GAME_LOSE) {
		for (i = 0; i < 16; i++) {
			IERG3810_TFTLCD_ShowChar_Overlay(50+10*i, 290, words_banner_END[i], 0xFFFF);
		}
	}
}


void update_low_banner() {
	IERG3810_TFTLCD_FillRectangle(0x0000, 0, 240, 0, 40);
	if (GAME_STATUS == GAME_WIN) {
		for (i = 0; i < 18; i++) {
			IERG3810_TFTLCD_ShowChar_Overlay(20+10*i, 10, words_banner_WIN[i], 0xFFE0);
		}
	}
	if (GAME_STATUS == GAME_LOSE) {
		for (i = 0; i < 13; i++) {
			IERG3810_TFTLCD_ShowChar_Overlay(50+10*i, 10, words_banner_LOSE[i], 0xF800);
		}
	}
	
}


void draw_block(u8 block, u8 i, u8 j) {
	if (block == WALL) {
		IERG3810_TFTLCD_FillRectangle(0xBCCB, 0+(20*j), 20, 280-(20*(i+1)), 20);
		IERG3810_TFTLCD_FillRectangle(0x07FF, 0+(20*j), 20, 285-(20*(i+1)), 2);
		IERG3810_TFTLCD_FillRectangle(0x07FF, 0+(20*j), 20, 295-(20*(i+1)), 2);
		IERG3810_TFTLCD_FillRectangle(0x07FF, 0+(20*j)+5, 2, 280-(20*(i+1)), 20);
		IERG3810_TFTLCD_FillRectangle(0x07FF, 0+(20*j)+15, 2, 280-(20*(i+1)), 20);
	} else if (block == BOX) {
		IERG3810_TFTLCD_FillRectangle(0x7F1F, 0+(20*j), 20, 280-(20*(i+1)), 20);
	} else {
		IERG3810_TFTLCD_FillRectangle(0xFFFF, 0+(20*j), 20, 280-(20*(i+1)), 20);
	}
	
	if (block == BOX && curr_target[i][j] == TARGET) {
		IERG3810_TFTLCD_FillRectangle(0xFD20, 0+(20*j), 20, 280-(20*(i+1)), 20);
	} else if (curr_target[i][j] == TARGET) {
		IERG3810_TFTLCD_FillRectangle(0xFC10, 0+(20*j), 20, 280-(20*(i+1)), 20);
		for (u = 0; u < 20; u++) {
			IERG3810_TFTLCD_DrawDot(0+(20*j)+u, 280-(20*(i+1))+u, 0x0000);
			IERG3810_TFTLCD_DrawDot(0+(20*j)+20-u, 280-(20*(i+1))+u, 0x0000);
		}
	}
}


void check_point() {
	tmp_point = 0;
	for (i = 0; i < FULL_POINT; i++) {
		switch (select_map) {
				case 1:
					if (curr_map[target_p2[i][0]][target_p2[i][1]] == BOX) {
						tmp_point++;
					}
					break;
				case 2:
					if (curr_map[target_p1[i][0]][target_p1[i][1]] == BOX) {
						tmp_point++;
					}
					break;
				case 3:
					if (curr_map[target_p3[i][0]][target_p3[i][1]] == BOX) {
						tmp_point++;
					}
					break;
			}
		
	}
	curr_point = tmp_point;
	if (curr_point == FULL_POINT) {
		GAME_STATUS = GAME_WIN;
		update_map();
		update_banner();
		update_low_banner();
		USART_print(2,"Player win!!!\n");
	}
}



void TIM3_IRQHandler(void)
{
    if (TIM3->SR & 1<<0)	//To check is the register are updated? (hardware set update interrupt pending bit)
    {
        //GPIOB->ODR ^= 1<<5; //toggle DS0 with read-modify-write
			if (GAME_STATUS == GAME_ON&&TIMER_STATUS == ON) {
				update_map();
				update_timer();
				update_score();
			}
			if (GAME_STATUS == GAME_WIN) {
				DS0_OFF;
				GPIOE->ODR ^= 1<<5;	
			}
			if (GAME_STATUS == GAME_LOSE) {
				DS1_OFF;
				GPIOB->ODR ^= 1<<5;
			}
    }
    TIM3->SR &= ~(1<<0); //clear the update interrupt flag (no update occurred)
}


void EXTI15_10_IRQHandler(void) 
{
	pa2count++;
	if (GPIOC->IDR & (1<<10) && pa2count>1) { // read data from PC10 and receive bit
		pa2key |= 1 << (pa2count-2); //collect data
	}
	if (pa2count == 11) { //finished receive
		pa2key &= 0x000000FF;
		switch(pa2key) {
			case 0x70:
				USART_print(1,"key 0 ");
				PS2key_control(0);
				Delay(500);
				break;
			case 0x69: //KP 1
				USART_print(1,"key 1 ");
				PS2key_control(1);
				Delay(500);
				break;
			case 0x72: //KP 2 #down
				USART_print(1,"key 2 ");
				PS2key_control(2);
				Delay(500);
				break;
			case 0x7A: //KP 3
				USART_print(1,"key 3 ");
				PS2key_control(3);
				Delay(500);
				break;
			case 0x6B: //KP 4 #left
				USART_print(1,"key 4 ");
				PS2key_control(4);
				Delay(500);
				break;
			case 0x73: //KP 5
				USART_print(1,"key 4 ");
				PS2key_control(5);
				Delay(500);
				break;
			case 0x74: //KP 6 #right
				USART_print(1,"key 6 ");
				PS2key_control(6);
				Delay(500);
				break;
			case 0x6C: //KP 7
				USART_print(1,"key 7 ");
				PS2key_control(7);
				Delay(500);
				break;
			case 0x75: //KP 8 #up
				USART_print(1,"key 8 ");
				PS2key_control(8);
				Delay(1000);
				break;
			case 0x7D: //KP 9
				USART_print(1,"key 9 ");
				PS2key_control(9);
				Delay(500);
				break;
			case 0x71: //KP . 
				USART_print(1,"key . ");
				PS2key_control(11);
				Delay(500);
				break;
		}
	}
	Delay(10);
	EXTI->PR = 1<<11;
}


void PS2key_control(u8 key) {
	if (GAME_STATUS == GAME_ON&&TIMER_STATUS == ON) {
		switch(key) {
			case DOWN: //down
				handle_player_control(curr_player_p[1]+1, curr_player_p[0], DOWN);
				break;
			case LEFT: //left
				handle_player_control(curr_player_p[1], curr_player_p[0]-1, LEFT);
				break;
			case RIGHT: //right
				handle_player_control(curr_player_p[1], curr_player_p[0]+1, RIGHT);
				break;
			case UP: //up
				handle_player_control(curr_player_p[1]-1, curr_player_p[0], UP);
				break;
			case 0:
				TIMER_STATUS = OFF;
				update_banner();
				break;
		}
	} else if (GAME_STATUS == GAME_ON&&TIMER_STATUS == OFF) {
		switch(key) {
			case 0:
				TIMER_STATUS = ON;
				update_banner();
				break;
		}
	}
	if (GAME_STATUS == GAME_WIN || GAME_STATUS == GAME_LOSE ) {
		switch(key) {
			case 0:
				init_game();
				break;
		}
	}
	if (GAME_STATUS == GAME_MENU) {
		switch(key) {
			case 5:
				GAME_STATUS = GAME_INT;
				clear_screen(0x0000);
				IERG3810_PrintInstrut();
				break;
			case 1:
				start_game(1);
				break;
			case 2:
				start_game(2);
				break;
			case 3:
				start_game(3);
				break;
		}
	}
	if (GAME_STATUS == GAME_INT) {
		switch(key) {
			case 11:
				init_game();
				break;
		}
	}
}

void handle_player_control(u8 i, u8 j, u8 move) {
	if (curr_map[i][j] != WALL) {
		if (curr_map[i][j] == BOX) {
			switch(move) {
				case DOWN:
					if (curr_map[i+1][j] != WALL&&curr_map[i+1][j] != BOX) {
						curr_map[i][j] = EMPTY;
						curr_map[i+1][j] = BOX;
						curr_player_p[1] += 1;
						check_point();
					}
				break;
				case LEFT:
					if (curr_map[i][j-1] != WALL&&curr_map[i][j-1] != BOX) {
						curr_map[i][j] = EMPTY;
						curr_map[i][j-1] = BOX;
						curr_player_p[0] -= 1;
						check_point();
					}
				break;
				case RIGHT:
					if (curr_map[i][j+1] != WALL&&curr_map[i][j+1] != BOX) {
						curr_map[i][j] = EMPTY;
						curr_map[i][j+1] = BOX;
						curr_player_p[0] += 1;
						check_point();
					}
				break;
				case UP:
					if (curr_map[i-1][j] != WALL&&curr_map[i-1][j] != BOX) {
						curr_map[i][j] = EMPTY;
						curr_map[i-1][j] = BOX;
						curr_player_p[1] -= 1;
						check_point();
					}
				break;
			}
		} else {
		switch(move) {
				case DOWN:
					curr_player_p[1] += 1;
				break;
				case LEFT:
					curr_player_p[0] -= 1;
				break;
				case RIGHT:
					curr_player_p[0] += 1;
				break;
				case UP:
					curr_player_p[1] -= 1;
				break;
			}
		
	}
	} 
}


void IERG3810_PrintMenu(){
    IERG3810_TFTLCD_FillRectangle(0xFFE0, 0, 320, 0, 480);
    IERG3810_TFTLCD_FillRectangle(0x001F, 60, 120, 130, 120);
    IERG3810_TFTLCD_FillRectangle(0x07E0, 70, 100, 140, 100);
    IERG3810_TFTLCD_FillRectangle(0xF800, 90, 15, 160, 15);
    IERG3810_TFTLCD_FillRectangle(0xF800, 90, 15, 190, 15);
    IERG3810_TFTLCD_FillRectangle(0x0000, 110, 15, 160, 15);
    IERG3810_TFTLCD_FillRectangle(0x0000, 110, 15, 190, 15);
    IERG3810_TFTLCD_FillRectangle(0x0000, 110, 15, 190, 15);
    IERG3810_TFTLCD_FillRectangle(0xFFFF, 150, 15, 210, 15);
    IERG3810_TFTLCD_FillRectangle(0x0000, 158, 4, 220, 4);
    IERG3810_TFTLCD_FillRectangle(0x0000, 152, 4, 220, 4);
    IERG3810_TFTLCD_FillRectangle(0x0000, 152, 6, 215, 4);

    for (i = 1 ; i <= 23; i++) {
            IERG3810_TFTLCD_ShowChar_Overlay(10*(i), 80, play[i-1], 0xF800);
    }
    for (i = 1 ; i <= 23; i++) {
            IERG3810_TFTLCD_ShowChar_Overlay(10*(i), 110, ins[i-1], 0xF800);
    }
    for (i = 1 ; i <= 20; i++) {
            IERG3810_TFTLCD_ShowChar_Overlay(10*(i+2), 300, wel[i-1], 0x0000);
    }

    for (i = 1 ; i <= 10; i++) {

            IERG3810_TFTLCD_ShowChar_Overlay(10*i, 10, sid1[i-1], 0x0000);
            IERG3810_TFTLCD_ShowChar_Overlay(10*i, 30, sid2[i-1], 0x0000);
    }
    for (i = 1 ; i < 6; i++) {
        IERG3810_TFTLCD_ShowChar_Overlay(170+10*i, 20, Gp[i-1], 0x0000);
    }

    for (i = 0 ; i < 3; i++) {
            IERG3810_TFTLCD_ShowChinChar_Overlay(110+20*i, 10, i+3, 0x0000);
            IERG3810_TFTLCD_ShowChinChar_Overlay(110+20*i, 30, i, 0x0000);

    }

}

void IERG3810_PrintInstrut(){
	IERG3810_TFTLCD_FillRectangle(0xFFE0, 0, 320, 0, 480);
	for (i = 1 ; i <= 23; i++) {			 
		IERG3810_TFTLCD_ShowChar_Overlay(10*(i+3), 300, ins0[i-1], 0xF800);		 
		IERG3810_TFTLCD_ShowChar_Overlay(10*(i), 280, ins1[i-1], 0x0000);			 
		IERG3810_TFTLCD_ShowChar_Overlay(10*(i), 260, ins2[i-1], 0x0000);		 
		IERG3810_TFTLCD_ShowChar_Overlay(10*(i), 240, ins2_1[i-1], 0x0000);
		IERG3810_TFTLCD_ShowChar_Overlay(10*(i), 220, ins3[i-1], 0x0000);			
		IERG3810_TFTLCD_ShowChar_Overlay(10*(i), 200, ins3_1[i-1], 0x0000);
		IERG3810_TFTLCD_ShowChar_Overlay(10*(i), 180, ins3_2[i-1], 0x0000);
		IERG3810_TFTLCD_ShowChar_Overlay(10*(i), 160, ins4[i-1], 0x0000);
		IERG3810_TFTLCD_ShowChar_Overlay(10*(i), 140, ins4_1[i-1], 0x0000);
		IERG3810_TFTLCD_ShowChar_Overlay(10*(i), 120, ins4_2[i-1], 0x0000);
		IERG3810_TFTLCD_ShowChar_Overlay(10*(i), 100, ins5[i-1], 0x0000);
		IERG3810_TFTLCD_ShowChar_Overlay(10*(i), 80, ins5_1[i-1], 0x0000);
		IERG3810_TFTLCD_ShowChar_Overlay(10*(i), 60, ins6[i-1], 0x0000);
		IERG3810_TFTLCD_ShowChar_Overlay(10*(i), 40, ins6_1[i-1], 0x0000);
		IERG3810_TFTLCD_ShowChar_Overlay(10*(i), 20, ins6_2[i-1], 0x0000);	
		IERG3810_TFTLCD_ShowChar_Overlay(10*(i), 0 , ins6_3[i-1], 0x0000);						
	}
}

void IERG3810_PS2key_ExtiInit(void)
{
		//PS2key - PC11
    RCC->APB2ENR |= 1<<4; //enable port C clock
    GPIOC->CRH &= 0xFFFF0FFF; //init port C pin 11
    GPIOC->CRH |= 0x00008000; // set pin 11 input mode with pull up/pull down
    GPIOC->ODR |= 1<<11; //pull high
		// configure interrupt source
    RCC->APB2ENR |= 0x01; //enable alternat function IO clock
    AFIO->EXTICR[2] &= 0xFFFF0FFF; //init EXTI11
    AFIO->EXTICR[2] |= 0x00002000; //Select PC11 pin as EXIT11 input source (0010)
    EXTI->IMR |= 1<<11; // eadge trigger, configure mask bit 11 to interrupt mask on line 11(EXTI0 line 0 interrupt)
    EXTI->FTSR |= 1<<11; //rising trigger event configuration bit on line 11
    
		NVIC->IP[40] = 0x20; // set priority to 0x65
		NVIC->ISER[1] &= ~(1<<8); // set interrupt position 40 enable IRQ40
    NVIC->ISER[1] |= (1<<8); //set interrupt position 40 enable IRQ40
}


void IERG3810_key2_ExtiInit(void)
{
    RCC->APB2ENR |= 1<<6;
    GPIOE->CRL &= 0xFFFFF0FF;
    GPIOE->CRL |= 0x00000800;
    GPIOE->ODR |= 1<<2;
    RCC->APB2ENR |= 0x01;
    AFIO->EXTICR[0] &= 0xFFFFF0FF;
    AFIO->EXTICR[0] |= 0x00000400;
    EXTI->IMR |= 1<<2;
    EXTI->FTSR |= 1<<2; //falling trigger event configuration bit on lin 3

    NVIC->IP[8] = 0x65;
	
		NVIC->ISER[0] &= ~(1<<8);
    NVIC->ISER[0] |= (1<<8);
}


void EXTI2_IRQHandler(void)
{
    init_game();
    EXTI->PR = 1<<2;
}


void IERG3810_NVIC_SetPriorityGroup(u8 prigroup)
{
    u32 temp,temp1;
    temp1 = prigroup & 0x00000007;
    temp1 <<=8;
    temp = SCB->AIRCR;
    temp &= 0x0000F8FF;
    temp |= 0x05FA0000;
    temp |= temp1;
    SCB->AIRCR = temp;
}


void Delay(u32 count)
{
	u32 i;
	for (i=0; i<count; i++);
}


void IERG3810_TFTLCD_Init(void)
{
    RCC->AHBENR|=1<<8;            //FSMC
    RCC->APB2ENR|=1<<3;            //PORTB
    RCC->APB2ENR|=1<<5;            //PORTD
    RCC->APB2ENR|=1<<6;            //PORTE
    RCC->APB2ENR|=1<<8;            //PORTG
    GPIOB->CRL&=0xFFFFFFF0;    //PB0
    GPIOB->CRL|=0x00000003;

    //PORTD
    GPIOD->CRH&=0x00FFF000;
    GPIOD->CRH|=0xBB000BBB;
    GPIOD->CRL&=0xFF00FF00;
    GPIOD->CRL|=0x00BB00BB;
    //PORTE
    GPIOE->CRH&=0x00000000;
    GPIOE->CRH|=0xBBBBBBBB;
    GPIOE->CRL&=0x0FFFFFFF;
    GPIOE->CRL|=0xB0000000;
    //PORTG12
    GPIOG->CRH&=0xFFF0FFFF;
    GPIOG->CRH|=0x000B0000;
    GPIOG->CRL&=0xFFFFFFF0;    //PGO->RS
    GPIOG->CRL|=0x0000000B;

    //LCD uses FSMC Bank 4 memory bank
    //use mode A
    FSMC_Bank1->BTCR[6]=0x00000000;        //FSMC_BCR4 (reset)
    FSMC_Bank1->BTCR[7]=0x00000000;        //FSMC_BTR4 (reset)
    FSMC_Bank1E->BWTR[6]=0x00000000;    //FSMC_BWTR4(reset)
    FSMC_Bank1->BTCR[6]|=1<<12;                //FSMC_BCR4->WREN
    FSMC_Bank1->BTCR[6]|=1<<14;                //FSMC_BCR4->EXTMOD
    FSMC_Bank1->BTCR[6]|=1<<4;                //FSMC_BCR4->MWID
    FSMC_Bank1->BTCR[7]|=0<<28;                //FSMC_BTR4->ACCMOD
    FSMC_Bank1->BTCR[7]|=1<<0;                //FSMC_BTR4->ADDSET
    FSMC_Bank1->BTCR[7]|=0xF<<8;            //FSMC_BTR4->DATAST
    FSMC_Bank1E->BWTR[6]|=0<<28;            //FSMC_BWTR4->ACCMOD
    FSMC_Bank1E->BWTR[6]|=0<<0;                //FSMC_BWTR4->ADDSET
    FSMC_Bank1E->BWTR[6]|=3<<8;                //FSMC_BWTR4->DATAST
    FSMC_Bank1->BTCR[6]|=1<<0;                //FSMC_BCR4->FACCEN
    IERG3810_TFTLCD_SetParameter();        //special setting for LCD module
    LCD_LIGHT_ON;
}


void IERG3810_TFTLCD_SetParameter(void)
{
    IERG3810_TFTLCD_WrReg(0x01);        //software reset
    IERG3810_TFTLCD_WrReg(0x11);        //exit sleep mode

    IERG3810_TFTLCD_WrReg(0x3A);        //set pixel format
    IERG3810_TFTLCD_WrData(0x55);        //66536 colors

    IERG3810_TFTLCD_WrReg(0x29);        //display on

    IERG3810_TFTLCD_WrReg(0x36);        //memory access control
    IERG3810_TFTLCD_WrData(0xCA);
}


void IERG3810_TFTLCD_WrReg(u16 regval)
{
    LCD->LCD_REG=regval;
}


void IERG3810_TFTLCD_WrData(u16 data)
{
    LCD->LCD_RAM=data;
}


void IERG3810_TFTLCD_DrawDot(u16 x, u16 y, u16 color)
{
    IERG3810_TFTLCD_WrReg(0x2A);        //set x position
        IERG3810_TFTLCD_WrData(x>>8);
        IERG3810_TFTLCD_WrData(x & 0xFF);
        IERG3810_TFTLCD_WrData(0x01);
        IERG3810_TFTLCD_WrData(0x3F);
    IERG3810_TFTLCD_WrReg(0x2B);        //set y position
        IERG3810_TFTLCD_WrData(y>>8);
        IERG3810_TFTLCD_WrData(y & 0xFF);
        IERG3810_TFTLCD_WrData(0x01);
        IERG3810_TFTLCD_WrData(0xDF);
    IERG3810_TFTLCD_WrReg(0x2C);        //set point with color
    IERG3810_TFTLCD_WrData(color);
}


void IERG3810_TFTLCD_FillRectangle(u16 color, u16 start_x, u16 length_x, u16 start_y, u16 length_y)
{
    u32 index=0;
    IERG3810_TFTLCD_WrReg(0x2A);
        IERG3810_TFTLCD_WrData(start_x>>8);
        IERG3810_TFTLCD_WrData(start_x & 0xFF);
        IERG3810_TFTLCD_WrData((length_x + start_x -1)>>8);
        IERG3810_TFTLCD_WrData((length_x + start_x -1)& 0xFF);
    IERG3810_TFTLCD_WrReg(0x2B);
        IERG3810_TFTLCD_WrData(start_y>>8);
        IERG3810_TFTLCD_WrData(start_y & 0xFF);
        IERG3810_TFTLCD_WrData((length_y + start_y -1)>>8);
        IERG3810_TFTLCD_WrData((length_y + start_y -1)& 0xFF);
    IERG3810_TFTLCD_WrReg(0x2C);    //LCD_WriteRAM_Prepare();
    for (index=0;index<length_x * length_y;index++)
    {
        IERG3810_TFTLCD_WrData(color);
    }
}


void IERG3810_TFTLCD_SevenSegment(u16 color, u16 start_x, u16 start_y, u8 digit) {
	// draw 8
	switch(digit) {
		case 0:
			IERG3810_TFTLCD_FillRectangle(color, start_x+10, 55, start_y+130, 10); //a
			IERG3810_TFTLCD_FillRectangle(color, start_x+65, 10, start_y+75, 55); //b
			IERG3810_TFTLCD_FillRectangle(color, start_x+65, 10, start_y+10, 55); //c
			IERG3810_TFTLCD_FillRectangle(color, start_x+10, 55, start_y, 10); //d
			IERG3810_TFTLCD_FillRectangle(color, start_x, 10, start_y+10, 55); // e
			IERG3810_TFTLCD_FillRectangle(color, start_x, 10, start_y+75, 55); //f
			break;
		case 1:
			IERG3810_TFTLCD_FillRectangle(color, start_x+65, 10, start_y+75, 55); //b
			IERG3810_TFTLCD_FillRectangle(color, start_x+65, 10, start_y+10, 55); //c
			break;
		case 2:
			IERG3810_TFTLCD_FillRectangle(color, start_x+10, 55, start_y+130, 10); //a
			IERG3810_TFTLCD_FillRectangle(color, start_x+65, 10, start_y+75, 55); //b
			IERG3810_TFTLCD_FillRectangle(color, start_x+10, 55, start_y, 10); //d
			IERG3810_TFTLCD_FillRectangle(color, start_x, 10, start_y+10, 55); // e
			IERG3810_TFTLCD_FillRectangle(color, start_x+10, 55, start_y+65, 10); //g
			break;
		case 3:
			IERG3810_TFTLCD_FillRectangle(color, start_x+10, 55, start_y+130, 10); //a
			IERG3810_TFTLCD_FillRectangle(color, start_x+65, 10, start_y+75, 55); //b
			IERG3810_TFTLCD_FillRectangle(color, start_x+65, 10, start_y+10, 55); //c
			IERG3810_TFTLCD_FillRectangle(color, start_x+10, 55, start_y, 10); //d
			IERG3810_TFTLCD_FillRectangle(color, start_x+10, 55, start_y+65, 10); //g
			break;
		case 4:
			IERG3810_TFTLCD_FillRectangle(color, start_x+65, 10, start_y+75, 55); //b
			IERG3810_TFTLCD_FillRectangle(color, start_x+65, 10, start_y+10, 55); //c
			IERG3810_TFTLCD_FillRectangle(color, start_x, 10, start_y+75, 55); //f
			IERG3810_TFTLCD_FillRectangle(color, start_x+10, 55, start_y+65, 10); //g
			break;
		case 5:
			IERG3810_TFTLCD_FillRectangle(color, start_x+10, 55, start_y+130, 10); //a
			IERG3810_TFTLCD_FillRectangle(color, start_x+65, 10, start_y+10, 55); //c
			IERG3810_TFTLCD_FillRectangle(color, start_x+10, 55, start_y, 10); //d
			IERG3810_TFTLCD_FillRectangle(color, start_x, 10, start_y+75, 55); //f
			IERG3810_TFTLCD_FillRectangle(color, start_x+10, 55, start_y+65, 10); //g
			break;
		case 6:
			IERG3810_TFTLCD_FillRectangle(color, start_x+10, 55, start_y+130, 10); //a
			IERG3810_TFTLCD_FillRectangle(color, start_x+65, 10, start_y+10, 55); //c
			IERG3810_TFTLCD_FillRectangle(color, start_x+10, 55, start_y, 10); //d
			IERG3810_TFTLCD_FillRectangle(color, start_x, 10, start_y+10, 55); // e
			IERG3810_TFTLCD_FillRectangle(color, start_x, 10, start_y+75, 55); //f
			IERG3810_TFTLCD_FillRectangle(color, start_x+10, 55, start_y+65, 10); //g
			break;
		case 7:
			IERG3810_TFTLCD_FillRectangle(color, start_x+10, 55, start_y+130, 10); //a
			IERG3810_TFTLCD_FillRectangle(color, start_x+65, 10, start_y+75, 55); //b
			IERG3810_TFTLCD_FillRectangle(color, start_x+65, 10, start_y+10, 55); //c
			break;
		case 8:
			IERG3810_TFTLCD_FillRectangle(color, start_x+10, 55, start_y+130, 10); //a
			IERG3810_TFTLCD_FillRectangle(color, start_x+65, 10, start_y+75, 55); //b
			IERG3810_TFTLCD_FillRectangle(color, start_x+65, 10, start_y+10, 55); //c
			IERG3810_TFTLCD_FillRectangle(color, start_x+10, 55, start_y, 10); //d
			IERG3810_TFTLCD_FillRectangle(color, start_x, 10, start_y+10, 55); // e
			IERG3810_TFTLCD_FillRectangle(color, start_x, 10, start_y+75, 55); //f
			IERG3810_TFTLCD_FillRectangle(color, start_x+10, 55, start_y+65, 10); //g
			break;
		case 9:
			IERG3810_TFTLCD_FillRectangle(color, start_x+10, 55, start_y+130, 10); //a
			IERG3810_TFTLCD_FillRectangle(color, start_x+65, 10, start_y+75, 55); //b
			IERG3810_TFTLCD_FillRectangle(color, start_x+65, 10, start_y+10, 55); //c
			IERG3810_TFTLCD_FillRectangle(color, start_x+10, 55, start_y, 10); //d
			IERG3810_TFTLCD_FillRectangle(color, start_x, 10, start_y+75, 55); //f
			IERG3810_TFTLCD_FillRectangle(color, start_x+10, 55, start_y+65, 10); //g
			break;
	}
}


void IERG3810_TFTLCD_ShowChar(u16 x, u16 y, u8 ascii, u16 color, u16 bgcolor)
{
    u8 i,j;
    u8 index;
    u8 height=16, length=8;
    if (ascii<32 || ascii>127) return;
    ascii -= 32;
    IERG3810_TFTLCD_WrReg(0x2A);
        IERG3810_TFTLCD_WrData(x>>8);
        IERG3810_TFTLCD_WrData(x & 0xFF);
        IERG3810_TFTLCD_WrData((length+x-1)>>8);
        IERG3810_TFTLCD_WrData((length+x-1) & 0xFF);
    IERG3810_TFTLCD_WrReg(0x2B);
        IERG3810_TFTLCD_WrData(y>>8);
        IERG3810_TFTLCD_WrData(y & 0xFF);
        IERG3810_TFTLCD_WrData((height+y-1)>>8);
        IERG3810_TFTLCD_WrData((height+y-1) & 0xFF);
    IERG3810_TFTLCD_WrReg(0x2C);        //LCD_WriteRAM_Prepare();

    for (j = 0; j < height/8; j++)
    {
        for (i = 0; i < height/2; i++)
        {
            for (index = 0; index < length; index++)
            {
                if ((asc2_1608[ascii][index*2+1-j]>>i) &0x01) IERG3810_TFTLCD_WrData(color);
                    else IERG3810_TFTLCD_WrData(bgcolor);
            }
        }
    }
}

void IERG3810_TFTLCD_ShowChinChar(u16 x, u16 y, u8 chi, u16 color, u16 bgcolor)
{
    u8 i,j;
    u8 index;
    u8 height=16, length=16;
    IERG3810_TFTLCD_WrReg(0x2A);
        IERG3810_TFTLCD_WrData(x>>8);
        IERG3810_TFTLCD_WrData(x & 0xFF);
        IERG3810_TFTLCD_WrData((length+x-1)>>8);
        IERG3810_TFTLCD_WrData((length+x-1) & 0xFF);
    IERG3810_TFTLCD_WrReg(0x2B);
        IERG3810_TFTLCD_WrData(y>>8);
        IERG3810_TFTLCD_WrData(y & 0xFF);
        IERG3810_TFTLCD_WrData((height+y-1)>>8);
        IERG3810_TFTLCD_WrData((height+y-1) & 0xFF);
    IERG3810_TFTLCD_WrReg(0x2C);        //LCD_WriteRAM_Prepare();

    for (j = 0; j < height/8; j++)
    {
        for (i = 0; i < height/2; i++)
        {
            for (index = 0; index < length; index++)
            {
                if ((chi_1616[chi][index*2+1-j]>>i) &0x01) IERG3810_TFTLCD_WrData(color);
                    else IERG3810_TFTLCD_WrData(bgcolor);
            }
        }
    }
}

void IERG3810_TFTLCD_ShowChinChar_Overlay(u16 x, u16 y, u8 chi, u16 color)
{
    u8 i,j;
    u8 index;
    u8 height=16, length=16;
    for (j = 0; j < height/8; j++)
    {
        for (i = 0; i < height/2; i++)
        {
            for (index = 0; index < length; index++)
            {
                if ((chi_1616[chi][index*2+1-j]>>i) &0x01) {
									IERG3810_TFTLCD_DrawDot(x+index, y+i+j*8, color);
								}
            }
        }
    }
}

void IERG3810_TFTLCD_ShowChar_Overlay(u16 x, u16 y, u8 ascii, u16 color)
{
    u8 i,j;
    u8 index;
    u8 height=16, length=8;
    if (ascii<32 || ascii>127) return;
    ascii -= 32;
    for (j = 0; j < height/8; j++)
    {
        for (i = 0; i < height/2; i++)
        {
            for (index = 0; index < length; index++)
            {
                if ((asc2_1608[ascii][index*2+1-j]>>i) &0x01) {
									IERG3810_TFTLCD_DrawDot(x+index, y+i+j*8, color);
								}
            }
        }
    }
}


void IERG3810_SYSTICK_Init10ms(void)
{
    //SYSTICK
    SysTick->CTRL = 0; //clear control and status register
    SysTick->LOAD = 900000; //set the reload value reister to 90000 (72M/8/(1000ms/10ms))
    SysTick->CTRL |= 0x07; //set clock source core clock, set to countdown mode and pend SYSTICK handler, enable counter
}


void IERG3810_TIM3_Init(u16 arr, u16 psc)
{
    RCC->APB1ENR |= 1<<1; //enable TIM3 timer clock on APB1 peripheral
    TIM3->ARR = arr; //set the time auto-reload register into the value arr
    TIM3->PSC = psc; //set the pre-scalar into the value of psc
    TIM3->DIER |= 1<<0; //timer-3 update interrupt enable
    TIM3->CR1 |= 0x01; //timer-3 counter enable
    NVIC->IP[29] = 0x45; //TIM3, IQR#29, set the priority level to 0x45
    NVIC->ISER[0] |= (1<<29); //enable the interrupt line of TIM3
}
