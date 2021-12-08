# Projeto 3 - 3D Racer2

Disponível [aqui](https://isaquelc.github.io/abcg/3DRacer2/)

_OBS: Existem ainda alguns bugs, como spawnar fora da tela ao segurar umas das teclas para a direita ou esquerda enquanto o jogo reinicia._

Este terçeiro projeto é uma extensão do segundo projeto, e segue a mesma organização principal, as únicas mudanças são em relação à renderização dos objetos: esta versão agora implementa luz e texturas. Quatro elementos são afetados pelas mudanças implementadas:

## OpenGLWindow
A classe OpenGLWindow agora é responsável pela luz. Nós definimos os parâmetros da luz no arquivo **openglwindow.hpp**, e passamos eles para os arquivos de textura em **PaintGL**. 

## Player
O jogador sofreu algumas mudanças. Primeiramente, o arquivo .obj do carro foi modificado para ser mais simples, isto é, ter menos vértices. Isso ajuda na performance do modelo. O jogador tbm agora não é mais renderizado como uma cor sólida. Ele contém uma textura (que foi pintada no Blender) que é atribuída ao modelo por um UV unrwapping. Os detalhes do material do modelo estão no arquivo .mtl

## Ground
O chão agora não é mais composto por um quadrado definido na classe Ground e repetido diversas vezes. Agora, nós lemos um arquivo .obj, com um arquivo .mtl associado que contém as definições do material. Tbm usamos uma textura, baixada do site Textures.com. Para obtermos o tamanho correto, nós precisamos dividir o tamanho do objeto por 2.

## Enemies
Os inimigos não usam uma textura específica, pois cada inimigo tem uma cor aleatória atribuída a ele. Para isso, foi adicionado um **mappingMode** a mais além dos que já estavam implementados nos exemplo: mappingMode 4, que é utilizado para usarmos a função de renderização Blinn Phong sem textura alguma.

## Outras mudanças
Uma mudança feita no jogo em sí é que agora ele vai ficando progessivamente mais rápido com o passar do tempo, e o fundo também mudou de cor, sendo agora azul. 

Para resolver um bug na luz, é necessário calcular a viewMatrix antes de atribuirmos as variáveis de luz.