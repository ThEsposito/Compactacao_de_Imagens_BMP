#include <stdio.h>

#define TAM_MAX_VETORES 1000

void compactaBMP(int linhas_matriz, int colunas_pixel, unsigned char matriz_imagem_original[linhas_matriz][colunas_pixel*3],
    int lin_inicial, int lin_final, int col_inicial, int col_final,unsigned char R[],
    unsigned char G[], unsigned char B[], int *indice_vetores){
        // Estamos utilizando a var de colunas em formato de pixels. Dessa forma, não vamos perder a noção de
        // cor e podemos só multiplicar o valor por 3 ao acessar a matriz.
        // Como a linha não tem esse parâmetro, acessamos diretamente.

    // CASO BASE:
    int qtde_linhas_nessa_recursao = lin_final-lin_inicial + 1;
    int qtde_col_nessa_recursao = col_final - col_inicial + 1;    

    if(qtde_linhas_nessa_recursao<=3 || qtde_col_nessa_recursao<=3){
        int centro_col = (col_final + col_inicial)/2;
        int centro_lin = (lin_final + lin_inicial)/2; 
        int col_centro_byte = centro_col*3;
        
        R[*indice_vetores] = matriz_imagem_original[centro_lin][col_centro_byte];
        G[*indice_vetores] = matriz_imagem_original[centro_lin][col_centro_byte+1];
        B[*indice_vetores] = matriz_imagem_original[centro_lin][col_centro_byte+2];
        (*indice_vetores)++;

        return;
    }

    int meio_col = (col_final + col_inicial)/2;
    int meio_lin = (lin_final + lin_inicial)/2; 


    // PASSOS RECURSIVOS:
    // Quadrante superior esquerdo:
    compactaBMP(linhas_matriz, colunas_pixel, matriz_imagem_original, lin_inicial, meio_lin, col_inicial, meio_col, R, G, B, indice_vetores);
    
    // Quadrante superior direito:
    compactaBMP(linhas_matriz, colunas_pixel, matriz_imagem_original, lin_inicial, meio_lin, meio_col+1, col_final, R, G, B, indice_vetores);

    // Quadrante inferior esquerdo:
    compactaBMP(linhas_matriz, colunas_pixel, matriz_imagem_original, meio_lin+1, lin_final, col_inicial, meio_col, R, G, B, indice_vetores);

    // Quadrante inferior direito:
    compactaBMP(linhas_matriz, colunas_pixel, matriz_imagem_original, meio_lin+1, lin_final, meio_col+1, col_final, R, G, B, indice_vetores);
}

void descompactaBMP(int linhas, int colunas_pixel, unsigned char matriz_reconstruida[linhas][colunas_pixel*3],
                    int lin_inicial, int lin_final, int col_inicial, int col_final,
                    unsigned char R[], unsigned char G[], unsigned char B[], int *indice_vetores) {
    int qtde_linhas = lin_final - lin_inicial + 1;
    int qtde_colunas = col_final - col_inicial + 1;

    // Caso base: preenche toda a região com o pixel atual dos vetores R, G, B
    if (qtde_linhas <= 3 || qtde_colunas <= 3) {
        int col_inicial_byte = col_inicial*3;
        int col_final_byte = col_final *3;

        for (int i = lin_inicial; i <= lin_final; i++) {
            for (int j = col_inicial_byte; j <= col_final_byte; j+=3) {        // O problema tá aqui ó
                // int j_byte = j * 3;
                // matriz_reconstruida[i][j_byte] = R[*indice_vetores];
                // matriz_reconstruida[i][j_byte + 1] = G[*indice_vetores];
                // matriz_reconstruida[i][j_byte + 2] = B[*indice_vetores];
            }
        }
        (*indice_vetores)++;
        return;
    }

    // Divisão em quadrantes (mesma lógica da compactação)
    int meio_lin = (lin_final + lin_inicial) / 2;
    int meio_col = (col_final + col_inicial) / 2;

    descompactaBMP(linhas, colunas_pixel, matriz_reconstruida, lin_inicial, meio_lin, col_inicial, meio_col, R, G, B, indice_vetores);
    descompactaBMP(linhas, colunas_pixel, matriz_reconstruida, lin_inicial, meio_lin, meio_col + 1, col_final, R, G, B, indice_vetores);
    descompactaBMP(linhas, colunas_pixel, matriz_reconstruida, meio_lin + 1, lin_final, col_inicial, meio_col, R, G, B, indice_vetores);
    descompactaBMP(linhas, colunas_pixel, matriz_reconstruida, meio_lin + 1, lin_final, meio_col + 1, col_final, R, G, B, indice_vetores);
}

void leituraArquivo(); // Vou implementar uma função só pra ler os arquivos, evitando repetições 
                       // desnecessárias no código

int main(){
    const char *FILENAME = "imagemOriginal.bmp";
	const char *FILENAME_COMPACTADA = "imagemCompactada.zmp";
    const char *FILENAME_DESCOMPACTADA = "imagemDescompactada.bmp";

    FILE *arquivo_entrada = fopen(FILENAME, "rb");

    if(arquivo_entrada == NULL) {
		printf("Erro na abertura do arquivo de entrada\n");
		return -1;
	}

    //LEITURA DO CABEÇALHO
    unsigned char cabecalho[12]; // Lendo apenas até o offset

	unsigned char byte;
    for(int i=0; i<12; i++){
        fread(&byte, 1, 1, arquivo_entrada);
        cabecalho[i] = byte;
    }

    // Para juntar esses dois bytes, foi necessário deslocar os bits o byte que vem à esquerda.
    // Dessa forma, corrige-se sua "posição" na hora de converter de hex para dec.
    // 8 bits foram deslocados, porque é necessário deslocar 2 bits à esquerda em hexa. 
    // Cada (1) bit em hex equivale a 4 bits em binário. Como temos 2 bits hex, precisamos deslocar 8 casas
    // à esquerda

    printf("%02x %02x\n", cabecalho[11], cabecalho[10]);
    int offset = (cabecalho[11]<<8) + cabecalho[10]; // Também indica o tam do cabeçalho COMPLETO (incluindo o "extra", que é opcional)
    int tam_arquivo = (cabecalho[5]<<24) + (cabecalho[4]<<16) + (cabecalho[3]<<8) + cabecalho[2] ; // 0x86 0x05 --> 0x0586, tamanho correto do arquivo

    unsigned char cabecalho_completo[offset];
    // Sei que precisar ler o cabeçalho todo de novo não é a melhor abordagem.
    // Porém, não sei se é permitido utilizar funções extras (como o fseek) para acessar o offset imediatamente 
    // (sem armazenar nada) e criar o vetor com esse tamanho.

    // Também é possível aumentar o tamanho do vetor cabecalho, mas seria necessário utilizar alocação dinâmica de memória
    
    rewind(arquivo_entrada); // Move o ponteiro de volta pro inicio do arquivo
    for(int i=0; i<offset; i++){
        fread(&byte, 1, 1, arquivo_entrada);
        cabecalho_completo[i] = byte;
    }
    int colunas_pixels = (cabecalho_completo[21]<<24) + (cabecalho_completo[20]<<16) + (cabecalho_completo[19]<<8) + cabecalho_completo[18]; // Posições 18-21
    int linhas_pixels = (cabecalho_completo[25]<<24) + (cabecalho_completo[24]<<16) + (cabecalho_completo[23]<<8) + cabecalho_completo[22]; // Posições 22-25

    printf("\nTamanho do Arquivo em bytes: %d", tam_arquivo);
	printf("\nOffset da Imagem: %d", offset);
    printf("\nQuantidade de pixels por linha (colunas): %d", colunas_pixels);
    printf("\nQuantidade de linhas de pixels: %d\n", linhas_pixels);


// Leitura da imagem
    int linhas = linhas_pixels; // Não precisava já que os valores são iguais, mas preferi manter a organização
    int colunas = colunas_pixels*3;
    int zeros_por_linha = (4-(colunas%4))%4; // Garantir que a qtde de bytes das colunas seja mult de 4

    unsigned char matriz_imagem_original[linhas][colunas+zeros_por_linha];
    unsigned char zero_hex = 0x0;
    for(int i=0; i<linhas; i++){
        for(int j=0; j<colunas; j++){ 
            fread(&byte, 1, 1, arquivo_entrada);
            matriz_imagem_original[i][j] = byte;
        }
        for(int k=0; k<zeros_por_linha; k++){
            matriz_imagem_original[i][colunas+k]=zero_hex;
        }
    }

    fclose(arquivo_entrada);


    // COMPACTAÇÃO DA IMAGEM
    int indice_vetores=0;
    unsigned char R[TAM_MAX_VETORES];
    unsigned char G[TAM_MAX_VETORES];
    unsigned char B[TAM_MAX_VETORES];
    compactaBMP(linhas, colunas_pixels, matriz_imagem_original, 0, linhas-1, 0, colunas_pixels-1, R, G, B, &indice_vetores);
    
    FILE *arquivo_compactado = fopen(FILENAME_COMPACTADA, "wb");
    // Gravação da Imagem Compactada
    for(int i=0; i<offset; i++){
        fwrite(&cabecalho_completo[i], 1, 1, arquivo_compactado);
    }

    for (int i = 0; i < indice_vetores; i++) {
        fwrite(&R[i], 1, 1, arquivo_compactado);
        fwrite(&G[i], 1, 1, arquivo_compactado);
        fwrite(&B[i], 1, 1, arquivo_compactado);
    }
    fclose(arquivo_compactado);        
    
    FILE *arquivo_descompactado = fopen(FILENAME_DESCOMPACTADA, "wb");
	
    unsigned char matriz_reconstruida[linhas][colunas + zeros_por_linha];
    indice_vetores = 0;
    descompactaBMP(linhas, colunas_pixels, matriz_reconstruida, 0, linhas - 1, 0, colunas_pixels - 1, R, G, B, &indice_vetores);
    
    for(int i=0; i<offset; i++){        // Cabeçalho
        fwrite(&cabecalho_completo[i], 1, 1, arquivo_descompactado);
    }

    for (int i = 0; i < linhas; i++) {  // Cores
        // Gravar pixels da linha
        for(int j=0; j<colunas; j++){
            fwrite(&matriz_reconstruida[i][j], 1, 1, arquivo_descompactado);
        }
        
        // Gravar padding (zeros) se necessário
        for (int j = 0; j < zeros_por_linha; j++) {
            fwrite(&zero_hex, 1, 1, arquivo_descompactado);
        }
    }

    fclose(arquivo_descompactado);
    return 0;
}