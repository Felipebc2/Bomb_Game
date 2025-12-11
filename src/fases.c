#include "fases.h"

// Configurações para cada dificuldade/fase
static const ConfigFase config_fases[] = {
    // DIFICULDADE_FACIL
    {
        .num_tedax = 2,
        .num_bancadas = 1,
        .modulos_necessarios = 4,
        .intervalo_geracao = 65,        // 13 segundos (65 ticks * 0.2s)
        .modulos_iniciais = 1,
        .tempo_total_partida = 80,      // em Segundos
        .tempo_minimo_execucao = 3,     // 3 segundos mínimo
        .tempo_variacao_execucao = 8    // Variação de 0 a 8 segundos
    },
    
    // DIFICULDADE_MEDIO
    {
        .num_tedax = 3,
        .num_bancadas = 2,
        .modulos_necessarios = 8,
        .intervalo_geracao = 50,        // 10 segundos (50 ticks * 0.2s)
        .modulos_iniciais = 2,
        .tempo_total_partida = 120,     // em Segundos
        .tempo_minimo_execucao = 5,     // 5 segundos mínimo (+2 do fácil)
        .tempo_variacao_execucao = 15   // Variação de 0 a 15 segundos (50% mais que fácil)
    },
    
    // DIFICULDADE_DIFICIL
    {
        .num_tedax = 4,
        .num_bancadas = 3,
        .modulos_necessarios = 12,
        .intervalo_geracao = 35,        // 7 segundos (35 ticks * 0.2s)
        .modulos_iniciais = 3,
        .tempo_total_partida = 100,     // em Segundos
        .tempo_minimo_execucao = 9,     // 9 segundos mínimo (+4 do fácil)
        .tempo_variacao_execucao = 20   // Variação de 0 a 20 segundos (100% mais que fácil)
    }
};

// Retorna a configuração da fase baseada na dificuldade
const ConfigFase* obter_config_fase(Dificuldade dificuldade) {
    int index = 0;
    
    switch (dificuldade) {
        case DIFICULDADE_FACIL:
            index = 0;
            break;
        case DIFICULDADE_MEDIO:
            index = 1;
            break;
        case DIFICULDADE_DIFICIL:
            index = 2;
            break;
        default:
            index = 0; // Default para fácil
            break;
    }
    
    return &config_fases[index];
}

