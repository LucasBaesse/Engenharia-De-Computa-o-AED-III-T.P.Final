/*
Programa feito por:

Arthur Vinicius - 666455;
Lucas Baesse - 667339;
Pedro Grojpen - 664255;
Rafael Ferreira - 651158;

Usamos como parâmetro do programa as variáveis globais que estão definidas.
Para medição de tempo, utilizamos o código disponibilizado no Eitas.
*/


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <windows.h>
#include <winbase.h>
#include <psapi.h>

//Tamanho do Bucket.
#define Buckets_Qtde 2
//Profundidade inicial.
#define Prof_Ini 2
//Tamanho anotacoes.
#define Tam_Anot 50


/*
Estrutura que define o Diretorio.
Composta por variavel que define a profundidade do diretorio e outra que define o vetor do diretorio.
*/
typedef struct Diretorio{
    unsigned int ProfundidadeDir;
    unsigned int *Diretorio;
}Diretorio;
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------

/*
Estrutura do bucket.
Com variavel da chave principal, CPF e iendereco do indece, que e o endereco do conteudo no arquivo mestre.
*/
typedef struct Bucket{
    int cpf;
    int fimIndice;
}Bucket;
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------

/*
Estrutura que define o arquivo do Indice.
Cria um apontador para um bucket e tem uma variavel da pronfundidade local do arquivo indice.
*/
typedef struct Indice{
    unsigned int profundidadeLocal;
    Bucket *buckets;
}Indice;
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------

/*
Estrutura que define o registro do arquivo mestre.
Sem Indice.
Carregando as informacoes de nome, dados e sexo.
*/
typedef struct ConteudoMestre{
    unsigned int cpf;
    char nome[16];
    char data[16];
    char sexo[1];
    char anotacoes[Tam_Anot];
}ConteudoMestre;
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------

/*
Funcao hash.
*/
int FuncaoHash(int cpf, int prof){
    return cpf % (int)pow(2,prof);
}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------

/*
Preenche os arquivos com o valor "val".
A função é chamada na função criaArquivos() para popular os índices com 0 (indicando que está vazio)
*/
void PopulaArq(FILE *file, int tam, int val){
    for(int i = 0; i < tam; i++){
         fputc(val,file);
    }
}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------

/*
Função que retorna o tamanho de um arquivo.
*/
int ArqTam(FILE *file){
    fseek(file, 0, SEEK_END);
    return ftell(file);
}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------

/*
Função que retorna quantidade de bytes(tamanho) do indice.
4 bytes: profundidade local
8 bytes: CPF e endereco indice
*/
int IndiceTam(){
    return (4 + (8 * Buckets_Qtde));
}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------

/*
Função que retorna a quantidade de bytes (tamanho) de uma posicao do arquivo mestre.
4 bytes: CPF
16 bytes: nome
16 bytes: dados
1 bytes: sexo
Tam_Anot: Anotacao
*/
int MestreTam(){
    return (4 + 16 + 16 + 1 + Tam_Anot);
}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------

/*
Função que carrega o indice em uma determinada poisicao desejada (i).
*/
Indice CarregaIndice(int i){
    FILE * file_Indice = fopen("arquivoIndice.dat","rb+");
    Indice ind;
    if(ArqTam(file_Indice) <= 0){
        printf("Erro, arquivo não encontrado!");
    }else{
        fseek(file_Indice,i * IndiceTam(), SEEK_SET);
        fread(&ind.profundidadeLocal,sizeof(int), 1, file_Indice);
        if(ind.profundidadeLocal < Prof_Ini){
            ind.profundidadeLocal = Prof_Ini;
        }
        ind.buckets = malloc(sizeof(Bucket) * Buckets_Qtde);
        for(int j = 0; j < Buckets_Qtde; j++){
            fread(&ind.buckets[j].cpf, sizeof(int), 1, file_Indice);
            fread(&ind.buckets[j].fimIndice, sizeof(int), 1, file_Indice);
        }
    }
    fclose(file_Indice);
    return ind;
}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------

/*
Função que salva o indice na posicao desejada.
*/
void SalvarIndice(Indice ind, int pos){
    int d;
    FILE * file_Indice = fopen("arquivoIndice.dat", "rb+");
    fseek(file_Indice, pos * IndiceTam(), SEEK_SET);
    d = fwrite(&ind.profundidadeLocal,sizeof(int), 1, file_Indice);
    for(int i = 0; i < Buckets_Qtde; i++){
        fwrite(&ind.buckets[i].cpf,sizeof(int), 1, file_Indice);
        fwrite(&ind.buckets[i].fimIndice,sizeof(int), 1, file_Indice);
    }
    fclose(file_Indice);
}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------

/*
Carrega o arquivo mestre na posicao desejada.
*/
ConteudoMestre CarregaMestre(int fimIndice){
    FILE * file_mestre = fopen("arquivoMestre.dat", "rb+");
    ConteudoMestre dados;
    if(ArqTam(file_mestre) <= 0){
        printf("Erro, arquivo nao encontrado!");
    }else{
        fseek(file_mestre, fimIndice * MestreTam(), SEEK_SET);
        fread(&dados.cpf, sizeof(unsigned int), 1, file_mestre);
        fread(&dados.data, sizeof(char), 16, file_mestre);
        fread(&dados.nome,sizeof(char), 16, file_mestre);
        fread(&dados.sexo,sizeof(char), 1, file_mestre);
        fread(&dados.anotacoes,sizeof(char), Tam_Anot,file_mestre);
    }
    fclose(file_mestre);
    return dados;
}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------

/*
Salva o arquivo mestre na posicao desejada.
*/
void SalvarMestre(ConteudoMestre dados, int fimIndice){
    FILE *file_mestre = fopen("arquivoMestre.dat", "rb+");
    fseek(file_mestre,fimIndice * MestreTam(), SEEK_SET);
    fwrite(&dados.cpf, sizeof(unsigned int), 1, file_mestre);
    fwrite(&dados.data, sizeof(char), 16, file_mestre);
    fwrite(&dados.nome, sizeof(char), 16, file_mestre);
    fwrite(&dados.sexo, sizeof(char), 1, file_mestre);
    fwrite(&dados.anotacoes, sizeof(char), Tam_Anot,file_mestre);

    fclose(file_mestre);
}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------

/*
Carrega o arquivo diretorio.
*/
void carregaDiretorio(Diretorio *dir){
    FILE *file_dir = fopen("arquivoDir.dat", "rb+");
    int n = ArqTam(file_dir);
    if(n <= 0){
        printf("Erro, arquivo não encontrado!");
    }else{
        (*dir).Diretorio = malloc(n);
        n/=sizeof(int);
        (*dir).ProfundidadeDir = log2(n);
        rewind(file_dir);
        fread((*dir).Diretorio,sizeof(int), n, file_dir);
    }
    fclose(file_dir);
}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------

/*
Salva o vetor diretorio no arquivo diretorio.
*/
void salvaDiretorio(Diretorio dir){
    FILE *file_dir = fopen("arquivoDir.dat","wb");
    fwrite(dir.Diretorio,sizeof(int), pow(2, dir.ProfundidadeDir), file_dir);
    fclose(file_dir);
}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------

/*
Dobra a profundidade do diretorio.
*/
void dobraDiretorio(Diretorio *dir){
    int n = pow(2, (*dir).ProfundidadeDir);
    (*dir).ProfundidadeDir++;
    unsigned int * newDiretorio=malloc(n * 2 * sizeof(int));
    for(int i = 0; i < (n * 2); i++){
        newDiretorio[i] = (*dir).Diretorio[i%n];
    }
    free((*dir).Diretorio);
    (*dir).Diretorio = newDiretorio;
}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------

/*
Aumenta um indice no arquivo e retorna posicao do indice.
*/
int incrementaIndice(){
    FILE *file_Indice = fopen("arquivoIndice.dat","rb+");

    int pos = ArqTam(file_Indice);
    int size = IndiceTam();
    for(int i = 0; i < size; i++){
        fputc(0, file_Indice);
    }
    fclose(file_Indice);
    pos /= size;
    return pos;
}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------

/*
Procura e retorna o primeiro bucket vazio.
*/
int procuraBucketVazio(Indice ind){
    for(int i = 0; i < Buckets_Qtde; i++){
        if(ind.buckets[i].cpf == 0){
            return i;
        }
    }
    return -1;
}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------

/*
Procura conteudo utilizando o CPF.
*/
int procuraPorCPF(int cpf, Indice ind){
	for(int i = 0; i < Buckets_Qtde; i++){
		if(ind.buckets[i].cpf == cpf){
			return i;
		}
	}
	return -1;
}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------

/*
Metodo que cria todos os arquivos: Diretorio, indice e mestre.
*/
void criaArquivos(){
    //Arquivo Diretorio
    FILE *file_dir = file_dir = fopen("arquivoDir.dat","wb+");
    for(int i = 0; i < 4; i++){
        fwrite(&i,sizeof(int),1,file_dir);
    }
    int byteSize = ArqTam(file_dir);
    fclose(file_dir);

    //Arquivo Indice
    FILE *file_Indice=fopen("arquivoIndice.dat","wb+");
    PopulaArq(file_Indice,IndiceTam()*4,0);
    byteSize=ArqTam(file_Indice);
    fclose(file_Indice);

    //Arquivo Mestre
    FILE *file_mestre=fopen("arquivoMestre.dat","wb+");
    byteSize=ArqTam(file_mestre);
    fclose(file_mestre);
}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------

/*
Metodo que imprime diretorios.
*/
void imprimeDiretorio(Diretorio dir){
    int n = pow(2, dir.ProfundidadeDir);
    printf("Diretorio\n");
    printf("Diretorio(%i): [", n);
    for(int i = 0; i < n; i++){
        printf("%i,", dir.Diretorio[i]);
    }
    printf("]\n");
    printf("\n");
}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------

/*
Metodo que imprime indeces.
*/
void imprimeIndices(){
    FILE *file_Indice = fopen("arquivoIndice.dat","rb+");
    int n = ArqTam(file_Indice)/IndiceTam();
    Indice ind;
    printf("INDICE\n");
    for(int i = 0; i < n; i++){
        ind = CarregaIndice(i);
        printf("Indice [%i]:\n\tP': %i\n\tBUCKETS: \n", i, ind.profundidadeLocal);
        for(int j = 0; j < Buckets_Qtde; j++){
            printf("\t\t[%i]: CPF: %i, MASTER_Indice: %i\n", j, ind.buckets[j].cpf, ind.buckets[j].fimIndice);
        }
        printf("\n");
    }
    fclose(file_Indice);
    printf("\n");
}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------

/*
Metodo que imprime arquivo mestre
*/
void imprimeMestre(){
    FILE *file_mestre = fopen("arquivoMestre.dat", "rb+");
    int n = ArqTam(file_mestre) / MestreTam();
    ConteudoMestre dados;
    printf("Mestre\n");
    for(int i = 0; i < n; i++){
        dados = CarregaMestre(i);
        printf("MASTER [%i]:\n\tCPF: %i\n\tNome: %s\n\tData: %s\n\tSexo: %s\n\tAnotações: %s\n", i, dados.cpf, dados.nome, dados.data, dados.sexo, dados.anotacoes);
    }
    printf("\n");
}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------

/*
Metodo que divide o indece quando numero de dados e menor que o de indices.
*/
void divideIndice(Indice *ind, Diretorio *dir, int hash, Bucket b){
    if((*dir).ProfundidadeDir == (*ind).profundidadeLocal){
        dobraDiretorio(dir);
    }
    (*ind).profundidadeLocal++;
    for(int i = 0; i < Buckets_Qtde; i++){
        if((*ind).buckets[i].cpf > 0){
            int rehash = FuncaoHash((*ind).buckets[i].cpf, (*dir).ProfundidadeDir);
            if(hash != rehash){
                Indice rehashIndice;
                int rehash_Indice_Indice;
                if((*dir).Diretorio[hash] == (*dir).Diretorio[rehash]){
                    rehash_Indice_Indice = incrementaIndice();
                    (*dir).Diretorio[rehash] = rehash_Indice_Indice;
                    rehashIndice = CarregaIndice(rehash_Indice_Indice);
                    rehashIndice.profundidadeLocal = (*ind).profundidadeLocal;
                }else{
                    rehash_Indice_Indice = (*dir).Diretorio[rehash];
                    rehashIndice = CarregaIndice(rehash_Indice_Indice);
                }

                int emptyBucket = procuraBucketVazio(rehashIndice);
                if(emptyBucket >= 0){
                    rehashIndice.buckets[emptyBucket] = (*ind).buckets[i];
                    (*ind).buckets[i].cpf = 0;
                }
                SalvarIndice(rehashIndice, rehash_Indice_Indice);
            }else if(b.cpf < (*ind).buckets[i].cpf){
                Bucket temp = (*ind).buckets[i];
                (*ind).buckets[i] = b;
                b = temp;
            }
        }
    }
    int newHash = FuncaoHash(b.cpf, (*dir).ProfundidadeDir);
    int new_Indice_Indice;
    Indice newIndice;
    if(hash != newHash){
        if((*dir).Diretorio[hash] == (*dir).Diretorio[newHash]){
            new_Indice_Indice = incrementaIndice();
            (*dir).Diretorio[newHash] = new_Indice_Indice;
            newIndice = CarregaIndice(new_Indice_Indice);
            newIndice.profundidadeLocal = (*ind).profundidadeLocal;
        }else{
            new_Indice_Indice = (*dir).Diretorio[newHash];
            newIndice = CarregaIndice(new_Indice_Indice);
        }
        int emptyBucket = procuraBucketVazio(newIndice);
        if(emptyBucket >= 0){
            newIndice.buckets[emptyBucket] = b;
        }else{
            divideIndice(&newIndice, dir, newHash, b);
        }
        SalvarIndice(newIndice, new_Indice_Indice);
    }else{
        int emptyBucket = procuraBucketVazio((*ind));
        if(emptyBucket >= 0){
            (*ind).buckets[emptyBucket] = b;
        }else{
            divideIndice(ind, dir, hash, b);
        }
    }
}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------

/*
Procedimento que define o uso de memória do programa
*/
double get_memory_used_MB(){
    PROCESS_MEMORY_COUNTERS info;
    GetProcessMemoryInfo( GetCurrentProcess( ), &info, sizeof(info) );
    return (double)info.PeakWorkingSetSize/ (1024*1024);
}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------

/*
Funcao main do arquivo.
*/
int main(int argc, char *argv[]){
    ConteudoMestre dados;
    Diretorio dir;
    carregaDiretorio(&dir);

    int a = 0;

    FILE *sim;
    sim = fopen("simulacao.txt", "w+");

    clock_t start_timeInsercao, end_timeInsercao;
    double cpu_time_usedInsercao, memory_usedInsercao;
    size_t peakSizeInsercao;

    clock_t start_timePesquisa, end_timePesquisa;
    double cpu_time_usedPesquisa, memory_usedPesquisa;
    size_t peakSizePesquisa;

    clock_t start_timeGeral, end_timeGeral;
    double cpu_time_usedGeral, memory_usedGeral;
    size_t peakSizeGeral;

    int op;
    printf("Bem vindo!\n\n");
    printf("Escolha uma opcao para iniciar:\n");
    do{
        printf("\n");
        printf("[1] - Criar Arquivo\n");
        printf("[2] - Inserir Registro\n");
        printf("[3] - Editar Registro\n");
        printf("[4] - Remover Registro\n");
        printf("[5] - Imprimir Arquivos\n");
        printf("[6] - Simulacao\n");
        printf("[7] - Sair");

        printf("\nDigite a opcao: ");
        scanf("%i", &op);
        printf("\n");

        switch(op){
            //Criar arquivos
            case 1:
                printf("Criando arquivos...\n");
                criaArquivos();
                carregaDiretorio(&dir);
                printf("Arquivos criados!\n");
            break;

            //Inserir registro
            case 2:
                printf("CPF do cliente: ");
                scanf("%d", &dados.cpf);
                printf("Nome do cliente: ");
                scanf("%s", &dados.nome);
                printf("Data de nascimento: ");
                scanf("%s", &dados.data);
                printf("Sexo (m ou f): ");
                scanf("%s", &dados.sexo);

                FILE *file_mestre = fopen("arquivoMestre.dat",  "rb+");
                int fimIndice = ArqTam(file_mestre) / MestreTam();
                fclose(file_mestre);
                Bucket b;
                b.cpf = dados.cpf;
                b.fimIndice = fimIndice;

                int hash = FuncaoHash(b.cpf, dir.ProfundidadeDir);
                int Indice_Indice = dir.Diretorio[hash];
                printf("\nHash function result: %i\nValue in Diretorio: %i\nMaster Indice: %i\n",hash, Indice_Indice, fimIndice);

                Indice ind = CarregaIndice(Indice_Indice);
                int emptyBucket = procuraBucketVazio(ind);
                    if(emptyBucket >= 0){
                        ind.buckets[emptyBucket] = b;
                    }else{
                        divideIndice(&ind,&dir,hash,b);
                    }
                SalvarIndice(ind, Indice_Indice);
                salvaDiretorio(dir);
                SalvarMestre(dados, fimIndice);
            break;

            //Editar registro
            case 3:
            	printf("Digite o CPF do registro quer editar:\n");
            	scanf("%i", &dados.cpf);
            	hash = FuncaoHash(dados.cpf, dir.ProfundidadeDir);
            	Indice_Indice = dir.Diretorio[hash];
            	ind = CarregaIndice(Indice_Indice);
            	int bucketConteudoMestre = procuraPorCPF(dados.cpf,ind);
            	if(bucketConteudoMestre == -1){
					printf("Registro inexistente");
					break;
				}
                printf("Anotacoes Medico: ");
                scanf("%s", &dados.anotacoes);
				int pos_m = ind.buckets[bucketConteudoMestre].fimIndice;
                SalvarMestre(dados,pos_m);
            break;

            //Remover registro
            case 4:
            	printf("Digite o CPF do registro que deseja remover:\n");
            	scanf("%i", &dados.cpf);
            	hash = FuncaoHash(dados.cpf, dir.ProfundidadeDir);
            	Indice_Indice = dir.Diretorio[hash];
            	ind = CarregaIndice(Indice_Indice);
            	bucketConteudoMestre = procuraPorCPF(dados.cpf,ind);
            	if(bucketConteudoMestre == -1){
					printf("Registro inexistente");
					break;
				}
				b.cpf = 0;
				ind.buckets[bucketConteudoMestre] = b;
				SalvarIndice(ind, Indice_Indice);
            break;

            //Imprimir aquivos
            case 5:
                printf("Arquivos impressos:\n ");
                imprimeDiretorio(dir);
                imprimeIndices();
                imprimeMestre();
            break;

            //Sair do programa
            case 6:
                system("CLS");
                start_timeGeral = clock();
                printf("k\tTempo\tMemoria\t\tk\tTempo\tMemoria\n");
                fprintf(sim, "k\tTempo\tMemoria\t\tk\tTempo\tMemoria\n");
                for(int k = 0; k < 50000; k++){
                    /*
                    Inserção
                    */

                    //Começa a medição de tempo na inserção
                    start_timeInsercao = clock();

                    //Implementação do procedimento de inserção
                    srand(time(NULL));
                    a++;

                    dados.cpf = a;
                    strcpy(dados.nome, "aaaaaaaaaaaaaaaa");
                    strcpy(dados.data, "01/01/2001");
                    strcpy(dados.sexo, "f");

                    FILE *file_mestre = fopen("arquivoMestre.dat",  "rb+");
                    int fimIndice = ArqTam(file_mestre) / MestreTam();
                    fclose(file_mestre);
                    Bucket b;
                    b.cpf = dados.cpf;
                    b.fimIndice = fimIndice;

                    int hash = FuncaoHash(b.cpf, dir.ProfundidadeDir);
                    int Indice_Indice = dir.Diretorio[hash];
                    //printf("\nHash function result: %i\nValue in Diretorio: %i\nMaster Indice: %i\n",hash, Indice_Indice, fimIndice);

                    Indice ind = CarregaIndice(Indice_Indice);
                        int emptyBucket = procuraBucketVazio(ind);
                            if(emptyBucket >= 0){
                                ind.buckets[emptyBucket] = b;
                            }else{
                                divideIndice(&ind,&dir,hash,b);
                            }
                    SalvarIndice(ind, Indice_Indice);
                    salvaDiretorio(dir);
                    SalvarMestre(dados, fimIndice);

                    //Termina a medição de tempo da inserção
                    end_timeInsercao = clock();
                    cpu_time_usedInsercao = ((double) (end_timeInsercao - start_timeInsercao)) / CLOCKS_PER_SEC;
                    memory_usedInsercao = get_memory_used_MB();
                    printf("%i\t%0.2f\t%0.2f MB\t\t", k, cpu_time_usedInsercao, memory_usedInsercao);
                    fprintf(sim, "%i\t%0.2f\t%0.2f MB\t\t", k, cpu_time_usedInsercao, memory_usedInsercao);

                    //-------------------------------------------------------------------------------------------------------------//

                    /*
                    Pesquisa
                    */

                    //Começa a medição de tempo na pesquisa
                    start_timePesquisa = clock();

                    //Implementação do procedimento de pesquisa
                    hash = FuncaoHash(dados.cpf, dir.ProfundidadeDir);
                    Indice_Indice = dir.Diretorio[hash];
                    ind = CarregaIndice(Indice_Indice);
                    procuraPorCPF(dados.cpf,ind);

                    //Termina a medição de tempo da pesquisa
                    end_timePesquisa = clock();
                    cpu_time_usedPesquisa = ((double) (end_timePesquisa - start_timePesquisa)) / CLOCKS_PER_SEC;
                    memory_usedPesquisa = get_memory_used_MB();
                    printf("%i\t%0.2f\t%0.2f MB\n", k,cpu_time_usedPesquisa, memory_usedPesquisa);
                    fprintf(sim, "%i\t%0.2f\t%0.2f MB\n", k,cpu_time_usedPesquisa, memory_usedPesquisa);
                }
                end_timeGeral = clock();
                cpu_time_usedGeral = ((double) (end_timeGeral - start_timeGeral)) / CLOCKS_PER_SEC;
                memory_usedGeral = get_memory_used_MB();
                printf("Tempo Geral: %0.2f segundos - Memoria Geral: %0.2f MB\n", cpu_time_usedGeral, memory_usedGeral);
                fprintf(sim, "Tempo Geral: %0.2f segundos - Memoria Geral: %0.2f MB\n", cpu_time_usedGeral, memory_usedGeral);

                fclose(sim);

                int opSair;
                scanf("%i", &opSair);
                if(opSair == 9){
                    op = 7;
                }
            break;

            case 7:
                printf("Obrigado por utilizar o programa! Ate a proxima\n");

            //Padrao de resposta
            default:
                printf("Opcao invalida, tente novamente!\n");
            break;
        }
    }while(op != 7);
    return 0;
}
