#include "TextArea.h"
#include <string>

bool g_exitApp = false;

void clear_bar(WINDOW* bar) {
	wclear(bar);
	int i;
	for(i=0;i<COLS;i++) waddch(bar, ' ');
	wmove(bar, 0, 0);
}

int main(int argc, char** args)
{
	WINDOW* titlebar;
	WINDOW* statusbar;
	

	initscr();
	noecho();
	raw();
	start_color();
	use_default_colors();
	refresh();

	titlebar  = newwin(1, COLS, 0, 0);
	// statusbar = newwin(1, COLS, LINES-1, 0);

	wattron(titlebar, A_REVERSE);
	// wattron(statusbar, A_REVERSE);

	clear_bar(titlebar);
	// clear_bar(statusbar);
	

	wprintw(titlebar, "Titlebar");
	// wprintw(statusbar, "Statusbar");
	// wprintw(textarea, "Test");
	

	// wrefresh(statusbar); //The cursor will NOT skip a column near the right edge when you comment out this line (???)
	wrefresh(titlebar);
	
	TextArea textArea;
	textArea.DrawBoder();

	while(!g_exitApp) {
		textArea.HanldeEvents();
		textArea.Render();
	}

exit:
	endwin();
	return 0;
}