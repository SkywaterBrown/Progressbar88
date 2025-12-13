/*
===================
    SEGMENTS
===================
1 - Blue   ' '  -  %25
2 - Cyan   'X'  -  %10
3 - Yellow ' '  -  %25
4 - Pink   '-'  -  %15
5 - Gray   '0'  -  %15
6 - Red    '!'  -  %8
7 - Green  'W'  -  %2

===================
*/

#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <string.h>
#include <time.h>

#define TARGET_FPS 60
#define FRAME_DURATION_NS (1000000000 / TARGET_FPS)
#define MAX_SEGMENTS 20

int row, col, isdebug;
char key;
WINDOW *bar;
struct BarStruct{
	int X, Y, height, width;
	char include[11];
};
struct Segments{
	int type;
	char symbol;
	double x ,y, speed;
};



void get_input();
void draw_bar(int height, int width, int start_y, int start_x);
void erase_bar(int height, int width, int start_y, int start_x);
void draw_segments(struct Segments segments[MAX_SEGMENTS], int segment_count);
void erase_segments(struct Segments segments[MAX_SEGMENTS], int segment_count);
void handle_segments(struct Segments segments[MAX_SEGMENTS], int *segment_count_ptr);

int main(int argc, char *argv[])
{
	 if(argc==2 && !strcmp(argv[1], "debug")) { isdebug=1; }
	 
	 initscr();
	 noecho();
	 curs_set(0);
	 keypad(stdscr, TRUE);
	 nodelay(stdscr, TRUE);
	 mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
	 mouseinterval(0);
	 printf("\033[?1003h\n");  // Enable all mouse events
	 fflush(stdout);
	 if(has_colors() == TRUE) start_color();
	 
	 MEVENT event; // Mouse event
	 int mouse_valid = 0;  // Flag to track if we have valid mouse data
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
	 
	 for(int i=0; i<=col; i++) { for(int j=0; j<=row; j++) { mvprintw(i,j, " "); } }
	 
	 struct timespec start, end;
	 
	 struct BarStruct Bar;
	 Bar.height=3;
	 Bar.width=22;
	 Bar.X=(col/2)-(Bar.width/2);
	 Bar.Y=(row/2)-(Bar.height/2);
	 
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
		else if(key == KEY_MOUSE)
		{
			if(getmouse(&event) == OK)
			{
				Bar.X = event.x - (Bar.width / 2);
				Bar.Y = event.y - (Bar.height / 2);
				mouse_valid = 1;  // Mark data as valid
			}
		}
		
		if (Bar.Y < 0) { Bar.Y=0; }
		else if (Bar.Y > row-Bar.height) { Bar.Y=row-Bar.height; }
		if (Bar.X < 0) { Bar.X=0; }
		else if (Bar.X > col-Bar.width) { Bar.X=col-Bar.width; }
		        
		erase_segments(segments, segment_count);
		erase_bar(Bar.height, Bar.width, Bar.Y, Bar.X);
		
		
		handle_segments(segments, &segment_count);
		
		
		draw_segments(segments, segment_count);
		draw_bar(Bar.height, Bar.width, Bar.Y, Bar.X);
		
		
		if(isdebug)
		{
			mvprintw(0, 0, "Mouse: X=%d, Y=%d, Valid: %d", event.x, event.y, mouse_valid);
			mvprintw(1, 0, "Bar: X=%d, Y=%d", Bar.X, Bar.Y);
		}
		
		refresh();
		key='x';
		
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
	 printf("\033[?1003l\n");
	 fflush(stdout);
	 endwin();
	 return(0);
}

void handle_segments(struct Segments segments[MAX_SEGMENTS], int *segment_count_ptr)
{
	int i, j, random, do_generate;
	int segment_count = *segment_count_ptr;  // Get local copy of segment_count from pointer
	
	do_generate = rand()%30 + 1;
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
			symbol = ' ';
		}
		else if(random >= 25 && random<50) // Yellow Segment
		{
			type = 3;
			symbol = ' ';
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
		segments[i].y += segments[i].speed;
		
		if(1) // check for collision with bar
		{
			
		}
		
		if((int)segments[i].y > row - 1) // delete segment if it is out of screen
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
		attron(COLOR_PAIR(0));

	}
}

void draw_bar(int height, int width, int start_y, int start_x)
{
	bar=newwin(height, width, start_y, start_x);
	box(bar, 0,0);
	wrefresh(bar);
	
}

void erase_bar(int height, int width, int start_y, int start_x)
{
	wclear(bar); wrefresh(bar);
	delwin(bar);
	bar=NULL;
	refresh();
	
}

void get_input()
{
	 int ch;
	 ch=getch();
	 
	 if(ch!=ERR) { key=ch; }
}
