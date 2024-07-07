#include "../include/filelib.h"
#include <io.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>


inline int file_write_text(const char* path_file, char* content, int length)
{
	FILE* file;
	errno_t fe = fopen_s(&file, path_file, "w");

	if (fe == 0)
	{
		size_t leng = fwrite(content, 8, length, file);

		fclose(file);
	}
}


inline int file_read_text(const char* path_file, char** out, int* out_length)
{
	FILE* File;
	errno_t fe = fopen_s(&File, path_file, "r");

	if (fe == 0)
	{
		long leng = 0;

		fseek(File, 0, SEEK_END);
		leng = ftell(File);
		fseek(File, 0, SEEK_SET);

		char* o = (char*)malloc(leng + 1);

		long ix = 0;
		int c = 1;
		while (((c = fgetc(File)) != EOF) && ix < leng)
		{
			o[ix] = c;
			ix++;
		}
		o[ix] = 0;

		fclose(File);

		*out_length = ix;
		*out = o;
		return 1;
	}
	return 0;
}
