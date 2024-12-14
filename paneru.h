// paneru: launcher panel in your terminal
// pondda@protonmail.com
// 2024
// GNU General Public License, version 3 (GPL-3.0)

#pragma once
#include <iostream>
#include <ncurses.h>
#include <string>
#include <cstring>
#include <algorithm>
#include <vector>

// config info
struct PanelConf {
	int numx;
	int numy;
	int sizex;
	int sizey;
	int gapx;
	int gapy;
};

struct RunnerConf {
	std::string label;
	std::string cmd;
	bool bArg;
	int idxx;
	int idxy;
	int scalex;
	int scaley;
};

// button state
enum State {neutral, pressed, argEdit};

// CMD RUNNER ------------------------------------------------------
// UI element that runs the specified system command
// bArg = false: runs cmd w/o arguments when clicked -- i.e. a button.
// bArg = true: accepts user input as arguments for cmd,
// and runs it when enter is hit -- i.e an input field.

class CmdRunner {
        std::string cmd;
        bool bArg;
	std::string argStr;
	std::string label;
        int posx, posy, sizex, sizey;
        State state = neutral;
	
	void drawTop(int x, int y);
	void drawMid(int x, int y);
	void drawMidWithText(int x, int y);
	void drawBottom(int x, int y);
	std::string centrePad(std::string);
	std::string rightPad(std::string);

        public:
                CmdRunner(std::string _cmd, bool _bArg, 
				int _posx, int _posy, 
				int _sizex, int _sizey, 
				std::string _label 
			);
                bool getIsOverlapped(int x, int y);
                void draw(int x, int y);
                void onSelected();
		void runCommand();
		void reset();

		friend class Panel;
};

// PANEL --------------------------------------------
// manages & draws CmdRunners on a grid,
// and interfaces with key/mouse input

class Panel {
	std::vector<CmdRunner *> runners;
	CmdRunner *currRunner = nullptr;
	bool *bRunMain;

	// grid management
	int numx, numy;
	int sizex, sizey;
	int gapx, gapy;
	int width, height;
	int offsetx, offsety;

	int getRunnerPos(int idx, int size, int gap);
	int getRunnerSize(int scale, int size);
	void calculateOffset(int scrx, int scry);
	CmdRunner *getOverlappedRunner(int x, int y);
	void reset();

	void addCmdRunner(std::string cmd, bool bArg, 
			int idxx, int idxy, 
			int scalex, int scaley, 
			std::string label
			);


	public:
		Panel(int _numx, int _numy, int _sizex, int _sizey, int _gapx, int _gapy);
		void addCmdRunners(std::vector<RunnerConf *> confs);

		void initCols();
		void setBRunMain(bool *_bRunMain);
		void draw(int scrx, int scry);

		// interaction
		void mclicked(int x, int y);
		void mpressed(int x, int y);
		void mreleased(int x, int y);
		void key(int c);
};

// utils ---------------------------
int halfDiff(int, int);
std::string repeatCh(int n, char c);
