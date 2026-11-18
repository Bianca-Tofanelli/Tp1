Trabalho prático - Computação Gráfica
Bianca Andrade Tofanelli
Atividades realizadas: 

[PARTE 1 - Metodos de Varredura e Interface Grafica]
- A1 (Concluida): Implementacao da classe Polyline para representacao de
  polilinhas abertas e fechadas (poligonos regulares e arcos circulares),
  com suporte a escala, rotacao e extrusao interna.
- A2 e A3 (Concluidas): Implementacao do processo de varredura para
  geracao de espirais (HelixProxy) com ajuste interativo via ImGui de
  largura inicial, revolucoes, subdivisoes, incrementos de altura e
  largura e fechamento por tampas.
- A4 e A5 (Concluidas): Implementacao do processo de varredura
  translacional com torcao (SweepMeshProxy) com ajuste interativo via
  ImGui de comprimento, deslocamentos horizontal e vertical, escalas
  inicial e final, subdivisoes, torcao e tampas.
- A6 (Concluida): Exportacao das malhas de triangulos geradas para
  arquivos no formato Wavefront OBJ (assets/meshes/*.obj).

[PARTE 2 - Modelo de Iluminacao PBR e Tracado de Raios]
- A1 e A2 (Concluidas): Implementacao da classe MyMaterial (derivada de
  cg::Material) adotando o modelo fisicamente baseado (PBR) como modelo
  unico de material, com cor difusa (Od), cor especular (Os), rugosidade
  (r) e metalicidade (m). Implementacao da BRDF de Cook-Torrance
  (distribuicao normal Trowbridge-Reitz, atenuacao geometrica Smith-Schlick
  e aproximacao de Fresnel-Schlick) e do calculo dinamico ponto a ponto
  da refletancia especular indireta (Or) pela aproximacao de Karis.
- A3 (Concluida): Atualizacao do metodo RayTracer::shade para tonalizacao
  direta PBR e tracado de raios secundarios ponderados dinamicamente por Or.
- A4 (Concluida): Extensao do Inspector (MainWindow) via Dear ImGui para
  criacao (Create -> Material) e edicao em tempo real de todas as
  propriedades fisicas de MyMaterial, com sincronizacao imediata para a
  pre-visualizacao em OpenGL.
- A5 [Bonus] (Em implementacao): Persistencia de materiais MyMaterial
  (roughness e metalness) nos arquivos de cena (.scn) via SceneWriter e
  SceneReader.
- A6 e A7 (Concluidas): Cenas de demonstracao e video explicativo.

------------------------------------------------------------------------
2. INSTRUCOES DE COMPILACAO E EXECUCAO
------------------------------------------------------------------------

Pre-requisitos:
- Biblioteca Ds compilada previamente no diretorio ../../cg (conforme
  estrutura original do repositorio https://github.com/paulo-pagliosa/Ds).
- Compilador com suporte a C++20 (Visual Studio 2022/2026 ou g++),
  CMake (3.16+) e OpenGL.

Via CMake / Visual Studio (Windows):
1. Certifique-se de que a pasta 'assets' esteja no diretorio raiz deste
   projeto (junto ao CMakeLists.txt).
2. Abra um terminal no diretorio do projeto e execute:
      cmake -S . -B build
      cmake --build build --config Release
3. Execute o binario gerado (cgdemo.exe) a partir do diretorio raiz do
   projeto para que os caminhos relativos de 'assets/' e 'scenes/' sejam
   carregados corretamente:
      .\cgdemo.exe

Via g++ / CMake (Linux):
1. No diretorio do projeto, execute:
      cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
      cmake --build build
2. Execute a aplicacao a partir do diretorio raiz do projeto:
      ./cgdemo

------------------------------------------------------------------------
3. VIDEO DE DEMONSTRACAO (ATIVIDADE A7)
------------------------------------------------------------------------
- Arquivo incluido no pacote compactado ou disponivel em:
  <INSIRA_AQUI_A_URL_DO_VIDEO_SE_HOSPEDADO_ONLINE>