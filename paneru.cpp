// paneru: launcher panel in your terminal
// pondda@protonmail.com
// 2024
// GNU General Public License, version 3 (GPL-3.0)

#include "paneru.h"

// utils
int halfDiff(int a, int b) {
	return (a - b) / 2;
}

std::string repeatCh(int n, char c) {
	if (n>0) return std::string(n, c);
	else return "";
}

/*
CMD RUNNER ---------------------------------------------------------
UI element that runs the specified system command
bArg = false: runs cmd w/o arguments when clicked -- i.e. a button.
bArg = true: accepts user input as arguments for cmd,
and runs it when enter is hit -- i.e. an input field.
*/

CmdRunner::CmdRunner(std::string _cmd, bool _bArg, int _posx, int _posy, int _sizex, int _sizey, std::string _label){
	bArg = _bArg;
	cmd = _cmd;
	posx = _posx;
	posy = _posy;
	sizex = _sizex;
	sizey = _sizey;
	label = _label;
}

void CmdRunner::reset() {
	state = neutral;
	argStr.clear();
}

void CmdRunner::runCommand() {
	// enclose argStr in single quotes so that it doesn't exapand
	// and supress stdout and stderr
	if (bArg) system((cmd + " " + "'" + argStr + "'" + " >/dev/null 2>&1 &").c_str());
	else system((cmd + " >/dev/null 2>&1 &").c_str());
	reset();
}

void CmdRunner::onSelected() {
	if (!bArg) runCommand();
	else state = argEdit;
}

bool CmdRunner::getIsOverlapped(int x, int y) {
	return ( (x >= posx && x < posx + sizex) && (y >= posy && y < posy + sizey) );
}

// drawing on screen with ncurses ------------------------

std::string CmdRunner::centrePad(std::string text) {
	int half = halfDiff(sizex, text.size());

	std::string s = "│";
	s += repeatCh(half-1, ' ');
	s += text;
	s += repeatCh(sizex - s.size() + 1, ' ');
	s += "│";

	return s;
}

std::string CmdRunner::rightPad(std::string text) {
	int n = int(sizex - 2 - text.size());

	std::string s = "│" + text;
	s += repeatCh(n, ' ');
	s += "│";

	return s;
}

void CmdRunner::drawTop(int offsetX, int offsetY) {
	int x = offsetX + posx;
	int y = offsetY + posy;

	// weird character, don't use repeatCh()!
	std::string s = "╭";
	for (int i=0; i<sizex-2; i++) s += "─";
	s += "╮";
	
	mvprintw(y, x, s.c_str());
}

void CmdRunner::drawMid(int offsetX, int offsetY) {
	std::string s = "│";
	s += repeatCh(sizex-2, ' ');
	s += "│";
	
	int x = offsetX + posx;
	int y = offsetY + posy;
	mvprintw(y, x, s.c_str());
}

void CmdRunner::drawMidWithText(int offsetX, int offsetY) {
	int x = offsetX + posx;
	int y = offsetY + posy;

	std::string t = state == argEdit? argStr + "|": label;
	std::string s;
	if (bArg) s = rightPad(t);
	else s = centrePad(t);

	mvprintw(y, x, s.c_str());
}

void CmdRunner::drawBottom(int offsetX, int offsetY) {
	// weird character, don't use repeatCh()!
	std::string s = "╰";
	for (int i=0; i<sizex-2; i++) s += "─";
	s += "╯";

	int x = offsetX + posx;
	int y = offsetY + posy + sizey - 1;
	mvprintw(y, x, s.c_str());
}

void CmdRunner::draw(int offsetX, int offsetY) {
	std::string s;
	if (bArg && state == argEdit) s = argStr;
	else s = label;

	if (state == pressed) attron(COLOR_PAIR(1));
	drawTop(offsetX, offsetY);
	for (int i=0; i<sizey-2; i++) {
		if (i == (sizey-2)/2) drawMidWithText(offsetX, offsetY+i+1);
		else drawMid(offsetX, offsetY+i+1);
	}
	drawBottom(offsetX, offsetY);
	if (state == pressed) attroff(COLOR_PAIR(1));
}

/*
PANEL -----------------------------------------------------------------------------
manages & draws CmdRunners on a grid,
and interfaces with key/mouse input
*/
Panel::Panel(int _numx, int _numy, int _sizex, int _sizey, int _gapx, int _gapy) {
	numx = _numx;
	numy = _numy;
	sizex = _sizex;
	sizey = _sizey;
	gapx = _gapx;
	gapy = _gapy;
}

// setup -----------------------------------------------------

int Panel::getRunnerPos(int idx, int size, int gap) {
	return (size * idx) + (gap * (idx +  1));
}

int Panel::getRunnerSize(int scale, int size) {
	return size * scale;
}

void Panel::addCmdRunner(std::string cmd, bool bArg, int idxx, int idxy, int scalex, int scaley, std::string label) {
	// position
	int px = getRunnerPos(idxx, sizex, gapx);
	int py = getRunnerPos(idxy, sizey, gapy);
	// size
	int sx = getRunnerSize(scalex, sizex);
	int sy = getRunnerSize(scaley, sizey);

	CmdRunner *r = new CmdRunner(cmd, bArg, px, py, sx, sy, label);
	runners.push_back(r);
}

void Panel::addCmdRunners(std::vector<RunnerConf *> confs) {
	for (RunnerConf *c: confs) {
		addCmdRunner(c->cmd, c->bArg, c->idxx, c->idxy, c->scalex, c->scaley, c->label);
	}
}

void Panel::initCols() {
	start_color();
	use_default_colors();
	init_pair(1, COLOR_CYAN, -1);
}

void Panel::setBRunMain(bool *_bRunMain) {
	bRunMain = _bRunMain;
}

// drawing buttons on screen ----------------------------------
// recenters the panel when screen is resized

void Panel::calculateOffset(int scrx, int scry) {
	int canvasx = numx * sizex + (numx + 1) * gapx;
	offsetx = (scrx - canvasx) / 2;
	int canvasy = numy * sizey + (numy + 1) * gapy;
	offsety = (scry - canvasy) / 2;
}

void Panel::draw(int scrx, int scry) {
	calculateOffset(scrx, scry);
	for (CmdRunner *r: runners) r->draw(offsetx, offsety);
}


// user interaction ------------------------------------------------------------
// mouse events are weird: "clicked" and "pressed" are separate events,
// and sometimes "clicked" and "released" fire in quick succession.
// this is accounted for and seems to work on all of my machines(TM)
// and peripherals, but could use further testing

void Panel::reset() {
	if (currRunner) currRunner -> reset();
	currRunner = nullptr;
}

CmdRunner *Panel::getOverlappedRunner(int x, int y) {
	for (CmdRunner *r: runners) {
		if (r->getIsOverlapped(x - offsetx, y - offsety)) return r;
	}
	return nullptr;
}

// press down on button to highlight it
void Panel::mpressed(int x, int y) {
	CmdRunner *r = getOverlappedRunner(x, y);
	if (r) {
		if (currRunner && currRunner != r) reset();
		currRunner = r;
		if (currRunner->state == neutral) currRunner->state = pressed;
	}
}

void Panel::mclicked(int x, int y) {
	CmdRunner *r = getOverlappedRunner(x, y);
	if (r) { 
		// check if mouse was dragged off of another button
		if (currRunner) { 
			if (currRunner == r) currRunner->onSelected();
			else reset();
		} else {
			currRunner = r;
			currRunner->onSelected();
		}
	} else reset(); // no button at event location
}

void Panel::mreleased(int x, int y) {
	// call mclicked only if the button was pressed down in previous tick
	if (currRunner) {
		if (currRunner->state == pressed || currRunner->state == argEdit) { 
			mclicked(x, y);
		}
	}
}

// key input: edit argStr of current input field
// enter to run
// escape to reset
// q while not editing arg to quit program
void Panel::key(int c) {
	if (currRunner && currRunner->state == argEdit) {
		switch (c) {
			case KEY_BACKSPACE:
				if (!currRunner->argStr.empty()) currRunner->argStr.pop_back();
				break;
			case '\n':
				currRunner -> runCommand();
				reset();
				break;
			case '\e':
				reset();
				break;
			default:
				// cull single quotes
				// see CmdRunner::RunCommand()
				if (c != '\'') {
					currRunner -> argStr += c;
				}
				break;
		}

	} else if (c == 'q') *bRunMain = false;
}
