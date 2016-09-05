
#include "elf_reader.h"

static void write_fun_to_file(int fd, const char *symbol)
{
    char buff[512];
    bzero (buff, 512);

    snprintf(buff, 512, "void __wrap__%s () {\n__real_%s();\n}\n\n", symbol, symbol);
    write(fd, buff, strlen(buff));
}

static int create_wrap_file()
{
    int intpu_fd;
    int wrap_fd;
    char symbol[256];

    intpu_fd = open("./wrap_list", O_RDONLY, S_IRUSR);
    if (intpu_fd < 0) {
        perror("wrap list open failed");
        return 1;
    }

    char wrap_name[256];
    bzero (wrap_name, 256);
    printf ("input filename:");
    scanf ("%s", wrap_name);

    if (wrap_name[0] == '\0') {
        printf ("there is no input filename\n");
        goto out_close_input;
    }

    wrap_fd = open (wrap_name, O_WRONLY | O_CREAT, S_IWUSR | S_IRUSR);
    if (wrap_fd < 0) {
        perror ("create wrap file failed");
        goto out_close_input;
    }

    bzero (symbol, 256);
    char c;
    int index = 0;
    while (read (intpu_fd, &c, 1) != 0) {
        if (c == 0x0a) {
            write_fun_to_file (wrap_fd, symbol);

            bzero (symbol, 256);
            index = 0;
            continue;
        }
        symbol[index++] = c;
    }

    close(wrap_fd);
out_close_input:
    close(intpu_fd);
    return 0;
}

int
main(int argc, char const *argv[])
{
    int ret;

    if(argc < 2) {
        printf ("Usage: elf-parser <ELF-file>\n");
        return 0;
    }

    ret = parse_elf_sym (argv[1]);
    if (ret != 0)
        return 1;

    ret = create_wrap_file();

    return ret;
}
