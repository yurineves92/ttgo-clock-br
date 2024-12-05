# Relógio Digital com Data em Português para Display TFT

Este projeto exibe a hora atual e a data formatada no padrão brasileiro em um display TFT utilizando a biblioteca **TFT_eSPI**.

## Funcionalidades

- **Exibição da data** no formato brasileiro: `DD de Mês de AAAA`.
- **Relógio digital** com atualização em tempo real.
- Tradução dos meses para português (sem acentuação).

## Pré-requisitos

- **Hardware**:
  - Display TFT com driver **ST7735** ou compatível.
  - Microcontrolador compatível com Arduino.

- **Bibliotecas**:
  - [`TFT_eSPI`](https://github.com/Bodmer/TFT_eSPI)
  - **SPI** (padrão do Arduino)

## Como Usar

1. Instale a biblioteca **TFT_eSPI** na IDE do Arduino.
2. Configure os pinos do display no arquivo `User_Setup.h` da biblioteca.
3. Copie o código para a IDE do Arduino e faça o upload para o microcontrolador.