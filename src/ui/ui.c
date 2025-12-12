#define _POSIX_C_SOURCE 200809L
#include "ui.h"
#include "../game/game.h"
#include "../modulos/modulos.h"
#include "../audio/audio.h"
#include <ncurses.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

// Helpers para impressão colorida de módulos
static void imprimir_cor_botao(CorBotao cor, int cores_disponiveis) {
    const char* cor_nome = (cor == COR_VERMELHO) ? "Vermelho" :
                           (cor == COR_VERDE) ? "Verde" : "Azul";
    int pair = 0;
    switch (cor) {
        case COR_VERMELHO: pair = 4; break;
        case COR_VERDE:    pair = 2; break;
        case COR_AZUL:     pair = 5; break;
    }
    if (cores_disponiveis && pair > 0) {
        attron(COLOR_PAIR(pair) | A_BOLD);
        printw("%s", cor_nome);
        attroff(COLOR_PAIR(pair) | A_BOLD);
    } else {
        printw("%s", cor_nome);
    }
}

static void imprimir_fio_token(const char* token, int cores_disponiveis) {
    int pair = 0;
    if (strcmp(token, "R") == 0) pair = 4;
    else if (strcmp(token, "G") == 0) pair = 2;
    else if (strcmp(token, "B") == 0) pair = 5;
    else if (strcmp(token, "Y") == 0) pair = 3;
    else if (strcmp(token, "W") == 0) pair = 6;
    else if (strcmp(token, "K") == 0) pair = 7;

    if (cores_disponiveis && pair > 0) {
        attron(COLOR_PAIR(pair) | A_BOLD);
        
        printw("%s", token);
        attroff(COLOR_PAIR(pair) | A_BOLD);
    } else {
        printw("%s", token);
    }
}

static void imprimir_sequencia_fios_colorida(const char* sequencia, int cores_disponiveis) {
    // sequencia vem no formato "/R/G/B/"
    char buffer[64];
    strncpy(buffer, sequencia, sizeof(buffer));
    buffer[sizeof(buffer) - 1] = '\0';

    char* token = strtok(buffer, "/");
    while (token != NULL) {
        printw("/");
        if (*token != '\0') {
            imprimir_fio_token(token, cores_disponiveis);
        }
        token = strtok(NULL, "/");
    }
    printw("/");
}

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

// Gera uma barra de progresso visual
// Retorna uma string formatada com a barra e porcentagem
// Exemplo: "██████------ 50%"
void gerar_barra_progresso(char *buffer, int tamanho_buffer, int tempo_total, int tempo_restante) {
    if (tempo_total <= 0) {
        snprintf(buffer, tamanho_buffer, "████████████ 100%%");
        return;
    }
    
    // Calcular porcentagem (0-100)
    int progresso = ((tempo_total - tempo_restante) * 100) / tempo_total;
    if (progresso < 0) progresso = 0;
    if (progresso > 100) progresso = 100;
    
    // Tamanho da barra (10 caracteres)
    const int tamanho_barra = 10;
    int blocos_preenchidos = (progresso * tamanho_barra) / 100;
    
    // Construir a barra usando caracteres ASCII
    int pos = 0;
    for (int i = 0; i < blocos_preenchidos && pos < tamanho_buffer - 8; i++) {
        buffer[pos++] = '#';
    }
    for (int i = blocos_preenchidos; i < tamanho_barra && pos < tamanho_buffer - 8; i++) {
        buffer[pos++] = '-';
    }
    buffer[pos] = '\0';
    
    // Adicionar porcentagem
    snprintf(buffer + pos, tamanho_buffer - pos, " %d%%", progresso);
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
        } else if (t->estado == TEDAX_ESPERANDO) {
            if (cores_disponiveis) {
                attron(COLOR_PAIR(3)); // Amarelo
            }
            if (t->modulo_atual >= 0) {
                const Modulo *mod = &g->modulos[t->modulo_atual];
                mvprintw(linha++, 0, "  Tedax %d: ESPERANDO (Bancada %d) - Aguardando para M%d", 
                         t->id, t->bancada_atual >= 0 ? g->bancadas[t->bancada_atual].id : 0,
                         mod->id);
            } else {
                mvprintw(linha++, 0, "  Tedax %d: ESPERANDO (Bancada %d)", t->id, 
                         t->bancada_atual >= 0 ? g->bancadas[t->bancada_atual].id : 0);
            }
            if (cores_disponiveis) {
                attroff(COLOR_PAIR(3));
            }
        } else {
            if (cores_disponiveis) {
                attron(COLOR_PAIR(3)); // Amarelo/Vermelho
            }
            if (t->modulo_atual >= 0) {
                const Modulo *mod = &g->modulos[t->modulo_atual];
                char barra[32];
                gerar_barra_progresso(barra, sizeof(barra), mod->tempo_total, mod->tempo_restante);
                mvprintw(linha++, 0, "  Tedax %d: OCUPADO - Desarmando M%d - %s",
                         t->id, mod->id, barra);
                // Mostrar módulo em espera na linha de baixo
                if (t->qtd_fila > 0 && t->fila_modulos[0] >= 0 && t->fila_modulos[0] < g->qtd_modulos) {
                    const Modulo *mod_fila = &g->modulos[t->fila_modulos[0]];
                    mvprintw(linha++, 0, "    Fila: M%d",
                             mod_fila->id);
                }
            } else {
                mvprintw(linha++, 0, "  Tedax %d: OCUPADO", t->id);
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
            if (b->tedax_esperando >= 0) {
                mvprintw(linha++, 0, "    Esperando: Tedax %d", b->tedax_esperando);
            }
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
        
        // Mostrar detalhes do módulo com cores (quando disponíveis)
        const char* estado_str = nome_estado_modulo(mod->estado);
        int cores_disponiveis = has_colors();
        if (cores_disponiveis) {
            // cores já inicializadas no start_color() da thread
        }

        if (mod->tipo == TIPO_BOTAO && cores_disponiveis) {
            move(linha, 0);
            printw("  M%d Botao ", mod->id);
            imprimir_cor_botao(mod->dados.botao.cor, cores_disponiveis);
            printw(" - %s", estado_str);
            if (mod->estado == MOD_PENDENTE) {
                printw(" - Execucao: %d sec", mod->tempo_total);
            }
        } else if (mod->tipo == TIPO_FIOS && cores_disponiveis) {
            move(linha, 0);
            printw("  M%d Fios ", mod->id);
            imprimir_sequencia_fios_colorida(mod->dados.fios.sequencia, cores_disponiveis);
            printw(" (Padrao %d) - %s", mod->dados.fios.padrao, estado_str);
            if (mod->estado == MOD_PENDENTE) {
                printw(" - Execucao: %d sec", mod->tempo_total);
            }
        } else {
            // Fallback para sem cores ou outros tipos
            char info_modulo[128];
            obter_info_exibicao_modulo(mod, info_modulo, sizeof(info_modulo));
            
            move(linha, 0);
            if (mod->estado == MOD_PENDENTE) {
                printw("  M%d %s - %s - Execucao: %d sec", 
                       mod->id, info_modulo, estado_str, mod->tempo_total);
            } else {
                printw("  M%d %s - %s", 
                       mod->id, info_modulo, estado_str);
            }
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
    

    
    // Área de entrada
    mvprintw(linha++, 0, "Comando: [%s]", buffer_instrucao);
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

// Mostra menu pós-jogo (vitória ou derrota)
// Retorna: 'q' ou 'Q' para sair, 'r' ou 'R' para voltar ao menu
int mostrar_menu_pos_jogo(int vitoria, int tempo_restante, int erros) {
    clear();
    int cores_disponiveis = has_colors();
    
    if (vitoria) {
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
        
        // Mostrar estatísticas de vitória
        mvprintw(LINES / 2 + 4, COLS / 2 - 15, "Tempo Restante: %d segundos", tempo_restante);
        mvprintw(LINES / 2 + 5, COLS / 2 - 15, "Erros: %d", erros);
    } else {
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
    }
    
    // Mostrar opções de menu (abaixo das estatísticas se vitória, ou abaixo da mensagem se derrota)
    int linha_opcoes = vitoria ? LINES / 2 + 7 : LINES / 2 + 4;
    mvprintw(linha_opcoes, COLS / 2 - 15, "Pressione R para voltar ao Menu");
    mvprintw(linha_opcoes + 1, COLS / 2 - 15, "Pressione Q para Sair");
    // Reconfigurar entrada para modo bloqueante e limpo
    // (as threads de jogo usam nodelay/timeout(0); aqui precisamos do oposto)
    refresh();
    flushinp();               // descarta teclas pendentes das threads anteriores
    nodelay(stdscr, FALSE);   // leitura bloqueante
    timeout(-1);              // espera indefinidamente
    keypad(stdscr, TRUE);     // teclas especiais (setas, etc.)
    cbreak();                 // modo de caractere imediato
    noecho();                 // não ecoar teclas
    curs_set(0);              // manter cursor oculto
    int ch;
    while (1) {
        ch = getch();
        if (ch == 'q' || ch == 'Q') {
            return 'q';
        } else if (ch == 'r' || ch == 'R') {
            return 'r';
        }
    }
}

// Mostra mensagem de vitória (mantida para compatibilidade)
void mostrar_mensagem_vitoria(void) {
    mostrar_menu_pos_jogo(1, 0, 0);
}

// Mostra mensagem de derrota (mantida para compatibilidade)
void mostrar_mensagem_derrota(void) {
    mostrar_menu_pos_jogo(0, 0, 0);
}

// Variável global para estado da música (mantém estado entre chamadas)
static int musica_ligada_global = 0;

// Declaração externa da flag de áudio (definida em main.c)
extern int audio_disponivel_global;
extern int audio_disponivel_global; // Declarada em main.c

// Mostra menu principal e retorna o modo escolhido
// Retorna: 0 = Classico, 1-6 = Outros modos (em breve), -1 = Sair
int mostrar_menu_principal(void) {
    clear();
    int cores_disponiveis = has_colors();
    int selecao = 0; // 0 = Classico, 1-5 = Outros, 6 = Configs, 7 = Musica, 8 = Sair
    
    // Configurar para não bloquear na leitura
    nodelay(stdscr, FALSE);
    timeout(-1); // Bloquear até receber entrada
    
    while (1) {
        clear();
        
        // Título
        attron(A_BOLD);
        mvprintw(LINES / 2 - 10, COLS / 2 - 20, "========================================");
        mvprintw(LINES / 2 - 9, COLS / 2 - 20, "  KEEP SOLVING AND NOBODY EXPLODES");
        mvprintw(LINES / 2 - 7, COLS / 2 - 20, "========================================");
        attroff(A_BOLD);
        
        mvprintw(LINES / 2 - 5, COLS / 2 - 15, "Selecione o Modo de Jogo:");
        
        // Opções do menu (texto dinâmico para música)
        const char* texto_musica = musica_ligada_global ? "M. Desligar Musica" : "M. Ligar Musica";
        
        const char* opcoes[9] = {
            "1. Classico",
            "2. Especialistas [Em Breve]",
            "3. Sobrevivencia [Em Breve]",
            "4. Extras [Em Breve]",
            "5. Treino [Em Breve]",
            "6. Custom [Em Breve]",
            "C. Configs [Em Breve]",
            texto_musica,
            "Q. Sair"
        };
        
        // Desenhar opções
        for (int i = 0; i < 9; i++) {
            int y;
            if (i < 6) {
                // Opções 1-6: posições normais
                y = LINES / 2 - 3 + i;
            } else {
                // Opções C, M e Q: adicionar espaço em branco antes (linha extra)
                // i=6 (C. Configs) vai para posição que seria i=7
                // i=7 (M. Ligar/Desligar Musica) vai para posição que seria i=8
                // i=8 (Q. Sair) vai para posição que seria i=9
                y = LINES / 2 - 3 + i + 1;
            }
            int x = COLS / 2 - 20;
            
            if (i == selecao) {
                if (cores_disponiveis) {
                    attron(A_REVERSE | COLOR_PAIR(1));
                } else {
                    attron(A_REVERSE);
                }
            }
            
            mvprintw(y, x, "%s", opcoes[i]);
            
            if (i == selecao) {
                if (cores_disponiveis) {
                    attroff(A_REVERSE | COLOR_PAIR(1));
                } else {
                    attroff(A_REVERSE);
                }
            }
        }
        
        refresh();
        
        // Ler entrada (bloqueia até receber)
        int ch = getch();
        if (ch == KEY_UP || ch == 'w' || ch == 'W') {
            selecao = (selecao - 1 + 10) % 10;
        } else if (ch == KEY_DOWN || ch == 's' || ch == 'S') {
            selecao = (selecao + 1) % 10;
        } else if (ch == '\n' || ch == '\r' || ch == KEY_ENTER) {
            if (selecao == 0) {
                // Classico - retorna 0
                return 0;
            } else if (selecao >= 1 && selecao <= 5) {
                // Modos em breve - não faz nada, apenas mostra que está em breve
                // Pode adicionar uma mensagem aqui se quiser
            } else if (selecao == 6) {
                // Configs em breve - não faz nada
            } else if (selecao == 7) {
                // Toggle música
                // Menu não é fase média, então volume normal
                definir_dificuldade_musica(0);
                if (!audio_disponivel_global) {
                    // Áudio não disponível - mostrar mensagem temporária
                    clear();
                    mvprintw(LINES / 2, COLS / 2 - 30, "Audio nao disponivel!");
                    mvprintw(LINES / 2 + 1, COLS / 2 - 35, "Instale: sudo apt-get install libsdl2-mixer-dev");
                    mvprintw(LINES / 2 + 3, COLS / 2 - 15, "Pressione qualquer tecla...");
                    refresh();
                    nodelay(stdscr, FALSE);
                    getch();
                    nodelay(stdscr, FALSE);
                    timeout(-1);
                } else if (musica_ligada_global) {
                    parar_musica();
                    musica_ligada_global = 0;
                    definir_musica_ligada(0); // Notificar audio.c que música foi desligada
                } else {
                    definir_musica_ligada(1); // Notificar audio.c que música foi ligada
                    if (tocar_musica("sounds/Menu.mp3")) {
                        musica_ligada_global = 1;
                    } else {
                        // Falha ao tocar música - mostrar mensagem
                        clear();
                        mvprintw(LINES / 2, COLS / 2 - 25, "Erro ao tocar musica!");
                        mvprintw(LINES / 2 + 1, COLS / 2 - 20, "Verifique: sounds/Menu.mp3");
                        mvprintw(LINES / 2 + 3, COLS / 2 - 15, "Pressione qualquer tecla...");
                        refresh();
                        nodelay(stdscr, FALSE);
                        getch();
                        nodelay(stdscr, FALSE);
                        timeout(-1);
                    }
                }
            } else if (selecao == 8) {
                // Sair
                return -1;
            }
        } else if (ch == 'q' || ch == 'Q') {
            return -1;
        } else if (ch == '1') {
            return 0; // Classico
        } else if (ch == 'm' || ch == 'M') {
            // Toggle música com tecla M
            if (!audio_disponivel_global) {
                // Áudio não disponível - mostrar mensagem temporária
                clear();
                mvprintw(LINES / 2, COLS / 2 - 30, "Audio nao disponivel!");
                mvprintw(LINES / 2 + 1, COLS / 2 - 35, "Instale: sudo apt-get install libsdl2-mixer-dev");
                mvprintw(LINES / 2 + 3, COLS / 2 - 15, "Pressione qualquer tecla...");
                refresh();
                nodelay(stdscr, FALSE);
                getch();
                nodelay(stdscr, FALSE);
                timeout(-1);
            } else if (musica_ligada_global) {
                parar_musica();
                musica_ligada_global = 0;
                definir_musica_ligada(0); // Notificar audio.c que música foi desligada
            } else {
                // Menu não é fase média, então volume normal
                definir_dificuldade_musica(0);
                definir_musica_ligada(1); // Notificar audio.c que música foi ligada
                if (tocar_musica("sounds/Menu.mp3")) {
                    musica_ligada_global = 1;
                } else {
                    // Falha ao tocar música
                    clear();
                    mvprintw(LINES / 2, COLS / 2 - 25, "Erro ao tocar musica!");
                    mvprintw(LINES / 2 + 1, COLS / 2 - 20, "Verifique: sounds/Menu.mp3");
                    mvprintw(LINES / 2 + 3, COLS / 2 - 15, "Pressione qualquer tecla...");
                    refresh();
                    nodelay(stdscr, FALSE);
                    getch();
                    nodelay(stdscr, FALSE);
                    timeout(-1);
                }
            }
        }
    }
}

// Mostra menu de dificuldades e retorna a dificuldade escolhida (ou -1 para sair)
int mostrar_menu_dificuldades(void) {
    clear();
    int cores_disponiveis = has_colors();
    int selecao = 0; // 0 = Fácil, 1 = Médio, 2 = Difícil, 3 = Sair
    
    // Configurar para bloquear na leitura
    nodelay(stdscr, FALSE);
    timeout(-1); // Bloquear até receber entrada
    
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
        
        // Ler entrada (bloqueia até receber)
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
    }
}

