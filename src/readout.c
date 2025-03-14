#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {

	int off = 0;
	if(argc == 2) {
		off = atoi(argv[1]);
		printf("off = %d\n", off);
	}

	FILE* fp = fopen("output.csv", "r"), *fout = fopen("real_out.csv", "w");
	if(!fp || !fout) {
		printf("Couldn't open file\n");
		if(fp)fclose(fp);
		if(fout)fclose(fout);
		return 1;
	}

	short data[2*2*1000];

	int count = 0;
	//while(!feof(fp) && count < 10) {
		fread(data, 2, 4000, fp);
		//for(int i = 0+off; i < 20+off; i++) {
		//	if(i % 4 == 0)printf("\n");
		//	printf("%hi, ", data[i]);
		//}
		//count++;
	//}
	
	for(int i = 0, j = 0; i < 2000; i++, j+=2) {
		fprintf(fout, "%hi, %hi\n", data[j], data[j+1]);
	}

	fclose(fp);
	fclose(fout);
	return 0;
}
