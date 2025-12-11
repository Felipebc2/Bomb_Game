#include "game.h"
#include "ui.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <ncurses.h>

// Inicializa o estado do jogo com a dificuldade escolhida
void inicializar_jogo(GameState *g, Dificuldade dificuldade, int num_tedax, int num_bancadas) {
    // Validar parâmetros
    if (num_tedax < 1) num_tedax = 1;
    if (num_tedax > 3) num_tedax = 3;
    if (num_bancadas < 1) num_bancadas = 1;
    if (num_bancadas > 3) num_bancadas = 3;
    
    g->dificuldade = dificuldade;
    g->tempo_total_partida = 120;  // 120 segundos de partida
    g->tempo_restante = g->tempo_total_partida;
    g->qtd_modulos = 0;
    g->proximo_id_modulo = 1;
    
    // Inicializar múltiplos tedax
    g->qtd_tedax = num_tedax;
    for (int i = 0; i < num_tedax; i++) {
        g->tedax[i].id = i + 1;
        g->tedax[i].estado = TEDAX_LIVRE;
        g->tedax[i].modulo_atual = -1;
        g->tedax[i].bancada_atual = -1;
    }
    
    // Inicializar múltiplas bancadas
    g->qtd_bancadas = num_bancadas;
    for (int i = 0; i < num_bancadas; i++) {
        g->bancadas[i].id = i + 1;
        g->bancadas[i].estado = BANCADA_LIVRE;
        g->bancadas[i].tedax_ocupando = -1;
    }
    
    // Inicializar semente do rand (deve ser antes de usar rand())
    srand(time(NULL));
    
    // Configuração baseada na dificuldade
    switch (dificuldade) {
        case DIFICULDADE_FACIL:
            g->modulos_necessarios = 4;
            g->intervalo_geracao = 100; // 20 segundos (100 ticks * 0.2s)
            break;
        case DIFICULDADE_MEDIO:
            g->modulos_necessarios = 8;
            g->intervalo_geracao = 75; // 15 segundos (75 ticks * 0.2s)
            break;
        case DIFICULDADE_DIFICIL:
            g->modulos_necessarios = 12;
            g->intervalo_geracao = 50; // 10 segundos (50 ticks * 0.2s)
            break;
    }
    
    g->max_modulos = g->modulos_necessarios; // máximo = necessário para vencer
    g->ticks_desde_ultimo_modulo = 0;
    g->jogo_rodando = 1;
    g->jogo_terminou = 0;
    g->mensagem_erro[0] = '\0'; // Inicializar mensagem de erro vazia
    
    // Inicializar mutex e condition variables
    pthread_mutex_init(&g->mutex_jogo, NULL);
    pthread_cond_init(&g->cond_modulo_disponivel, NULL);
    pthread_cond_init(&g->cond_bancada_disponivel, NULL);
    pthread_cond_init(&g->cond_tela_atualizada, NULL);
    
    // Gerar primeiro módulo imediatamente
    pthread_mutex_lock(&g->mutex_jogo);
    gerar_novo_modulo(g);
    pthread_mutex_unlock(&g->mutex_jogo);
}

// Finaliza o jogo e libera recursos
void finalizar_jogo(GameState *g) {
    g->jogo_rodando = 0;
    
    // Destruir mutex e condition variables
    pthread_mutex_destroy(&g->mutex_jogo);
    pthread_cond_destroy(&g->cond_modulo_disponivel);
    pthread_cond_destroy(&g->cond_bancada_disponivel);
    pthread_cond_destroy(&g->cond_tela_atualizada);
}

// Gera um novo módulo e adiciona ao jogo
// NOTA: Deve ser chamada com mutex_jogo já travado
void gerar_novo_modulo(GameState *g) {
    if (g->qtd_modulos >= 100) {
        return; // Limite máximo de módulos atingido
    }
    
    Modulo *novo = &g->modulos[g->qtd_modulos];
    
    // Atribuir ID
    novo->id = g->proximo_id_modulo++;
    
    // Sortear cor (0 = Vermelho, 1 = Verde, 2 = Azul)
    int cor_aleatoria = rand() % 3;
    novo->cor = (CorBotao)cor_aleatoria;
    
    // Definir tempo total (entre 3 e 8 segundos)
    novo->tempo_total = 3 + (rand() % 6);
    novo->tempo_restante = novo->tempo_total;
    
    // Definir instrução correta baseada na cor
    switch (novo->cor) {
        case COR_VERMELHO:
            strcpy(novo->instrucao_correta, "p");
            break;
        case COR_VERDE:
            strcpy(novo->instrucao_correta, "pp");
            break;
        case COR_AZUL:
            strcpy(novo->instrucao_correta, "ppp");
            break;
    }
    
    // Limpar instrução digitada
    novo->instrucao_digitada[0] = '\0';
    
    // Estado inicial: pendente
    novo->estado = MOD_PENDENTE;
    novo->tempo_desde_resolvido = -1; // -1 significa não resolvido ainda
    
    g->qtd_modulos++;
    
    // Sinalizar que há um novo módulo disponível
    pthread_cond_broadcast(&g->cond_modulo_disponivel);
}

// Função removida - lógica movida para thread_mural

// Função removida - lógica movida para thread_tedax

// Conta quantos módulos foram resolvidos
// NOTA: Deve ser chamada com mutex_jogo já travado
int contar_modulos_resolvidos(const GameState *g) {
    int resolvidos = 0;
    for (int i = 0; i < g->qtd_modulos; i++) {
        if (g->modulos[i].estado == MOD_RESOLVIDO) {
            resolvidos++;
        }
    }
    return resolvidos;
}

// Verifica se há módulos pendentes
// NOTA: Deve ser chamada com mutex_jogo já travado
int tem_modulos_pendentes(const GameState *g) {
    for (int i = 0; i < g->qtd_modulos; i++) {
        if (g->modulos[i].estado == MOD_PENDENTE) {
            return 1; // Há pelo menos um módulo pendente
        }
    }
    return 0; // Não há módulos pendentes
}

// Verifica se todos os módulos necessários foram resolvidos
// NOTA: Deve ser chamada com mutex_jogo já travado
int todos_modulos_resolvidos(const GameState *g) {
    int resolvidos = contar_modulos_resolvidos(g);
    // Vitória quando resolver pelo menos o número necessário de módulos
    return (resolvidos >= g->modulos_necessarios);
}

// Retorna o nome da cor como string
const char* nome_cor(CorBotao cor) {
    switch (cor) {
        case COR_VERMELHO:
            return "Vermelho";
        case COR_VERDE:
            return "Verde";
        case COR_AZUL:
            return "Azul";
        default:
            return "Desconhecida";
    }
}

// Retorna o estado do módulo como string
const char* nome_estado_modulo(EstadoModulo estado) {
    switch (estado) {
        case MOD_PENDENTE:
            return "PENDENTE";
        case MOD_EM_EXECUCAO:
            return "EM_EXECUCAO";
        case MOD_RESOLVIDO:
            return "RESOLVIDO";
        default:
            return "DESCONHECIDO";
    }
}

// Retorna o nome da dificuldade como string
const char* nome_dificuldade(Dificuldade dificuldade) {
    switch (dificuldade) {
        case DIFICULDADE_FACIL:
            return "FACIL";
        case DIFICULDADE_MEDIO:
            return "MEDIO";
        case DIFICULDADE_DIFICIL:
            return "DIFICIL";
        default:
            return "DESCONHECIDA";
    }
}

// ============================================================================
// IMPLEMENTAÇÃO DAS THREADS
// ============================================================================

// Thread do Mural de Módulos Pendentes
void* thread_mural(void* arg) {
    GameState *g = (GameState*)arg;
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 200000000L; // 0.2 segundos
    
    while (g->jogo_rodando && !g->jogo_terminou) {
        pthread_mutex_lock(&g->mutex_jogo);
        
        if (g->qtd_modulos < g->max_modulos) {
            g->ticks_desde_ultimo_modulo++;
            
            if (g->ticks_desde_ultimo_modulo >= g->intervalo_geracao) {
                gerar_novo_modulo(g);
                g->ticks_desde_ultimo_modulo = 0;
            }
        }
        
        // Se não há módulos pendentes e ainda não gerou o máximo, gerar imediatamente
        if (!tem_modulos_pendentes(g) && g->qtd_modulos < g->max_modulos) {
            gerar_novo_modulo(g);
        }
        
        pthread_mutex_unlock(&g->mutex_jogo);
        
        nanosleep(&ts, NULL);
    }
    
    return NULL;
}

// Thread de Exibição de Informações
void* thread_exibicao(void* arg) {
    GameState *g = (GameState*)arg;
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 200000000L; // 0.2 segundos
    
    // Buffer de instrução compartilhado (gerenciado pela thread coordenador)
    extern char buffer_instrucao_global[64];
    
    // Inicializar ncurses nesta thread
    inicializar_ncurses();
    
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_CYAN, COLOR_BLACK);
        init_pair(2, COLOR_GREEN, COLOR_BLACK);
        init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    }
    
    while (g->jogo_rodando && !g->jogo_terminou) {
        pthread_mutex_lock(&g->mutex_jogo);
        
        // Desenhar tela
        desenhar_tela(g, buffer_instrucao_global);
        
        pthread_mutex_unlock(&g->mutex_jogo);
        
        nanosleep(&ts, NULL);
    }
    
    finalizar_ncurses();
    
    return NULL;
}

// Thread de um Tedax
void* thread_tedax(void* arg) {
    typedef struct {
        GameState *g;
        int tedax_id;
    } TedaxArgs;
    
    TedaxArgs *args = (TedaxArgs*)arg;
    GameState *g = args->g;
    int tedax_id = args->tedax_id;
    Tedax *tedax = &g->tedax[tedax_id];
    
    struct timespec ts;
    ts.tv_sec = 1;  // 1 segundo
    ts.tv_nsec = 0;
    
    while (g->jogo_rodando && !g->jogo_terminou) {
        pthread_mutex_lock(&g->mutex_jogo);
        
        // Incrementar tempo desde resolvido para todos os módulos resolvidos
        for (int i = 0; i < g->qtd_modulos; i++) {
            if (g->modulos[i].estado == MOD_RESOLVIDO && g->modulos[i].tempo_desde_resolvido >= 0) {
                g->modulos[i].tempo_desde_resolvido++;
            }
        }
        
        if (tedax->estado == TEDAX_OCUPADO && tedax->modulo_atual >= 0) {
            Modulo *mod = &g->modulos[tedax->modulo_atual];
            
            // Decrementar tempo restante
            mod->tempo_restante--;
            
            // Quando o tempo acabar, verificar se a instrução estava correta
            if (mod->tempo_restante <= 0) {
            // Comparar instrução digitada com a correta
            if (strcmp(mod->instrucao_digitada, mod->instrucao_correta) == 0) {
                // Instrução correta: módulo resolvido
                mod->estado = MOD_RESOLVIDO;
                mod->tempo_desde_resolvido = 0; // Iniciar contador de tempo desde resolvido
            } else {
                // Instrução incorreta: módulo volta para o mural
                mod->estado = MOD_PENDENTE;
                mod->tempo_restante = mod->tempo_total; // Resetar tempo
                mod->instrucao_digitada[0] = '\0';      // Limpar instrução
                mod->tempo_desde_resolvido = -1;         // Resetar contador
            }
                
                    // Liberar bancada
                if (tedax->bancada_atual >= 0) {
                    g->bancadas[tedax->bancada_atual].estado = BANCADA_LIVRE;
                    g->bancadas[tedax->bancada_atual].tedax_ocupando = -1;
                    pthread_cond_broadcast(&g->cond_bancada_disponivel);
                }
                
                // Liberar tedax
                tedax->estado = TEDAX_LIVRE;
                tedax->modulo_atual = -1;
                tedax->bancada_atual = -1;
                
                // Se não há módulos pendentes e ainda não gerou o máximo, gerar um novo imediatamente
                if (!tem_modulos_pendentes(g) && g->qtd_modulos < g->max_modulos) {
                    gerar_novo_modulo(g);
                }
                
                // Sinalizar que há módulo disponível novamente
                pthread_cond_broadcast(&g->cond_modulo_disponivel);
            }
        }
        
        pthread_mutex_unlock(&g->mutex_jogo);
        
        nanosleep(&ts, NULL);
    }
    
    free(args);
    return NULL;
}

// Função auxiliar para processar comando do formato T1B1M1:ppp
// Retorna 1 se sucesso, 0 se falha
int processar_comando(const char* buffer, GameState *g, 
                      int *tedax_idx, int *bancada_idx, int *modulo_idx, 
                      char *instrucao) {
    // Inicializar valores como -1 (não especificado)
    *tedax_idx = -1;
    *bancada_idx = -1;
    *modulo_idx = -1;
    instrucao[0] = '\0';
    
    // Procurar por ':' que separa o comando da instrução
    const char *separador = strchr(buffer, ':');
    
    if (!separador) {
        // Se não há ':', tratar tudo como instrução (caso: apenas "ppp")
        // Verificar se há algum T, B ou M no buffer - se houver, é formato inválido
        int tem_comando = 0;
        for (int i = 0; buffer[i] != '\0'; i++) {
            if (buffer[i] == 'T' || buffer[i] == 't' || 
                buffer[i] == 'B' || buffer[i] == 'b' || 
                buffer[i] == 'M' || buffer[i] == 'm') {
                tem_comando = 1;
                break;
            }
        }
        
        if (tem_comando) {
            return 0; // Formato inválido - tem T/B/M mas falta ':'
        }
        
        // Apenas instrução, sem especificações
        strncpy(instrucao, buffer, 15);
        instrucao[15] = '\0';
        return 1;
    }
    
    // Copiar instrução (parte após ':')
    strncpy(instrucao, separador + 1, 15);
    instrucao[15] = '\0';
    
    // Se a parte antes do ':' está vazia (caso: ":ppp"), apenas instrução
    int len_comando = separador - buffer;
    if (len_comando == 0) {
        return 1; // Apenas instrução, sem especificações
    }
    
    // Processar parte antes do ':' (T1B1M1)
    char comando[32];
    strncpy(comando, buffer, len_comando);
    comando[len_comando] = '\0';
    
    // Fazer parsing do comando
    int i = 0;
    while (i < len_comando) {
        if (comando[i] == 'T' || comando[i] == 't') {
            // Ler número do tedax
            i++;
            int num = 0;
            while (i < len_comando && comando[i] >= '0' && comando[i] <= '9') {
                num = num * 10 + (comando[i] - '0');
                i++;
            }
            if (num >= 1 && num <= g->qtd_tedax) {
                *tedax_idx = num - 1; // Converter para índice (0-based)
            } else {
                return 0; // Tedax inválido
            }
        } else if (comando[i] == 'B' || comando[i] == 'b') {
            // Ler número da bancada
            i++;
            int num = 0;
            while (i < len_comando && comando[i] >= '0' && comando[i] <= '9') {
                num = num * 10 + (comando[i] - '0');
                i++;
            }
            if (num >= 1 && num <= g->qtd_bancadas) {
                *bancada_idx = num - 1; // Converter para índice (0-based)
            } else {
                return 0; // Bancada inválida
            }
        } else if (comando[i] == 'M' || comando[i] == 'm') {
            // Ler número do módulo (ID do módulo, não índice)
            i++;
            int num = 0;
            while (i < len_comando && comando[i] >= '0' && comando[i] <= '9') {
                num = num * 10 + (comando[i] - '0');
                i++;
            }
            // Procurar módulo pelo ID
            int encontrado = 0;
            for (int j = 0; j < g->qtd_modulos; j++) {
                if (g->modulos[j].id == num) {
                    *modulo_idx = j;
                    encontrado = 1;
                    break;
                }
            }
            if (!encontrado) {
                return 0; // Módulo não encontrado
            }
        } else {
            i++; // Ignorar caracteres desconhecidos
        }
    }
    
    return 1; // Sucesso
}

// Thread do Coordenador (Jogador)
void* thread_coordenador(void* arg) {
    GameState *g = (GameState*)arg;
    extern char buffer_instrucao_global[64];
    int buffer_len = 0;
    
    while (g->jogo_rodando && !g->jogo_terminou) {
        int ch = getch();
        
        if (ch == ERR) {
            // Nenhuma tecla pressionada
        } else if (ch == 'q' || ch == 'Q') {
            pthread_mutex_lock(&g->mutex_jogo);
            g->jogo_rodando = 0;
            pthread_mutex_unlock(&g->mutex_jogo);
            break;
        } else {
            pthread_mutex_lock(&g->mutex_jogo);
            
            // Processar BACKSPACE
            if (ch == KEY_BACKSPACE || ch == 127 || ch == '\b') {
                if (buffer_len > 0) {
                    buffer_len--;
                    buffer_instrucao_global[buffer_len] = '\0';
                }
            }
            // Processar ENTER para enviar comando
            else if (ch == '\n' || ch == '\r') {
                // Limpar mensagem de erro anterior
                g->mensagem_erro[0] = '\0';
                
                int tedax_idx = -1, bancada_idx = -1, modulo_idx = -1;
                char instrucao[16] = "";
                
                // Processar comando no formato T1B1M1:ppp
                if (processar_comando(buffer_instrucao_global, g, &tedax_idx, &bancada_idx, &modulo_idx, instrucao)) {
                    // Aplicar regras de default para valores não especificados
                    
                    // Se tedax não especificado, pegar o tedax livre com índice mais baixo
                    if (tedax_idx == -1) {
                        for (int i = 0; i < g->qtd_tedax; i++) {
                            if (g->tedax[i].estado == TEDAX_LIVRE) {
                                tedax_idx = i;
                                break;
                            }
                        }
                    }
                    
                    // Se bancada não especificada, pegar a bancada livre com índice mais baixo
                    if (bancada_idx == -1) {
                        for (int i = 0; i < g->qtd_bancadas; i++) {
                            if (g->bancadas[i].estado == BANCADA_LIVRE) {
                                bancada_idx = i;
                                break;
                            }
                        }
                    }
                    
                    // Se módulo não especificado, pegar o primeiro módulo pendente
                    if (modulo_idx == -1) {
                        for (int i = 0; i < g->qtd_modulos; i++) {
                            if (g->modulos[i].estado == MOD_PENDENTE) {
                                modulo_idx = i;
                                break;
                            }
                        }
                    }
                    
                    // Validar se todos os recursos estão disponíveis
                    int valido = 1;
                    
                    if (tedax_idx < 0 || tedax_idx >= g->qtd_tedax) {
                        valido = 0;
                    } else if (g->tedax[tedax_idx].estado != TEDAX_LIVRE) {
                        valido = 0; // Tedax especificado não está livre
                    }
                    
                    if (bancada_idx < 0 || bancada_idx >= g->qtd_bancadas) {
                        valido = 0;
                    } else if (g->bancadas[bancada_idx].estado != BANCADA_LIVRE) {
                        valido = 0; // Bancada especificada não está livre
                    }
                    
                    if (modulo_idx < 0 || modulo_idx >= g->qtd_modulos) {
                        valido = 0;
                    } else if (g->modulos[modulo_idx].estado != MOD_PENDENTE) {
                        valido = 0; // Módulo especificado não está pendente
                    }
                    
                    if (valido && strlen(instrucao) > 0) {
                        // Designar módulo para o tedax
                        Modulo *mod = &g->modulos[modulo_idx];
                        Tedax *t = &g->tedax[tedax_idx];
                        
                        // Copiar instrução
                        strncpy(mod->instrucao_digitada, instrucao, 15);
                        mod->instrucao_digitada[15] = '\0';
                        
                        // Mudar estado do módulo
                        mod->estado = MOD_EM_EXECUCAO;
                        mod->tempo_restante = mod->tempo_total;
                        
                        // Ocupar tedax e bancada
                        t->estado = TEDAX_OCUPADO;
                        t->modulo_atual = modulo_idx;
                        t->bancada_atual = bancada_idx;
                        
                        g->bancadas[bancada_idx].estado = BANCADA_OCUPADA;
                        g->bancadas[bancada_idx].tedax_ocupando = t->id;
                        
                        // Limpar buffer
                        buffer_len = 0;
                        buffer_instrucao_global[0] = '\0';
                    } else {
                        // Recursos não disponíveis ou inválidos
                        strncpy(g->mensagem_erro, "Entrada Invalida", 63);
                        g->mensagem_erro[63] = '\0';
                        buffer_len = 0;
                        buffer_instrucao_global[0] = '\0';
                    }
                }
                // Se o parsing falhou, mostrar erro e limpar buffer
                else {
                    strncpy(g->mensagem_erro, "Entrada Invalida", 63);
                    g->mensagem_erro[63] = '\0';
                    buffer_len = 0;
                    buffer_instrucao_global[0] = '\0';
                }
            }
            // Aceitar qualquer caractere imprimível
            else if (ch >= 32 && ch <= 126) {
                if (buffer_len < 63) { // Aumentar limite para suportar comandos maiores
                    buffer_instrucao_global[buffer_len] = (char)ch;
                    buffer_len++;
                    buffer_instrucao_global[buffer_len] = '\0';
                }
            }
            
            pthread_mutex_unlock(&g->mutex_jogo);
        }
        
        // Pequeno delay para não sobrecarregar
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 50000000L; // 50ms
        nanosleep(&ts, NULL);
    }
    
    return NULL;
}

