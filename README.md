# Keep Solving and Nobody Explodes

Jogo de desarmamento de bombas implementado em C com ncurses e pthreads.

## Estrutura do Projeto

```
bomb_game/
├── src/
│   ├── main.c
│   ├── game.h/.c                # Lógica do jogo e implementação das threads
│   └── ui.h/.c                  # Interface ncurses
├── others/           
│   └── trabalho-pc-2025-02.pdf
├── Manual.md                    # Manual de Instruções dos módulos
└── Makefile                     # Compilação
```

### Instalação do ncurses (Ubuntu/Debian)

```bash
sudo apt-get install libncurses5-dev libncursesw5-dev
```

## Compilação

### Usando Makefile

```bash
make
```

### Compilação manual

```bash
gcc -Wall -Wextra -std=c11 -Isrc -pthread src/main.c src/game.c src/ui.c -o jogo -lncurses -pthread
```

## Execução

```bash
./jogo
```

## Como Jogar

1. Ao iniciar o jogo, você verá um **menu de seleção de dificuldade** com três opções:
   - **Fácil**: 4 módulos, 1 a cada 20 segundos
   - **Médio**: 8 módulos, 1 a cada 15 segundos
   - **Difícil**: 12 módulos, 1 a cada 10 segundos
2. Use as setas do teclado para navegar e ENTER para selecionar a dificuldade.
3. O jogo começa com 1 módulo já gerado.
4. Cada módulo tem uma cor e requer uma sequência específica, que no arquivo de instruções está expecificado ao jogador
5. Digite a sequência de caracteres instruida pelo arquivo de instruções e pressione ENTER para designar o módulo ao tedax.
6. O tedax ocupará a bancada pelo tempo necessário para desarmar o módulo.
7. Se a instrução estiver correta, o módulo é desarmado. Se estiver incorreta, ele volta para o mural como ultimo da fila.
8. O jogo termina quando:
   - Todos os módulos necessários são desarmados (VITÓRIA)
   - O tempo acaba e ainda há módulos pendentes (DERROTA)

## Controles

- `BACKSPACE`: Remove o último caractere da instrução
- `ENTER`: Envia a instrução para o tedax (se ele estiver livre)
- `q`: Sair do jogo (força fim imediato)

## Funcionamento das Threads

O jogo foi implementado usando programação concorrente com pthreads. Cada componente principal do jogo roda em uma thread separada, permitindo execução paralela e melhor responsividade.

### Threads Implementadas

1. **Thread do Mural de Módulos Pendentes** (`thread_mural`)
   - Responsável por gerar novos módulos conforme o intervalo da dificuldade
   - Gera módulos automaticamente a cada X segundos (dependendo da dificuldade)
   - Gera imediatamente um novo módulo se não houver módulos pendentes
   - Executa em loop contínuo enquanto o jogo está rodando

2. **Thread de Exibição de Informações** (`thread_exibicao`)
   - Responsável por atualizar a interface do jogo na tela
   - Redesenha a tela a cada 0.2 segundos
   - Mostra estado dos tedax, bancadas, módulos e informações do jogo
   - Filtra módulos resolvidos antigos para manter a tela limpa

3. **Threads dos Tedax** (`thread_tedax`)
   - Uma thread para cada tedax disponível (1-3 tedax)
   - Cada tedax processa seu módulo em execução independentemente
   - Decrementa o tempo restante do módulo a cada segundo
   - Verifica se a instrução estava correta quando o tempo acaba
   - Incrementa o contador de tempo desde resolvido para módulos resolvidos

4. **Thread do Coordenador** (`thread_coordenador`)
   - Responsável por processar a entrada do jogador
   - Lê teclas do teclado em tempo real
   - Designa módulos pendentes para tedax livres
   - Verifica disponibilidade de bancadas antes de designar módulos
   - Gerencia o buffer de instrução do jogador

### Sincronização

O jogo utiliza mecanismos de sincronização para garantir consistência dos dados compartilhados:

- **Mutex (`mutex_jogo`)**: Protege todas as operações de leitura/escrita no estado do jogo
  - Lista de módulos
  - Vetor de tedax
  - Vetor de bancadas
  - Tempo restante
  - Flags de controle do jogo

- **Condition Variables**: Usadas para sinalizar eventos importantes
  - `cond_modulo_disponivel`: Sinaliza quando há um novo módulo disponível
  - `cond_bancada_disponivel`: Sinaliza quando uma bancada fica livre
  - `cond_tela_atualizada`: Sinaliza quando a tela precisa ser atualizada

### Múltiplos Tedax e Bancadas

O jogo suporta configuração de 1 a 3 tedax e 1 a 3 bancadas:
- Cada tedax pode trabalhar em paralelo em um módulo diferente
- Cada tedax precisa ocupar uma bancada livre para desarmar um módulo

### Remoção Automática de Módulos Resolvidos

Para evitar que a tela fique cheia de módulos resolvidos:
- Módulos resolvidos são removidos automaticamente da exibição após um tempo
- Isso garante que sempre haja espaço na tela para módulos pendentes

## Notas

- **Mural de Módulos Pendentes**: Implementado na thread `thread_mural`
- **Exibição de Informações**: Implementado na thread `thread_exibicao`
- **Coordenador (Jogador)**: Implementado na thread `thread_coordenador`
- **Tedax**: Implementado nas threads `thread_tedax` (uma por tedax)
