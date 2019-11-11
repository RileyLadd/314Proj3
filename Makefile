all:
	gcc -g fs.c filefs.c -o filefs
push:
	git add .
	git commit
	git push origin master
