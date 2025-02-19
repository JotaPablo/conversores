# Projeto: Controle de LEDs, Joystick e Display OLED com RP2040 - BitDogLab

Projeto desenvolvido para demonstrar a integração de múltiplos periféricos usando o microcontrolador RP2040 na placa BitDogLab. Este projeto explora o uso de ADC, PWM, I2C e interrupções para criar um sistema interativo que controla LEDs RGB, exibe gráficos em um display OLED e responde a entradas de um joystick e botões.

## 🎥 Vídeo Demonstrativo
[![Assista ao vídeo no YouTube](https://img.youtube.com/vi/vnDr6rufKY8/hqdefault.jpg)](https://youtu.be/vnDr6rufKY8)

## Arquitetura do Projeto

- **ADC:** Lê os valores analógicos do joystick e aplica um *deadzone* para estabilizar a leitura.
- **PWM:** Configurado para controlar a intensidade dos LEDs Azul e Vermelho conforme a posição do joystick.
- **Interrupções e Debouncing:** Implementadas para todos os botões, garantindo resposta rápida e evitando leituras erráticas.
- **Display OLED:** Atualizado periodicamente para exibir:
  - Um quadrado móvel que representa a posição do joystick.
  - Diferentes estilos de borda para fornecer feedback visual sobre as mudanças de estado.
- **Comunicação I2C:** Utilizada para a interface com o display OLED.

## Funcionalidades

- **Leitura Analógica via ADC:**
  - Captura os valores analógicos dos eixos do joystick (GPIO 26 e 27).
  - Aplica uma zona morta (*deadzone*) para evitar variações indesejadas quando o joystick está centralizado.
    
> **Observação:**  
> No projeto, utilizei o eixo X como **GPIO 27 (canal 1)** e o eixo Y como **GPIO 26 (canal 0)**, conforme a configuração padrão para o joystick.  
> Essa configuração respeita os eixos mencionados no [repositório do Bit Dog Lab](https://github.com/BitDogLab/BitDogLab-C/tree/main/joystick) e também está documentada no [arquivo da placa Bit Dog Lab](https://docs.google.com/document/d/13-68OqiU7ISE8U2KPRUXT2ISeBl3WPhXjGDFH52eWlU/edit?tab=t.0), disponível no [repositório principal do Bit Dog Lab](https://github.com/BitDogLab/BitDogLab?tab=readme-ov-file).


- **Controle de LEDs RGB via PWM:**
  - **LED Vermelho:** Intensidade controlada pelo valor do eixo X do joystick.
  - **LED Azul:** Intensidade controlada pelo valor do eixo Y do joystick.
  - **LED Verde:** Alterna estado (ligado/desligado) ao pressionar o botão do joystick.
  - Utiliza PWM nos LEDS Azul e vERMELHO para permitir variações suaves na intensidade luminosa.

- **Display OLED SSD1306 via I2C:**
  - Exibe um quadrado móvel de 8x8 pixels cuja posição é mapeada a partir dos valores lidos do joystick.
  - Alterna entre diferentes estilos de borda para indicar mudanças de estado (acionadas pelo Botão A).

- **Controle por Botões com Interrupções e Debouncing:**
  - **Botão A (GPIO 5):** Alterna o estado do PWM dos LEDs RGB e modifica o estilo da borda no display.
  - **Botão B (GPIO 6):** Exibe uma mensagem de "modo gravação" no display e entra em modo bootsell.
  - **Botão do Joystick (GPIO 22):** Alterna o estado do LED verde.
  - Todas as entradas dos botões são gerenciadas por interrupções, com debouncing implementado para evitar múltiplos acionamentos.

## Componentes Utilizados

- **Placa BitDogLab com RP2040**
- **LEDs RGB:**
  - Vermelho: GPIO 13
  - Verde: GPIO 11
  - Azul: GPIO 12
- **Botões:**
  - Botão A: GPIO 5
  - Botão B: GPIO 6
  - Botão do Joystick: GPIO 22
- **Joystick:**
  - Eixo X: Conectado ao ADC do pino GPIO 26
  - Eixo Y: Conectado ao ADC do pino GPIO 27
- **Display OLED SSD1306:** Conectado via I2C (GPIO 14 e 15)
- **Periféricos Adicionais:**
  - PWM para controle de intensidade dos LEDs
  - ADC para leitura dos valores do joystick

## ⚙️ Instalação e Uso

1. **Pré-requisitos**
   - Clonar o repositório:
     ```bash
     git clone https://github.com/JotaPablo/conversores.git
     cd conversores
     ```
   - Instalar o **Visual Studio Code** com as seguintes extensões:
     - **Raspberry Pi Pico SDK**
     - **Compilador ARM GCC**

2. **Compilação**
   - Compile o projeto no terminal:
     ```bash
     mkdir build
     cd build
     cmake ..
     make
     ```
   - Ou utilize a extensão da Raspberry Pi Pico no VS Code.

3. **Execução**
   - **Na placa física:** 
     - Conecte a placa ao computador em modo **BOOTSEL**.
     - Copie o arquivo `.uf2` gerado na pasta `build` para o dispositivo identificado como `RPI-RP2`, ou envie através da extensão da Raspberry Pi Pico no VS Code.


