#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_LINHA 512
#define MAX_PERFUMES 300 
#define MAX_RESP 4096    

// --- ESTRUTURA DO PERFUME ---
typedef struct {
    char nome[100];
    char genero[5];      
    char periodo[50];    
    char ocasiao[50];    
    char estilo[50];     
    char estacao[50];
    char orcamento[50];
    char intensidade[50];
    int ativo;           
} Perfume;

Perfume catalogo[MAX_PERFUMES];
int total_perfumes = 0;

// --- ESTRUTURA DA ÁRVORE ---
typedef struct No {
    char texto[200];       
    char tipo_filtro[20];  
    char valor_filtro[50]; 
    struct No *sim;
    struct No *nao;
} No;

No *raiz = NULL;

// --- FUNÇÃO LIMPEZA ---
void limpar_string(char *str) {
    if (!str) return;
    str[strcspn(str, "\n")] = 0;
    str[strcspn(str, "\r")] = 0;
    
    if (str[0] == '"') {
        memmove(str, str + 1, strlen(str)); 
    }
    int len = strlen(str);
    if (len > 0 && str[len-1] == '"') {
        str[len-1] = '\0'; 
    }
}

// --- CARREGAMENTO CORRIGIDO ---
void carregar_csv(char* nome_arquivo) {
    FILE *arquivo = fopen(nome_arquivo, "r");
    if (!arquivo) {
        printf("Erro ao abrir arquivo: %s\n", nome_arquivo);
        return; 
    }

    char linha[MAX_LINHA];
    // Pula cabeçalho
    fgets(linha, sizeof(linha), arquivo);

    while (fgets(linha, sizeof(linha), arquivo) && total_perfumes < MAX_PERFUMES) {
        // Limpa o \n final da linha bruta
        linha[strcspn(linha, "\n")] = 0;

        // 1. COLUNA: NOME
        char *t = strtok(linha, ","); 
        if(t) { limpar_string(t); strcpy(catalogo[total_perfumes].nome, t); }

        // 2. COLUNA: LINHA (Ignorar, mas precisamos ler para passar o cursor)
        t = strtok(NULL, ","); 
        
        // 3. COLUNA: GENERO
        t = strtok(NULL, ","); 
        if(t) { 
            limpar_string(t); 
            strcpy(catalogo[total_perfumes].genero, t); 
        }
        
       
        
        strtok(NULL, ","); // Pula Familia Olfativa 
        
        
        // A partir daqui, ambos os arquivos estão alinhados na coluna PERIODO
        
        // 4. PERIODO
        t = strtok(NULL, ","); if(t) { limpar_string(t); strcpy(catalogo[total_perfumes].periodo, t); }
        // 5. OCASIAO
        t = strtok(NULL, ","); if(t) { limpar_string(t); strcpy(catalogo[total_perfumes].ocasiao, t); }
        // 6. ESTILO
        t = strtok(NULL, ","); if(t) { limpar_string(t); strcpy(catalogo[total_perfumes].estilo, t); }
        // 7. ESTACAO
        t = strtok(NULL, ","); if(t) { limpar_string(t); strcpy(catalogo[total_perfumes].estacao, t); }
        // 8. ORCAMENTO
        t = strtok(NULL, ","); if(t) { limpar_string(t); strcpy(catalogo[total_perfumes].orcamento, t); }
        // 9. INTENSIDADE
        t = strtok(NULL, ","); if(t) { limpar_string(t); strcpy(catalogo[total_perfumes].intensidade, t); }

        catalogo[total_perfumes].ativo = 1; 
        total_perfumes++;
    }
    fclose(arquivo);
}

// --- FILTRAGEM ---
void aplicar_filtro(char* tipo, char* valor) {
    if (strcmp(tipo, "NENHUM") == 0) return; 

    for (int i = 0; i < total_perfumes; i++) {
        if (catalogo[i].ativo == 0) continue;

        int manter = 0; 

        if (strcmp(tipo, "GENERO") == 0) {
            // Comparação exata
            if (strcmp(catalogo[i].genero, valor) == 0) manter = 1;
        }
        else if (strcmp(tipo, "ESTILO") == 0) {
            if (strstr(catalogo[i].estilo, valor) != NULL) manter = 1;
        }
        else if (strcmp(tipo, "OCASIAO") == 0) {
            if (strstr(catalogo[i].ocasiao, valor) != NULL) manter = 1;
        }
        else if (strcmp(tipo, "ESTACAO") == 0) {
            if (strstr(catalogo[i].estacao, valor) != NULL) manter = 1;
        }
        else if (strcmp(tipo, "ORCAMENTO") == 0) {
            if (strstr(catalogo[i].orcamento, valor) != NULL) manter = 1;
        }
        else if (strcmp(tipo, "PERIODO") == 0) {
            if (strstr(catalogo[i].periodo, valor) != NULL) manter = 1;
        }

        catalogo[i].ativo = manter;
    }
}

// --- FUNÇÕES DA ÁRVORE ---
No* criar_no(char* texto, char* tipo, char* valor, No* sim, No* nao) {
    No* novo = (No*)malloc(sizeof(No));
    strcpy(novo->texto, texto);
    strcpy(novo->tipo_filtro, tipo);
    strcpy(novo->valor_filtro, valor);
    novo->sim = sim;
    novo->nao = nao;
    return novo;
}

void inicializar_arvore() {
    total_perfumes = 0;
    
    carregar_csv("perfumes_masculinos.csv"); 
    carregar_csv("perfumes_femininos.csv");

    No* fim = criar_no("FIM", "NENHUM", "", NULL, NULL);

    // --- RAMOS DE PERGUNTAS ---

    // Estação
    No* p_estacao_2 = criar_no("Prefere algo para um clima mais quente ?", "ESTACAO", "Primavera / Verão", fim, fim);
    No* p_estacao_1 = criar_no("Prefere algo versatil para todas as estacoes ?", "ESTACAO", "Todas as estações", fim, p_estacao_2);

    // Periodo
    No* p_periodo_2 = criar_no("Gostaria de um perfume para usar de Dia ?", "PERIODO", "Dia", p_estacao_1, p_estacao_1);
    No* p_periodo = criar_no("Gostaria de um perfume para qualquer periodo (Versatil) ?", "PERIODO", "Versatil", p_estacao_1, p_periodo_2);

    // Orçamento
    No* p_orcamento = criar_no("Seu orcamento é maior que R$ 200,00 ?", "ORCAMENTO", "maior que 200", p_periodo, p_periodo);

    // --- RAMO MASCULINO ---
    No* m_estilo_4 = criar_no("Gostaria de um estilo Determinado/Energético ?", "ESTILO", "Determinado", p_orcamento, p_orcamento);
    No* m_estilo_3 = criar_no("Gostaria de um estilo Jovem/Divertido ?", "ESTILO", "Jovem", p_orcamento, m_estilo_4);
    No* m_estilo_2 = criar_no("Gostaria de um estilo Sedutor ?", "ESTILO", "Sedutor", p_orcamento, m_estilo_3);
    No* m_estilo_1 = criar_no("Gostaria de um estilo Sofisticado ?", "ESTILO", "Sofisticado", p_orcamento, m_estilo_2);
    
    // --- RAMO FEMININO ---
    No* f_estilo_4 = criar_no("Gostaria de um estilo Elegante ?", "ESTILO", "Elegante", p_orcamento, p_orcamento);
    No* f_estilo_3 = criar_no("Gostaria de um estilo Jovem ?", "ESTILO", "Jovem", p_orcamento, f_estilo_4);
    No* f_estilo_2 = criar_no("Gostaria de um estilo Sensual ?", "ESTILO", "Sensual", p_orcamento, f_estilo_3);
    No* f_estilo_1 = criar_no("Gostaria de um estilo Delicado ?", "ESTILO", "Delicada", p_orcamento, f_estilo_2);

    // --- RAIZ ---
    raiz = criar_no("O perfume é para o Genero Feminino ?", "GENERO", "F", f_estilo_1, m_estilo_1);
}
// Transforma toda a lista de perfumes em 1 novamento
void resetar_catalogo() {
    for(int i=0; i < total_perfumes; i++) {
        catalogo[i].ativo = 1;
    }
}

void interagir_arvore(char* caminho_usuario, char* resposta) {
    if (raiz == NULL) inicializar_arvore();

    if (strlen(caminho_usuario) == 0) {
        resetar_catalogo();
        sprintf(resposta, "PERGUNTA:%s", raiz->texto);
        return;
    }

    No* atual = raiz;
    int len = strlen(caminho_usuario);
    
    resetar_catalogo(); 

    for (int i = 0; i < len; i++) {
        if (strcmp(atual->texto, "FIM") == 0) break;

        if (caminho_usuario[i] == '1') {
            aplicar_filtro(atual->tipo_filtro, atual->valor_filtro);
            if (atual->sim) atual = atual->sim;
        } else {
            // Lógica do NÃO
            if (strcmp(atual->tipo_filtro, "GENERO") == 0) aplicar_filtro("GENERO", "M");
            else if (strcmp(atual->tipo_filtro, "ORCAMENTO") == 0) aplicar_filtro("ORCAMENTO", "menor que 200");
            
            if (atual->nao) atual = atual->nao;
        }
    }

    if (strcmp(atual->texto, "FIM") == 0 || (atual->sim == NULL && atual->nao == NULL)) {
        char buffer_lista[MAX_RESP] = "";
        int cont = 0;

        for(int i=0; i < total_perfumes; i++) {
            if (catalogo[i].ativo == 1) {
                char item[300];
                sprintf(item, "• %s\n", catalogo[i].nome);
                if (strlen(buffer_lista) + strlen(item) < MAX_RESP - 100) {
                    strcat(buffer_lista, item);
                    cont++;
                }
            }
        }

        if (cont == 0) sprintf(resposta, "RESULTADO:Nenhum perfume encontrado.");
        else {
            char final[MAX_RESP + 100];
            sprintf(final, "RESULTADO:Encontramos %d opcoes:\n%s", cont, buffer_lista);
            strcpy(resposta, final);
        }

    } else {
        sprintf(resposta, "PERGUNTA:%s", atual->texto);
    }
}