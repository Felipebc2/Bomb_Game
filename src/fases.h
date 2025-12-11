#ifndef FASES_H
#define FASES_H

#include "game.h"

// Estrutura que contém todas as configurações de uma fase/dificuldade
typedef struct {
    int num_tedax;              // Número de tedax disponíveis
    int num_bancadas;           // Número de bancadas disponíveis
    int modulos_necessarios;    // Número de módulos necessários para vencer
    int intervalo_geracao;      // Intervalo de geração de módulos (em ticks de 0.2s)
    int modulos_iniciais;       // Número de módulos iniciais
    int tempo_total_partida;    // Tempo total da partida em segundos
    int tempo_minimo_execucao;  // Tempo mínimo de execução de um módulo (segundos)
    int tempo_variacao_execucao; // Variação do tempo de execução (segundos)
} ConfigFase;

// Retorna a configuração da fase baseada na dificuldade
const ConfigFase* obter_config_fase(Dificuldade dificuldade);

#endif // FASES_H

