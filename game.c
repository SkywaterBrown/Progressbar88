/*
-----------------------------------------------------
|                   SEGMENTS                        |
-----------------------------------------------------
| Number |  Name  | Symbol | Probablity |   Score   |
-----------------------------------------------------
|   1    | Blue   |  '+'   |    %25     |     +%5   |
|   2    | Cyan   |  'X'   |    %10     |     +%5   |
|   3    | Yellow |  'n'   |    %25     |     -%5   |
|   4    | Pink   |  '-'   |    %15     |      %0   |
|   5    | Gray   |  '0'   |    %15     |      %0   |
|   6    | Red    |  '!'   |    %8      |     -%100 |
|   7    | Green  |  'W'   |    %2      |     +%100 |
-----------------------------------------------------

Making this table was painful, 'cuz it is HANDMADE.

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ncurses.h>
#include <string.h>
#include <time.h>

#define TARGET_FPS 60
#define FRAME_DURATION_NS (1000000000 / TARGET_FPS)
#define BAR_FILL_COUNT 20 // amount of maximum segments bar can contain
#define MAX_SEGMENTS 20 // amount of maximum segments that can exist in one moment (excpet the ones in the bar)

struct Game{
	char name[15];
	char details[3][128];
	char credits[61];
};

int row, col, isdebug, round_end, system_failure, godly;
int key;
WINDOW *bar;
WINDOW *menu_window;


struct BarStruct{
	int X, Y, height, width;
	char include[BAR_FILL_COUNT + 1]; // 1 more for NULL term
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
void opening(struct Game game);
void menu(struct Game game, struct BarStruct *bar);
void pause_game();
void reset_variables(struct Segments segments[MAX_SEGMENTS], int *segment_count_ptr, struct BarStruct *bar);
void draw_bar(struct BarStruct *Bar, int height, int width, int start_y, int start_x);
void erase_bar(int height, int width, int start_y, int start_x);
void draw_segments(struct Segments segments[MAX_SEGMENTS], int segment_count);
void erase_segments(struct Segments segments[MAX_SEGMENTS], int segment_count);
void handle_segments(struct Segments segments[MAX_SEGMENTS], int *segment_count_ptr, struct BarStruct *Bar);
int calculate_score(struct BarStruct *Bar);

int main(int argc, char *argv[])
{
	 if(argc==2 && !strcmp(argv[1], "debug")) { isdebug=1; }
	 
	 struct Game game = {
		 "Progressbar 88", // Name
		 { // Detais ( line by line )
			"A casual arcade game",
			"Inspired from Progressbar 95 mobile game",
			"Full screen is recomended"
		 },	
		 "a game by Skywater, Kartopu, Kuftopagi and Limon 2025 - 2026" // Credits
	};
	  
	initscr();
	cbreak();
	noecho();
	curs_set(0);
	keypad(stdscr, TRUE);
	nodelay(stdscr, TRUE);
	mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
	mouseinterval(0);
	if(has_colors() == TRUE) { start_color(); if (isdebug) mvprintw(1,20, "Has colors."); }
	
	struct Mouse MOUSE;
	MEVENT event; // Mouse event
	getmaxyx(stdscr, row, col);
	srand(time(NULL));
	 
	if(has_colors())
	{
		if(can_change_color())
		{
			init_color(COLOR_BLACK, 0, 0, 0);
			init_color(COLOR_RED, 1000, 0, 0);
			init_color(COLOR_GREEN, 0, 1000, 0);
			init_color(COLOR_YELLOW, 1000, 1000, 0);
			init_color(COLOR_BLUE, 0 ,0, 1000);
			init_color(COLOR_MAGENTA, 1000, 0, 1000);
			init_color(COLOR_CYAN, 0, 1000, 1000);
			init_color(COLOR_WHITE, 1000, 1000, 1000);
		}
		
		init_pair(0, COLOR_WHITE, COLOR_BLACK);  // Default
		init_pair(1, COLOR_WHITE, COLOR_BLUE);   // Blue Segment
		init_pair(2, COLOR_WHITE, COLOR_CYAN);   // Cyan Segment
		init_pair(3, COLOR_WHITE, COLOR_YELLOW); // Yellow Segment
		init_pair(4, COLOR_WHITE, COLOR_MAGENTA);// Pink Segment
		init_pair(5, COLOR_BLACK, COLOR_WHITE);  // Gray Segment
		init_pair(6, COLOR_WHITE, COLOR_RED);    // Red Segment
		init_pair(7, COLOR_WHITE, COLOR_GREEN);  // Green Segment
		init_pair(8, COLOR_BLACK, COLOR_GREEN);  // Godly Mode Color
		attron(COLOR_PAIR(0));
	}
	else // No color support? Black & White then...
	{
		for(int abcdef = 0; abcdef <= 8; abcdef++)
			init_pair(abcdef, COLOR_WHITE, COLOR_BLACK);
	}
	 
	clear();
	
	struct timespec start, end;
	
	struct BarStruct Bar;
	Bar.height=3;
	Bar.width=22;
	Bar.X=(col/2)-(Bar.width/2);
	Bar.Y=(row/2)-(Bar.height/2);
	memset(&Bar.include, '\0', sizeof(Bar.include));
	
	struct Segments segments[MAX_SEGMENTS];
	int segment_count=0;
	
	opening(game);
	round_end = 0;
	
	while(1)
	{
		key = 0;
		clock_gettime(CLOCK_MONOTONIC, &start);
		 
		get_input();
		
		if(key==27 || key=='q') { goto E; }
		else if(key == KEY_RIGHT || key == 'd' || key == 5) { Bar.X++; }
		else if(key == KEY_LEFT  || key == 'a' || key == 4) { Bar.X--; }
		else if(key == KEY_DOWN  || key == 's' || key == 2) { Bar.Y++; }
		else if(key == KEY_UP    || key == 'w' || key == 3) { Bar.Y--; }
		
		else if (key == 'p') // pause the game untill any key is pressed
			pause_game();
		
		if(key == KEY_MOUSE)
		{
			if(getmouse(&event) == OK)
			{
				if (event.bstate & BUTTON3_PRESSED) // pause the game if right mouse key is pressed
					pause_game();
				
				MOUSE.X = event.x;
				MOUSE.Y = event.y;
				
				if(isdebug) mvprintw(0, 0, "Mouse Event: x=%d, y=%d, bstate=%d", event.x, event.y, event.bstate);
				
				
				Bar.X = MOUSE.X - (Bar.width / 2);
				Bar.Y = MOUSE.Y - (Bar.height / 2);
			}
		}
		
		if (Bar.Y < 0) { Bar.Y=0; }
		else if (Bar.Y > row-Bar.height) { Bar.Y=row-Bar.height; }
		if (Bar.X < 0) { Bar.X = 0; } // FUCK NCURSES (GO TO LINE 456 TO SEE WHY)
		else if (Bar.X > col- Bar.width / 2) { Bar.X=col - Bar.width / 2; }
		        
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
		
		
		if (round_end == 1) 
		{
			round_end = 0;
			menu(game, &Bar);
			reset_variables(segments, &segment_count, &Bar);
		}
	}
	 
	 E:
	 endwin();
	 printf("%s, %s\n\n",game.name, game.credits);
	 return(0);
}

void handle_segments(struct Segments segments[MAX_SEGMENTS], int *segment_count_ptr, struct BarStruct *Bar)
{
	int i, j, random, do_generate;
	int segment_count = *segment_count_ptr;  // Get local copy of segment_count from pointer
	
	do_generate = rand()%40 + 1;
	if(do_generate == 8 && segment_count < MAX_SEGMENTS) // generate a segment with 1/40 posibility
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
		int last_char;
		last_char = strlen(Bar->include);
		int is_collided = 0;  // Initialize to 0
		int del_last = 0;
		int add_double = 0;
		godly = 0;
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
					if(segments[i].symbol == 'W') // Green Segment: instant win
					{
						memset(Bar->include, 'W', BAR_FILL_COUNT);
						Bar->include[BAR_FILL_COUNT] = '\0';
						
						godly = 1;
						
					}
					else if(segments[i].symbol == '!') // Red Segment: system failure
					{
						system_failure = 1; round_end = 1;
					}
					else if(segments[i].symbol == '-') // Minus Segment: delete last segment from bar
					{
						del_last = 1;
					}
					else if(segments[i].symbol == 'X') // Double Segment: add two blue segments
					{
						add_double = 1;
					}
					else if(segments[i].symbol == '0') // Blank Segment: Add a blank segment
					{
						// NULL
					}
					
					filled_area = strlen(Bar->include);
					if(filled_area >= BAR_FILL_COUNT) // check if bar is filled
					{
						round_end = 1;
					}
					else // add more segments until bar is filled
					{
						is_collided = 1;
						if(isdebug) mvprintw(3, 0, "Collision at: X:%d, Y:%d", seg_x, seg_y);
						
						last_char = strlen(Bar->include);
						if(del_last != 1)
						{
							Bar->include[last_char] = segments[i].symbol;
							Bar->include[last_char + 1] = '\0';
						} // Add char to bar
						
						if(del_last == 1) // Delete last char
						{
							del_last = 0;
							last_char = strlen(Bar->include) - 1;
							if(last_char >= 0) Bar->include[last_char] = '\0';
						}
						else if (add_double == 1) // Add one MORE cyan
						{
							add_double = 0;
							last_char = strlen(Bar->include) - 1;
							filled_area = strlen(Bar->include);
							if(filled_area <= BAR_FILL_COUNT){
								Bar->include[last_char + 1] = 'X';
								if(filled_area <= BAR_FILL_COUNT)
									Bar->include[last_char + 2] = '\0';
							}
						}
					}
					if(filled_area >= BAR_FILL_COUNT) // check if bar is filled
					{ round_end = 1; }
				}
			}
	if(isdebug) mvprintw(4, 0, "Bar includes: %s  Total: %d  ", Bar->include, last_char);
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
	if(filled_area >= BAR_FILL_COUNT) // check if bar is filled
		{ round_end = 1; }
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
	
	if(bar != NULL)
	{
		wclear(bar);
		wrefresh(bar);
		delwin(bar);
	}
	
	bar=newwin(height, width, start_y, start_x);
	if(godly == 1) { wattron(bar, COLOR_PAIR(8)); attron(COLOR_PAIR(8));} // Godly Bar
	
	if( box(bar, 0,0) ) { }
// 	else // handling the bar borders if Bar.x < 0 
// 	{
// 		
// 		char h_edge = 196;
// 		char right_top_corner = 191;
// 		char right_bot_corner = 217;
// 		
// 		int start_x = Bar->width - strlen(Bar->include);
// 		int n_times = Bar->width - start_x;
// 		
// 		mvwhline(bar, 0, start_x, h_edge, n_times); // in order to find this function I dive deep into the ncurses library files...  No, actually I was bored, so I decided to do so. LOL
// 		mvwhline(bar, Bar->height, start_x, h_edge, n_times);
// 		mvwprintw(bar, 0, Bar->width, "%s", &right_top_corner);
// 		mvwprintw(bar, Bar->height, Bar->width, "%s", &right_bot_corner);
// 	}
// I DECIDED NOT TO HANDLE THIS.
// SINCE NCURSES DOES NOT LET ME CREATE A WINDOW BEYOND LEFT EDGE OF THE SCREEN (ex: start_x = -5),
// I DECIDED TO MAKE IT IMPOSIBLE TO GO BEYOND LEFT EDGE FOR BAR_FILL_COUNT
// 'CUZ I HAVE OTHER STUFF TO DO AND I DO NOT WANT TO SPEND MORE TIME ON THIS PROJECT 
	
	while(i < BAR_FILL_COUNT)
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
		if(godly == 1) { wattron(bar, COLOR_PAIR(8));} // Godly Bar
		mvwprintw(bar, 1, i + 1, "%c", Bar->include[i]);
		attron(COLOR_PAIR(0));
		attroff(COLOR_PAIR(0));
		i++;
	}
	if(godly == 1) godly = 0;
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

void opening(struct Game game)
{
	int n;
	int one_o_third_y = row / 3;
	
	// Right now I am making this over enginered, but I like to do so, hehehe...
	int str_len[16]; // I think 16 is unecessary but who cares
	int max_lines_det = sizeof(game.details) / sizeof(game.details[0]);
	
	str_len[0] = strlen(game.name);
	
	for(n = 0; n < max_lines_det; n++)
		str_len[n + 1] = strlen(game.details[n]);
	
	str_len[max_lines_det + 1] = strlen(game.credits);
	
	
	mvprintw(one_o_third_y + 0, (col/2) - (strlen(game.name) / 2), "%s", game.name);
	
	for(n = 0; n < max_lines_det; n++)
		mvprintw(one_o_third_y + 3 + n, (col/2) - (strlen(game.details[n]) / 2), "%s", game.details[n]);
	
	mvprintw(one_o_third_y + 4 + n, (col/2) - (strlen(game.credits) / 2), "%s", game.credits);
	mvprintw(one_o_third_y + 10, (col/2) - (strlen("Press any key to continue...") / 2), "Press any key to continue...");
	
	nodelay(stdscr, FALSE);
	getch();
	nodelay(stdscr, TRUE);
	
	clear();
}

void menu(struct Game game, struct BarStruct *Bar)
{
	int height, width, start_y, start_x;
	char text[12] = "You scored:";
	int score = calculate_score(Bar);
	char score_text[16];
	
	erase_bar(Bar->height, Bar->width, Bar->Y, Bar->X);
	snprintf(score_text, sizeof(score_text),"%%%d", score);
	
	height  = row / 2;
	width   = col / 2;
	start_y = row / 4;
	start_x = col / 4;
	
	clear();
	refresh();
	
	menu_window = newwin(height, width, start_y, start_x);
	box(menu_window, 0,0);
	
	int name_len = strlen(game.name);
	int text_len = strlen(text);
	int score_len = strlen(score_text);
	mvwprintw(menu_window, 1, (width / 2) - (name_len / 2), "%s", game.name);
	mvwprintw(menu_window, 7, (width / 2) - (text_len / 2),"%s", text);
	mvwprintw(menu_window, 8, (width / 2) - (score_len) / 2,"%s", score_text);
	wrefresh(menu_window);
	
	draw_bar(Bar, Bar->height, Bar->width, start_y + 3, start_x + width / 2 - Bar->width / 2); // Draw the bar after creating menu window to avoid collision
	
	flushinp(); // clear all pending inputs
	
	nodelay(stdscr, FALSE);
	nodelay(menu_window, FALSE);
	
	int ch = getch();
	key = ch;
	
	nodelay(stdscr, TRUE);
	nodelay(menu_window, TRUE);
	
	wclear(menu_window);
	wrefresh(menu_window);
	delwin(menu_window);
	menu_window = NULL;
	
}

void reset_variables(struct Segments segments[MAX_SEGMENTS], int *segment_count_ptr, struct BarStruct *Bar)
{
	int i;
	
	system_failure = 0; // reset global variables
	godly = 0;
	
	memset(Bar->include, '\0', sizeof(Bar->include)); // reset bar
	Bar->X=(col/2)-(Bar->width/2);
	Bar->Y=(row/2)-(Bar->height/2);
	
	
	while (i < MAX_SEGMENTS) // reset segment info
	{
		segments[i].type = 0;
		segments[i].symbol = '\0';
		segments[i].x = 0;
		segments[i].y = 0;
		segments[i].speed = 0;
		
		i++;
	}
	
	*segment_count_ptr = 0; // reset segment count
	
	key = 0;
	
}

int calculate_score(struct BarStruct *Bar)
{
	int i = 0, score = 0;
	
	while(Bar->include[i] != '\0')
	{
		if(Bar->include[i] == '+') score += 5; // Blue segment
		else if(Bar->include[i] == 'X') score += 5; // Cyan segment
		else if(Bar->include[i] == 'n') score -= 5; // Yellow segment
		else if(Bar->include[i] == '!') score -= 100; // Red segment
		else if(Bar->include[i] == 'W') score += 100; // Green segment
		else;
		
		i++;
	}
	return(score);
}

void pause_game()
{
	flushinp();
	nodelay(stdscr, FALSE);
	getch();
	nodelay(stdscr, TRUE);
}

void get_input()
{
	int ch;
	ch=getch();
	if(ch == ERR) key = 0;
	if(ch == KEY_MOUSE) key = KEY_MOUSE;
	else if(ch!=ERR) key=ch;
}
