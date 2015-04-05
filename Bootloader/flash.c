/* Copyright (c) 2011,2012 Simon Schubert <2@0x2c.org>.
 * Modifications by Jacob Alexander 2014-2015 <haata@kiibohd.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// ----- Local Includes -----

#include "mchck.h"



// ----- Variables -----

uint32_t flash_ALLOW_BRICKABLE_ADDRESSES;



// ----- Functions -----

/* This will have to live in SRAM. */
__attribute__((section(".ramtext.ftfl_submit_cmd"), long_call))
int ftfl_submit_cmd(void)
{
	FTFL.fstat.raw = ((struct FTFL_FSTAT_t){
			.ccif = 1,
			//.rdcolerr = 1,
			.accerr = 1,
			.fpviol = 1
		}).raw;

	// Wait for the operation to complete
	struct FTFL_FSTAT_t stat;
	while (!(stat = FTFL.fstat).ccif); // XXX maybe WFI?

	// Mask error bits
	return stat.raw & (FTFL_FSTAT_RDCOLERR | FTFL_FSTAT_ACCERR | FTFL_FSTAT_FPVIOL | FTFL_FSTAT_MGSTAT0);
	//return (!!stat.mgstat0);
}

int flash_prepare_flashing(void)
{
	/* switch to FlexRAM */
	if (!FTFL.fcnfg.ramrdy) {
		FTFL.fccob.set_flexram.fcmd = FTFL_FCMD_SET_FLEXRAM;
		FTFL.fccob.set_flexram.flexram_function = FTFL_FLEXRAM_RAM;
		return (ftfl_submit_cmd());
	}
	return (0);
}

int flash_erase_sector(uintptr_t addr)
{
	if (addr < (uintptr_t)&_app_rom &&
		flash_ALLOW_BRICKABLE_ADDRESSES != 0x00023420)
		return (-1);
	FTFL.fccob.erase.fcmd = FTFL_FCMD_ERASE_SECTOR;
	FTFL.fccob.erase.addr = addr;
	return (ftfl_submit_cmd());
}

int flash_program_section_longwords(uintptr_t addr, size_t num_words)
{
	FTFL.fccob.program_section.fcmd = FTFL_FCMD_PROGRAM_SECTION;
	FTFL.fccob.program_section.addr = addr;
	FTFL.fccob.program_section.num_words = num_words;

	return ftfl_submit_cmd();
}

int flash_program_section_phrases(uintptr_t addr, size_t num_phrases)
{
	FTFL.fccob.program_section.fcmd = FTFL_FCMD_PROGRAM_SECTION;
	FTFL.fccob.program_section.addr = addr;
	FTFL.fccob.program_section.num_words = num_phrases;

	return ftfl_submit_cmd();
}

int flash_program_longword(uintptr_t addr, uint8_t *data)
{
	FTFL.fccob.program_longword.fcmd = FTFL_FCMD_PROGRAM_LONGWORD;
	FTFL.fccob.program_longword.addr = addr;
	FTFL.fccob.program_longword.data_be[0] = data[0];
	FTFL.fccob.program_longword.data_be[1] = data[1];
	FTFL.fccob.program_longword.data_be[2] = data[2];
	FTFL.fccob.program_longword.data_be[3] = data[3];

	return ftfl_submit_cmd();
}

int flash_program_sector(uintptr_t addr, size_t len)
{
#if defined(_mk20dx128vlf5_)
	return (len != FLASH_SECTOR_SIZE ||
		(addr & (FLASH_SECTOR_SIZE - 1)) != 0 ||
		flash_erase_sector(addr) ||
		flash_program_section_longwords(addr, FLASH_SECTOR_SIZE / 4));
#elif defined(_mk20dx256vlh7_)
	/*
	return (len != FLASH_SECTOR_SIZE ||
		(addr & (FLASH_SECTOR_SIZE - 1)) != 0 ||
		flash_erase_sector(addr) ||
		flash_program_section_phrases(addr, FLASH_SECTOR_SIZE / 8));
	*/
	return (len != FLASH_SECTOR_SIZE ||
		(addr & (FLASH_SECTOR_SIZE - 1)) != 0 ||
		flash_erase_sector(addr) ||
		flash_program_section_phrases(addr, FLASH_SECTOR_SIZE / 8));
#endif
}

int flash_prepare_reading(void)
{
	return (0);
}

int flash_read_sector(uintptr_t addr, size_t len)
{
	return (0);
}

void *flash_get_staging_area(uintptr_t addr, size_t len)
{
	if ((addr & (FLASH_SECTOR_SIZE - 1)) != 0 ||
	    len != FLASH_SECTOR_SIZE)
		return (NULL);
	return (FlexRAM);
}

