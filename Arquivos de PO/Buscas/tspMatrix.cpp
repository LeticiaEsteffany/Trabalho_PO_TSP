#include <stdio.h> // print, fgets function
#include <stdlib.h> // exit, atoi function
#include <string.h> //strlen function
#include <math.h> // sqrt function
#include <algorithm>    // std::min
#include <limits.h> // INT_MAX
#include <time.h>
#include <iostream>

int calculateTourDistance(int *tour);//calculate the distance of a tour
void imprimirMatriz(); //mostra a matriz das distancias
void busca_gulosa(int* caminho); //busca gulosa
void imprimirCaminho(int n, int *caminho); //mostra a sequencia de cidades do caminho
int tamanho_do_caminho(int *caminho); //calcula a distancia completa de um caminho
void copia_caminho(int *caminho, int *copia); //faz a copia de um caminho escolhido em outro endereco
void simulated_annealing(int *caminho, int *copia, int *auxiliar); //busca simulated annealing
float potencia(float base, float expoente); //calculo de potencia

int **distanceMatrix, size=-1, pos=0;

int main(const int argc, const char **inputFile){
	double *x, *y;
	char type[10]="";
	srand((unsigned)time(0));

	if(argc < 2){ //verify if an argument was passed (the first argument in C is the name of the executable)
		fprintf(stderr,"use: tspMatrix <tsp file>\n\n");
		exit(1);
	}
	char s[500];
	FILE *stream = fopen(inputFile[1], "r"); //read file (read-only permission)
	if( stream == NULL ){ // if the file was not read, error
		fprintf(stderr,"\nFail to Open tsp File!!\n");
		exit(1);
	}
	while(fgets(s,500,stream)){ //read every line (not larger than 500 chars) of the input
		if(strlen(s)-1>0) //remove the last break line of the line
			s[strlen(s)-1]='\0';
		if(('\r'==s[strlen(s)-1]))//in some files there is a carriage return at the end, don't know why. This command removes it
			s[strlen(s)-1]=0;

		char* value1 = strtok(s," "); //creating sub-strings separated by space
		char* value2 = strtok(NULL," ");
		char* value3 = strtok(NULL," ");

		if(!strcmp(value1, "EDGE_WEIGHT_TYPE")){ //verify if the instance is of type EUC_2D, ATT or CEIL_2D, only the calculation for these types were implemented
			if(strcmp(value3, "EUC_2D") && strcmp(value3, "ATT") && strcmp(value3, "CEIL_2D")){
				fprintf(stderr,"\nERROR! tsp file is not of type EUC_2D, ATT or CEIL_2D aborting!!\n");
				exit(1);
			}else
				strcpy(type,value3);
		}

		if(!strcmp(value1, "TYPE") && (strcmp(value3, "TSP")) ){ //verify if the instance is of type TSP, the other types will not be considered
			fprintf(stderr,"\nERROR! tsp file is not of type TSP, aborting!!\n");
			exit(1);
		}

		if(!strcmp(value1, "DIMENSION")){//read the dimension from the header and allocate memory for the cities
			size = atoi(value3);
			distanceMatrix=(int**)malloc(size * sizeof(int*));
			x=(double*)malloc(size * sizeof(double*));
			y=(double*)malloc(size * sizeof(double*));

			for (int i=0;i<size;i++)
				distanceMatrix[i] = (int*)malloc(size * sizeof(int));
		}

		if(atoi(value1)){//if the first substring is a number, the list of cities started
			if(size==-1){//if the size was not set, it was not in the header, error
				fprintf(stderr,"\nERROR! Dimension not set in file header!!\n");
				exit(1);
			}
			x[pos]=atof(value2);//storing the values from the file in the arrays
			y[pos]=atof(value3);
			pos++;
		}
	}

	if(!strcmp(type, "EUC_2D")){
		for(int i=0; i<size; i++){
			for(int j=0; j<size; j++){
				double xd = x[i]-x[j];
				double yd = y[i]-y[j];
				double dist=sqrt(xd*xd+yd*yd);
				distanceMatrix[i][j]=(int)(dist+0.5);//calculating the euclidean distance, rounding to int and storing in the distance matrix
			}
		}
	}else if(!strcmp(type, "CEIL_2D")){
		for(int i=0; i<size; i++){
			for(int j=0; j<size; j++){
				double xd = x[i]-x[j];
				double yd = y[i]-y[j];
				double dist=sqrt(xd*xd+yd*yd);
				distanceMatrix[i][j]=(int)(dist+0.000000001);
			}
		}
	}else if(!strcmp(type, "ATT")){
		for(int i=0; i<size; i++){
			for(int j=0; j<size; j++){
				double xd = x[i]-x[j];
				double yd = y[i]-y[j];
				double rij=sqrt((xd*xd+yd*yd)/10.0);
				double tij=(int)(rij+0.5);
				if(tij<rij)
					distanceMatrix[i][j]=tij+1;
				else
					distanceMatrix[i][j]=tij;
			}
		}
	}

	int tour[size];
	for(int i=0; i<size; i++){
		tour[i]=i;
	}

	imprimirMatriz();
	printf("\nComprimento da rota mais trivial da cidade 0 a cidade n em sequencia: %d\n", calculateTourDistance(tour));

	int *solucao_gulosa = (int*)malloc((size+1)* sizeof(int));
	int *copia_solucao = (int*)malloc((size+1)* sizeof(int));
	int *auxiliar = (int*)malloc((size+1)* sizeof(int));

	busca_gulosa(solucao_gulosa);
	simulated_annealing(solucao_gulosa, copia_solucao, auxiliar);
}

int calculateTourDistance(int *tour){
	int dist=0;
	for(int i=0; i<size-1; i++){
		dist+=distanceMatrix[tour[i]][tour[i+1]];
	}
	dist+=distanceMatrix[tour[size-1]][tour[0]];
	return dist;
}

void imprimirMatriz(){
    printf("\nMatriz de distancias:\n\n");
    int cont=0;
    printf("Cidade| ");
    for(int k=0; k<size; k++){
        printf("%3d ", k);
    }
    printf("\n-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n");
    for(int i=0; i<size; i++){
        for(int j=0; j<size; j++){
            if(j==0){
                printf("%4d  | ", cont);
                cont ++;
            }
            printf("%3d ", distanceMatrix[i][j]); //impressão do conteudo da matriz de distancias
        }
        printf("\n");
	}
}

void busca_gulosa(int *caminho){
    int menor_soma = INT_MAX; //la em baixo esta variavel sera comparada com a soma do primeiro caminho, entao ela deve ser um numero grande inicialmente para que ela receba logo o valor total do primeiro caminho

    int *inseridos = (int*) malloc(size *sizeof(int)); //vetor para armazenar os itens ja inseridos
    int cidade_inicial = 0;
    int n = 0;
    for(int k=0 ; k<=size ; k++){

        if(k == size) // ultima iteracao realiza o calculo da busca gulosa com o melhor inicio de busca encontrado neste mesmo metodo
            n = cidade_inicial;
        else
            n = k;     //iniciando o caminho com todas as possibilidades

        for(int i=0; i<size; i++){ //inicializando vetor
            inseridos[i] = false;
        }
        caminho[0] = n; //modificar primeiro valor recebido
        inseridos[n] = true; //modificar qual nodo foi inserido primeiro

        //printf("\nPartindo de (%2d):    ", n);// ao lado deste print sera mostrado a distancia de cada cidade do caminho que estara sendo feito no momento

        int soma = 0;
        for(int i=0; i<size; i++){
            int custo = INT_MAX; //esta variavel deve ser um numero bem grande para que na primeira interacao do proximo for a condicao 'custo > distanceMatrix[caminho[i]][j]' seja verdadeira
            int vizinho_selecionado;

            if(i==size-1){ //para podermos voltar ao caminho inicial dizemos que o primeiro nodo nao foi inserido ainda (isto na ultima interacao somente)
                inseridos[n] = false;
            }

            for(int j=0; j<size; j++){
                if(((!inseridos[j]) && (custo > distanceMatrix[caminho[i]][j]) && (i!=j))|| ( ((i==0 && j==0) && (!inseridos[j])) || ((i==size-1 && j==size-1) && (!inseridos[j])) ) ){
                                                                                                    //i!=j pois nao pode viajar para cidade que voce ja esta (diagonal principal)
                    vizinho_selecionado = j;                                                       //i e j iguais a 0 caso a melhor opcao seja viajar para cidade 0 (zero)
                    custo = distanceMatrix[caminho[i]][j];                                        // A cidade 0 eh um elemento da diagonal principal por isso deve ser feito
                }                                                                                // este tratamento especial. A ultima cidade tambem faz parte da diagonal
            }                                                                                   // principal entao tambem precisa deste tratamento (i e j iguais a size-1)

            if(custo != INT_MAX){
                soma += custo;          //somando o custo dos caminhos caso encontrou um
            }
            //printf("%2d ", custo); //imprimindo a distancia de cada cidade na ordem do caminho que esta sendo feito
            caminho[i + 1] = vizinho_selecionado;
            inseridos[vizinho_selecionado] = true;
        }

        //printf(" --> %d CUSTO TOTAL", soma);
        //imprimirCaminho(size+1, caminho);

        if(k == size){ //ultima iteracao e apresentacao de resultado na tela
            printf("\n\n                            !!!!!! O MELHOR CAMINHO COM A BUSCA GULOSA SE INICIA NA CIDADE '%d' !!!!!!\n", cidade_inicial);
            imprimirCaminho(size+1, caminho); //o tamanho eh size+1 para poder mostrar a ultima cidade visitada voltando para cidade de partida
        }

        if(soma < menor_soma){ //verificando se o novo caminho retornou um custo menor que o anterior
            menor_soma = soma;
            cidade_inicial = n; //guardando o melhor inicio de caminho, ele basta para nos dizer o caminho completo
                                //de menor custo, uma vez que o algoritmo roda com as mesmas caracteristicas
        }
    }
    free(inseridos);
    printf("\n\n---------------------------------------------- Total da Busca Gulosa: %d ------------------------------------------------------------\n", menor_soma);
}

void imprimirCaminho(int n, int *caminho){
    printf("\nSolucao gerada -> ");
    for(int i=0; i<n; i++){
        printf("%2d ", caminho[i]); //impressao do caminho
    }
    printf("\n");
}

int tamanho_do_caminho(int *caminho){
    int tamanho = 0;
    for(int i=0 ; i<size ; i++){
        tamanho += distanceMatrix[caminho[i]][caminho[i+1]]; //somando valores de distancia de cada cidade contando com o retorno para a cidade inicial
    }

    return tamanho;
}

void copia_caminho(int *caminho, int *copia){ //copiando o caminho para outro espaco alocado, auxilio para outros metodos
    for(int i=0 ; i<size+1 ; i++){
        copia[i] = caminho[i];
    }
}

void simulated_annealing(int *caminho, int *copia, int *auxiliar){
    float e = 2.7182; //numero de euller
    float t = 100; //um valor pequeno para t
    copia_caminho(caminho, copia); //caminho guarda o melhor de todos os caminhos
    copia_caminho(caminho, auxiliar); //auxiliar guarda temporariamente

    printf("\n\n-------------------------------------------------------- Simulated Annealing ------------------------------------------------------------------\n");
    printf("Calculando...");
    for(int i=0 ; i<=100000000 ; i++){ //rodara um bilhao de vezes
        t = ((float)i * 100.00 / 100000000.00) ; //temperatura
        int cidade_um = rand() % size+1; //sorteio de uma cidade
        int cidade_dois = rand() % size+1; //sorteio da segunda cidade
        if( cidade_um==cidade_dois || ((cidade_um==0 && cidade_dois==size) || (cidade_um==size && cidade_dois==0)) ){ //se alguma destas situacoes acontecer quer dizer que foi sorteado duas posicoes iguais
            while( cidade_um==cidade_dois || ((cidade_um==0 && cidade_dois==size) || (cidade_um==size && cidade_dois==0)) ){
                cidade_um = rand() % size+1;        //o sorteio eh refeito ate que ambas sejam diferentes, conforme especificado na estrutura de repeticao while
                cidade_dois = rand() % size+1;
            }
        }

        if(cidade_um == 0){ //se a primeira cidade sorteada for o numero de posicao zero no caminho, a posicao escolhida para fazer a troca com ele, no caso a posicao [cidade_dois] devera ira para ultima posicao tambem
            copia[size] = copia[cidade_dois]; //aqui setamos ele para ultima posicao caso isso ocorra
        }
        if(cidade_dois == size){ //a mesma coisa deve ser levada em consideracao caso seja sorteada a ultima cidade.
            copia[0] = copia[cidade_um];//aqui ele eh setado para primeira posicao caso isso ocorra
        }
        if(cidade_um == size){ //pode acontecer de ser sorteado a ultima ou a primeira posicao para qualquer umas das variaveis, por isso sao 4 casos
            copia[0] = copia[cidade_dois];
        }
        if(cidade_dois == 0){
            copia[size] = copia[cidade_um];
        }


        int aux = copia[cidade_um];             //troca de posicao
        copia[cidade_um] = copia[cidade_dois];  //das duas cidades
        copia[cidade_dois]=aux;                 //escolhidas.

        float valorR = (float)tamanho_do_caminho(copia);
        float valorS = (float)tamanho_do_caminho(auxiliar);
        float expoente = (valorS-valorR)/t;
        float equacao = potencia(e, expoente);
        float aleatorio = rand() % 2;

        if((valorR < valorS) || (aleatorio < equacao)){ //verificando se o caminho encontrado eh melhor do que o que temos ou permitindo que ele pegue um caminho pior
            copia_caminho(copia, auxiliar);
        }
        else{
            copia_caminho(auxiliar, copia);//caso nao tenha sido melhor a troca dos valores sorteados a operacao de troca eh desfeita

        //LEMBRANDO QUE A EQUACAO NA CONDICAO ACIMA PODE PERMITIR A TROCA MESMO QUE SEJA UMA TROCA QUE PIORE O RESULTADO
        }
        if(tamanho_do_caminho(auxiliar) < tamanho_do_caminho(caminho)){
            copia_caminho(auxiliar, caminho);
        }
    }
    printf("\n");
    imprimirCaminho(size+1, caminho);//o tamanho eh size+1 para poder mostrar a ultima cidade visitada voltando para cidade de partida
    printf("\nTAMANHO TOTAL = %d\n", tamanho_do_caminho(caminho));
}

float potencia(float base, float expoente){

    float resultado = 0;

    resultado = pow(base, expoente);

    return resultado;
}
