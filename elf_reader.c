
#include "elf_reader.h"

static void read_elf_header(int fd, Elf64_Ehdr *elf_header)
{
    assert(elf_header != NULL);
    assert(lseek(fd, (off_t)0, SEEK_SET) == (off_t)0);
    assert(read(fd, (void *)elf_header, sizeof(Elf64_Ehdr)) == sizeof(Elf64_Ehdr));
}

static char * read_section(int32_t fd, Elf64_Shdr sh)
{
    char* buff = malloc(sh.sh_size);
    if(!buff) {
        printf("%s:Failed to allocate %ld bytes\n",
            __func__, sh.sh_size);
    }

    assert(buff != NULL);
    assert(lseek(fd, (off_t)sh.sh_offset, SEEK_SET) == (off_t)sh.sh_offset);
    assert(read(fd, (void *)buff, sh.sh_size) == sh.sh_size);

    return buff;
}

static void iter_symbol_table(int fd,
            Elf64_Ehdr eh,
            Elf64_Shdr sh_table[],
            uint32_t symbol_table)
{
    Elf64_Sym* sym_tbl;
    char *str_tbl;
    uint32_t i, symbol_count;
    FILE *out_conf_fd;
    const char *symbol;

    out_conf_fd = fopen("./wrap_list", "w");
    if (out_conf_fd == NULL) {
        perror("wrap out conf open failed");
        return;
    }

    sym_tbl = (Elf64_Sym*)read_section(fd, sh_table[symbol_table]);

    uint32_t str_tbl_ndx = sh_table[symbol_table].sh_link;
    str_tbl = read_section(fd, sh_table[str_tbl_ndx]);

    symbol_count = (sh_table[symbol_table].sh_size/sizeof(Elf64_Sym));
    printf("%d symbols\n", symbol_count);

    for(i=0; i< symbol_count; i++) {
        int ret;
        Elf64_Section shndx = sym_tbl[i].st_shndx;
        if (shndx != 0)
            continue;

        symbol = str_tbl + sym_tbl[i].st_name;

        if (!symbol || strlen(symbol) == 0)
            continue;

        ret = fprintf(out_conf_fd, "%s\n", symbol);
        if (ret < 0) {
            perror("fprintf");
            return;
        }
    }

    fclose(out_conf_fd);
}

static void iter_symbols(int fd, Elf64_Ehdr eh, Elf64_Shdr sh_table[])
{
    uint32_t i;

    for(i=0; i<eh.e_shnum; i++) {
        if ((sh_table[i].sh_type==SHT_SYMTAB)
         || (sh_table[i].sh_type==SHT_DYNSYM)) {
            printf("\n[Section %03d]", i);
            iter_symbol_table(fd, eh, sh_table, i);
        }
    }
}

static void read_section_header_table(int32_t fd, Elf64_Ehdr eh, Elf64_Shdr sh_table[])
{
    uint32_t i;

    assert(lseek(fd, (off_t)eh.e_shoff, SEEK_SET) == (off_t)eh.e_shoff);

    for(i=0; i<eh.e_shnum; i++) {
        assert(read(fd, (void *)&sh_table[i], eh.e_shentsize)
             == eh.e_shentsize);
    }

}

int parse_elf_sym(const char *name)
{
    int fd;
    Elf64_Ehdr eh;
    Elf64_Shdr* sh_tbl;

    fd = open(name, O_RDONLY | O_SYNC);
    if(fd<0) {
        printf("Error %d Unable to open %s\n", fd, name);
        return 1;
    }

    read_elf_header(fd, &eh);

    sh_tbl = calloc(sizeof(Elf64_Shdr), eh.e_shentsize * eh.e_shnum);
    if(!sh_tbl) {
        printf("Failed to allocate %d bytes\n",
            (eh.e_shentsize * eh.e_shnum));
        return 1;
    }

    read_section_header_table(fd, eh, sh_tbl);

    iter_symbols(fd, eh, sh_tbl);

    return 0;
}
