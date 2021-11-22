# Projeto 2 - 3D Racer

Disponível [aqui](https://isaquelc.github.io/abcg/3DRacer/)
Caso o WebAssembly não esteja funcionaddo, o vídeo do projeto está [aqui](https://drive.google.com/file/d/19ij_IkFtALSpDX7OoxUU54B7N4pa753F/view?usp=sharing)

_OBS: o jogo demora para carregar, acho que é devido ao modelo 3D que não foi otimizado e contém muitas vértices e objetos separados, o que não é ideal, e portanto o aplicativo não roda bem no WebAssembly por enquanto._

Este é o segundo projeto da matéria Computação Gráfica da UFABC. Neste projeto, precisávamos desenvolver uma aplicação 3D, e eu fiz uma versão 3D levemente modificada do meu projeto 1, chamada 3D Racer.  

Neste jogo o jogador controla um carro (mais especificamente um DeLorean), e precisa desviar de outros carros que vem na direção oposta. A pontuação é proporcional ao tempo que o jogador sobrevive sem colidir, porém o jogo não fica mais rápido com o passar do tempo. 

A estrutura do projeto é similar à estrutura do projeto 1, com as seguintes classes

### OpenGLWindow
Responsável por gerenciar a aplicação e a janela OpenGL. Também contém a função responsável por checar colisões. As funções das outras classes são chamadas por essa classe.

### Player
Responsável pelo jogador, contém as funções de carregar o modelo 3D (que eu tinha feito em 2017 num projeto pessoal), uma função de update que move o carro de um lado para o outro (com uma rotação para adicionar realismo). demtre piutras funções e variáveis necessárias. 

O jeito que movemos o modelo 3D é modficando uma variável de translação, que é utilizada na matriz de modelo

### Enemies
Muito similar à Player, com a diferença que não controlamos os inimigos: eles automaticamente vem em direção ao jogador, e quando estão atrás da camera, sua posição é mudada para um ponto aleatório no eixo z entre -100 e -200 com valores no eixo x entre -2 e 2. A cor do carro também é aleatoria. 

Nós definimos a quantidade de carros numa variável (no caso 5) e assim criamos um array de posições e de cores, que são utilizados para criar as instâncias de inimigos.

A variável de posição, além de ser usada na matriz de modelo, é utilizada para verificar a colisão com o jogador.

### Ground
Baseado no exemplo "lookat", o ground é composto por um padrão quadriculado, definido na função de inicialização, com as cores e posições armazenadas em arrays. Nós então mudamos a posição no eixo z contínuamente, sempre mandando cada _tile_ para frente da camera quando ele vai para trás da camera. 

Há um problema onde quando o jogo começa, o chão não renderiza corretamente, porém uma vez que vc perde e o jogo reinicia, ele funciona da maneira esperada.

### GameData
Esta classe é utilizada para definir se o jogo acabou ou não, e a pontuação. Nós tamém definimos o struct Vertex aqui, já que todas as outras classes usam essa.

### Camera
Responsável pela camera, que no momento é estática e olha na direção do eixo z negativo, um pouco acima da origem.

======

Além destas classes, temos alguns outros arquivos. 

#### Assets
Contém a fonte utilizada para escrever na tela, e mais dois arquivos:
 - *depth.frag*: simplismente atriui uma cor de entrada como a cor de saída para a renderização dos objetos.
 - *depth.vert*: Atualiza a intensidade dos objetos de acordo com a distância da câmera.
 - *Inconsolata-Medium.ttf*: fonte utilizada para mostrar o texto.
 - *DeLorean_DMC-12_V2.obj*: um arquivo do modelo 3D do DeLorean que eu fiz para um projeto pessoal que é utilizado como modelo dos carros.

#### Arquivos para o Web Assembly
São os arquivos *index.html*, *projeto_1.js*, *projeto_1.wasm*, responsáveis pela versão do jogo que roda no web browser.

Além desses arquivos, temos o folder *build*, que contém iniformações geradas pelo compilador na hora de gerar o .exe, e o *CMakeLists.txt*, que também é utilizada para compilar o projeto.

Isto tudo é unido pelo arquivo *main.cpp*, que inicia a janela e o jogo.