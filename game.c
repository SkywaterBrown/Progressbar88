/*
===================
    SEGMENTS
===================
1 - Blue   '+'  -  %25
2 - Cyan   'X'  -  %10
3 - Yellow 'n'  -  %25
4 - Pink   '-'  -  %15
5 - Gray   '0'  -  %15
6 - Red    '!'  -  %8
7 - Green  'W'  -  %2

===================
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ncurses.h>
#include <string.h>
#include <time.h>

#define BAR_FILL_COUNT 20
#define TARGET_FPS 60
#define FRAME_DURATION_NS (1000000000 / TARGET_FPS)
#define MAX_SEGMENTS 20

int row, col, isdebug, round_end, system_failure;
char key;
WINDOW *bar;
struct BarStruct{
	int X, Y, height, width;
	char include[BAR_FILL_COUNT];
};
struct Segments{
	int type;
	char symbol;
	double x ,y, speed;
};
struct Mouse{
	int X, Y;
};

void get_input();
void draw_bar(struct BarStruct *Bar, int height, int width, int start_y, int start_x);
void erase_bar(int height, int width, int start_y, int start_x);
void draw_segments(struct Segments segments[MAX_SEGMENTS], int segment_count);
void erase_segments(struct Segments segments[MAX_SEGMENTS], int segment_count);
void handle_segments(struct Segments segments[MAX_SEGMENTS], int *segment_count_ptr, struct BarStruct *Bar);

int main(int argc, char *argv[])
{
	 if(argc==2 && !strcmp(argv[1], "debug")) { isdebug=1; }
	 
//	 printf("\033[?1003h\n");
//	 fflush(stdout);
	 
	 initscr();
	 cbreak();
	 noecho();
	 curs_set(0);
	 keypad(stdscr, TRUE);
	 nodelay(stdscr, TRUE);
	 mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
	 mouseinterval(0);
	 if(has_colors() == TRUE) start_color();
	 
	 struct Mouse MOUSE;
	 MEVENT event; // Mouse event
	 getmaxyx(stdscr, row, col);
	 srand(time(NULL));
	 
	 init_pair(0, COLOR_WHITE, COLOR_BLACK); attron(COLOR_PAIR(0));
	 init_pair(1, COLOR_WHITE, COLOR_BLUE);   // Blue Segment
	 init_pair(2, COLOR_WHITE, COLOR_CYAN);   // Cyan Segment
	 init_pair(3, COLOR_WHITE, COLOR_YELLOW); // Yellow Segment
	 init_pair(4, COLOR_WHITE, COLOR_MAGENTA);   // Pink Segment
	 init_pair(5, COLOR_BLACK, COLOR_WHITE);   // Gray Segment
	 init_pair(6, COLOR_WHITE, COLOR_RED);    // Red Segment
	 init_pair(7, COLOR_WHITE, COLOR_GREEN);  // Green Segment
	 
	 for(int i=0; i<=col; i++) { for(int j=0; j<=row; j++) { mvprintw(j,i, " "); } }
	 
	 struct timespec start, end;
	 
	 struct BarStruct Bar;
	 Bar.height=3;
	 Bar.width=22;
	 Bar.X=(col/2)-(Bar.width/2);
	 Bar.Y=(row/2)-(Bar.height/2);
	 memset(&Bar.include, '\0', sizeof(Bar.include));
	 
	 struct Segments segments[MAX_SEGMENTS];
	 int segment_count=0;
	 
	 
	 while(1)
	 {
		clock_gettime(CLOCK_MONOTONIC, &start);
		 
		get_input();
		
		if(key==27 || key=='q') { goto E; }
		else if(key==KEY_RIGHT || key=='d' || key==5) { Bar.X++; }
		else if(key==KEY_LEFT  || key=='a' || key==4) { Bar.X--; }
		else if(key==KEY_DOWN  || key=='s' || key==2) { Bar.Y++; }
		else if(key==KEY_UP    || key=='w' || key==3) { Bar.Y--; }
		if(key == KEY_MOUSE)
		{
			if(getmouse(&event) == OK)
			{
				MOUSE.X = event.x;
				MOUSE.Y = event.y;
				
				if(isdebug) mvprintw(0, 0, "Mouse Event: x=%d, y=%d, bstate=%d", event.x, event.y, event.bstate);
				
				
				Bar.X = MOUSE.X - (Bar.width / 2);
				Bar.Y = MOUSE.Y - (Bar.height / 2);
			}
		}
		
		if (Bar.Y < 0) { Bar.Y=0; }
		else if (Bar.Y > row-Bar.height) { Bar.Y=row-Bar.height; }
		if (Bar.X < 0) { Bar.X=0; }
		else if (Bar.X > col-Bar.width) { Bar.X=col-Bar.width; }
		        
		erase_segments(segments, segment_count);
		erase_bar(Bar.height, Bar.width, Bar.Y, Bar.X);
		
		
		handle_segments(segments, &segment_count, &Bar);
		
		
		draw_segments(segments, segment_count);
		draw_bar(&Bar, Bar.height, Bar.width, Bar.Y, Bar.X);
		
		
		if(isdebug)
		{
			mvprintw(1, 0, "Bar: X=%d, Y=%d", Bar.X, Bar.Y);
			mvprintw(2, 0, "Segments: %d", segment_count);
		}
		
		refresh();
		if (key != KEY_MOUSE) key='x';
		
		clock_gettime(CLOCK_MONOTONIC, &end);
		long delta_ns = (end.tv_sec - start.tv_sec) *1e9 + (end.tv_nsec - start.tv_nsec);
		if(delta_ns < FRAME_DURATION_NS)
		{
			long sleep_ns = FRAME_DURATION_NS - delta_ns;
			struct timespec sleep_time;
			sleep_time.tv_sec = sleep_ns / 1000000000;
			sleep_time.tv_nsec = sleep_ns % 1000000000;
			nanosleep(&sleep_time, NULL);
		}
	 }
	 
	 E:
//	 fflush(stdout);
	 endwin();
	 printf("Progressbar88, a game by Skywater, Kartopu and Kuftopagi, 2025\n\n");
	 return(0);
}

void handle_segments(struct Segments segments[MAX_SEGMENTS], int *segment_count_ptr, struct BarStruct *Bar)
{
	int i, j, random, do_generate;
	int segment_count = *segment_count_ptr;  // Get local copy of segment_count from pointer
	
	do_generate = rand()%40 + 1;
	if(do_generate == 8 && segment_count < MAX_SEGMENTS)
	{
		int type;
		double x, y, speed;
		char symbol;
		y = 0;
		x = rand() % col;
		speed = (rand() % 3 + 1) / 10.0; // Set speed in between 0.1 and 0.3
		
		random = rand()%100+1;
		if(random < 25) // Blue Segment
		{
			type = 1;
			symbol = '+';
		}
		else if(random >= 25 && random<50) // Yellow Segment
		{
			type = 3;
			symbol = 'n';
		}
		else if(random >= 50 && random<60) // Cyan Segment
		{
			type = 2;
			symbol = 'X';
		}
		else if(random >= 60 && random<75) // Pink Segment
		{
			type = 4;
			symbol = '-';
		}
		else if(random >= 75 && random<90) // Gray Segment
		{
			type = 5;
			symbol = '0';
		}
		else if(random >= 90 && random<98) // Red Segment
		{
			type = 6;
			symbol = '!';
		}
		else if(random >= 98 && random<100) // Green Segment
		{
			type = 7;
			symbol = 'W';
		}
		
		segments[segment_count].type = type;
		segments[segment_count].symbol = symbol;
		segments[segment_count].x = x;
		segments[segment_count].y = y;
		segments[segment_count].speed = speed;
		
		segment_count += 1;
	}
	
	for(i = 0; i < segment_count ; i++)
	{
		int is_collided = 0;  // Initialize to 0
		segments[i].y += segments[i].speed;
		
		// Convert segment positions to int for comparison
		int seg_x = (int)segments[i].x;
		int seg_y = (int)segments[i].y;
		int filled_area = strlen(Bar->include);
		
		if(Bar->X <= seg_x && seg_x <= (Bar->X + Bar->width)) // check for X axis collision with bar
		{
			if(Bar->Y <= seg_y && seg_y <= (Bar->Y + Bar->height)) // check for Y axis collision with bar
			{
				if(seg_x < Bar->X + filled_area) { is_collided = 1; }
				else
				{
					if(Bar->include[i] == 'W') // Green Segment: instant win
					{
						memset(Bar->include, '+', sizeof(Bar->include));
						
					}
					else if(Bar->include[i] == '!') // Red Segment: system failure
					{
						system_failure = 1;
					}
					
					filled_area = strlen(Bar->include);
					if(filled_area >= MAX_SEGMENTS) // check if are is filled
					{
						round_end = 1;
					}
					else // add more segments until bar is filled
					{
						is_collided = 1;
						if(isdebug) mvprintw(3, 0, "Collision at: X:%d, Y:%d", seg_x, seg_y);
						
						int last_char = strlen(Bar->include);
						Bar->include[last_char] = segments[i].symbol;
					}
				}
			}
		
	}
	
	if((int)segments[i].y > row - 1 || is_collided) // delete segment if it is out of screen or collided with bar
	{
		for(j = i; j < segment_count - 1; j++)
		{
			segments[j] = segments[j+1];
			
		}
		segment_count -= 1;
		i--;
	}
}
*segment_count_ptr = segment_count;  // Update the original segment_count to pointer
}
void draw_segments(struct Segments segments[MAX_SEGMENTS], int segment_count)
{
	int i, x, y;
	
	for(i = 0; i < segment_count; i++)
	{
		x = (int)segments[i].x;
		y = (int)segments[i].y;
		
		attron(COLOR_PAIR(segments[i].type));
		mvprintw(y, x, "%c", segments[i].symbol);
		attroff(COLOR_PAIR(segments[i].type));
	}
}
void erase_segments(struct Segments segments[MAX_SEGMENTS], int segment_count)
{
	int i, x, y;
	
	for(i = 0; i < segment_count; i++)
	{
		x = (int)segments[i].x;
		y = (int)segments[i].y;
		
		attron(COLOR_PAIR(0));
		mvprintw(y, x, " ");
		attroff(COLOR_PAIR(0));

	}
}

void draw_bar(struct BarStruct *Bar, int height, int width, int start_y, int start_x)
{
	int i = 0;
	bar=newwin(height, width, start_y, start_x);
	box(bar, 0,0);
	while(Bar->include[i] != '\0')
	{
		switch(Bar->include[i])
		{
			case '+':
				wattron(bar, COLOR_PAIR(1));
				break;
			case 'n':
				wattron(bar, COLOR_PAIR(3));
				break;
			case 'X':
				wattron(bar, COLOR_PAIR(2));
				break;
			case '-':
				wattron(bar, COLOR_PAIR(4));
				break;
			case '0':
				wattron(bar, COLOR_PAIR(5));
				break;
			case '!':
				wattron(bar, COLOR_PAIR(6));
				break;
			case 'W':
				wattron(bar, COLOR_PAIR(7));
				break;
			default:
				wattron(bar, COLOR_PAIR(0));
				break;
		}
		mvwprintw(bar, 1, i + 1, "%c", Bar->include[i]);
		attron(COLOR_PAIR(0));
		attroff(COLOR_PAIR(0));
		i++;
	}
	wrefresh(bar);
}

void erase_bar(int height, int width, int start_y, int start_x)
{
	if(bar != NULL)
	{
		wclear(bar); 
		wrefresh(bar);
		delwin(bar);
		bar = NULL;
	}
	
}

void get_input()
{
	 int ch;
	 ch=getch();
	 if(ch == KEY_MOUSE) key = 409;
	 else if(ch!=ERR) key=ch; 
}
