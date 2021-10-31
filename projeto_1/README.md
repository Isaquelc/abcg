# Projeto 1

Disponível [aqui](https://isaquelc.github.io/abcg/projeto_1/)

Este é o primeiro projeto da matéria Computação Gráfica da UFABC. Neste projeto, precisávamos desenvolver uma aplicação 2D interativa, e eu fiz um jogo 2D inspirado em jogos antigos de Atari. 

O jogador controla um carro, e precisa desviar de outros carros que vem na direção oposta. O jogo fica progressiivamente mais rápido, e a pontuação é defeniida pela distância que o jogador percorre. 

Eu organizei o projeto em classes, das seguinte maneira:

### OpenGLWindow
Responsável por gerenciar a aplicação e a janela OpenGL. Aqui são definidos os arquivos:
 - **openglwindow.hpp**: define a classe OpenGLWindow, com as declarações de variáveis e funções
 - **openglwindow.cpp**: define as funções de OpenGLWindow, que são responsáveis por receber os inputs do usuário, iniicilizar, reiniciar, atualiziar, desenhar, a mudar o tamanho, e fechar a janela OpenGL, assim como desenhar elementos da interface e cehcar colisões entre objetos.

### Player
Responsável pelo jogador, é também composta por dois arquivos: **player.hpp** e **player.cpp**. Esta classe define o jogador principal:
 - seu formato é definido por meio de um VBO de véritices, um EBO de íncices e um VAO que junta tudo isso e gera a entrada para o vertex shader, na função *initializeGL*, e sua cor é definida na variável *m_color*.
 - O movimento do jogador é defiiniido na função *update*, e depende também do input do jogador, que receberá do OpenGLWindow por meio do GameData. Quando o jogador apertar as teclas da direita, esquera, cima ou baixo, o carro se movimentará na respectiva direção. Para a velociade ser constante, a posição x ou y será ajustada por meio da variável deltaTime. Também fazemos um check na posição para garantir que não vamos para fora da área de jogo.

### Enemies
Responsável por gerenciar os inimigos, isto é, os carros vindo na direção oposta. Estes são definidos nos arquivos **enemies.hpp** e **enemies.cpp**.
 - Os inimigos são iguais ao jogador, porém invertidos no eixo y, e com cor aleatória. 
 - Um inimigo é definido por um struct, e assim é construída uma lista de inimigos (no caso deste jogo são inicializados 4). 
 - Uma vez inicializados, eles se movem de cima para baixo com uma velocidade proporcional ao tempo desde que o jogo começou (portanto vão acelerando ao passo que o jogo avança). Depois que um carro inimigo sai completamente da janela pela parte de baixo, é definida uuma nova posição x e uma nova posição y (que seja acima da tela) aleatórias, assim como uma nova cor aleatória, para o carro aparecer de novo como um novo carro.

### Road
Também composta por dois arquivos, o **road.hpp** e o **road.cpp**. Ela definie a divisão entre duas ruas e serve para adicionar claridade ao jogo, pois indica a direção e velocidade em que o jogador está se movendo. 
 - Um retângulo amarelo é desenhado na tela repetidas vezes
 - Ele é transladado para baixo, também à uma proporção que aumenta ao passo que o tempo de jogo aumenta (mas numa velocidade menor que a dos carros vindo na direção oposta).
 - Quando verificamos que o retângulo já está foera da tela, mudamos sua posição para cima da área visível, de tal forma que ele aparece novamente na dela vindo de cima para baixo (indicando que o jogador está se movimentando para cima).

### GameData
Esta classe é somente composta por um arquivo **gamedata.hpp**, e é responsável por facilitar a comunicação entre as classes. Ela contém as seguintes veriáveis:
 - *m_state*: indica o estado do jogo, isto é, se ele está sendo jogado ou se já acbaou.
 - *m_iniput*: indica o tipo de input, para que o jogador possa se movimentar na direção correta.
 - *gameScore*: armazena a pontuação do jogador, que começa como 0 e depois vai aumentando junto com o tempo passado e a velocidade do jogo.
 - *gameSpeed*: inidica a velociidade do jogo. Começa como 1, e vai aumentando junto com o tempo que passou desde o jogo começar. É um fator multiplicativo que é aplicado à velocidade da rua, dos inimigos, e da pontuação.

======

Além destas classes, temos alguns outros arquivos. 

#### Assets
Contém a fonte utilizada para escrever na tela, e mais dois arquivos:
 - *objects.frag*: simplismente atriui uma cor de entrada como a cor de saída para o desenho de diferentes objetos.
 - *objects.vert*: Atualiza a posição posição do objeto e sua cor.

#### Arquivos para o Web Assembly
São os arquivos *index.html*, *projeto_1.js*, *projeto_1.wasm*, responsáveis pela versão do jogo que roda no web browser.

Além desses arquivos, temos o folder *build*, que contém iniformações geradas pelo compilador na hora de gerar o .exe, e o *CMakeLists.txt*, que também é utilizada para compilar o projeto.

Isto tudo é unido pelo arquivo *main.cpp*, que inicia a janela e o jogo.