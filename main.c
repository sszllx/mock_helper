
#include <unistd.h>

#include "elf_reader.h"
#include "list.h"

extern char *optarg;

static void write_fun_to_file(int fd, const char *symbol)
{
    char buff[512];
    bzero (buff, 512);

    snprintf(buff, 512, "void __wrap_%s () {\n    __real_%s();\n}\n\n", symbol, symbol);
    write(fd, buff, strlen(buff));
}

static int
create_wrap_file(list_node_t *functions_header,
    const char *filter,
    const char *efilter)
{
    int intpu_fd;
    int wrap_fd;
    char symbol[SYMBOL_LENGTH];

    intpu_fd = open("./wrap_list", O_RDONLY, S_IRUSR);
    if (intpu_fd < 0) {
        perror("wrap list open failed");
        return 1;
    }

    char wrap_name[SYMBOL_LENGTH];
    bzero (wrap_name, SYMBOL_LENGTH);
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

    bzero (symbol, SYMBOL_LENGTH);
    char c;
    int index = 0;
    while (read (intpu_fd, &c, 1) != 0) {
        if (c == 0x0a) {
            if ((filter != NULL &&
                strstr(symbol, filter) == NULL) ||
                (efilter != NULL &&
                strstr(symbol, efilter) != NULL)) {
                bzero (symbol, SYMBOL_LENGTH);
                index = 0;
                continue;
            }

            write_fun_to_file (wrap_fd, symbol);
            if (index == 0) {
                memcpy(functions_header->s_name, symbol, SYMBOL_LENGTH);
            } else {
                list_node_t *node = calloc(sizeof(list_node_t), 1);
                memcpy(node->s_name, symbol, SYMBOL_LENGTH);
                list_insert(node, functions_header);
            }

            bzero (symbol, SYMBOL_LENGTH);
            index = 0;
            continue;
        }
        symbol[index++] = c;
    }

    close(wrap_fd);
    close(intpu_fd);

    return 0;

out_close_input:
    close(intpu_fd);
    return 1;
}

static int create_makefile(list_node_t *functions_header)
{
    FILE *makefile_file;
    list_node_t *pos;

    makefile_file = fopen("./makefile.wrap", "w+r");
    if (makefile_file == NULL) {
        perror("makefile.wrap open failed");
        return 1;
    }

    list_for_each(pos, functions_header) {
        list_node_t *func_node = pos;
        fprintf(makefile_file, "-Wl,--wrap=%s\n", func_node->s_name);
    }

    fclose(makefile_file);

    return 0;
}

int
main(int argc, char *const argv[])
{
    char opt;
    int ret;
    const char *filter = NULL;
    const char *efilter = NULL;
    list_node_t functions_header;

    if(argc < 2) {
        printf ("Usage: mock_helper elf_image [options] [file]\n \
            parameters: \n \
            -f 仅为包含有某字符串的函数生成wrap函数\n \
            -e 不处理包含有某字符串的函数\n");
        return 1;
    }

    while ((opt = getopt(argc, argv, "f:e:")) != EOF) {
        switch (opt) {
            case 'f':
            filter = optarg;
            printf("filter:%s\n", filter);
            break;
            case 'e':
            efilter = optarg;
            break;
        }

    }

    list_init (&functions_header);

    ret = parse_elf_sym (argv[optind]);
    if (ret != 0)
        return 1;

    printf("you can edit wrap_list file and then press 'enter'\n");
    getchar();

    ret = create_wrap_file (&functions_header, filter, efilter);
    if (ret != 0)
        return 1;

    ret = create_makefile(&functions_header);

    return ret;
}
