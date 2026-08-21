# DeskInfo C++

Overlay nativo e leve para exibir informações do Windows em tempo real sobre a
área de trabalho. Esta versão substitui Python, PyQt6, `psutil` e `pywin32` por
C++ e pelas APIs do próprio Windows.

## O que foi preservado

- atualização de CPU, memória, discos e rede a cada segundo;
- janela transparente, sem borda, sem foco e que ignora cliques;
- posicionamento no canto superior direito da tela principal;
- cor configurável para o título e os separadores;
- proteção contra duas instâncias usando mutex do Windows;
- compatibilidade com o formato existente do `config.cfg`;
- criação automática de um `config.cfg` padrão quando ele não existe.

## Executar

Baixe o pacote mais recente na página de
[Releases](https://github.com/marcioscarv/desk-info/releases), extraia os arquivos
em uma pasta e execute `DeskInfo.exe`.

Ao compilar localmente, o executável fica em:

```text
dist-cpp\DeskInfo.exe
```

Mantenha `config.cfg` ao lado do executável. O programa não requer Python, Qt
ou instalação de pacotes. Para encerrar, finalize `DeskInfo.exe` pelo Gerenciador
de Tarefas. Para iniciar junto com o Windows, crie um atalho do executável na
pasta `shell:startup`.

## Personalizar

Edite `config.cfg` com um editor de texto. A opção `Main_color` define a cor
do nome da máquina e dos separadores. São aceitos os formatos hexadecimais
`#RGB`, `#RGBA`, `#RRGGBB` e `#RRGGBBAA`:

```text
Main_color: #1E90FF
```

O azul `#1E90FF` é usado quando a opção não existe ou possui um valor
inválido. O canal alfa dos formatos `#RGBA` e `#RRGGBBAA` é aceito, mas a
renderização atual utiliza somente os componentes RGB. Os marcadores suportados
são:

| Marcador | Conteúdo |
|---|---|
| `{machine_domain}` | domínio da máquina ou `WORKGROUP` |
| `{ip_address}` | adaptadores IPv4 ativos |
| `{user_name}` | usuário conectado |
| `{logon_domain}` | usuário e domínio |
| `{os_version}` | versão e build do Windows |
| `{system_type}` | arquitetura do sistema |
| `{cpu_usage}` | utilização da CPU |
| `{mem_used_gb}` | memória utilizada |
| `{mem_total_gb}` | memória física total |
| `{disk_c_usage}` | utilização do disco C: |
| `{disk_info}` | espaço livre e total dos discos |
| `{SEPARATOR}` | linha separadora |

Marcadores desconhecidos permanecem visíveis no texto, como na versão Python.
O arquivo é relido a cada atualização, portanto alterações aparecem sem reiniciar.

## Compilar

Requisitos:

- Windows 10 ou 11, 64 bits;
- Visual Studio Build Tools com **Desenvolvimento para Desktop com C++**;
- CMake.

Execute `build-release.bat`. O script compila em modo Release e monta a pasta
`dist-cpp`. Também é possível usar CMake manualmente em um terminal de
desenvolvimento do Visual Studio:

```bat
cmake -S . -B build-native -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-native
cmake --install build-native --prefix dist-cpp
```

O runtime C/C++ é incorporado ao executável (`/MT`), deixando a distribuição
autônoma e sem DLLs adicionais do projeto.

## Estrutura da versão nativa

- `src/main.cpp`: ponto de entrada do programa;
- `src/app.cpp` e `src/app.h`: ciclo de vida, janela e desenho do overlay;
- `src/system_info.cpp` e `src/system_info.h`: coleta de dados do Windows;
- `src/template_renderer.cpp` e `src/template_renderer.h`: leitura do
  `config.cfg` e substituição dos marcadores;
- `src/DeskInfo.rc`: ícone do executável;
- `CMakeLists.txt`: configuração de compilação;
- `build-release.bat`: compilação automatizada;
- `config.cfg`: modelo de conteúdo exibido.
