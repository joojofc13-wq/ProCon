# ProCon

Universal USB controller plugin for jailbroken PS4 consoles using GoldHEN.

-----------------------------------------------------------------------------

ProCon started as a small personal project designed to use a Nintendo Switch Pro Controller directly via USB on a jailbroken PS4.

Starting with **ProCon 0.2**, the project has been expanded into an experimental universal controller plugin, adding support for multiple USB controllers and generic HID gamepads.

The plugin reads controller input directly over USB and translates it into input that PS4 games can use, sharing the same player slot as the connected DualShock 4.

## What's New in ProCon 0.2

ProCon 0.2 expands the original Nintendo Switch Pro Controller implementation with experimental support for:

- Nintendo Switch Pro Controller
- DualSense (PlayStation 5)
- Xbox One controllers
- Xbox Series controllers
- Generic USB HID gamepads
- Other compatible HID controllers

### Important

Support for controllers other than the Nintendo Switch Pro Controller is currently **experimental and unverified**.

I do not own all of these controllers and therefore cannot personally test every implementation.

The required support has been added to the plugin, but actual compatibility may vary depending on the controller model, USB implementation, firmware and game.

If you own one of these controllers, feel free to test it and report whether it works, partially works, or causes any issues.

## Features

- Universal USB controller support
- Nintendo Switch Pro Controller support
- Experimental DualSense support
- Experimental Xbox One / Xbox Series controller support
- Experimental generic HID gamepad support
- Controllers connect directly to the PS4 via USB
- No PC or external controller adapter required
- Works directly inside PS4 games through a GoldHEN plugin
- Controller input is translated and combined with the DualShock 4 input for the same player

## Requirements

- Jailbroken PS4
- GoldHEN
- Compatible USB controller
- USB cable
- DualShock 4

## How to Use

1. Make sure your PS4 is jailbroken and running GoldHEN.
2. Enable plugins in the GoldHEN settings.
3. Download and install the ProCon PKG.
4. Open the ProCon application.
5. Press **OK** and wait for the **"Plugin installed successfully."** message.
6. The application will close/crash after installation.
7. Connect your controller to the PS4 via USB.
8. Launch a game.
9. Keep the DualShock 4 powered on and connected while using ProCon.

## Compatibility

ProCon was originally developed and tested using:

- Nintendo Switch Pro Controller
- PS4 firmware **13.50**
- GoldHEN

Starting with version 0.2, support for additional controllers has been implemented but has **not been fully tested on real hardware**.

Compatibility reports are welcome.

If you test a controller, please report:

- Controller name/model
- PS4 firmware
- GoldHEN version
- Whether buttons work correctly
- Whether analog sticks and triggers work correctly
- Any crashes or unexpected behavior

These reports will help determine which controllers are actually compatible with ProCon 0.2.

## Limitations

ProCon 0.2 is still experimental.

Depending on the controller, the following features may be unavailable or incomplete:

- Bluetooth
- Vibration / rumble
- Gyroscope / motion controls
- Touchpad functionality
- PS/Home/Guide button functionality
- Controller-specific features

The DualShock 4 must currently remain connected while using another controller.

Generic HID compatibility is not guaranteed. HID gamepads can use different report formats, mappings and implementations even when they identify as standard USB HID devices.

## Disclaimer

ProCon is an experimental homebrew project provided as-is.

Version 0.2 significantly expands the scope of the original project, but many of the newly supported controller types have not been tested by me because I do not have access to the required hardware.

Support being present in the plugin does **not** guarantee that every controller or model will work correctly.

Testing is currently left to users who own compatible hardware.

Bug reports, compatibility reports and technical feedback are welcome.

There is no guarantee of future updates, additional features, or compatibility with every PS4 game, firmware, GoldHEN version or USB controller.

ill post source later

------------------------------------------------------------------------------

Plugin universal para controles USB em consoles PS4 com jailbreak utilizando GoldHEN.

-----------------------------------------------------------------------------

O ProCon começou como um pequeno projeto pessoal criado para permitir o uso do Pro Controller do Nintendo Switch diretamente via USB em um PS4 com jailbreak.

A partir do **ProCon 0.2**, o projeto foi expandido para se tornar um plugin experimental de controles universais, adicionando suporte a diferentes controles USB e gamepads HID genéricos.

O plugin lê diretamente os comandos do controle através da USB e os traduz para comandos que os jogos de PS4 conseguem utilizar, compartilhando o mesmo jogador do DualShock 4 conectado.

## Novidades do ProCon 0.2

O ProCon 0.2 expande a implementação original do Nintendo Switch Pro Controller com suporte experimental para:

- Nintendo Switch Pro Controller
- DualSense (PlayStation 5)
- Controles Xbox One
- Controles Xbox Series
- Gamepads USB HID genéricos
- Outros controles HID compatíveis

### Importante

O suporte aos controles além do Pro Controller é atualmente **experimental e não verificado**.

Eu não tenho todos esses controles e não consigo testar pessoalmente todas as implementações.

O suporte necessário foi adicionado ao plugin, porém a compatibilidade real pode variar dependendo do modelo do controle, implementação USB, firmware e jogo.

Caso você possua um desses controles, fique à vontade para testá-lo e informar se funciona corretamente, parcialmente ou se apresenta algum problema.

## Recursos

- Suporte universal a controles USB
- Suporte ao Nintendo Switch Pro Controller
- Suporte experimental ao DualSense
- Suporte experimental aos controles Xbox One / Xbox Series
- Suporte experimental a gamepads HID genéricos
- Controles conectados diretamente ao PS4 via USB
- Não requer PC ou adaptador externo para controles
- Funciona diretamente dentro dos jogos através de um plugin do GoldHEN
- Os comandos são traduzidos e combinados com os comandos do DualShock 4 para o mesmo jogador

## Requisitos

- PS4 com jailbreak
- GoldHEN
- Controle USB compatível
- Cabo USB
- DualShock 4

## Como usar

1. Certifique-se de que seu PS4 possui jailbreak e está executando o GoldHEN.
2. Ative os plugins nas configurações do GoldHEN.
3. Baixe e instale o PKG do ProCon.
4. Abra o aplicativo ProCon.
5. Pressione **OK** e aguarde a mensagem **"Plugin installed successfully."**
6. O aplicativo irá fechar/crashar após a instalação.
7. Conecte seu controle ao PS4 através da USB.
8. Abra um jogo.
9. Mantenha o DualShock 4 ligado e conectado enquanto estiver utilizando o ProCon.

## Compatibilidade

O ProCon foi originalmente desenvolvido e testado utilizando:

- Nintendo Switch Pro Controller
- PS4 firmware **13.50**
- GoldHEN

A partir da versão 0.2, o suporte a controles adicionais foi implementado, porém **ainda não foi completamente testado em hardware real**.

Relatórios de compatibilidade são bem-vindos.

Caso teste algum controle, informe:

- Nome/modelo do controle
- Firmware do PS4
- Versão do GoldHEN
- Se os botões funcionam corretamente
- Se os analógicos e gatilhos funcionam corretamente
- Crashes ou outros comportamentos inesperados

Esses relatos ajudarão a determinar quais controles realmente são compatíveis com o ProCon 0.2.

## Limitações

O ProCon 0.2 ainda é experimental.

Dependendo do controle, os seguintes recursos podem estar indisponíveis ou incompletos:

- Bluetooth
- Vibração
- Giroscópio / controles de movimento
- Funcionalidades do touchpad
- Botão PS/Home/Guide
- Recursos específicos de determinados controles

Atualmente, o DualShock 4 deve permanecer conectado enquanto outro controle estiver sendo utilizado.

A compatibilidade com dispositivos HID genéricos não é garantida. Diferentes gamepads HID podem utilizar formatos de reports, mapeamentos e implementações diferentes mesmo quando são identificados como dispositivos USB HID padrão.

## Aviso

O ProCon é um projeto homebrew experimental fornecido no estado em que se encontra.

A versão 0.2 expande significativamente o escopo do projeto original, porém vários dos novos tipos de controle suportados não foram testados por mim por não possuir o hardware necessário.

A presença do suporte no plugin **não garante que todos os controles ou modelos funcionarão corretamente**.

Os testes desses controles ficam atualmente a critério dos usuários que possuem hardware compatível.

Relatórios de bugs, compatibilidade e feedback técnico são bem-vindos.

Não há garantia de futuras atualizações, recursos adicionais ou compatibilidade com todos os jogos de PS4, versões de firmware, versões do GoldHEN ou controles USB.
