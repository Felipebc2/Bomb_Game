# Instruções - Keep Solving and Nobody Explodes

## Níveis de Dificuldade

O jogo possui três níveis de dificuldade, cada um com características diferentes:

### Fácil
- **Módulos necessários**: 4 módulos
- **Tempo total**: 120 segundos
- **Geração de módulos**: 1 novo módulo a cada 20 segundos
- **Ideal para**: Iniciantes que estão aprendendo o jogo

### Médio
- **Módulos necessários**: 8 módulos
- **Tempo total**: 120 segundos
- **Geração de módulos**: 1 novo módulo a cada 15 segundos
- **Ideal para**: Jogadores com experiência intermediária

### Difícil
- **Módulos necessários**: 12 módulos
- **Tempo total**: 120 segundos
- **Geração de módulos**: 1 novo módulo a cada 10 segundos
- **Ideal para**: Jogadores experientes que buscam desafio

### Regras Gerais
- O jogo começa com **1 módulo já gerado**
- Novos módulos são gerados automaticamente conforme o intervalo da dificuldade escolhida
- A geração de módulos **para** quando o número máximo necessário é atingido
- Para vencer, você precisa resolver **todos os módulos necessários** da dificuldade escolhida
- O jogo termina em **VITÓRIA** quando todos os módulos necessários são resolvidos
- O jogo termina em **DERROTA** se o tempo de 120 segundos acabar e você não tiver resolvido todos os módulos necessários

## Como Resolver os Módulos

O jogo possui módulos de botão que precisam ser desarmados. Cada módulo tem uma cor específica e requer uma sequência de apertos diferente.

### Módulo de Botão

Cada módulo de botão possui uma cor (Vermelho, Verde ou Azul) e você deve enviar a instrução correta para o tedax desarmá-lo.

#### Regras de Resolução:

- **Vermelho** → Digite `p` e pressione ENTER
  - Requer **1 aperto** do botão
  - Instrução: `p`

- **Verde** → Digite `pp` e pressione ENTER
  - Requer **2 apertos** do botão
  - Instrução: `pp`

- **Azul** → Digite `ppp` e pressione ENTER
  - Requer **3 apertos** do botão
  - Instrução: `ppp`

### Como Enviar Instruções

O jogo utiliza um sistema de comandos explícito onde você especifica qual tedax, bancada e módulo deseja usar.

#### Formato do Comando

O formato é: `T<tedax>B<bancada>M<modulo>:<instrucao>`

**Exemplos:**
- `T1B1M1:ppp` - Usa Tedax 1, Bancada 1, Módulo 1, com instrução "ppp"
- `T3B2M1:pp` - Usa Tedax 3, Bancada 2, Módulo 1, com instrução "pp"
- `B1M2:p` - Usa Bancada 1, Módulo 2, com instrução "p" (tedax será o primeiro disponível)
- `M1:ppp` - Usa Módulo 1, com instrução "ppp" (tedax e bancada serão os primeiros disponíveis)
- `:pp` - Apenas instrução "pp" (tedax, bancada e módulo serão os primeiros disponíveis)

#### Passo a Passo

1. Observe a cor do módulo pendente na lista de módulos
2. Determine qual tedax e bancada você deseja usar (ou deixe em branco para usar os primeiros disponíveis)
3. Digite o comando no formato `T<tedax>B<bancada>M<modulo>:<instrucao>`
   - Para Vermelho: instrução `p`
   - Para Verde: instrução `pp`
   - Para Azul: instrução `ppp`
4. Use BACKSPACE para corrigir se necessário
5. Pressione ENTER para enviar o comando

#### Regras de Default

Se você não especificar algum componente:
- **Tedax não especificado**: Usa o tedax livre com o índice mais baixo
- **Bancada não especificada**: Usa a bancada livre com o índice mais baixo
- **Módulo não especificado**: Usa o primeiro módulo pendente da fila

#### Exemplos Práticos

- Você tem 3 tedax e 2 bancadas disponíveis, e quer usar o Tedax 2 na Bancada 1 para resolver o Módulo 3 (Azul):
  - Digite: `T2B1M3:ppp`
  
- Você quer usar qualquer tedax disponível na Bancada 2 para resolver o primeiro módulo pendente (Verde):
  - Digite: `B2:pp`
  
- Você quer resolver o Módulo 5 (Vermelho) usando os primeiros recursos disponíveis:
  - Digite: `M5:p`

### Importante

- Cada tedax só pode desarmar um módulo por vez
- Cada tedax precisa de uma bancada livre para trabalhar
- Se o tedax ou bancada especificados estiverem ocupados, o comando será ignorado
- Se a instrução estiver **correta**, o módulo é desarmado
- Se a instrução estiver **incorreta**, o módulo volta para o mural de módulos pendentes e você pode tentar novamente
- Cada módulo tem um tempo limite para ser desarmado
- Você pode especificar parcialmente o comando (apenas tedax, apenas bancada, apenas módulo, ou combinações)
- O sistema valida automaticamente se todos os recursos especificados estão disponíveis

### Dicas

- Preste atenção na cor do módulo antes de enviar a instrução
- Você pode ver a instrução correta necessária na lista de módulos pendentes
- Gerencie bem o tempo, pois novos módulos aparecem regularmente
- O jogo termina quando todos os módulos são desarmados (VITÓRIA) ou quando o tempo acaba com módulos pendentes (DERROTA)

