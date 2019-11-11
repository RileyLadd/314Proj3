all:
	gcc -g fs.c filefs.c -o filefs
push:
	git add .
	git commit -m "blah blah blah"
	git push origin master
