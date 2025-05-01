![image](https://github.com/user-attachments/assets/f2a5c9b8-6208-4723-8f46-1d74be421827)
# Semáforo Inteligente com Modo Noturno e Acessibilidade

# 📋 Sumário

## Autor
## Descrição do Projeto
## Funcionalidades
## Componentes Utilizados
## Como Compilar e Executar
## Estrutura do Código
## Demonstração em Vídeo


# 👨💻 Autor
Atenilton Santos de Souza Júnior

GitHub [ateniltonjr](https://github.com/ateniltonjr)

# 🚦 Descrição do Projeto
Implementação de um sistema de semáforo inteligente com dois modos de operação utilizando FreeRTOS no RP2040 (BitDog Lab). O sistema inclui acessibilidade para pessoas com deficiência visual através de sinais sonoros.

# 💡 Funcionalidades
- Modo Normal: Ciclo completo de semáforo (verde → amarelo → vermelho)

- Modo Noturno: Luz amarela piscante

- Sinalização sonora para cada estado

- Display OLED com mensagens centralizadas

- Alternância de modos via botão físico

# 🔧 Componentes Utilizados
- Matriz de LEDs (WS2812B)

- Display OLED 128x64

- Buzzer para sinalização sonora

- Botões para controle

- Microcontrolador RP2040

# 🛠 Como Compilar e Executar
## Pré-requisitos
- Toolchain do Raspberry Pi Pico

- VS Code com extensão PlatformIO

- BitDog Lab ou placa compatível com RP2040

## Passos para compilação
Clone o repositório:

bash
git clone https://github.com/ateniltonjr/semaforo_inteligente.git

Importe o projeto com a extensão do raspberry pi pico

No CmakeLists.txt, susbtitua o endereço do FreeRTOSS-Kernel pelo local em que esteja do seu computador.

Caso não tenha baixado, faça o download do zip e extraia por meio do link:

https://github.com/FreeRTOS/FreeRTOS-Kernel.git

Substitua isso:

[set(FREERTOS_KERNEL_PATH "C:/Users/ateni/CODIGOS/TESTES/FreeRTOS-Kernel/")](#)

por isso:

[set(FREERTOS_KERNEL_PATH "C:/caminho/correto/para/FreeRTOS-Kernel")](#)

Apos isso, na extensão do raspberry pi pico, clique na opção [Clean Cmake]() para atualizar a build.

Feito isso:

Conecte a placa ao computador.

Compile e envie para a placa.


# 🎥 Demonstração em Vídeo
Vídeo de demonstração

[Clique aqui para assistir o vídeo](#)
