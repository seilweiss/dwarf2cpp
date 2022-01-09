#pragma once

#include "elf.h"

#include <map>
#include <vector>
#include <iostream>
#include <unordered_map>

#define DW_TAG_padding                0x0000
#define DW_TAG_array_type             0x0001
#define DW_TAG_class_type             0x0002
#define DW_TAG_entry_point            0x0003
#define DW_TAG_enumeration_type       0x0004
#define DW_TAG_formal_parameter       0x0005
#define DW_TAG_global_subroutine      0x0006
#define DW_TAG_global_variable        0x0007
#define DW_TAG_label                  0x000a
#define DW_TAG_lexical_block          0x000b
#define DW_TAG_local_variable         0x000c
#define DW_TAG_member                 0x000d
#define DW_TAG_pointer_type           0x000f
#define DW_TAG_reference_type         0x0010
#define DW_TAG_compile_unit           0x0011
#define DW_TAG_source_file            0x0011
#define DW_TAG_string_type            0x0012
#define DW_TAG_structure_type         0x0013
#define DW_TAG_subroutine             0x0014
#define DW_TAG_subroutine_type        0x0015
#define DW_TAG_typedef                0x0016
#define DW_TAG_union_type             0x0017
#define DW_TAG_unspecified_parameters 0x0018
#define DW_TAG_variant                0x0019
#define DW_TAG_common_block           0x001a
#define DW_TAG_common_inclusion       0x001b
#define DW_TAG_inheritance            0x001c
#define DW_TAG_inlined_subroutine     0x001d
#define DW_TAG_module                 0x001e
#define DW_TAG_ptr_to_member_type     0x001f
#define DW_TAG_set_type               0x0020
#define DW_TAG_subrange_type          0x0021
#define DW_TAG_with_stmt              0x0022
#define DW_TAG_lo_user                0x4080
#define DW_TAG_hi_user                0xffff

#define DW_FORM_ADDR   0x1
#define DW_FORM_REF    0x2
#define DW_FORM_BLOCK2 0x3
#define DW_FORM_BLOCK4 0x4
#define DW_FORM_DATA2  0x5
#define DW_FORM_DATA4  0x6
#define DW_FORM_DATA8  0x7
#define DW_FORM_STRING 0x8

#define DW_AT_sibling              (0x0010|DW_FORM_REF)
#define DW_AT_location             (0x0020|DW_FORM_BLOCK2)
#define DW_AT_name                 (0x0030|DW_FORM_STRING)
#define DW_AT_fund_type            (0x0050|DW_FORM_DATA2)
#define DW_AT_mod_fund_type        (0x0060|DW_FORM_BLOCK2)
#define DW_AT_user_def_type        (0x0070|DW_FORM_REF)
#define DW_AT_mod_u_d_type         (0x0080|DW_FORM_BLOCK2)
#define DW_AT_ordering             (0x0090|DW_FORM_DATA2)
#define DW_AT_subscr_data          (0x00a0|DW_FORM_BLOCK2)
#define DW_AT_byte_size            (0x00b0|DW_FORM_DATA4)
#define DW_AT_bit_offset           (0x00c0|DW_FORM_DATA2)
#define DW_AT_bit_size             (0x00d0|DW_FORM_DATA4)
#define DW_AT_element_list         (0x00f0|DW_FORM_BLOCK4)
#define DW_AT_stmt_list            (0x0100|DW_FORM_DATA4)
#define DW_AT_low_pc               (0x0110|DW_FORM_ADDR)
#define DW_AT_high_pc              (0x0120|DW_FORM_ADDR)
#define DW_AT_language             (0x0130|DW_FORM_DATA4)
#define DW_AT_member               (0x0140|DW_FORM_REF)
#define DW_AT_discr                (0x0150|DW_FORM_REF)
#define DW_AT_discr_value          (0x0160|DW_FORM_BLOCK2)
#define DW_AT_string_length        (0x0190|DW_FORM_BLOCK2)
#define DW_AT_common_reference     (0x01a0|DW_FORM_REF)
#define DW_AT_comp_dir             (0x01b0|DW_FORM_STRING)
#define DW_AT_const_value_string   (0x01c0|DW_FORM_STRING)
#define DW_AT_const_value_data2    (0x01c0|DW_FORM_DATA2)
#define DW_AT_const_value_data4    (0x01c0|DW_FORM_DATA4)
#define DW_AT_const_value_data8    (0x01c0|DW_FORM_DATA8)
#define DW_AT_const_value_block2   (0x01c0|DW_FORM_BLOCK2)
#define DW_AT_const_value_block4   (0x01c0|DW_FORM_BLOCK4)
#define DW_AT_containing_type      (0x01d0|DW_FORM_REF)
#define DW_AT_default_value_addr   (0x01e0|DW_FORM_ADDR)
#define DW_AT_default_value_data2  (0x01e0|DW_FORM_DATA2)
#define DW_AT_default_value_data8  (0x01e0|DW_FORM_DATA8)
#define DW_AT_default_value_string (0x01e0|DW_FORM_STRING)
#define DW_AT_friends              (0x01f0|DW_FORM_BLOCK2)
#define DW_AT_inline               (0x0200|DW_FORM_STRING)
#define DW_AT_is_optional          (0x0210|DW_FORM_STRING)
#define DW_AT_lower_bound_ref      (0x0220|DW_FORM_REF)
#define DW_AT_lower_bound_data2    (0x0220|DW_FORM_DATA2)
#define DW_AT_lower_bound_data4    (0x0220|DW_FORM_DATA4)
#define DW_AT_lower_bound_data8    (0x0220|DW_FORM_DATA8)
#define DW_AT_program              (0x0230|DW_FORM_STRING)
#define DW_AT_private              (0x0240|DW_FORM_STRING)
#define DW_AT_producer             (0x0250|DW_FORM_STRING)
#define DW_AT_protected            (0x0260|DW_FORM_STRING)
#define DW_AT_prototyped           (0x0270|DW_FORM_STRING)
#define DW_AT_public               (0x0280|DW_FORM_STRING)
#define DW_AT_pure_virtual         (0x0290|DW_FORM_STRING)
#define DW_AT_return_addr          (0x02a0|DW_FORM_BLOCK2)
#define DW_AT_specification        (0x02b0|DW_FORM_REF)
#define DW_AT_start_scope          (0x02c0|DW_FORM_DATA4)
#define DW_AT_stride_size          (0x02e0|DW_FORM_DATA4)
#define DW_AT_upper_bound_ref      (0x02f0|DW_FORM_REF)
#define DW_AT_upper_bound_data2    (0x02f0|DW_FORM_DATA2)
#define DW_AT_upper_bound_data4    (0x02f0|DW_FORM_DATA4)
#define DW_AT_upper_bound_data8    (0x02f0|DW_FORM_DATA8)
#define DW_AT_virtual              (0x0300|DW_FORM_STRING)
#define DW_AT_lo_user              (0x2000)
#define DW_AT_hi_user              (0x3ff0)
// User attributes
#define DW_AT_mangled_name         (DW_AT_lo_user|DW_FORM_STRING)

#define DW_OP_REG     0x01
#define DW_OP_BASEREG 0x02
#define DW_OP_ADDR    0x03
#define DW_OP_CONST   0x04
#define DW_OP_DEREF2  0x05
#define DW_OP_DEREF   0x06
#define DW_OP_DEREF4  0x06
#define DW_OP_ADD     0x07
#define DW_OP_lo_user 0xe0
#define DW_OP_hi_user 0xff

#define DW_FT_char             0x0001
#define DW_FT_signed_char      0x0002
#define DW_FT_unsigned_char    0x0003
#define DW_FT_short            0x0004
#define DW_FT_signed_short     0x0005
#define DW_FT_unsigned_short   0x0006
#define DW_FT_integer          0x0007
#define DW_FT_signed_integer   0x0008
#define DW_FT_unsigned_integer 0x0009
#define DW_FT_long             0x000a
#define DW_FT_signed_long      0x000b
#define DW_FT_unsigned_long    0x000c
#define DW_FT_pointer          0x000d
#define DW_FT_float            0x000e
#define DW_FT_dbl_prec_float   0x000f
#define DW_FT_ext_prec_float   0x0010
#define DW_FT_complex          0x0011
#define DW_FT_dbl_prec_complex 0x0012
#define DW_FT_void             0x0014
#define DW_FT_boolean          0x0015
#define DW_FT_ext_prec_complex 0x0016
#define DW_FT_label            0x0017
#define DW_FT_lo_user          0x8000
#define DW_FT_hi_user          0xffff

#define DW_FT_long_long          0x8008
#define DW_FT_signed_long_long   0x8108
#define DW_FT_unsigned_long_long 0x8208

#define DW_MOD_pointer_to   0x01
#define DW_MOD_reference_to 0x02
#define DW_MOD_const        0x03
#define DW_MOD_volatile     0x04
#define DW_MOD_lo_user      0x80
#define DW_MOD_hi_user      0xff

#define DW_LANG_C89         0x00000001
#define DW_LANG_C           0x00000002
#define DW_LANG_ADA83       0x00000003
#define DW_LANG_C_PLUS_PLUS 0x00000004
#define DW_LANG_COBOL74     0x00000005
#define DW_LANG_COBOL85     0x00000006
#define DW_LANG_FORTRAN77   0x00000007
#define DW_LANG_FORTRAN90   0x00000008
#define DW_LANG_PASCAL83    0x00000009
#define DW_LANG_MODULA2     0x0000000a
#define DW_LANG_lo_user     0x00008000
#define DW_LANG_hi_user     0x0000ffff

#define DW_ORD_row_major 0
#define DW_ORD_col_major 1

#define DW_FMT_FT_C_C 0x0
#define DW_FMT_FT_C_X 0x1
#define DW_FMT_FT_X_C 0x2
#define DW_FMT_FT_X_X 0x3
#define DW_FMT_UT_C_C 0x4
#define DW_FMT_UT_C_X 0x5
#define DW_FMT_UT_X_C 0x6
#define DW_FMT_UT_X_X 0x7
#define DW_FMT_ET     0x8

#define SHT_DWARF1 0x70000005

class Dwarf
{
public:
	enum Error
	{
		ERR_NONE = 0,
		ERR_NO_DWARF,
		ERR_INVALID_ENTRY,
		ERR_INVALID_ATTRIBUTE
	};

	struct Attribute;
	struct Entry;

	struct Attribute
	{
		Dwarf* dwarf;
		int entryIndex;
		Elf32_Off offset;
		Elf32_Half name;
		Elf32_Word size;
		void *value;

		inline Elf32_Half getForm()
		{
			return name & 0xf;
		}

		inline Elf32_Addr getAddress()
		{
			return dwarf->read<Elf32_Addr>(value);
		}

		inline Elf32_Off getReference()
		{
			return dwarf->read<Elf32_Off>(value);
		}

		inline char* getBlock()
		{
			return (char*)value;
		}

		inline Elf32_Half getHword()
		{
			return dwarf->read<Elf32_Half>(value);
		}

		inline Elf32_Word getWord()
		{
			return dwarf->read<Elf32_Word>(value);
		}

		inline uint64_t getDword()
		{
			return dwarf->read<uint64_t>(value);
		}

		inline char* getString()
		{
			return (char*)value;
		}
	};

	struct Entry
	{
		Dwarf *dwarf;
		Elf32_Off offset;
		int index;
		Elf32_Word length;
		Elf32_Half tag;
		std::vector<Attribute> attributes;

		inline bool isNullEntry()
		{
			return length < 8;
		}

		inline Entry* getSibling()
		{
			if (index == dwarf->entries.size() - 1)
				return nullptr;

			size_t numAttributes = attributes.size();

			for (size_t i = 0; i < numAttributes; i++)
			{
				Attribute *attr = &attributes[i];

				if (attr->name == DW_AT_sibling)
				{
					Elf32_Off offset = attr->getReference();
					Entry *sibling = dwarf->getEntryFromReference(offset);

					if (sibling)
						return sibling;

					break;
				}
			}

			return this + 1;
		}
	};

	struct LineEntry
	{
		int lineNumber;
		short charOffset;
		int hexAddressOffset;
	};

	std::multimap<int, LineEntry> lineEntryMap;
	std::vector<Entry> entries;

	Dwarf(ElfFile *elf)
	{
		m_error = ERR_NONE;
		m_elf = elf;

		m_section = m_elf->getSectionHeader(".debug");

		if (!m_section)
		{
			m_error = ERR_NO_DWARF;
			return;
		}

		m_sectionData = m_elf->getSectionData(m_section);
		m_sectionSize = m_section->sh_size;

		Elf32_Off offset = 0;

		while (offset < m_sectionSize && !m_error)
			offset = readEntry(offset);

		// Read debug line data.
		Elf32_Shdr* m_lineHeader;
		m_lineHeader = m_elf->getSectionHeader(".line");
		if (m_lineHeader)
		{
			char* m_lineSectionDataStart;
			char* m_lineSectionData;
			m_lineSectionDataStart = m_elf->getSectionData(m_lineHeader);
			m_lineSectionData = m_lineSectionDataStart;

			while (m_lineHeader->sh_size > (int)(m_lineSectionData - m_lineSectionDataStart)) {
				int byteSize;
				int funcPtr;
				char* m_lineSectionDataChunkEnd;
				m_lineSectionDataChunkEnd = m_lineSectionData;

				byteSize = read<int>(m_lineSectionData);
				m_lineSectionData += sizeof(int);
				m_lineSectionDataChunkEnd += byteSize;

				funcPtr = read<int>(m_lineSectionData);
				m_lineSectionData += sizeof(int);

				while (m_lineSectionDataChunkEnd > m_lineSectionData) {
					LineEntry entry;

					entry.lineNumber = read<int>(m_lineSectionData);
					m_lineSectionData += sizeof(int);
					entry.charOffset = read<short>(m_lineSectionData);
					m_lineSectionData += sizeof(short);
					entry.hexAddressOffset = read<int>(m_lineSectionData);
					m_lineSectionData += sizeof(int);
					lineEntryMap.insert(std::pair<int, LineEntry>(funcPtr, entry));
					if (entry.lineNumber == 0)
						break; // End.
				}
			}
		}
	}

	Elf32_Off readEntry(Elf32_Off offset, int *outIndex = nullptr)
	{
		Entry entry;

		entry.dwarf = this;
		entry.index = entries.size();
		entry.offset = offset;
		entry.length = read<Elf32_Word>(m_sectionData + offset);

		m_entryIndexRefMap[offset] = entry.index;

		Elf32_Word end = offset + entry.length;

		if (entry.isNullEntry()) // Null entry
			offset = end;
		else
		{
			offset += sizeof(Elf32_Word);

			entry.tag = read<Elf32_Half>(m_sectionData + offset);
			offset += sizeof(Elf32_Half);

			while (offset < end && !m_error)
				offset = readAttribute(offset, &entry);

			if (offset > end)
			{
				m_error = ERR_INVALID_ENTRY;
				return 0;
			}
		}

		entries.push_back(entry);

		if (outIndex)
			*outIndex = entry.index;

		return offset;
	}

	Elf32_Off readAttribute(Elf32_Off offset, Entry *entry, int *outIndex = nullptr)
	{
		Attribute attribute;

		attribute.dwarf = entry->dwarf;
		attribute.entryIndex = entry->index;
		attribute.offset = offset;
		attribute.name = read<Elf32_Half>(m_sectionData + offset);
		offset += sizeof(Elf32_Half);

		Elf32_Half form = attribute.getForm();

		switch (form)
		{
		case DW_FORM_ADDR:
			attribute.size = sizeof(Elf32_Addr);
			break;
		case DW_FORM_REF:
			attribute.size = sizeof(Elf32_Off);
			break;
		case DW_FORM_BLOCK2:
			attribute.size = read<Elf32_Half>(m_sectionData + offset);
			offset += sizeof(Elf32_Half);
			break;
		case DW_FORM_BLOCK4:
			attribute.size = read<Elf32_Word>(m_sectionData + offset);
			offset += sizeof(Elf32_Word);
			break;
		case DW_FORM_DATA2:
			attribute.size = sizeof(Elf32_Half);
			break;
		case DW_FORM_DATA4:
			attribute.size = sizeof(Elf32_Word);
			break;
		case DW_FORM_DATA8:
			attribute.size = sizeof(uint64_t);
			break;
		case DW_FORM_STRING:
			attribute.size = strlen(m_sectionData + offset) + 1;
			break;
		default:
			m_error = ERR_INVALID_ATTRIBUTE;
			return 0;
		}

		attribute.value = m_sectionData + offset;

		int index = entry->attributes.size();

		entry->attributes.push_back(attribute);
		
		if (outIndex)
			*outIndex = index;

		return offset + attribute.size;
	}

	inline Error getError()
	{
		return m_error;
	}

	inline Entry* getEntryFromReference(Elf32_Off ref)
	{
		if (m_entryIndexRefMap.count(ref) == 0)
			return nullptr;

		return &entries[m_entryIndexRefMap[ref]];
	}

	inline Elf32_Off pointerToOffset(char *ptr)
	{
		return ptr - m_sectionData;
	}

	inline char* offsetToPointer(Elf32_Off offset)
	{
		return m_sectionData + offset;
	}

	template<class T>
	inline T read(void *data)
	{
		return m_elf->read<T>(data);
	}

private:
	Error m_error;

	ElfFile *m_elf;
	Elf32_Shdr *m_section;
	char *m_sectionData;
	Elf32_Word m_sectionSize;

	std::unordered_map<Elf32_Off, int> m_entryIndexRefMap;
};
