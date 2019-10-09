/**********************************************************/
/* http://www.skyfree.org/linux/references/ELF_Format.pdf */
/**********************************************************/

// Only supports 32-bit files currently :(

#pragma once

#include <cstdint>
#include <cstdio>
#include <cstring>

typedef uint32_t Elf32_Addr;
typedef uint16_t Elf32_Half;
typedef uint32_t Elf32_Off;
typedef int32_t Elf32_Sword;
typedef uint32_t Elf32_Word;

#define EI_MAG0    0
#define EI_MAG1    1
#define EI_MAG2    2
#define EI_MAG3    3
#define EI_CLASS   4
#define EI_DATA    5
#define EI_VERSION 6
#define EI_PAD     7
#define EI_NIDENT 16

#define ELFCLASSNONE 0
#define ELFCLASS32   1
#define ELFCLASS64   2

#define ELFDATANONE  0
#define ELFDATA2LSB  1
#define ELFDATA2MSB  2

#define ET_NONE   0
#define ET_REL    1
#define ET_EXEC   2
#define ET_DYN    3
#define ET_CORE   4
#define ET_LOPROC 0xff00
#define ET_HIPROC 0xffff

#define EM_NONE  0
#define EM_M32   1
#define EM_SPARC 2
#define EM_386   3
#define EM_68K   4
#define EM_88K   5
#define EM_860   7
#define EM_MIPS  8

#define EV_NONE    0
#define EV_CURRENT 1

#define SHN_UNDEF     0
#define SHN_LORESERVE 0xff00
#define SHN_LOPROC    0xff00
#define SHN_HIPROC    0xff1f
#define SHN_ABS       0xfff1
#define SHN_COMMON    0xfff2
#define SHN_HIRESERVE 0xffff

#define swap4(x) (((x >> 24) & 0xff) | ((x << 8) & 0xff0000) |\
	((x >> 8) & 0xff00) | ((x << 24) & 0xff000000))
#define swap2(x) (((x << 8) & 0xff00) | ((x >> 8) & 0x00ff))

struct Elf32_Ehdr
{
	unsigned char e_ident[EI_NIDENT];
	Elf32_Half    e_type;
	Elf32_Half    e_machine;
	Elf32_Word    e_version;
	Elf32_Addr    e_entry;
	Elf32_Off     e_phoff;
	Elf32_Off     e_shoff;
	Elf32_Word    e_flags;
	Elf32_Half    e_ehsize;
	Elf32_Half    e_phentsize;
	Elf32_Half    e_phnum;
	Elf32_Half    e_shentsize;
	Elf32_Half    e_shnum;
	Elf32_Half    e_shstrndx;
};

struct Elf32_Shdr
{
	Elf32_Word sh_name;
	Elf32_Word sh_type;
	Elf32_Word sh_flags;
	Elf32_Addr sh_addr;
	Elf32_Off  sh_offset;
	Elf32_Word sh_size;
	Elf32_Word sh_link;
	Elf32_Word sh_info;
	Elf32_Word sh_addralign;
	Elf32_Word sh_entsize;
};

class ElfFile
{
public:
	enum Error
	{
		ERR_NONE = 0,
		ERR_FILE_NOT_OPEN,
		ERR_FILE_EMPTY,
		ERR_FILE_READ,
		ERR_INVALID_HEADER
	};

	ElfFile(const char *filename)
	{
		m_error = ERR_NONE;

		loadFile(filename);
		initEndian();

		if (m_error)
			return;

		Elf32_Ehdr *ehdr = getElfHeader();

		if (ehdr->e_ident[EI_MAG0] != 0x7f ||
			ehdr->e_ident[EI_MAG1] != 'E' ||
			ehdr->e_ident[EI_MAG2] != 'L' ||
			ehdr->e_ident[EI_MAG3] != 'F' ||
			ehdr->e_ident[EI_CLASS] != ELFCLASS32 ||
			ehdr->e_ident[EI_DATA] == ELFDATANONE ||
			ehdr->e_ident[EI_VERSION] == EV_NONE ||
			read<Elf32_Half>(&ehdr->e_ehsize) != sizeof(Elf32_Ehdr) ||
			read<Elf32_Half>(&ehdr->e_shentsize) != sizeof(Elf32_Shdr) ||
			read<Elf32_Word>(&ehdr->e_version) == EV_NONE)
		{
			m_error = ERR_INVALID_HEADER;
		}
	}

	inline Elf32_Ehdr* getElfHeader() const
	{
		return (Elf32_Ehdr*)m_file;
	}

	inline unsigned char getClass() const
	{
		return getElfHeader()->e_ident[EI_CLASS];
	}

	inline unsigned char getDataEncoding() const
	{
		return getElfHeader()->e_ident[EI_DATA];
	}

	inline Elf32_Shdr* getSectionHeader(Elf32_Half index) const
	{
		return (Elf32_Shdr*)(m_file + getElfHeader()->e_shoff) + index;
	}

	inline char* getSectionName(Elf32_Shdr *shdr) const
	{
		return m_file + getSectionHeader(getElfHeader()->e_shstrndx)->sh_offset + shdr->sh_name;
	}

	inline char* getSectionData(Elf32_Shdr *shdr) const
	{
		return m_file + shdr->sh_offset;
	}

	inline Elf32_Shdr* getSectionHeader(const char *name) const
	{
		for (int i = 0; i < getElfHeader()->e_shnum; i++)
		{
			if (strcmp(getSectionName(getSectionHeader(i)), name) == 0)
				return getSectionHeader(i);
		}

		return nullptr;
	}

	inline Error getError() const
	{
		return m_error;
	}

	template<class T>
	inline T read(void *data)
	{
		T x = *(T*)data;

		if (m_shouldReverseEndian)
		{
			if (sizeof(T) == 2)
				x = swap2((uint16_t)x);
			else
				x = swap4((uint32_t)x);
		}

		return x;
	}

private:
	Error m_error;
	char *m_file;
	bool m_shouldReverseEndian;

	void loadFile(const char *filename)
	{
		FILE *file = fopen(filename, "rb");

		if (!file)
		{
			m_error = ERR_FILE_NOT_OPEN;
			return;
		}

		fseek(file, 0, SEEK_END);
		int size = ftell(file);
		fseek(file, 0, SEEK_SET);

		if (size == 0)
		{
			m_error = ERR_FILE_EMPTY;
			fclose(file);
			return;
		}

		m_file = new char[size];

		size_t bytesRead = fread(m_file, sizeof(char), size, file);
		fclose(file);

		if (bytesRead != size)
		{
			m_error = ERR_FILE_READ;
		}
	}

	inline void initEndian()
	{
		int x = 1;
		m_shouldReverseEndian = (*(char*)&x == 1 && getDataEncoding() != ELFDATA2LSB);
	}
};