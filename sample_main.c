/* COMP2215 15/16 Task 5---SKELETON */

#include <stdlib.h>
#include "os.h"
#include "printf.h"



//Paddle
#define PADDLE_WIDTH        40
#define PADDLE_HEIGHT       4
#define PADDLE_SPEED        5

//Ball
#define BALL_R 3

//BRICK
#define BRICK_WIDTH        30
#define BRICK_HEIGHT       15

//CREATOR CONSTANTS
#define BORDER_HEIGHT      130
#define STEP_SIZE			10

typedef struct {
    uint16_t x, y;
} paddle_s;

typedef struct {
    uint16_t x, y;
	int16_t speedx, speedy;
} ball_s;

typedef struct {
    uint16_t x, y;
	uint16_t hits_to_destroy;
} brick_s;



const paddle_s start_paddle = {(LCDHEIGHT-PADDLE_WIDTH)/2, LCDWIDTH-PADDLE_HEIGHT-1};
const ball_s start_ball = {(LCDHEIGHT-BALL_R)/2, 160,4,4};
paddle_s paddle;
ball_s ball;
brick_s bricks[14];
uint16_t selected_mode;
int16_t last_selected_mode;
int16_t selected_slot;
/*
The variable representing current state
game_started=0 MAIN MENU
game_started=1 GAME
game_started=2 YOU WON / YOU LOST
game_started=3 LEVEL CREATOR
game_started=4 SELECT SLOT
*/
uint16_t game_started;
uint16_t bricks_alive;
uint16_t initial_bricks_num;
uint16_t lives_left;
uint16_t slots_free[10];

brick_s new_brick;
uint16_t new_brick_count;

int blink(int);
int update_dial(int);
int collect_delta(int);
int check_switches(int);
void move_paddle();
void move_ball();
void random_start();
void clear_display();
void draw_level_creator();


FIL File;  						/* FAT File */

int position = 0;

void main(void) {
	game_started =0;
	selected_mode=0;
	
	TCCR2B |= (1 << CS10);

    os_init();
	
    os_add_task( move_paddle, 20, 1);
    os_add_task( move_ball,   40, 1);
	os_add_task(check_switches, 80,1);
     
    sei();
    for(;;)
	{

	}
    
}

//Redrawing lives in case of missed ball
void redraw_lives(uint16_t lives)
{
	cli();
	if(lives==3)
	{
		display_string_xy("Lives: 3",LCDHEIGHT-50,10);
	}
	else if(lives==2)
	{
		display_string_xy("Lives: 2",LCDHEIGHT-50,10);
	}
	else if(lives==1)
	{
		display_string_xy("Lives: 1",LCDHEIGHT-50,10);
	}
	sei();
	
}

//Clearing display
void clear_display()
{
	cli();
	rectangle r = {0, LCDHEIGHT, 0, LCDWIDTH};
	fill_rectangle(r,BLACK);
	sei();
}

//Default map loading
void draw_bricks(){
	cli();
	bricks_alive=0;
	rectangle r5 = {0, LCDWIDTH, 0, LCDHEIGHT};
	fill_rectangle(r5,BLACK);
	int16_t j;
	//Drawing bricks
	for(j=0;j<6;j++)
	{
		brick_s brick = {10 + j*(BRICK_WIDTH + 20), 70,1};
		rectangle r = {brick.x, brick.x+BRICK_WIDTH, brick.y, brick.y+BRICK_HEIGHT};
		fill_rectangle(r,WHITE);
		rectangle r1 = {brick.x+1, brick.x+BRICK_WIDTH-1, brick.y+1, brick.y+BRICK_HEIGHT-1};
		fill_rectangle(r1,RED);
		rectangle r2 = {brick.x+BRICK_WIDTH/3, brick.x+BRICK_WIDTH/3+1, brick.y, brick.y+BRICK_HEIGHT/2};
		fill_rectangle(r2,WHITE);
		rectangle r3 = {brick.x, brick.x+BRICK_WIDTH, brick.y+BRICK_HEIGHT/2, brick.y+BRICK_HEIGHT/2+1};
		fill_rectangle(r3,WHITE);
		rectangle r4 = {brick.x+2*BRICK_WIDTH/3, brick.x+2*BRICK_WIDTH/3+1, brick.y+BRICK_HEIGHT/2, brick.y+BRICK_HEIGHT};
		fill_rectangle(r4,WHITE);
		bricks[j].x =10 + j*(BRICK_WIDTH + 20);
		bricks[j].y =70;
		bricks[j].hits_to_destroy=1;
		bricks_alive+=1;

		
	}
	
	for(j=6;j<12;j++)
	{
		brick_s brick = {20 + (j-6)*(BRICK_WIDTH + 20), 30,1};
		rectangle r = {brick.x, brick.x+BRICK_WIDTH, brick.y, brick.y+BRICK_HEIGHT};
		fill_rectangle(r,WHITE);
		rectangle r1 = {brick.x+1, brick.x+BRICK_WIDTH-1, brick.y+1, brick.y+BRICK_HEIGHT-1};
		fill_rectangle(r1,RED);
		rectangle r2 = {brick.x+BRICK_WIDTH/3, brick.x+BRICK_WIDTH/3+1, brick.y, brick.y+BRICK_HEIGHT/2};
		fill_rectangle(r2,WHITE);
		rectangle r3 = {brick.x, brick.x+BRICK_WIDTH, brick.y+BRICK_HEIGHT/2, brick.y+BRICK_HEIGHT/2+1};
		fill_rectangle(r3,WHITE);
		rectangle r4 = {brick.x+2*BRICK_WIDTH/3, brick.x+2*BRICK_WIDTH/3+1, brick.y+BRICK_HEIGHT/2, brick.y+BRICK_HEIGHT};
		fill_rectangle(r4,WHITE);
		bricks[j].x =20 + (j-6)*(BRICK_WIDTH + 20);
		bricks[j].y =30;
		bricks[j].hits_to_destroy=1;
		bricks_alive+=1;
	}
	initial_bricks_num=bricks_alive;
	sei();
	lives_left=3;
	redraw_lives(lives_left);
}

//Getting bricks from the file on the SD card
void get_bricks()
{
	cli();
	bricks_alive=0;
	char str2[30]="level";
	char temp[30];
	sprintf(temp, "%d", selected_slot);
	strcat(str2, temp);
	strcat(str2, ".txt");
	
	char line[800];
	f_mount(&FatFs, "", 0);
    if (f_open(&File, str2, FA_READ) == FR_OK) {
        f_gets(line, 800, &File);
	}
	char * pch;
	uint16_t num;
	uint16_t j;
	j=0;
	pch = strtok (line,"|;");
	pch = strtok (NULL,"|;");
	//Reading the data from the file
	while (pch != NULL)
	{
		num = atoi(pch);
		bricks[j].x=num;
		
		pch = strtok (NULL,"|;");
		num = atoi(pch);
		bricks[j].y=num;
		
		pch = strtok (NULL,"|;");
		num = atoi(pch);
		bricks[j].hits_to_destroy=num;
		
		pch = strtok (NULL, "|;");
		j++;
	}
	rectangle r = {0, LCDHEIGHT, 0, LCDWIDTH};
	fill_rectangle(r,BLACK);
	uint16_t i;
	//Drawing bricks
	for(i=0;i<j;i++)
	{
		rectangle r = {bricks[i].x, bricks[i].x+BRICK_WIDTH, bricks[i].y, bricks[i].y+BRICK_HEIGHT};
		fill_rectangle(r,WHITE);
		rectangle r1 = {bricks[i].x+1, bricks[i].x+BRICK_WIDTH-1, bricks[i].y+1, bricks[i].y+BRICK_HEIGHT-1};
		fill_rectangle(r1,RED);
		rectangle r2 = {bricks[i].x+BRICK_WIDTH/3, bricks[i].x+BRICK_WIDTH/3+1, bricks[i].y, bricks[i].y+BRICK_HEIGHT/2};
		fill_rectangle(r2,WHITE);
		rectangle r3 = {bricks[i].x, bricks[i].x+BRICK_WIDTH, bricks[i].y+BRICK_HEIGHT/2, bricks[i].y+BRICK_HEIGHT/2+1};
		fill_rectangle(r3,WHITE);
		rectangle r4 = {bricks[i].x+2*BRICK_WIDTH/3, bricks[i].x+2*BRICK_WIDTH/3+1, bricks[i].y+BRICK_HEIGHT/2, bricks[i].y+BRICK_HEIGHT};
		fill_rectangle(r4,WHITE);
		bricks_alive+=1;

	}
	initial_bricks_num=bricks_alive;
	sei();
	lives_left=3;
	redraw_lives(lives_left);
	
}

//Generating random starting velocities based on the timer seed
void random_start(){
	int16_t sxrand = rand()%7;
	int16_t syrand = rand()%7;
	if(sxrand>=1 && sxrand<=3)sxrand=1;
	if(sxrand>=3 && sxrand<=5)sxrand=5;
	
	if(syrand>=1 && syrand<=3)syrand=1;
	if(syrand>=3 && syrand<=5)syrand=5;

	ball_s b = {(LCDHEIGHT-BALL_R)/2, 160,-3+sxrand,-3+syrand};
	ball = b;
} 

//Checking collision between the vertices of ball and edges of bricks
void check_collision(uint16_t j)
{
	brick_s brick = bricks[j];
	if(bricks[j].hits_to_destroy)
	{
		int16_t a_ball_line, b_ball_line, c_ball_line;
		int16_t a1_brick, b1_brick,c1_brick;
		int16_t a2_brick, b2_brick,c2_brick;
		int16_t a3_brick, b3_brick,c3_brick;
		int16_t a4_brick, b4_brick,c4_brick;
		int16_t c1_ball, c2_ball, c3_ball, c4_ball;
		
		//Ball's and bricks' lines equations
		a_ball_line=ball.speedy;
		b_ball_line=-ball.speedx;
		//Top left corner
		c_ball_line=a_ball_line*ball.x+b_ball_line*ball.y;
		//Top left corner
		c1_ball=a_ball_line*ball.x+b_ball_line*ball.y;
		//Bottom left corner
		c2_ball=a_ball_line*ball.x+b_ball_line*(ball.y+2*BALL_R);
		//Bottom right corner
		c3_ball=a_ball_line*(ball.x+2*BALL_R)+b_ball_line*(ball.y+2*BALL_R);
		//Top right corner
		c4_ball=a_ball_line*(ball.x+2*BALL_R)+b_ball_line*ball.y;


		//Top edge
		a1_brick =0;
		b1_brick =BRICK_WIDTH;
		c1_brick = a1_brick*brick.x+b1_brick*brick.y;
		
		//Left edge
		a2_brick =BRICK_HEIGHT;
		b2_brick =0;
		c2_brick = a2_brick*brick.x+b2_brick*brick.y;
		
		//Bottom edge
		a3_brick =0;
		b3_brick =BRICK_WIDTH;
		c3_brick = a3_brick*brick.x+b3_brick*(brick.y+BRICK_HEIGHT);
		
		//Right edge
		a4_brick =BRICK_HEIGHT;
		b4_brick =0;
		c4_brick = a4_brick*(brick.x+BRICK_WIDTH)+b4_brick*brick.y;
		
		if(ball.x + ball.speedx<= brick.x+BRICK_WIDTH+20 && ball.x + ball.speedx+20>= brick.x
			&& ball.y+ball.speedy<=brick.y+BRICK_HEIGHT+20 && ball.y+ball.speedy+20>=brick.y)
		{
			//Top edge
			if(ball.speedy>0)
			{	
				double det1 =a_ball_line*b1_brick-a1_brick*b_ball_line;
				if(det1!=0)
				{
					double collx1 = (b1_brick*c2_ball-b_ball_line*c1_brick)/det1;
					double colly1 = (a_ball_line*c1_brick-a1_brick*c2_ball)/det1;	

					double collx1b = (b1_brick*c3_ball-b_ball_line*c1_brick)/det1;
					double colly1b = (a_ball_line*c1_brick-a1_brick*c3_ball)/det1;		
					
					if((collx1>=brick.x && collx1<=brick.x+BRICK_WIDTH && colly1>=ball.y+2*BALL_R && colly1<=ball.y+2*BALL_R+ball.speedy)||
					   (collx1b>=brick.x && collx1b<=brick.x+BRICK_WIDTH && colly1b>=ball.y+2*BALL_R && colly1b<=ball.y+2*BALL_R+ball.speedy))
					{
						ball.speedy = -ball.speedy;
						rectangle r3 = {brick.x, brick.x+BRICK_WIDTH, brick.y, brick.y+BRICK_HEIGHT};
						fill_rectangle(r3,BLACK);
						if(bricks[j].hits_to_destroy==1)
						{					
							bricks[j].hits_to_destroy=0;
							bricks_alive-=1;
						}
					}
				}
			}
			
			//Left edge
			if(ball.speedx>0)
			{
				double det2 =a_ball_line*b2_brick-a2_brick*b_ball_line;
				if(det2!=0)
				{
					double collx2 = (b2_brick*c3_ball-b_ball_line*c2_brick)/det2;
					double colly2 = (a_ball_line*c2_brick-a2_brick*c3_ball)/det2;
					
					double collx2b = (b2_brick*c4_ball-b_ball_line*c2_brick)/det2;
					double colly2b = (a_ball_line*c2_brick-a2_brick*c4_ball)/det2;
					if((colly2>=brick.y && colly2<=brick.y+BRICK_HEIGHT && collx2>=ball.x+2*BALL_R && collx2<=ball.x+2*BALL_R+ball.speedx)||
					   (colly2b>=brick.y && colly2b<=brick.y+BRICK_HEIGHT && collx2b>=ball.x+2*BALL_R && collx2b<=ball.x+2*BALL_R+ball.speedx))
					{
						ball.speedx = -ball.speedx;
						rectangle r3 = {brick.x, brick.x+BRICK_WIDTH, brick.y, brick.y+BRICK_HEIGHT};
						fill_rectangle(r3,BLACK);
						if(bricks[j].hits_to_destroy==1)
						{					
							bricks[j].hits_to_destroy=0;
							bricks_alive-=1;
						}
					}
				}
			}
			
			//Bottom edge
			if(ball.speedy<0)
			{
			
				double det3 =a_ball_line*b3_brick-a3_brick*b_ball_line;
				if(det3!=0)
				{
					double collx3 = (b3_brick*c_ball_line-b_ball_line*c3_brick)/det3;
					double colly3 = (a_ball_line*c3_brick-a3_brick*c_ball_line)/det3;
					double collx3b = (b3_brick*c4_ball-b_ball_line*c3_brick)/det3;
					double colly3b = (a_ball_line*c3_brick-a3_brick*c4_ball)/det3;

					if((collx3>=brick.x && collx3<=brick.x+BRICK_WIDTH && colly3>=ball.y+ball.speedy && colly3<=ball.y)||
					   (collx3b>=brick.x && collx3b<=brick.x+BRICK_WIDTH && colly3b>=ball.y+ball.speedy && colly3b<=ball.y))
					{
						ball.speedy = -ball.speedy;
						rectangle r3 = {brick.x, brick.x+BRICK_WIDTH, brick.y, brick.y+BRICK_HEIGHT};
						fill_rectangle(r3,BLACK);
						if(bricks[j].hits_to_destroy==1)
						{					
							bricks[j].hits_to_destroy=0;
							bricks_alive-=1;

						}
					}
				}
			}
		
			//Right edge
			if(ball.speedx<0)
			{
				double det4 =a_ball_line*b4_brick-a4_brick*b_ball_line;
				if(det4!=0)
				{
					double collx4 = (b4_brick*c_ball_line-b_ball_line*c4_brick)/det4;
					double colly4 = (a_ball_line*c4_brick-a4_brick*c_ball_line)/det4;
					
					double collx4b = (b4_brick*c2_ball-b_ball_line*c4_brick)/det4;
					double colly4b = (a_ball_line*c4_brick-a4_brick*c2_ball)/det4;
					if((colly4>=brick.y && colly4<=brick.y+BRICK_HEIGHT && collx4>=ball.x+ball.speedx && collx4<=ball.x)||
					   (colly4b>=brick.y && colly4b<=brick.y+BRICK_HEIGHT && collx4b>=ball.x+ball.speedx && collx4b<=ball.x))
					{
						ball.speedx = -ball.speedx;
						rectangle r3 = {brick.x, brick.x+BRICK_WIDTH, brick.y, brick.y+BRICK_HEIGHT};
						fill_rectangle(r3,BLACK);
						if(bricks[j].hits_to_destroy==1)
						{					
							bricks[j].hits_to_destroy=0;
							bricks_alive-=1;

						}
					}
				}
			}
			
			//Display "You won" view
			if(bricks_alive==0)
			{
				cli();
				rectangle r5 = {0, LCDHEIGHT, 0, LCDWIDTH};
				fill_rectangle(r5,BLACK);
				game_started=2;
				sei();
				
				display_color(WHITE, BLACK);
				display_string_xy("YOU WON !", 120,100);
				display_color(WHITE, BLACK);
				display_string_xy("Press center to go to menu", 70,200);
				

			}

		}
	}
}

//Moving ball
void move_ball()
{
	if(game_started==1)
	{
		rectangle r1 = {ball.x, ball.x+2*BALL_R, ball.y, ball.y+2*BALL_R};
		fill_rectangle(r1,BLACK);
		
		uint16_t j;
		//Checking collisions with bricks
		for(j=0;j<initial_bricks_num;j++)
		{
			check_collision(j);
		}
		
		if(ball.speedx < 0 ) {
			if(ball.x > (-1)*ball.speedx)
			{
				ball.x += ball.speedx;
			}
			//Collision with right edge
			else
			{
				ball.speedx = -ball.speedx;
			}
		}
		else if (ball.speedx > 0) {
			if(ball.x + 2*BALL_R + ball.speedx < LCDHEIGHT)
			{
				ball.x += ball.speedx;
			}
			//Collision with left edge
			else
			{
				ball.speedx = -ball.speedx;
			}
		}

		
		if(ball.speedy < 0 ) {
			if(ball.y > (-1)*ball.speedy)
			{
				ball.y += ball.speedy;
			}
			//Colision with to edge
			else
			{
				ball.speedy = -ball.speedy;
			}
		}
		else if (ball.speedy > 0) {
			if(ball.y + 2*BALL_R + ball.speedy < LCDWIDTH-PADDLE_HEIGHT-1)
			{
				ball.y += ball.speedy;		
			}
			else
			{
				//Collision with paddle
				if((ball.x >= paddle.x && ball.x<=paddle.x+PADDLE_WIDTH+5)
					||(ball.x+2*BALL_R >= paddle.x && ball.x+2*BALL_R<=paddle.x+PADDLE_WIDTH))
				{
					int16_t diff = (paddle.x + PADDLE_WIDTH/2)-ball.x;
					if(diff<0)diff*= -1;
					if(ball.speedx<0)
					{
						if(diff<10)ball.speedx = -3;
						else ball.speedx = -4;
					}
					else
					{
						if(diff<10)ball.speedx = 3;
						else ball.speedx = 4;
					}
					ball.speedy = -ball.speedy;
				}
				//Ball missed
				else
				{
					srand(TCNT2);
					lives_left--;
					redraw_lives(lives_left);
					//Displaying "You lost" view
					if(lives_left==0)
					{
						cli();
						rectangle r5 = {0, LCDHEIGHT, 0, LCDWIDTH};
						fill_rectangle(r5,BLACK);
						game_started=2;
						sei();
						
						display_color(WHITE, BLACK);
						display_string_xy("YOU LOST !", 120,100);
						display_color(WHITE, BLACK);
						display_string_xy("Press center to go to menu", 70,200);
					}
					else
					{
						random_start();
					}
				}
			}
		}
		if(lives_left>0)
		{
			if(ball.x>LCDHEIGHT-70 && ball.y<30)
			{
				redraw_lives(lives_left);
			}
			//Check needed as may be interupted by cancel
			if(game_started==1)
			{
				rectangle r2 = {ball.x, ball.x+2*BALL_R, ball.y, ball.y+2*BALL_R};
				if(game_started==1)fill_rectangle(r2,WHITE);
			}
		}
		
	}
}

//Responsible for moving paddle in a game as well as drawing menus
void move_paddle()
{
	//Moving paddle
	if(game_started==1)
	{		
		rectangle r1 = {paddle.x, paddle.x+PADDLE_WIDTH, paddle.y, paddle.y+PADDLE_HEIGHT};
		fill_rectangle(r1,BLACK);
		int8_t rotary;
		rotary = os_enc_delta();
		if(rotary < 0 && paddle.x > PADDLE_SPEED) {
			paddle.x -= PADDLE_SPEED;
		}
		else if (rotary > 0 && paddle.x + PADDLE_WIDTH + PADDLE_SPEED < LCDHEIGHT) {
			paddle.x += PADDLE_SPEED;
		}
		
		rectangle r2 = {paddle.x, paddle.x+PADDLE_WIDTH, paddle.y, paddle.y+PADDLE_HEIGHT};
		fill_rectangle(r2,WHITE);
	}
	//Displaying "Main menu" view
	else if(game_started==0)
	{
		cli();
		if(game_started==0)
		{
			
			if(selected_mode==0)
			{
				display_color(BLUE, BLACK);
			}
			else{
				display_color(WHITE, BLACK);
			}
			display_string_xy("Start",80,80);
			
			if(selected_mode==1)
			{
				display_color(BLUE, BLACK);
			}
			else{
				display_color(WHITE, BLACK);
			}
			display_string_xy("Choose level",80,100);
			
			if(selected_mode==2)
			{
				display_color(BLUE, BLACK);
			}
			else{
				display_color(WHITE, BLACK);
			}
			display_string_xy("Create level",80,120);
			
			
			int8_t rotary;
			rotary = os_enc_delta();
			if(rotary < 0) {
				selected_mode -= 1;
				if(selected_mode==-1)selected_mode=2;
			}
			else if (rotary > 0) {
				selected_mode += 1;
				if(selected_mode==3)selected_mode=0;
			}
			sei();
		}
	}
	//Displaying empty slot / level in "Select slot" view
	else if (game_started==4)
	{
		display_color(WHITE, BLACK);
		display_string_xy("Choose slot:",80,60);
		uint16_t q;
		for(q=1;q<6;q++)
		{
			if(selected_slot==q)
			{
				display_color(BLUE, BLACK);
			}
			else{
				display_color(WHITE, BLACK);
			}

			char str[30]="Level ";
			char str2[30]="level";
			char temp[30];
			sprintf(temp, "%d", q);
			strcat(str, temp);
			strcat(str2, temp);
			strcat(str2, ".txt");
			FILINFO* fileifo;
			if (f_stat(str2, fileifo ) != FR_NO_FILE) {
				display_string_xy(str, 80,60+q*10);
				slots_free[q]=0;
			}
			else
			{
				display_string_xy("Empty slot", 80,60+q*10);
				slots_free[q]=1;
			}
		}
		
		int8_t rotary;
		rotary = os_enc_delta();
		if(rotary < 0) {
			selected_slot -= 1;
			if(selected_slot==0)selected_slot=5;
		}
		else if (rotary > 0) {
			selected_slot += 1;
			if(selected_slot==6)selected_slot=1;
		}

	}
}

//Getting data from rotary encoder
int collect_delta(int state) {
	position += os_enc_delta();
	return state;
}


//Checking buttons
int check_switches(int state) {
	
	if (get_switch_press(_BV(SWN))) {
		//Moving bricks up in level creator
		if(game_started==3 && new_brick.hits_to_destroy!=0)
		{
			uint16_t j;
			uint16_t test;
			test=0;
			//Assuring no collisions with already set bricks
			for(j=0;j<new_brick_count;j++)
			{
				if (new_brick.x+BRICK_WIDTH<bricks[j].x || bricks[j].x+BRICK_WIDTH<new_brick.x || new_brick.y-STEP_SIZE+BRICK_HEIGHT<bricks[j].y || bricks[j].y+BRICK_HEIGHT<new_brick.y-STEP_SIZE)
				{
					
				}
				else
				{
					test=1;
				}

			}
			//Assuring that brick doesn't cross the top edge
			if(test==0 && new_brick.y>STEP_SIZE)
			{	
				
				rectangle r1 = {new_brick.x, new_brick.x+BRICK_WIDTH, new_brick.y, new_brick.y+BRICK_HEIGHT};
				fill_rectangle(r1,BLACK);
				if(new_brick.y+BRICK_HEIGHT+10>BORDER_HEIGHT)
				{
					rectangle r3 = {0,LCDHEIGHT,BORDER_HEIGHT,BORDER_HEIGHT+1};
					fill_rectangle(r3,RED);
				}
				new_brick.y-=STEP_SIZE;
				rectangle r2 = {new_brick.x, new_brick.x+BRICK_WIDTH, new_brick.y, new_brick.y+BRICK_HEIGHT};
				fill_rectangle(r2,WHITE);
			}
			
		}

	}
		
	if (get_switch_press(_BV(SWE))) {
		//Moving bricks right in level creator
		if(game_started==3 && new_brick.hits_to_destroy!=0)
		{
			uint16_t j;
			uint16_t test;
			test=0;
			//Assuring no collisions with already set bricks
			for(j=0;j<new_brick_count;j++)
			{
				if (new_brick.x+BRICK_WIDTH+STEP_SIZE<bricks[j].x || bricks[j].x+BRICK_WIDTH<new_brick.x+STEP_SIZE || new_brick.y+BRICK_HEIGHT<bricks[j].y || bricks[j].y+BRICK_HEIGHT<new_brick.y)
				{
					
				}
				else
				{
					test=1;
				}

			}
			//Assuring that brick doesn't cross the right edge
			if(test==0 && new_brick.x+BRICK_WIDTH+STEP_SIZE<LCDHEIGHT)
			{
					
				rectangle r1 = {new_brick.x, new_brick.x+BRICK_WIDTH, new_brick.y, new_brick.y+BRICK_HEIGHT};
				fill_rectangle(r1,BLACK);
				if(new_brick.y+BRICK_HEIGHT+10>BORDER_HEIGHT)
				{
					rectangle r3 = {0,LCDHEIGHT,BORDER_HEIGHT,BORDER_HEIGHT+1};
					fill_rectangle(r3,RED);
				}	
				new_brick.x+=STEP_SIZE;
				rectangle r2 = {new_brick.x, new_brick.x+BRICK_WIDTH, new_brick.y, new_brick.y+BRICK_HEIGHT};
				fill_rectangle(r2,WHITE);
			}
			
		}
	}
		
	if (get_switch_press(_BV(SWS))) {
		//Moving bricks down in level creator
		if(game_started==3 && new_brick.hits_to_destroy!=0)
		{
			uint16_t j;
			uint16_t test;
			test=0;
			//Assuring no collisions with already set bricks
			for(j=0;j<new_brick_count;j++)
			{
				if (new_brick.x+BRICK_WIDTH<bricks[j].x || bricks[j].x+BRICK_WIDTH<new_brick.x || new_brick.y+STEP_SIZE+BRICK_HEIGHT<bricks[j].y || bricks[j].y+BRICK_HEIGHT<new_brick.y+STEP_SIZE)
				{
					
				}
				else
				{
					test=1;
				}

			}
			//Assuring that brick doesn't cross the red line
			if(test==0 && new_brick.y+STEP_SIZE+BRICK_HEIGHT<BORDER_HEIGHT)
			{
				
				rectangle r1 = {new_brick.x, new_brick.x+BRICK_WIDTH, new_brick.y, new_brick.y+BRICK_HEIGHT};
				fill_rectangle(r1,BLACK);
				if(new_brick.y+BRICK_HEIGHT+10>BORDER_HEIGHT)
				{
					rectangle r3 = {0,LCDHEIGHT,BORDER_HEIGHT,BORDER_HEIGHT+1};
					fill_rectangle(r3,RED);
				}
				new_brick.y+=STEP_SIZE;
				rectangle r2 = {new_brick.x, new_brick.x+BRICK_WIDTH, new_brick.y, new_brick.y+BRICK_HEIGHT};
				fill_rectangle(r2,WHITE);
			}
			
		}

	}
		
	if (get_switch_press(_BV(SWW))) {
		//Moving bricks left in level creator
		if(game_started==3 && new_brick.hits_to_destroy!=0)
		{
			uint16_t j;
			uint16_t test;
			test=0;
			//Assuring no collisions with already set bricks
			for(j=0;j<new_brick_count;j++)
			{
				if (new_brick.x+BRICK_WIDTH-STEP_SIZE<bricks[j].x || bricks[j].x+BRICK_WIDTH<new_brick.x-STEP_SIZE || new_brick.y+BRICK_HEIGHT<bricks[j].y || bricks[j].y+BRICK_HEIGHT<new_brick.y)
				{
					
				}
				else
				{
					test=1;
				}

			}
			//Assuring that brick doesn't cross the left edge
			if(test==0 && new_brick.x>STEP_SIZE)
			{
				
				rectangle r1 = {new_brick.x, new_brick.x+BRICK_WIDTH, new_brick.y, new_brick.y+BRICK_HEIGHT};
				fill_rectangle(r1,BLACK);
				if(new_brick.y+BRICK_HEIGHT+10>BORDER_HEIGHT)
				{
					rectangle r3 = {0,LCDHEIGHT,BORDER_HEIGHT,BORDER_HEIGHT+1};
					fill_rectangle(r3,RED);
				}
				new_brick.x-=STEP_SIZE;
				rectangle r2 = {new_brick.x, new_brick.x+BRICK_WIDTH, new_brick.y, new_brick.y+BRICK_HEIGHT};
				fill_rectangle(r2,WHITE);
			}
			
		}
		//Going back to "Main menu" by pressing left in "Select slot"
		else if (game_started==4)
		{
			clear_display();
			game_started=0;
		}

	}
		
	if (get_switch_long(_BV(SWC))) {
		//Saving the bricks location to the file and going back to "Main menu"
		if(game_started==3 && new_brick_count>0)
		{
			char str2[30]="level";
			char temp[30];
			sprintf(temp, "%d", selected_slot);
			strcat(str2, temp);
			strcat(str2, ".txt");
			f_mount(&FatFs, "", 0);
			f_unlink(str2);
			if (f_open(&File, str2, FA_WRITE | FA_OPEN_ALWAYS) == FR_OK) {
				uint16_t j;
				char str[800];
				strcpy(str, "level1|");
				for(j=0;j<new_brick_count;j++)
				{
					char temp[30];
					sprintf(temp, "%d", bricks[j].x);
					strcat(str, temp);
					strcat(str, ";");
					sprintf(temp, "%d", bricks[j].y);
					strcat(str, temp);
					strcat(str, ";");
					sprintf(temp, "%d", bricks[j].hits_to_destroy);
					strcat(str, temp);
					strcat(str, "|");

				}
				
				f_lseek(&File, f_size(&File));
				f_printf(&File, str, position);
				f_close(&File);
				//printf(str);
				clear_display();
				game_started=0;
			} else {
				display_string("Can't write file! \n");	
			}
		}
		
	}

	if (get_switch_short(_BV(SWC))) {
		//display_string("[S] Centre\n");
		//Pressing center in "Main menu"
		if(game_started==0)
		{
			//Start default map
			if(selected_mode==0)
			{
				srand(TCNT2);
				paddle = start_paddle;
				draw_bricks();
				game_started=1;
				random_start();
			}
			//Choose level - before checks the SD card
			else if(selected_mode==1)
			{
				uint16_t sdtest=0;
				f_mount(&FatFs, "", 0);
				f_unlink("test.txt");
				if (f_open(&File, "test.txt", FA_WRITE | FA_OPEN_ALWAYS) == FR_OK) {
					f_lseek(&File, f_size(&File));
					f_printf(&File, "Card was read correctly", position);
					f_close(&File);
				} else {
					sdtest=1;
					cli();
					display_color(WHITE,BLACK);
					display_string_xy("The SD card is not working", 60,200);
					sei();
				}
				if(sdtest==0)
				{
					cli();
					rectangle r = {0, LCDHEIGHT, 0, LCDWIDTH};
					fill_rectangle(r,BLACK);
					game_started=4;
					selected_slot=1;
					sei();
				}
					
			}
			//Choose level - before checks the SD card
			else if(selected_mode==2)
			{
				uint16_t sdtest=0;
				f_mount(&FatFs, "", 0);
				f_unlink("test.txt");
				if (f_open(&File, "test.txt", FA_WRITE | FA_OPEN_ALWAYS) == FR_OK) {
					f_lseek(&File, f_size(&File));
					f_printf(&File, "Card was read correctly", position);
					f_close(&File);
				} else {
					sdtest=1;
					cli();
					display_color(WHITE,BLACK);
					display_string_xy("The SD card is not working", 60,200);
					sei();
				}
				if(sdtest==0)
				{
					cli();
					rectangle r = {0, LCDHEIGHT, 0, LCDWIDTH};
					fill_rectangle(r,BLACK);
					game_started=4;
					selected_slot=1;
					sei();
				}
				
			}
		}
		//Pressing center in "You won" / "You lost" views gets you back to "Main menu"
		else if(game_started==2)
		{
			clear_display();
			game_started=0;
		}
		//Pressing the center in level creator 
		else if(game_started==3 && new_brick.hits_to_destroy!=0)
		{
			//If brick above red line create it
			if(new_brick.y+BRICK_HEIGHT+5<BORDER_HEIGHT)
			{	
				rectangle r = {new_brick.x, new_brick.x+BRICK_WIDTH, new_brick.y, new_brick.y+BRICK_HEIGHT};
				fill_rectangle(r,WHITE);
				rectangle r1 = {new_brick.x+1, new_brick.x+BRICK_WIDTH-1, new_brick.y+1, new_brick.y+BRICK_HEIGHT-1};
				fill_rectangle(r1,RED);
				rectangle r2 = {new_brick.x+BRICK_WIDTH/3, new_brick.x+BRICK_WIDTH/3+1, new_brick.y, new_brick.y+BRICK_HEIGHT/2};
				fill_rectangle(r2,WHITE);
				rectangle r3 = {new_brick.x, new_brick.x+BRICK_WIDTH, new_brick.y+BRICK_HEIGHT/2, new_brick.y+BRICK_HEIGHT/2+1};
				fill_rectangle(r3,WHITE);
				rectangle r4 = {new_brick.x+2*BRICK_WIDTH/3, new_brick.x+2*BRICK_WIDTH/3+1, new_brick.y+BRICK_HEIGHT/2, new_brick.y+BRICK_HEIGHT};
				fill_rectangle(r4,WHITE);
				bricks[new_brick_count].x =new_brick.x;
				bricks[new_brick_count].y =new_brick.y;
				bricks[new_brick_count].hits_to_destroy=new_brick.hits_to_destroy;
				new_brick_count+=1;
				brick_s brick = {0, 0,0};
				new_brick = brick;
			}
		}
		//Choosing a slot in "Select slot"
		else if(game_started==4)
		{
			//Within choose level
			if(selected_mode==1)
			{
				if (slots_free[selected_slot]==0) {
					srand(TCNT2);
					paddle = start_paddle;
					get_bricks();
					game_started=1;
					random_start();
				}
			}
			//or within create level
			else if(selected_mode==2)
			{
				cli();
				rectangle r = {0, LCDHEIGHT, 0, LCDWIDTH};
				fill_rectangle(r,BLACK);
				game_started=3;
				brick_s brick = {0, 0,0};
				new_brick = brick;
				new_brick_count=0;
				sei();
				draw_level_creator();
			}
			
		}
			
			
	}

	if (get_switch_rpt(_BV(SWN))) {
		//display_string("[R] North\n");
	}
		
	if (get_switch_rpt(_BV(SWE))) {
		//display_string("[R] East\n");
	}
		
	if (get_switch_rpt(_BV(SWS))) {
		//display_string("[R] South\n");
		//Adding new brick in level creator
		if(game_started==3 && new_brick.hits_to_destroy==0)
		{
			brick_s brick = {LCDHEIGHT/2-BRICK_WIDTH/2, BORDER_HEIGHT-BRICK_HEIGHT+20,1};
			new_brick = brick;
			rectangle r = {new_brick.x, new_brick.x+BRICK_WIDTH, new_brick.y, new_brick.y+BRICK_HEIGHT};
			fill_rectangle(r,WHITE);

		}
			
	}
		
	if (get_switch_rpt(_BV(SWW))) {
		//display_string("[R] West\n");
		//Cancel game or level creation by holding left button
		if (game_started==1 || game_started==3)
		{
			cli();
			rectangle r = {0, LCDHEIGHT, 0, LCDWIDTH};
			fill_rectangle(r,BLACK);
			game_started=0;
			sei();

		}
	}

	if (get_switch_rpt(SWN)) {
		//display_string("[R] North\n");
	}


	if (get_switch_long(_BV(OS_CD))) {
		//display_string("Detected SD card.\n");
	}

	return state;	
}

//Drawing background in level creator
void draw_level_creator()
{
	paddle=start_paddle;
	rectangle r1 = {paddle.x, paddle.x+PADDLE_WIDTH, paddle.y, paddle.y+PADDLE_HEIGHT};
	fill_rectangle(r1,WHITE);
	
	ball=start_ball;
	rectangle r2 = {ball.x, ball.x+2*BALL_R, ball.y, ball.y+2*BALL_R};
	fill_rectangle(r2,WHITE);
	
	rectangle r3 = {0,LCDHEIGHT,BORDER_HEIGHT,BORDER_HEIGHT+1};
	fill_rectangle(r3,RED);
	
	display_string_xy("To add a brick - hold south button, position it",20,180);
	display_string_xy("and press center",110,190);
	display_string_xy("To save a configuration - hold center", 55,200);

	
}




int blink(int state) {
	static int light = 0;
	uint8_t level;
	
	if (light < -120) {
		state = 1;
	} else if (light > 254) {
		state = -20;
	}


	/* Compensate somewhat for nonlinear LED 
       output and eye sensitivity:
    */
	if (state > 0) {
		if (light > 40) {
			state = 2;
		}
		if (light > 100) {
			state = 5;
		}
	} else {
		if (light < 180) {
			state = -10;
		}
		if (light < 30) {
			state = -5;
		}
	}
	light += state;

	if (light < 0) {
		level = 0;
	} else if (light > 255) {
		level = 255;
	} else {
		level = light;
	}
	
	os_led_brightness(level);
	return state;
}

