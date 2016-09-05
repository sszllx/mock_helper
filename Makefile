all:
	gcc -Wall -Werror -g -o mock_helper main.c

clean:
	rm -f mock_helper
