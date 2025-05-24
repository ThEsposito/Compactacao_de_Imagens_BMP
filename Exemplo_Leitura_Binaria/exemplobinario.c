#include <stdio.h>

int main() {
	
	
	const char *FILENAME = "e:\\entrada.BMP"; 
	const char *FILENAME_SAIDA = "e:\\saida.BMP";
    FILE *arquivo = fopen(FILENAME, "rb"); 
    if(arquivo == NULL) {
		printf("Erro na abertura do arquivo de entrada");
		return -1;
	}
	FILE *arquivo_saida = fopen(FILENAME_SAIDA, "wb");
	if(arquivo == NULL) {
		printf("Erro na abertura do arquivo de saída");
		return -1;
	}
	unsigned char byte;
	
	
	while(!feof(arquivo)) {
		fread(&byte, 1, 1, arquivo);
		fwrite(&byte, 1, 1, arquivo_saida);
        printf("%02x ", byte);
	}
	fclose(arquivo);
	fclose(arquivo_saida);
}
	
	