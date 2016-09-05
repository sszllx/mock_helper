all:
	gcc -Wall -Werror -g -o mock_helper main.c elf_reader.c

clean:
	rm -f mock_helper
