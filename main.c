#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef enum { VERMELHO, AMARELO, VERDE, AZUL } Cor;
typedef enum { NUMERO, COMPRA2, INVERTER, PULAU } Tipo;

typedef struct Carta {
    Cor cor;
    Tipo tipo;
    int valor; // 0 a 9 para cartas normais, -1 para especiais
} Carta;

// 1. PILHA - Baralho de compra
typedef struct NoPilha {
    Carta carta;
    struct NoPilha* proximo;
} NoPilha;

typedef struct {
    NoPilha* topo;
} Pilha;

// 2. LISTA ENCADEADA - Mão dos jogadores
typedef struct NoLista {
    Carta carta;
    struct NoLista* proximo;
} NoLista;

typedef struct {
    NoLista* inicio;
    int tamanho;
} Lista;

typedef struct {
    char nome[30];
    Lista mao;
    int ehHumano; // 1 = Você, 0 = Bot/Máquina
} Jogador;

// --- AUXILIARES DE TELA ---
const char* getNomeCor(Cor c) {
    switch (c) {
        case VERMELHO: return "Vermelho";
        case AMARELO: return "Amarelo";
        case VERDE: return "Verde";
        case AZUL: return "Azul";
        default: return "Desconhecido";
    }
}

void imprimirCarta(Carta c) {
    if (c.tipo == NUMERO) {
        printf("[%s %d]", getNomeCor(c.cor), c.valor);
    } else if (c.tipo == COMPRA2) {
        printf("[%s +2]", getNomeCor(c.cor));
    } else if (c.tipo == INVERTER) {
        printf("[%s Inverter]", getNomeCor(c.cor));
    } else if (c.tipo == PULAU) {
        printf("[%s Pular]", getNomeCor(c.cor));
    }
}

// --- FUNÇÕES DA PILHA ---
Pilha* criarPilha() {
    Pilha* p = (Pilha*)malloc(sizeof(Pilha));
    p->topo = NULL;
    return p;
}

void empilhar(Pilha* p, Carta c) {
    NoPilha* novo = (NoPilha*)malloc(sizeof(NoPilha));
    novo->carta = c;
    novo->proximo = p->topo;
    p->topo = novo;
}

Carta desempilhar(Pilha* p) {
    Carta c_vazia = {0, 0, -1};
    if (p->topo == NULL) return c_vazia;
    
    NoPilha* temp = p->topo;
    Carta c = temp->carta;
    p->topo = p->topo->proximo;
    free(temp);
    return c;
}

int pilhaVazia(Pilha* p) {
    return p->topo == NULL;
}

// --- FUNÇÕES DA LISTA ---
Lista* criarLista() {
    Lista* l = (Lista*)malloc(sizeof(Lista));
    l->inicio = NULL;
    l->tamanho = 0;
    return l;
}

void inserirLista(Lista* l, Carta c) {
    NoLista* novo = (NoLista*)malloc(sizeof(NoLista));
    novo->carta = c;
    novo->proximo = l->inicio;
    l->inicio = novo;
    l->tamanho++;
}

Carta removerLista(Lista* l, int indice) {
    Carta c_vazia = {0, 0, -1};
    if (indice < 1 || indice > l->tamanho || l->inicio == NULL) return c_vazia;

    NoLista* atual = l->inicio;
    NoLista* anterior = NULL;

    for (int i = 1; i < indice; i++) {
        anterior = atual;
        atual = atual->proximo;
    }

    Carta c = atual->carta;
    if (anterior == NULL) {
        l->inicio = atual->proximo;
    } else {
        anterior->proximo = atual->proximo;
    }

    free(atual);
    l->tamanho--;
    return c;
}

void mostrarMao(Lista* l) {
    NoLista* atual = l->inicio;
    int i = 1;
    while (atual != NULL) {
        printf("%d:", i);
        imprimirCarta(atual->carta);
        printf(" ");
        atual = atual->proximo;
        i++;
    }
    printf("\n");
}

Carta obterCartaPorIndice(Lista* l, int indice) {
    NoLista* atual = l->inicio;
    for (int i = 1; i < indice && atual != NULL; i++) {
        atual = atual->proximo;
    }
    return atual->carta;
}

// --- REGRAS DO JOGO ---
void inicializarBaralho(Pilha* baralho) {
    Carta aux[100];
    int qtd = 0;

    for (int c = 0; c < 4; c++) {
        for (int v = 0; v <= 9; v++) {
            aux[qtd].cor = (Cor)c;
            aux[qtd].tipo = NUMERO;
            aux[qtd].valor = v;
            qtd++;
        }
        aux[qtd].cor = (Cor)c; aux[qtd].tipo = COMPRA2; aux[qtd].valor = -1; qtd++;
        aux[qtd].cor = (Cor)c; aux[qtd].tipo = INVERTER; aux[qtd].valor = -1; qtd++;
        aux[qtd].cor = (Cor)c; aux[qtd].tipo = PULAU; aux[qtd].valor = -1; qtd++;
    }

    // Embaralhar
    for (int i = 0; i < qtd; i++) {
        int r = rand() % qtd;
        Carta temp = aux[i];
        aux[i] = aux[r];
        aux[r] = temp;
    }

    for (int i = 0; i < qtd; i++) {
        empilhar(baralho, aux[i]);
    }
}

int validarJogada(Carta cartaJogada, Carta mesa) {
    if (cartaJogada.cor == mesa.cor) return 1;
    if (cartaJogada.tipo == NUMERO && mesa.tipo == NUMERO && cartaJogada.valor == mesa.valor) return 1;
    if (cartaJogada.tipo != NUMERO && cartaJogada.tipo == mesa.tipo) return 1;
    return 0;
}

int encontrarJogadaValidaBot(Lista* mao, Carta mesa) {
    NoLista* atual = mao->inicio;
    int idx = 1;
    while (atual != NULL) {
        if (validarJogada(atual->carta, mesa)) {
            return idx;
        }
        atual = atual->proximo;
        idx++;
    }
    return 0;
}

// --- MAIN ---
int main() {
    srand(time(NULL));

    Pilha* baralho = criarPilha();
    inicializarBaralho(baralho);

    int numJogadores = 4;
    Jogador jogadores[4];

    strcpy(jogadores[0].nome, "Voce");
    jogadores[0].ehHumano = 1;

    strcpy(jogadores[1].nome, "Bot 1");
    jogadores[1].ehHumano = 0;

    strcpy(jogadores[2].nome, "Bot 2");
    jogadores[2].ehHumano = 0;

    strcpy(jogadores[3].nome, "Bot 3");
    jogadores[3].ehHumano = 0;

    for (int i = 0; i < numJogadores; i++) {
        jogadores[i].mao = *criarLista();
        for (int c = 0; c < 7; c++) {
            inserirLista(&jogadores[i].mao, desempilhar(baralho));
        }
    }

    Carta mesa = desempilhar(baralho);
    while (mesa.tipo != NUMERO) {
        empilhar(baralho, mesa);
        mesa = desempilhar(baralho);
    }

    int turno = 0;
    int sentido = 1; // 1 = horario, -1 = anti-horario
    int jogoAtivo = 1;

    printf("=========================================\n");
    printf("     INICIANDO UNO (4 JOGADORES)\n");
    printf("=========================================\n");

    while (jogoAtivo) {
        Jogador* jAtual = &jogadores[turno];
        int cartaFoiJogada = 0; // Flag para saber se alguém baixou uma carta no turno

        printf("\n-----------------------------------------\n");
        printf("MESA: ");
        imprimirCarta(mesa);
        printf("\nTurno de: %s (Possui %d cartas)\n", jAtual->nome, jAtual->mao.tamanho);

        if (jAtual->ehHumano) {
            printf("Sua mao: ");
            mostrarMao(&jAtual->mao);

            printf("\nOpcoes:\n[1-%d] Escolha uma carta\n[0] Comprar do baralho\nEscolha: ", jAtual->mao.tamanho);
            int escolha;
            scanf("%d", &escolha);

            if (escolha == 0) {
                if (!pilhaVazia(baralho)) {
                    Carta c = desempilhar(baralho);
                    inserirLista(&jAtual->mao, c);
                    printf("Voce comprou: ");
                    imprimirCarta(c);
                    printf("\n");
                } else {
                    printf("Baralho esgotado!\n");
                }
            } else if (escolha >= 1 && escolha <= jAtual->mao.tamanho) {
                Carta escolhida = obterCartaPorIndice(&jAtual->mao, escolha);

                if (validarJogada(escolhida, mesa)) {
                    mesa = removerLista(&jAtual->mao, escolha);
                    cartaFoiJogada = 1;
                    printf("Voce jogou: ");
                    imprimirCarta(mesa);
                    printf("\n");
                } else {
                    printf("\n[X] Jogada invalida! Tente novamente.\n");
                    continue; // Pede a jogada novamente ao humano
                }
            } else {
                printf("\nOpcao invalida!\n");
                continue;
            }

        } else {
            // TURNO DO BOT
            int indiceJogada = encontrarJogadaValidaBot(&jAtual->mao, mesa);

            if (indiceJogada > 0) {
                mesa = removerLista(&jAtual->mao, indiceJogada);
                cartaFoiJogada = 1;
                printf("%s jogou: ", jAtual->nome);
                imprimirCarta(mesa);
                printf("\n");
            } else {
                if (!pilhaVazia(baralho)) {
                    Carta comprada = desempilhar(baralho);
                    inserirLista(&jAtual->mao, comprada);
                    printf("%s nao tinha carta e comprou 1 do baralho.\n", jAtual->nome);
                } else {
                    printf("Baralho esgotado!\n");
                }
            }
        }

        // Checar vitória
        if (jAtual->mao.tamanho == 0) {
            printf("\n=========================================\n");
            printf("  %s VENCEU O JOGO! PARABENS!\n", jAtual->nome);
            printf("=========================================\n");
            jogoAtivo = 0;
            break;
        }

        // APLICAR EFEITO SOMENTE SE A CARTA FOI JOGADA NESTE TURNO
        if (cartaFoiJogada) {
            if (mesa.tipo == INVERTER) {
                sentido *= -1;
                printf(">> Sentido do jogo invertido!\n");
            } else if (mesa.tipo == PULAU) {
                turno = (turno + sentido + numJogadores) % numJogadores;
                printf(">> %s foi pulado!\n", jogadores[turno].nome);
            } else if (mesa.tipo == COMPRA2) {
                int proximo = (turno + sentido + numJogadores) % numJogadores;
                printf(">> %s recebe +2 cartas e perde a vez!\n", jogadores[proximo].nome);
                for (int k = 0; k < 2; k++) {
                    if (!pilhaVazia(baralho)) inserirLista(&jogadores[proximo].mao, desempilhar(baralho));
                }
                turno = proximo; // Pula quem teve que comprar 2
            }
        }

        // Passa o turno normalmente
        turno = (turno + sentido + numJogadores) % numJogadores;
    }

    return 0;
}