#include <stdio.h>

#define TAM_MAX_VETORES 100

void compactaBMP(int linhas_matriz, int colunas_pixel, int zeros_por_linha, unsigned char matriz_imagem_original[linhas_matriz][(colunas_pixel*3)+zeros_por_linha],
    int lin_inicial, int lin_final, int col_inicial, int col_final,unsigned char R[TAM_MAX_VETORES],
    unsigned char G[TAM_MAX_VETORES], unsigned char B[TAM_MAX_VETORES], int *indice_vetores) {
    
    int qtde_linhas_nessa_recursao = lin_final - lin_inicial + 1;
    int qtde_col_nessa_recursao = col_final - col_inicial + 1;    

    if(qtde_linhas_nessa_recursao<=3 || qtde_col_nessa_recursao<=3){        
        int centro_col = (col_final + col_inicial)/2;                       
        int centro_lin = (lin_final + lin_inicial)/2; 
        int centro_col_byte = centro_col*3;

        R[*indice_vetores] = matriz_imagem_original[centro_lin][centro_col_byte];
        G[*indice_vetores] = matriz_imagem_original[centro_lin][centro_col_byte+1];
        B[*indice_vetores] = matriz_imagem_original[centro_lin][centro_col_byte+2];
        (*indice_vetores)++;
        return;
    }

    int meio_col = (col_final + col_inicial)/2;
    int meio_lin = (lin_final + lin_inicial)/2; 

    compactaBMP(linhas_matriz, colunas_pixel,zeros_por_linha, matriz_imagem_original, lin_inicial, meio_lin, col_inicial, meio_col, R, G, B, indice_vetores);
    compactaBMP(linhas_matriz, colunas_pixel,zeros_por_linha, matriz_imagem_original, lin_inicial, meio_lin, meio_col+1, col_final, R, G, B, indice_vetores);
    compactaBMP(linhas_matriz, colunas_pixel,zeros_por_linha, matriz_imagem_original, meio_lin+1, lin_final, col_inicial, meio_col, R, G, B, indice_vetores);
    compactaBMP(linhas_matriz, colunas_pixel,zeros_por_linha, matriz_imagem_original, meio_lin+1, lin_final, meio_col+1, col_final, R, G, B, indice_vetores);
}

void descompactaBMP(int linhas, int colunas_pixel, int zeros_por_linha, unsigned char matriz_reconstruida[linhas][colunas_pixel*3+zeros_por_linha],
                    int lin_inicial, int lin_final, int col_inicial, int col_final,
                    unsigned char R[], unsigned char G[], unsigned char B[], int *indice_vetores) {
    
    int qtde_linhas = lin_final - lin_inicial + 1;
    int qtde_colunas = col_final - col_inicial + 1;

    if (qtde_linhas <= 3 || qtde_colunas <= 3) {
        for (int i = lin_inicial; i <= lin_final; i++) {
            for (int j = col_inicial; j <= col_final; j++) {
                int col_byte = j * 3;
                matriz_reconstruida[i][col_byte] = R[*indice_vetores];
                matriz_reconstruida[i][col_byte + 1] = G[*indice_vetores];
                matriz_reconstruida[i][col_byte + 2] = B[*indice_vetores];
            }
        }
        (*indice_vetores)++;
        return;
    }

    int meio_lin = (lin_final + lin_inicial) / 2;
    int meio_col = (col_final + col_inicial) / 2;

    descompactaBMP(linhas, colunas_pixel,zeros_por_linha, matriz_reconstruida, lin_inicial, meio_lin, col_inicial, meio_col, R, G, B, indice_vetores);
    descompactaBMP(linhas, colunas_pixel,zeros_por_linha, matriz_reconstruida, lin_inicial, meio_lin, meio_col + 1, col_final, R, G, B, indice_vetores);
    descompactaBMP(linhas, colunas_pixel,zeros_por_linha, matriz_reconstruida, meio_lin + 1, lin_final, col_inicial, meio_col, R, G, B, indice_vetores);
    descompactaBMP(linhas, colunas_pixel, zeros_por_linha,matriz_reconstruida, meio_lin + 1, lin_final, meio_col + 1, col_final, R, G, B, indice_vetores);
}

void compactarImagem(const char *FILENAME, const char *FILENAME_COMPACTADA) {
    FILE *arquivo_entrada = fopen(FILENAME, "rb");

    if(arquivo_entrada == NULL) {
        printf("Erro na abertura do arquivo de entrada\n");
        return;
    }

    unsigned char cabecalho[12];
    unsigned char byte;
    for(int i=0; i<12; i++){        // Lendo até a posição do offset
        fread(&byte, 1, 1, arquivo_entrada);
        cabecalho[i] = byte;
    }

    int offset = (cabecalho[11]<<8) + cabecalho[10];
    int tam_arquivo = (cabecalho[5]<<24) + (cabecalho[4]<<16) + (cabecalho[3]<<8) + cabecalho[2];

    unsigned char cabecalho_completo[offset];
    rewind(arquivo_entrada);

    for(int i=0; i<offset; i++){    // Lendo o cabeçalho completo agora (incluindo o extra, se houver)
        fread(&byte, 1, 1, arquivo_entrada);
        cabecalho_completo[i] = byte;
    }

    int colunas_pixels = (cabecalho_completo[21]<<24) + (cabecalho_completo[20]<<16) + (cabecalho_completo[19]<<8) + cabecalho_completo[18];
    int linhas_pixels = (cabecalho_completo[25]<<24) + (cabecalho_completo[24]<<16) + (cabecalho_completo[23]<<8) + cabecalho_completo[22];

    int linhas = linhas_pixels;
    int colunas = colunas_pixels*3;
    int zeros_por_linha = (4-(colunas%4))%4;

    unsigned char matriz_imagem_original[linhas][colunas+zeros_por_linha];
    for(int i=0; i<linhas; i++){
        for(int j=0; j<colunas+zeros_por_linha; j++){   
            fread(&byte, 1, 1, arquivo_entrada);
            matriz_imagem_original[i][j] = byte;
        }
    }
    fclose(arquivo_entrada);

    int indice_vetores=0;
    unsigned char R[TAM_MAX_VETORES];
    unsigned char G[TAM_MAX_VETORES];
    unsigned char B[TAM_MAX_VETORES];
    compactaBMP(linhas, colunas_pixels, zeros_por_linha, matriz_imagem_original, 0, linhas-1, 0, colunas_pixels-1, R, G, B, &indice_vetores);
    
    FILE *arquivo_compactado = fopen(FILENAME_COMPACTADA, "wb");
    for(int i=0; i<offset; i++){
        fwrite(&cabecalho_completo[i], 1, 1, arquivo_compactado);
    }
    for (int i = 0; i < indice_vetores; i++) {
        fwrite(&R[i], 1, 1, arquivo_compactado);
        fwrite(&G[i], 1, 1, arquivo_compactado);
        fwrite(&B[i], 1, 1, arquivo_compactado);
    }
    fclose(arquivo_compactado);

    printf("Imagem compactada com sucesso!\n");
}

void descompactarImagem(const char *FILENAME_COMPACTADA, const char *FILENAME_DESCOMPACTADA) {
    FILE *arquivo_compactado = fopen(FILENAME_COMPACTADA, "rb");

    if(arquivo_compactado == NULL) {
        printf("Erro na abertura do arquivo de entrada\n");
        return;
    }

    unsigned char cabecalho[12];
    unsigned char byte;
    for(int i=0; i<12; i++){
        fread(&byte, 1, 1, arquivo_compactado);
        cabecalho[i] = byte;
    }

    int offset = (cabecalho[11]<<8) + cabecalho[10];
    unsigned char cabecalho_completo[offset];
    rewind(arquivo_compactado);
    for(int i=0; i<offset; i++){
        fread(&byte, 1, 1, arquivo_compactado);
        cabecalho_completo[i] = byte;
    }

    int colunas_pixels = (cabecalho_completo[21]<<24) + (cabecalho_completo[20]<<16) + (cabecalho_completo[19]<<8) + cabecalho_completo[18];
    int linhas_pixels = (cabecalho_completo[25]<<24) + (cabecalho_completo[24]<<16) + (cabecalho_completo[23]<<8) + cabecalho_completo[22];

    int linhas = linhas_pixels;
    int colunas = colunas_pixels*3;
    int zeros_por_linha = (4-(colunas%4))%4;

    unsigned char matriz_reconstruida[linhas][colunas + zeros_por_linha];
    int indice_vetores = 0;
    unsigned char R[TAM_MAX_VETORES];
    unsigned char G[TAM_MAX_VETORES];
    unsigned char B[TAM_MAX_VETORES];

    fseek(arquivo_compactado, offset, SEEK_SET);     
    while(fread(&byte, 1, 1, arquivo_compactado)) {
        R[indice_vetores] = byte;
        fread(&G[indice_vetores], 1, 1, arquivo_compactado);
        fread(&B[indice_vetores], 1, 1, arquivo_compactado);
        indice_vetores++;
    }
    fclose(arquivo_compactado);

    indice_vetores = 0;
    descompactaBMP(linhas, colunas_pixels, zeros_por_linha, matriz_reconstruida, 0, linhas - 1, 0, colunas_pixels - 1, R, G, B, &indice_vetores);

    FILE *arquivo_descompactado = fopen(FILENAME_DESCOMPACTADA, "wb");
    unsigned char zero_hex = 0x0;

    for(int i=0; i<offset; i++){
        fwrite(&cabecalho_completo[i], 1, 1, arquivo_descompactado);
    }
    for (int i = 0; i < linhas; i++) {
        for(int j=0; j<colunas; j++){
            fwrite(&matriz_reconstruida[i][j], 1, 1, arquivo_descompactado);
        }
        for (int j = 0; j < zeros_por_linha; j++) {
            fwrite(&zero_hex, 1, 1, arquivo_descompactado);
        }
    }
    fclose(arquivo_descompactado);

    printf("Imagem descompactada com sucesso!\n");
}

long int retornaTamanhoArquivo(const char *nomeArquivo){
    FILE *arquivo = fopen(nomeArquivo, "rb");
    if(arquivo==NULL){
        printf("Erro ao abrir o arquivo\n");
        return -1;
    }
    fseek(arquivo, 0, SEEK_END);
    long int tamArquivo = ftell(arquivo);
    fclose(arquivo);
    return tamArquivo;
    }

int main() {
    const char *FILENAME = "imagemOriginal.bmp";
    const char *FILENAME_COMPACTADA = "imagemCompactada.zmp";
    const char *FILENAME_DESCOMPACTADA = "imagemDescompactada.bmp";
    
    int opcao;
    do {
        printf("\nMenu de Operações:\n");
        printf("1. Compactar Imagem\n");
        printf("2. Descompactar Imagem\n");
        printf("3. Sair\n");
        printf("Escolha uma opção: ");
        scanf("%d", &opcao);

        switch(opcao) {
            case 1:
                compactarImagem(FILENAME, FILENAME_COMPACTADA);
                break;
            case 2:
                descompactarImagem(FILENAME_COMPACTADA, FILENAME_DESCOMPACTADA);
                break;
            case 3:
                printf("Até mais!\n");
                break;
            default:
                printf("Opção inválida!\n");
        }
    } while(opcao != 3); //roda até o usuario desejar sair

    // Vai dar problema se o arquivo não tiver sido compactado ou descompactado antes
    long int tamArquivoOriginal = retornaTamanhoArquivo(FILENAME);
    long int tamArquivoCompactado = retornaTamanhoArquivo(FILENAME_COMPACTADA); 
    long int tamArquivoDescompactado = retornaTamanhoArquivo(FILENAME_DESCOMPACTADA);
    
    printf("\nTamanho da imagem Original: %ld bytes\n", tamArquivoOriginal);
    printf("Tamanho da imagem Compactada: %ld bytes\n", tamArquivoCompactado);
    printf("Tamanho da imagem Remontada: %ld bytes\n", tamArquivoDescompactado);

    int porcentagemCompactacao = (int)(((float)(tamArquivoOriginal - tamArquivoCompactado) / tamArquivoOriginal) * 100);
    printf("Porcentagem de compactação: %d%%\n", porcentagemCompactacao);
    printf("\nIntegrantes do grupo:\n");
    printf("   - Theo Espósito Simões Resende    RA: 10721356\n");
    printf("   - Kauê Lima Rodrigues Meneses     RA: 10410594\n");

    return 0;
}