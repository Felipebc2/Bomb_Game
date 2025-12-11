#define _POSIX_C_SOURCE 200809L
#include "ui.h"
#include <ncurses.h>
#include <string.h>
#include <time.h>

// Inicializa o ncurses
void inicializar_ncurses(void) {
    initscr();                  // Inicializa a tela
    cbreak();                   // Desativa buffer de linha (CTRL+Z e CTRL+C ainda funcionam)
    noecho();                   // Não exibe o que é digitado
    keypad(stdscr, TRUE);       // Habilita teclas especiais
    curs_set(0);                // Esconde o cursor
    nodelay(stdscr, TRUE);      // getch() não bloqueia
    timeout(0);                 // getch() retorna ERR imediatamente se não houver entrada
}

// Finaliza o ncurses
void finalizar_ncurses(void) {
    keypad(stdscr, FALSE);
    echo();
    curs_set(1);
    endwin();
}

// Desenha toda a interface do jogo na tela
void desenhar_tela(const GameState *g, const char *buffer_instrucao) {
    clear();
    
    int linha = 0;
    int cores_disponiveis = has_colors();
    
    // Título do jogo
    attron(A_BOLD);
    mvprintw(linha++, 0, "=== KEEP SOLVING AND NOBODY EXPLODES ===");
    attroff(A_BOLD);
    linha++;
    
    // Tempo restante e dificuldade
    if (cores_disponiveis) {
        attron(A_BOLD | COLOR_PAIR(1));
    } else {
        attron(A_BOLD);
    }
    mvprintw(linha++, 0, "Dificuldade: %s | Tempo Restante: %d segundos | Modulos: %d/%d resolvidos", 
             nome_dificuldade(g->dificuldade), g->tempo_restante, 
             contar_modulos_resolvidos(g), g->modulos_necessarios);
    if (cores_disponiveis) {
        attroff(A_BOLD | COLOR_PAIR(1));
    } else {
        attroff(A_BOLD);
    }
    linha++;
    
    // Estado dos Tedax
    mvprintw(linha++, 0, "--- TEDAX (%d total) ---", g->qtd_tedax);
    for (int i = 0; i < g->qtd_tedax; i++) {
        const Tedax *t = &g->tedax[i];
        if (t->estado == TEDAX_LIVRE) {
            if (cores_disponiveis) {
                attron(COLOR_PAIR(2)); // Verde
            }
            mvprintw(linha++, 0, "  Tedax %d: LIVRE", t->id);
            if (cores_disponiveis) {
                attroff(COLOR_PAIR(2));
            }
        } else {
            if (cores_disponiveis) {
                attron(COLOR_PAIR(3)); // Amarelo/Vermelho
            }
            mvprintw(linha++, 0, "  Tedax %d: OCUPADO", t->id);
            if (t->modulo_atual >= 0) {
                const Modulo *mod = &g->modulos[t->modulo_atual];
                mvprintw(linha++, 0, "    Desarmando modulo %d (%s) - Tempo restante: %d segundos",
                         mod->id, nome_cor(mod->cor), mod->tempo_restante);
            }
            if (cores_disponiveis) {
                attroff(COLOR_PAIR(3));
            }
        }
    }
    linha++;
    
    // Estado das Bancadas
    mvprintw(linha++, 0, "--- BANCADAS (%d total) ---", g->qtd_bancadas);
    for (int i = 0; i < g->qtd_bancadas; i++) {
        const Bancada *b = &g->bancadas[i];
        if (b->estado == BANCADA_LIVRE) {
            if (cores_disponiveis) {
                attron(COLOR_PAIR(2));
            }
            mvprintw(linha++, 0, "  Bancada %d: LIVRE", b->id);
            if (cores_disponiveis) {
                attroff(COLOR_PAIR(2));
            }
        } else {
            if (cores_disponiveis) {
                attron(COLOR_PAIR(3));
            }
            mvprintw(linha++, 0, "  Bancada %d: OCUPADA (Tedax %d)", b->id, b->tedax_ocupando);
            if (cores_disponiveis) {
                attroff(COLOR_PAIR(3));
            }
        }
    }
    linha++;
    
    // Lista de módulos
    mvprintw(linha++, 0, "--- MODULOS (%d total) ---", g->qtd_modulos);
    
    // Primeiro, contar quantos módulos resolvidos estão dentro do tempo limite de 20s
    int resolvidos_visiveis_20s = 0;
    for (int i = 0; i < g->qtd_modulos; i++) {
        const Modulo *mod = &g->modulos[i];
        if (mod->estado == MOD_RESOLVIDO && mod->tempo_desde_resolvido >= 0 && mod->tempo_desde_resolvido < 20) {
            resolvidos_visiveis_20s++;
        }
    }
    
    // Determinar tempo limite para remoção (10s se tiver 8-9 resolvidos, 20s caso contrário)
    int tempo_limite_remocao = (resolvidos_visiveis_20s >= 8) ? 10 : 20;
    
    int pendentes = 0, em_execucao = 0, resolvidos = 0;
    int modulos_exibidos = 0;
    int modulos_nao_exibidos = 0;
    
    for (int i = 0; i < g->qtd_modulos; i++) {
        const Modulo *mod = &g->modulos[i];
        
        // Filtrar módulos resolvidos que já passaram do tempo limite
        int deve_exibir = 1;
        if (mod->estado == MOD_RESOLVIDO) {
            if (mod->tempo_desde_resolvido < 0 || mod->tempo_desde_resolvido >= tempo_limite_remocao) {
                deve_exibir = 0; // Não exibir módulos resolvidos antigos
                modulos_nao_exibidos++;
            }
        }
        
        if (!deve_exibir) {
            continue; // Pular módulos resolvidos antigos
        }
        
        switch (mod->estado) {
            case MOD_PENDENTE:
                pendentes++;
                break;
            case MOD_EM_EXECUCAO:
                em_execucao++;
                break;
            case MOD_RESOLVIDO:
                resolvidos++;
                break;
        }
        
        // Mostrar detalhes do módulo
        if (mod->estado == MOD_EM_EXECUCAO) {
            mvprintw(linha, 0, "  Modulo %d: %s - %s", 
                     mod->id, nome_cor(mod->cor), nome_estado_modulo(mod->estado));
        } else if (mod->estado == MOD_RESOLVIDO) {
            mvprintw(linha, 0, "  Modulo %d: %s", 
                     mod->id, nome_estado_modulo(mod->estado));
        } else {
            mvprintw(linha, 0, "  Modulo %d: %s - %s - Execucao: %d sec", 
                     mod->id, nome_cor(mod->cor), nome_estado_modulo(mod->estado),
                     mod->tempo_total);
        }
        
        linha++;
        modulos_exibidos++;
        
        // Limitar quantidade de módulos exibidos para não ultrapassar a tela
        if (linha >= LINES - 8) {
            // Contar quantos módulos ainda não foram processados
            int restantes = 0;
            for (int j = i + 1; j < g->qtd_modulos; j++) {
                const Modulo *mod_rest = &g->modulos[j];
                if (mod_rest->estado != MOD_RESOLVIDO || 
                    (mod_rest->tempo_desde_resolvido >= 0 && mod_rest->tempo_desde_resolvido < tempo_limite_remocao)) {
                    restantes++;
                }
            }
            if (restantes > 0) {
                mvprintw(linha++, 0, "  ... (mais %d modulos)", restantes);
            }
            break;
        }
    }
    
    // Se houver módulos não exibidos (resolvidos antigos), informar
    if (modulos_nao_exibidos > 0) {
        linha++;
        mvprintw(linha++, 0, "  (%d resolvidos removidos)", modulos_nao_exibidos);
    }
    
    linha++;
    
    // Resumo
    mvprintw(linha++, 0, "Resumo: %d PENDENTE | %d EM_EXECUCAO | %d RESOLVIDO", 
             pendentes, em_execucao, resolvidos);
    linha++;
    
    // Área de entrada
    // Verificar se há algum tedax livre
    int tem_tedax_livre = 0;
    for (int i = 0; i < g->qtd_tedax; i++) {
        if (g->tedax[i].estado == TEDAX_LIVRE) {
            tem_tedax_livre = 1;
            break;
        }
    }
    
    if (!tem_tedax_livre) {
        // Input desabilitado quando todos os tedax estão ocupados
        if (cores_disponiveis) {
            attron(COLOR_PAIR(3)); // Amarelo/Vermelho para indicar desabilitado
        } else {
            attron(A_DIM); // Texto com menos brilho
        }
        mvprintw(linha++, 0, "Comando: [DESABILITADO - Todos os tedax ocupados]");
        if (cores_disponiveis) {
            attroff(COLOR_PAIR(3));
        } else {
            attroff(A_DIM);
        }
    } else {
        mvprintw(linha++, 0, "Comando: [%s]", buffer_instrucao);
    }
    linha++;
    
    // Mostrar mensagem de erro se houver
    if (g->mensagem_erro[0] != '\0') {
        if (cores_disponiveis) {
            attron(A_BOLD | COLOR_PAIR(3)); // Bold e amarelo/vermelho
        } else {
            attron(A_BOLD);
        }
        mvprintw(linha++, 0, "%s", g->mensagem_erro);
        if (cores_disponiveis) {
            attroff(A_BOLD | COLOR_PAIR(3));
        } else {
            attroff(A_BOLD);
        }
        linha++;
    }
    
    refresh();
}

// Mostra mensagem de vitória
void mostrar_mensagem_vitoria(void) {
    clear();
    int cores_disponiveis = has_colors();
    
    if (cores_disponiveis) {
        attron(A_BOLD | COLOR_PAIR(2));
    } else {
        attron(A_BOLD);
    }
    mvprintw(LINES / 2 - 1, COLS / 2 - 15, "================================");
    mvprintw(LINES / 2, COLS / 2 - 15, "    BOMBA DESARMADA!");
    mvprintw(LINES / 2 + 1, COLS / 2 - 15, "    VITORIA!");
    mvprintw(LINES / 2 + 2, COLS / 2 - 15, "================================");
    if (cores_disponiveis) {
        attroff(A_BOLD | COLOR_PAIR(2));
    } else {
        attroff(A_BOLD);
    }
    mvprintw(LINES / 2 + 4, COLS / 2 - 15, "Pressione qualquer tecla para sair...");
    refresh();
    
    nodelay(stdscr, FALSE);
    getch();
    nodelay(stdscr, TRUE);
}

// Mostra mensagem de derrota
void mostrar_mensagem_derrota(void) {
    clear();
    int cores_disponiveis = has_colors();
    
    if (cores_disponiveis) {
        attron(A_BOLD | COLOR_PAIR(3));
    } else {
        attron(A_BOLD);
    }
    mvprintw(LINES / 2 - 1, COLS / 2 - 15, "================================");
    mvprintw(LINES / 2, COLS / 2 - 15, "    BOMBA EXPLODIU!");
    mvprintw(LINES / 2 + 1, COLS / 2 - 15, "    DERROTA!");
    mvprintw(LINES / 2 + 2, COLS / 2 - 15, "================================");
    if (cores_disponiveis) {
        attroff(A_BOLD | COLOR_PAIR(3));
    } else {
        attroff(A_BOLD);
    }
    mvprintw(LINES / 2 + 4, COLS / 2 - 15, "Pressione qualquer tecla para sair...");
    refresh();
    
    nodelay(stdscr, FALSE);
    getch();
    nodelay(stdscr, TRUE);
}

// Mostra menu inicial e retorna a dificuldade escolhida (ou -1 para sair)
int mostrar_menu_inicial(void) {
    clear();
    int cores_disponiveis = has_colors();
    int selecao = 0; // 0 = Fácil, 1 = Médio, 2 = Difícil, 3 = Sair
    
    while (1) {
        clear();
        
        // Título
        attron(A_BOLD);
        mvprintw(LINES / 2 - 8, COLS / 2 - 20, "========================================");
        mvprintw(LINES / 2 - 7, COLS / 2 - 20, "  KEEP SOLVING AND NOBODY EXPLODES");
        mvprintw(LINES / 2 - 6, COLS / 2 - 20, "         VERSAO DE TREINO");
        mvprintw(LINES / 2 - 5, COLS / 2 - 20, "========================================");
        attroff(A_BOLD);
        
        mvprintw(LINES / 2 - 3, COLS / 2 - 15, "Selecione a Dificuldade:");
        
        // Opções do menu
        const char* opcoes[4] = {
            "FACIL",
            "MEDIO",
            "DIFICIL",
            "SAIR"
        };
        
        for (int i = 0; i < 4; i++) {
            if (selecao == i) {
                if (cores_disponiveis) {
                    attron(A_BOLD | COLOR_PAIR(2));
                } else {
                    attron(A_BOLD | A_REVERSE);
                }
                mvprintw(LINES / 2 + i, COLS / 2 - 20, "> %s", opcoes[i]);
                if (cores_disponiveis) {
                    attroff(A_BOLD | COLOR_PAIR(2));
                } else {
                    attroff(A_BOLD | A_REVERSE);
                }
            } else {
                mvprintw(LINES / 2 + i, COLS / 2 - 20, "  %s", opcoes[i]);
            }
        }
        
        mvprintw(LINES / 2 + 5, COLS / 2 - 15, "Use SETAS para navegar, ENTER para selecionar");
        
        refresh();
        
        int ch = getch();
        
        if (ch == KEY_UP || ch == 'w' || ch == 'W') {
            selecao = (selecao - 1 + 4) % 4;
        } else if (ch == KEY_DOWN || ch == 's' || ch == 'S') {
            selecao = (selecao + 1) % 4;
        } else if (ch == '\n' || ch == '\r') {
            if (selecao == 3) {
                return -1; // Sair
            } else {
                return selecao; // Retorna DIFICULDADE_FACIL, DIFICULDADE_MEDIO ou DIFICULDADE_DIFICIL
            }
        } else if (ch == 'q' || ch == 'Q' || ch == 27) { // ESC ou Q
            return -1;
        }
        
        // Pequeno delay para evitar processamento excessivo
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 100000000L; // 100ms
        nanosleep(&ts, NULL);
    }
}

