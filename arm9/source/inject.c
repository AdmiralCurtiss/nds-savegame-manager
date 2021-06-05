#include <nds.h>
#include <fat.h>
#include <nds/arm9/dldi.h>
#include <unistd.h>
#include <nds/interrupts.h>
#include <nds/arm9/console.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include <dswifi9.h>
#include <nds/card.h>
#include <malloc.h>
#include <stdio.h>

#include "crc.h"

int bin_inject() {
	
	char template[64];
	sprintf(template,"/ereader/template.sav");
	char vpkin[64];
	sprintf(vpkin,"/ereader/tempgarfieldinject.sav");
			FILE *injectbin, *blanksav, *file, *lendi, *titlebin;
			
		
			
						char data[4096];
						char endi[1];
						char title[35];
						int savend;
					
						//char ch, blanksav[20], injectbin[20];
						blanksav = fopen("/ereader/template.sav", "rb");
						injectbin = fopen("/ereader/tempgarfieldinject.bin", "wb");
						titlebin = fopen("/ereader/tempgarfieldtitle.bin", "rb");
						file = fopen("/ereader/tempgarfieldvpk.bin", "rb");
						lendi = fopen("/ereader/tempgarfieldlendi.bin", "w+b");

						int drowning = 0;
							while (drowning < 32) {
						fread(data, 4096, 1, blanksav);
						fwrite(data, 4096, 1, injectbin);
						drowning++;
							}
					//	fread(data, 65536, 1, blanksav);
						//fwrite(data, 65536, 1, injectbin);


						fseek(injectbin, 65540, SEEK_SET);
						fread(title, 35, 1, titlebin);
						fwrite(title, 35, 1, injectbin);
						fputc(0x00, injectbin);
						fputc(0x04, injectbin);
						fputc(0x00, injectbin);
						fputc(0x00, injectbin);
						fputc(0x00, injectbin);



						short vpksize;
						fseek( file, 81, SEEK_SET );
						for(vpksize = 0; getc(file) != EOF; ++vpksize);
						fwrite(&vpksize, 2, 1, lendi);
						fseek( lendi, 0, SEEK_SET );
						fread(endi, 1, 1, lendi);
						fwrite(endi, 1, 1, injectbin);
						fseek( lendi, 1, SEEK_SET );
						fread(endi, 1, 1, lendi);
						fwrite(endi, 1, 1, injectbin);
						fputc(0x00, injectbin);
						fputc(0x00, injectbin);
						fputc(0x00, injectbin);
						fputc(0x00, injectbin);
						fputc(0x00, injectbin);
						fputc(0x00, injectbin);
						fseek( lendi, 0, SEEK_SET );
						fseek( file, 83, SEEK_SET );
						for(vpksize = 0; getc(file) != EOF; ++vpksize);
						fwrite(&vpksize, 2, 1, lendi);
						fseek( lendi, 0, SEEK_SET );
						fread(endi, 1, 1, lendi);
						fwrite(endi, 1, 1, injectbin);
						fseek( lendi, 1, SEEK_SET );
						fread(endi, 1, 1, lendi);
						fwrite(endi, 1, 1, injectbin);
						fseek( lendi, 0, SEEK_SET );
						fseek(file, 0, SEEK_END);
						long vpklength = ftell(file);
						fseek(file, 83, SEEK_SET);  
						char *vpkdata = malloc(vpklength + 1);
						fread(vpkdata, 1, vpklength, file);
						fwrite(vpkdata, 1, vpklength, injectbin);
						fclose(titlebin);
						fclose(injectbin);
						fclose(file);
						fclose(blanksav);
						fclose(lendi);
						crcaitsith();
						
						remove("/ereader/tempgarfieldinject.bin");
						remove("/ereader/tempgarfieldlendi.bin");
						remove("/ereader/tempgarfieldtitle.bin");
						remove("/ereader/tempgarfieldvpk.bin");
							return 0;
}