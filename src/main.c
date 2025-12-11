#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <ncurses.h>
#include "game.h"
#include "ui.h"

// Buffer de instrução global (compartilhado entre threads)
// Aumentado para suportar comandos do formato T1B1M1:ppp
char buffer_instrucao_global[64] = "";

int main(void) {
    GameState g;
    
    while (1) {
        // Inicializar ncurses temporariamente para o menu
        inicializar_ncurses();
        if (has_colors()) {
            start_color();
            init_pair(1, COLOR_CYAN, COLOR_BLACK);
            init_pair(2, COLOR_GREEN, COLOR_BLACK);
            init_pair(3, COLOR_YELLOW, COLOR_BLACK);
        }
        
        // Mostrar menu principal
        int modo_escolhido = mostrar_menu_principal();
        if (modo_escolhido == -1) {
            finalizar_ncurses();
            printf("Jogo encerrado.\n");
            return 0;
        }
        
        // Se escolheu Classico, mostrar menu de dificuldades
        if (modo_escolhido == 0) {
            int dificuldade_escolhida = mostrar_menu_dificuldades();
            if (dificuldade_escolhida == -1) {
                finalizar_ncurses();
                continue; // Voltar ao menu principal
            }
    
    // Definir número de tedax e bancadas baseado na dificuldade
    int num_tedax, num_bancadas;
    switch (dificuldade_escolhida) {
        case DIFICULDADE_FACIL:
            num_tedax = 1;
            num_bancadas = 1;
            break;
        case DIFICULDADE_MEDIO:
            num_tedax = 2;
            num_bancadas = 2;
            break;
        case DIFICULDADE_DIFICIL:
            num_tedax = 3;
            num_bancadas = 3;
            break;
        default:
            num_tedax = 1;
            num_bancadas = 1;
            break;
    }
    
    // Finalizar ncurses temporário (será reinicializado nas threads)
    finalizar_ncurses();
    
    // Inicializar jogo com a dificuldade escolhida
    inicializar_jogo(&g, dificuldade_escolhida, num_tedax, num_bancadas);
    
    // Criar threads
    pthread_t thread_mural_id;
    pthread_t thread_exibicao_id;
    pthread_t thread_tedax_ids[3];
    pthread_t thread_coordenador_id;
    
    // Thread do Mural
    pthread_create(&thread_mural_id, NULL, thread_mural, &g);
    
    // Thread de Exibição
    pthread_create(&thread_exibicao_id, NULL, thread_exibicao, &g);
    
    // Threads dos Tedax
    for (int i = 0; i < g.qtd_tedax; i++) {
        typedef struct {
            GameState *g;
            int tedax_id;
        } TedaxArgs;
        
        TedaxArgs *args = malloc(sizeof(TedaxArgs));
        args->g = &g;
        args->tedax_id = i;
        pthread_create(&thread_tedax_ids[i], NULL, thread_tedax, args);
    }
    
    // Thread do Coordenador
    pthread_create(&thread_coordenador_id, NULL, thread_coordenador, &g);
    
    // Thread principal: controla o tempo e verifica condições de vitória/derrota
    int tick_count = 0;
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 200000000L; // 0.2 segundos
    
    while (g.jogo_rodando && !g.jogo_terminou) {
        nanosleep(&ts, NULL);
        
        tick_count++;
        if (tick_count >= 5) { // 1 segundo
            pthread_mutex_lock(&g.mutex_jogo);
            
            g.tempo_restante--;
            
            // Verificar condições de fim de jogo
            if (todos_modulos_resolvidos(&g) && g.qtd_modulos > 0) {
                g.jogo_terminou = 1;
                g.jogo_rodando = 0;
                pthread_mutex_unlock(&g.mutex_jogo);
                break;
            }
            
            if (g.tempo_restante <= 0) {
                g.jogo_terminou = 1;
                g.jogo_rodando = 0;
                pthread_mutex_unlock(&g.mutex_jogo);
                break;
            }
            
            pthread_mutex_unlock(&g.mutex_jogo);
            tick_count = 0;
        }
    }
    
    // Aguardar todas as threads terminarem
    pthread_join(thread_mural_id, NULL);
    pthread_join(thread_exibicao_id, NULL);
    for (int i = 0; i < g.qtd_tedax; i++) {
        pthread_join(thread_tedax_ids[i], NULL);
    }
    pthread_join(thread_coordenador_id, NULL);
    
    // Mostrar mensagem final (ncurses já foi finalizado pela thread de exibição)
    // Reinicializar para mostrar mensagem
    inicializar_ncurses();
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_CYAN, COLOR_BLACK);
        init_pair(2, COLOR_GREEN, COLOR_BLACK);
        init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    }
    
            pthread_mutex_lock(&g.mutex_jogo);
            int vitoria = 0;
            if (todos_modulos_resolvidos(&g) && g.qtd_modulos > 0) {
                vitoria = 1;
            } else if (g.qtd_modulos > 0) {
                vitoria = 0;
            } else {
                clear();
                mvprintw(LINES / 2, COLS / 2 - 15, "Tempo esgotado - Nenhum modulo gerado");
                refresh();
                sleep(2);
                finalizar_ncurses();
                continue;
            }
            pthread_mutex_unlock(&g.mutex_jogo);
            
            // Mostrar menu pós-jogo
            int opcao = mostrar_menu_pos_jogo(vitoria);
            finalizar_ncurses();
            
            if (opcao == 'q' || opcao == 'Q') {
                printf("Jogo encerrado.\n");
                return 0;
            } else if (opcao == 'r' || opcao == 'R') {
                // Voltar ao menu principal (continuar o loop)
                continue;
            }
        } else {
            // Outros modos em breve - por enquanto apenas fecha
            finalizar_ncurses();
        }
    }
    
    return 0;
    finalizar_jogo(&g);
    
    printf("Fim da execucao\n");
    return 0;
}

/*
 * Compilar com:
 * gcc -Wall -Wextra -std=c11 main.c game.c ui.c -o jogo -lncurses
 * 
 * Ou usar o Makefile:
 * make
 */

