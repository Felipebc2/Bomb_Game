# Manual de Instruções

O jogo possui módulos que precisam ser desarmados. Cada módulo necessita de uma instrução expecifica para ser resolvida baseada em algumas pistas, neste manual você vai ver como identificar os módulos e pegar suas instruções para serem informadas aos tedaxes.

Formato do input: `T<tedax>B<bancada>M<modulo>:<instrucao>`

---

### Módulo de Botão

Cada módulo de botão possui uma cor e você deve apertar ele uma cenrta quantidade de vezes para conseguir resolver esse módulo, para enviar a instrução para o tedax desarmá-lo você deve digitar p a quantidade de vezes que é necessária apertar o botão, segue os casos:

- **Vermelho** → Digite `p` e pressione ENTER
  - Requer **1 aperto** do botão
  - Instrução: `p`

- **Verde** → Digite `pp` e pressione ENTER
  - Requer **2 apertos** do botão
  - Instrução: `pp`

- **Azul** → Digite `ppp` e pressione ENTER
  - Requer **3 apertos** do botão
  - Instrução: `ppp`

---

# Como Enviar Instruções

O jogo utiliza um sistema de comandos onde você especifica qual tedax, bancada e módulo deseja usar. Caso deseje enviar apenas a instrução, os componentes (tedax, bancada e módulo) serão considerados default. Você também pode expecificar apenas um componente.

Relembrando, o formato do input é: `T<tedax>B<bancada>M<modulo>:<instrucao>`

#### Regras de Default

Se você não especificar algum componente:
- **Tedax não especificado**: Usa o tedax livre com o índice mais baixo
- **Bancada não especificada**: Usa a bancada livre com o índice mais baixo
- **Módulo não especificado**: Usa o primeiro módulo pendente da fila

### Importante

- Cada tedax só pode desarmar um módulo por vez
- Cada tedax precisa de uma bancada livre para trabalhar
- Se o tedax ou bancada especificados estiverem ocupados, o comando será ignorado
- Se a instrução estiver **correta**, o módulo é desarmado
- Se a instrução estiver **incorreta**, o módulo volta para o mural de módulos pendentes e você pode tentar novamente
- Cada módulo tem um tempo limite para ser desarmado
- Você pode especificar parcialmente o comando (apenas tedax, apenas bancada, apenas módulo, ou combinações)
- O sistema valida automaticamente se todos os recursos especificados estão disponíveis
