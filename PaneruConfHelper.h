// paneru: launcher panel in your terminal
// pondda@protonmail.com
// 2024
// GNU General Public License, version 3 (GPL-3.0)

#pragma once
#include "paneru.h"
#include <filesystem>
#include <fstream>
#include <sstream>

enum ConfType {none, panel, runner};

class PaneruConfHelper {
	PanelConf pconf;
	std::vector<RunnerConf *> rconfs;
	std::string parseStr(std::string);

	public:
		void parseConf();
		PanelConf getPanelConf();
		std::vector<RunnerConf *> getRunnerConfs();
};

PanelConf PaneruConfHelper::getPanelConf(){
	return pconf;
}

std::vector<RunnerConf *> PaneruConfHelper::getRunnerConfs() {
	return rconfs;
}

std::string PaneruConfHelper::parseStr(std::string s){
	std::string::difference_type n = std::count(s.begin(), s.end(), '"');
	if (n != 2) {
		std::cout << "Invalid string formatting in config file" << std::endl;
		std::cout << "please enclose string with double quotes" << std::endl;
		exit(1);
	}

	std::string::size_type start = 0;
	std::string::size_type end = 0;

	start = s.find('"') + 1;
	s = s.substr(start);
	end = s.find('"');
	s = s.substr(0, end);
	return s;

	/*
	start = s.find('"') + 1;
	end = s.find('"');
	return s.substr(start, end - start);
	*/
}

void PaneruConfHelper::parseConf() {
	if (const char* home = std::getenv("HOME")) {
		std::string path = home;
		path += "/.config/paneru.conf";
		std::ifstream f(path);
		if (f.fail()) {
			std::cout << "Config file could not be opened" << std::endl;
			exit(1);
		}

		ConfType type = none;
		RunnerConf *rconf = nullptr;
		for (std::string line; std::getline(f, line);) {
			std::istringstream ss(line);
			std::string key;
			ss >> key;
			if (key[0] != '#') { // if not a comment
				// check for new block & its type
				// create a new button/inputfield conf if needed
				// and populate the vector with previous button/input conf
				if (key == "[panel]") {
					if (type == runner) rconfs.push_back(rconf);
					type = panel;
				} else if (key == "[button]") {
					if (type == runner) rconfs.push_back(rconf);
					rconf = new RunnerConf;
					rconf->bArg = false;
					type = runner;
				} else if (key == "[input]") {
					if (type == runner) rconfs.push_back(rconf);
					rconf = new RunnerConf;
					rconf->bArg = true;
					type = runner;
				} else {
					// not a new block, set params for current conf
					if (type == panel) {
						if (key == "numx") ss >> pconf.numx;
						else if (key == "numy") ss >> pconf.numy;
						else if (key == "sizex") ss >> pconf.sizex;
						else if (key == "sizey") ss >> pconf.sizey;
						else if (key == "gapx") ss >> pconf.gapx;
						else if (key == "gapy") ss >> pconf.gapy;
					} else if (type == runner) {
						if (key == "idxx") ss >> rconf->idxx;
						else if (key == "idxy") ss >> rconf->idxy;
						else if (key == "scalex") ss >> rconf->scalex;
						else if (key == "scaley") ss >> rconf->scaley;
						else if (key == "cmd") rconf->cmd = parseStr(line);
						else if (key == "label") rconf->label = parseStr(line);
					}
				}
			}
		}
		// EOF: add final button/inputfield conf
		if (type == runner) rconfs.push_back(rconf);

	} else{
		std::cout << "$HOME not found. Could not load config file" <<std::endl;
		exit(1);
	}
}
