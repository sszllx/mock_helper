all:
	gcc -Wall -Werror -g -o mock_helper main.c elf_reader.c list.c

clean:
	rm -f mock_helper
