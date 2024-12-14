make:
	g++ main.cpp paneru.cpp -lncursesw -o paneru
	if [ ! -f ~/.config/paneru.conf ]; then cp example.conf ~/.config/paneru.conf; fi

run: paneru
	./paneru

install: paneru
	cp paneru /bin/

uninstall:
	rm /bin/paneru
