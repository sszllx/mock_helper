
// #define _LARGEFILE64_SOURCE

#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

#include <elf.h>

void read_elf_header(int fd, Elf64_Ehdr *elf_header)
{
    assert(elf_header != NULL);
    assert(lseek(fd, (off_t)0, SEEK_SET) == (off_t)0);
    assert(read(fd, (void *)elf_header, sizeof(Elf64_Ehdr)) == sizeof(Elf64_Ehdr));
}

char * read_section(int32_t fd, Elf64_Shdr sh)
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

void iter_symbol_table(int fd,
            Elf64_Ehdr eh,
            Elf64_Shdr sh_table[],
            uint32_t symbol_table)
{

    char *str_tbl;
    Elf64_Sym* sym_tbl;
    uint32_t i, symbol_count;

    sym_tbl = (Elf64_Sym*)read_section(fd, sh_table[symbol_table]);

    /* Read linked string-table
     * Section containing the string table having names of
     * symbols of this section
     */
    uint32_t str_tbl_ndx = sh_table[symbol_table].sh_link;
    // debug("str_table_ndx = 0x%x\n", str_tbl_ndx);
    str_tbl = read_section(fd, sh_table[str_tbl_ndx]);

    symbol_count = (sh_table[symbol_table].sh_size/sizeof(Elf64_Sym));
    printf("%d symbols\n", symbol_count);

    for(i=0; i< symbol_count; i++) {
        printf("0x%lx10x ", sym_tbl[i].st_value);
        printf("0x%02x ", ELF64_ST_BIND(sym_tbl[i].st_info));
        printf("0x%02x ", ELF64_ST_TYPE(sym_tbl[i].st_info));
        printf("%d ", sym_tbl[i].st_shndx);
        printf("%s\n", (str_tbl + sym_tbl[i].st_name));
    }
}

void iter_symbols(int fd, Elf64_Ehdr eh, Elf64_Shdr sh_table[])
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

void read_section_header_table(int32_t fd, Elf64_Ehdr eh, Elf64_Shdr sh_table[])
{
    uint32_t i;

    assert(lseek(fd, (off_t)eh.e_shoff, SEEK_SET) == (off_t)eh.e_shoff);

    for(i=0; i<eh.e_shnum; i++) {
        assert(read(fd, (void *)&sh_table[i], eh.e_shentsize)
             == eh.e_shentsize);
    }

}

int main(int argc, char const *argv[])
{
    int fd;
    Elf64_Ehdr eh;      /* elf-header is fixed size */
    Elf64_Shdr* sh_tbl; /* section-header table is variable size */

    if(argc!=2) {
        printf("Usage: elf-parser <ELF-file>\n");
        return 0;
    }

    fd = open(argv[1], O_RDONLY|O_SYNC);
    if(fd<0) {
        printf("Error %d Unable to open %s\n", fd, argv[1]);
        return 0;
    }

    read_elf_header(fd, &eh);

    sh_tbl = calloc(sizeof(Elf64_Shdr), eh.e_shentsize * eh.e_shnum);
    if(!sh_tbl) {
        printf("Failed to allocate %d bytes\n",
            (eh.e_shentsize * eh.e_shnum));
    }

    read_section_header_table(fd, eh, sh_tbl);

    iter_symbols(fd, eh, sh_tbl);

    return 0;
}
