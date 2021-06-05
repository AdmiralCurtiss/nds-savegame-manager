/* Nintendo E-Reader save CRC calculation tool
** Copyrighted by CaitSith2
**
**
**
** Credits
** Sven Reifegerste ( http://rcswww.urz.tu-dresden.de/~sr21/ ) - Used the generate_crc_table() and reflect() from his
**		CRC calculation C source code.
*/

#include <stdio.h>

void generate_crc_table();
unsigned long Calc_crc(unsigned int size);
unsigned long reflect (unsigned long crc, int bitnum);

unsigned long crctab[256];


unsigned char savdata[0x10000];
unsigned char savheader[0x10000];


unsigned long Calc_crc(unsigned int size)
{
    unsigned long initial=0xAA478422;
    unsigned int dataptr=0;
    unsigned long data=0;
    unsigned long data2=0;
	generate_crc_table();	//We need a CRC table to calculate the CRC with.

    for(dataptr=4;dataptr<size+4;dataptr++)
    {
        data = initial >> 8;
        data2 = savdata[dataptr];
        initial = initial ^ data2;
        initial = initial & 0xFF;
		initial = crctab[initial];
        initial = initial ^ data;
    }
 
    return initial;
}

unsigned long reflect (unsigned long crc, int bitnum) {

	// reflects the lower 'bitnum' bits of 'crc'

	unsigned long i, j=1, crcout=0;

	for (i=(unsigned long)1<<(bitnum-1); i; i>>=1) {
		if (crc & i) crcout|=j;
		j<<= 1;
	}
	return (crcout);
}


void generate_crc_table() {

	// make CRC lookup table used by table algorithms
	// E-Reader uses the standard CRC32 table, but a
	// custom routine.

	int i, j;
	unsigned long bit, crc;
	unsigned long crchighbit = 0x80000000;
	unsigned long crcmask = 0xFFFFFFFF;
	unsigned long polynom = 0x4c11db7;
	int order = 32;

	for (i=0; i<256; i++) {

		crc=(unsigned long)i;
		crc=reflect(crc, 8);
		crc<<= order-8;

		for (j=0; j<8; j++) {

			bit = crc & crchighbit;
			crc<<= 1;
			if (bit) crc^= polynom;
		}			

		crc = reflect(crc, order);
		crc&= crcmask;
		crctab[i]= crc;
	}
}



int crcaitsith()
{
	unsigned int datasize=0;
	unsigned long CRC;
	FILE *f;

	//printf("Nintendo E-Reader save CRC calculator\n");
	//printf("by CaitSith2\n\n");

	//if(argc < 2)
	
	//	printf("Usage: savCRC.exe <Infile> [<Outfile]\n\n");
	//	printf("<infile> = File to be corrected\n");
	//	printf("<outfile> = corrected file to be outputted\n\n");
	//	printf("If <outfile> is not specified, then the infile\n");
	//	printf("will be used as the <outfile>\n");

	
	f=fopen("/ereader/tempgarfieldinject.bin","rb");

	fread(savheader,1,0x10000,f);
	fread(savdata,1,0x10000,f);
	fclose(f);

	datasize = savdata[0x2C];
	datasize += savdata[0x2D] << 8;
	datasize += savdata[0x2E] << 16;
	datasize += savdata[0x2F] << 24;
	datasize += savdata[0x30];
	datasize += savdata[0x31] << 8;
	datasize += savdata[0x32] << 16;
	datasize += savdata[0x33] << 24;
	datasize += 0x30;

	CRC=Calc_crc(datasize);

	savdata[3] = (CRC & 0xFF000000) >> 24;
	savdata[2] = (CRC & 0x00FF0000) >> 16;
	savdata[1] = (CRC & 0x0000FF00) >> 8;
	savdata[0] = (CRC & 0x000000FF);

	
	f=fopen("/ereader/tempgarfieldinject.sav","wb");
	fwrite(savheader,1,0x10000,f);
	fwrite(savdata,1,0x10000,f);
	fclose(f);
	

	return 0;
}