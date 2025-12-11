#ifndef UI_H
#define UI_H

#include "game.h"

// FUTURO: Esta função será uma thread (Exibição de Informações)
// Desenha toda a interface do jogo na tela
void desenhar_tela(const GameState *g, const char *buffer_instrucao);

// Inicializa o ncurses
void inicializar_ncurses(void);

// Finaliza o ncurses
void finalizar_ncurses(void);

// Mostra mensagem de vitória
void mostrar_mensagem_vitoria(void);

// Mostra mensagem de derrota
void mostrar_mensagem_derrota(void);

// Mostra menu principal e retorna o modo escolhido
// Retorna: 0 = Classico, 1-6 = Outros modos (em breve), -1 = Sair
int mostrar_menu_principal(void);

// Mostra menu de dificuldades e retorna a dificuldade escolhida (ou -1 para sair)
// Retorna DIFICULDADE_FACIL, DIFICULDADE_MEDIO, DIFICULDADE_DIFICIL, ou -1 para sair
int mostrar_menu_dificuldades(void);

// Mostra menu pós-jogo (vitória ou derrota)
// Retorna: 'q' ou 'Q' para sair, 'r' ou 'R' para voltar ao menu
int mostrar_menu_pos_jogo(int vitoria);

#endif // UI_H

