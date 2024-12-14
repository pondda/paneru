// paneru: launcher panel in your terminal
// pondda@protonmail.com
// 2024
// GNU General Public License, version 3 (GPL-3.0)

#include "paneru.h"
#include "PaneruConfHelper.h"

int main(){
	// load config from ~/.config/paneru.conf
	PaneruConfHelper helper;
	helper.parseConf();
	PanelConf pconf = helper.getPanelConf();
	std::vector<RunnerConf *> rconfs = helper.getRunnerConfs();

	Panel panel = Panel(	pconf.numx, 	pconf.numy,
				pconf.sizex, 	pconf.sizey,
				pconf.gapx, 	pconf.gapy
			);
	panel.addCmdRunners(rconfs);

	// set up ncurses
	setlocale(LC_ALL, "");
	initscr();
	noecho();
	curs_set(0);
	cbreak();
	keypad(stdscr, TRUE);
	mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
	printf("\033[?1003h\n"); // enable mouse movement

	bool bRunMain = true;
	panel.setBRunMain(&bRunMain);
	panel.initCols();

	int scrx, scry;

	while (bRunMain) {
		erase();
		getmaxyx(stdscr, scry, scrx);
		panel.draw(scrx, scry);

		// pass mouse/key events to paneru
		int c = wgetch(stdscr);
		if (c == KEY_MOUSE) {
			MEVENT event;
			if (getmouse(&event) == OK) {
				if (event.bstate & BUTTON1_CLICKED) {
					panel.mclicked(event.x, event.y);
				} else if (event.bstate & BUTTON1_PRESSED) {
					panel.mpressed(event.x, event.y);
				} else if (event.bstate & BUTTON1_RELEASED) {
					panel.mreleased(event.x, event.y);
				}
			}
		} else panel.key(c);

		refresh();
	}

	printf("\033[?1003l\n"); // disable mouse movement
	endwin();
	return 0;
}
