# Changelog

## 2.3.0 - 2026-10-11

- distribuição Windows x64 recompilada a partir do código C++ atual;
- identificação da versão 2.3.0 nas propriedades do executável;
- manutenção da opção `Main_color` no `config.cfg` para escolher a cor do título e dos separadores.

## 2.2.0 - 2026-08-21

- reescrita completa do DeskInfo em C++20 e APIs nativas do Windows;
- executável independente de Python, Qt e pacotes externos;
- código separado em módulos de aplicação, sistema e configuração;
- atualização em tempo real de CPU, memória, discos e adaptadores IPv4;
- personalização da cor do nome da máquina e dos separadores;
- suporte a `#RGB`, `#RGBA`, `#RRGGBB` e `#RRGGBBAA` em `Main_color`;
- compilação automatizada em modo Release com CMake e Visual Studio Build Tools.
