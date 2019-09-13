#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xmn_crc32.h"

XMNCRC32::XMNCRC32()
{
	memset(crc32_table, 0, sizeof(unsigned int) * 0xff);
	InitCRC32Table();
}

XMNCRC32::~XMNCRC32()
{
}

unsigned int XMNCRC32::Reflect(unsigned int ref, char ch)
{
	unsigned int value(0);
	for (int i = 1; i < (ch + 1); i++)
	{
		if (ref & 1)
			value |= 1 << (ch - i);
		ref >>= 1;
	}
	return value;
}

void XMNCRC32::InitCRC32Table()
{
	unsigned int ulPolynomial = 0x04c11db7;

	// 256 values representing ASCII character codes.
	for (int i = 0; i <= 0xFF; i++)
	{
		crc32_table[i] = Reflect(i, 8) << 24;

		for (int j = 0; j < 8; j++)
		{
			crc32_table[i] = (crc32_table[i] << 1) ^ (crc32_table[i] & (1 << 31) ? ulPolynomial : 0);
		}
		//if (i == 1)printf("old2--i=%d,crc32_table[%d] = %lu\r\n",i,i,crc32_table[i]);
		crc32_table[i] = Reflect(crc32_table[i], 32);
	}
}

int XMNCRC32::GetCRC(unsigned char *buffer, unsigned int dwSize)
{
	unsigned int crc(0xffffffff);
	int len;

	len = dwSize;
	// Perform the algorithm on each character
	// in the string, using the lookup table values.
	while (len--)
		crc = (crc >> 8) ^ crc32_table[(crc & 0xFF) ^ *buffer++];
	// Exclusive OR the result with the beginning value.
	return crc ^ 0xffffffff;
}
