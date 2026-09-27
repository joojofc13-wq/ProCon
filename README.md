# ProCon

Universal USB input plugin for jailbroken PS4 consoles using GoldHEN.
to report errors @dirtyocean on discord

-----------------------------------------------------------------------------

ProCon is an experimental homebrew plugin that allows you to use different USB input devices directly on a jailbroken PS4.

It supports multiple controllers, generic HID gamepads, Keyboard and mouse by reading their input directly over USB and translating it into controller input that PS4 games can use.

Input from connected devices is combined with the controller data received by the game, using the same player slot as the connected DualShock 4.

## Mouse & Keyboard Mapping

You may need to disconnect and reconnect your keyboard and mouse while in-game if either one isn't working.

| Mouse / Keyboard | PS4 Input |
|------------------|-----------|
| **WASD** | Left Analog Stick |
| **Mouse Movement** | Right Analog Stick |
| **Left Click** | R2 |
| **Right Click** | L2 |
| **Space** | X |
| **E** | Square |
| **Q** | Triangle |
| **Ctrl** | Circle |
| **Enter** | Options |

## Supported Devices

ProCon includes support for:

- Nintendo Switch Pro Controller
- DualSense (PlayStation 5)
- Xbox One controllers
- Xbox Series controllers
- Generic USB HID gamepads
- USB mouse
- USB keyboards
- Other compatible HID devices

### Important

ProCon is an **experimental project**.

Not every supported device has been tested on real hardware, as I do not own all of the controllers and devices listed above.

Support has been implemented for these devices, but actual compatibility may vary depending on the device model, USB/HID implementation, PS4 firmware and game.

If you own one of these devices, feel free to test it and report whether it works, partially works, or causes any issues.

## Features

- Multiple USB controller support
- Generic USB HID gamepad support
- USB mouse support
- USB keyboard support
- Mouse movement translated into right analog stick input
- Keyboard input translated into controller input
- Devices connect directly to the PS4 via USB
- No PC or external controller adapter required
- Works directly inside PS4 games through a GoldHEN plugin
- Input is combined with the DualShock 4 input for the same player

## Requirements

- Jailbroken PS4
- GoldHEN
- Compatible USB input device
- USB cable when required
- DualShock 4

## How to Use

1. Make sure your PS4 is jailbroken and running GoldHEN.
2. Enable plugins in the GoldHEN settings.
3. Download and install the ProCon PKG.
4. Open the ProCon application.
5. Press **OK** and wait for the **"Plugin installed successfully."** message.
6. Close the application after install.
7. Connect your controller, mouse or keyboard to the PS4 via USB.
8. Launch a game.
9. Keep the DualShock 4 powered on and connected while using ProCon.

## Compatibility

ProCon has been developed and tested on:

- PS4 firmware **13.50**
- GoldHEN

Compatibility with other firmware versions has not been fully verified.

Device and game compatibility may vary.

Compatibility reports are welcome.

If you test a device, please report:

- Device name/model
- PS4 firmware
- GoldHEN version
- Whether buttons/keys work correctly
- Whether analog sticks, triggers or mouse movement work correctly
- Any crashes or unexpected behavior

These reports help determine which devices are actually compatible with ProCon.

## Mouse and Keyboard

Keyboard keys are mapped to controller buttons and movement inputs.

This is **controller emulation**, not native mouse and keyboard input. Games still receive controller-style input, so mouse behavior may differ from native mouse input depending on the game's deadzones, sensitivity and analog stick processing.

## Limitations

the following features are unavailable:

- Bluetooth
- Vibration / rumble
- Gyroscope / motion controls
- Touchpad functionality
- PS/Home/Guide button functionality
- Controller-specific features
- Advanced mouse buttons
- Special keyboard keys

If you need any of them, use your DS4 to perform those actions.

The DualShock 4 must currently remain connected while using another input device.

Generic HID compatibility is not guaranteed. HID devices can use different report formats, mappings and implementations even when they identify as standard USB HID devices.

## Disclaimer

ProCon is an experimental free homebrew project provided as-is.

Some supported device types have not been tested by me because I do not have access to all of them.

Support being present in ProCon **does not guarantee that every controller, mouse, keyboard or device model will work correctly**.

Testing is currently left to users who own compatible hardware.

There is no guarantee of future updates, additional features, or compatibility with every PS4 game, firmware, GoldHEN version or USB device.

------------------------------------------------------------------------------

# ProCon

Plugin universal para dispositivos de entrada USB pra PS4 com jailbreak utilizando GoldHEN. Para reportar erros chame @dirtyocean no discord.

-----------------------------------------------------------------------------

O ProCon é um plugin homebrew experimental que permite utilizar diferentes dispositivos de entrada USB diretamente em um PS4 com jailbreak.

Ele oferece suporte a diferentes controles, gamepads HID genéricos, mouses e teclados, lendo seus comandos diretamente através da USB e traduzindo-os para comandos de controle que os jogos de PS4 conseguem utilizar.

Os comandos dos dispositivos conectados são combinados com os dados do controle recebidos pelo jogo, utilizando o mesmo jogador do DualShock 4 conectado.

**## Mouse & Keyboard Mapping

Talvez seja necessário desconectar e reconectar o teclado e o mouse enquanto estiver dentro do jogo caso um dos dois não funcione.

| Mouse / Teclado | PS4 Input |
|------------------|-----------|
| **WASD** | Left Analog Stick |
| **Mouse Movement** | Right Analog Stick |
| **Left Click** | R2 |
| **Right Click** | L2 |
| **Space** | X |
| **E** | Square |
| **Q** | Triangle |
| **Ctrl** | Circle |
| **Enter** | Options |**

## Dispositivos suportados

O ProCon inclui suporte para:

- Nintendo Switch Pro Controller
- DualSense (PlayStation 5)
- Controles Xbox One
- Controles Xbox Series
- Gamepads USB HID genéricos
- Mouses USB
- Teclados USB
- Outros dispositivos HID compatíveis

### Importante

O ProCon é um **projeto experimental**.

Nem todos os dispositivos suportados foram testados em hardware real, pois não possuo todos os controles e dispositivos listados acima.

O suporte foi implementado para esses dispositivos, porém a compatibilidade real pode variar dependendo do modelo, implementação USB/HID, firmware do PS4 e jogo.

Caso você possua um desses dispositivos, fique à vontade para testá-lo e informar se funciona corretamente, parcialmente ou se apresenta algum problema.

## Recursos

- Suporte a diferentes controles USB
- Suporte a gamepads USB HID genéricos
- Suporte a mouse USB
- Suporte a teclado USB
- Não requer PC ou adaptador externo
- Funciona diretamente dentro dos jogos através de um plugin do GoldHEN

## Requisitos

- PS4 com jailbreak
- GoldHEN
- Dispositivo de entrada USB compatível
- DualShock 4

## Como usar

1. Certifique-se de que seu PS4 possui jailbreak e está executando o GoldHEN.
2. Ative os plugins nas configurações do GoldHEN.
3. Baixe e instale o PKG do ProCon.
4. Abra o aplicativo ProCon.
5. Pressione **OK** e aguarde a mensagem **"Plugin installed successfully."**
6. Feche o aplicativo após a instalação.
7. Conecte seu controle, mouse ou teclado ao PS4 através da USB.
8. Abra um jogo.
9. Mantenha o DualShock 4 ligado e conectado enquanto estiver utilizando o ProCon.

## Compatibilidade

O ProCon foi desenvolvido e testado em:

- PS4 firmware **13.50**
- GoldHEN

A compatibilidade com outras versões de firmware ainda não foi completamente verificada.

A compatibilidade pode variar dependendo do dispositivo e do jogo.

Relatórios de compatibilidade são bem-vindos.

Caso teste algum dispositivo, informe:

- Nome/modelo do dispositivo
- Firmware do PS4
- Versão do GoldHEN
- Se os botões/teclas funcionam corretamente
- Se os analógicos, gatilhos ou movimento do mouse funcionam corretamente
- Crashes ou outros comportamentos inesperados

Esses relatos ajudam a determinar quais dispositivos são realmente compatíveis com o ProCon.

## Mouse e teclado

As teclas do teclado são mapeadas para botões e movimentos do controle.

Isso funciona através de **emulação de controle**, e não como suporte nativo a mouse e teclado. Os jogos continuam recebendo comandos equivalentes aos de um controle, portanto o comportamento do mouse pode variar dependendo da deadzone, sensibilidade e processamento dos analógicos de cada jogo.

## Limitações

os seguintes recursos podem estam indisponíveis:

- Bluetooth
- Vibração
- Giroscópio / controles de movimento
- Funcionalidades do touchpad
- Botão PS/Home/Guide
- Recursos específicos de determinados controles
- Botões adicionais de alguns mouses
- Teclas especiais de alguns teclados

Se precisar de algum deles, faca os comandos no seu DS4.

Atualmente, o DualShock 4 deve permanecer conectado enquanto outro dispositivo de entrada estiver sendo utilizado.

A compatibilidade com dispositivos HID genéricos não é garantida. Dispositivos HID podem utilizar diferentes formatos de reports, mapeamentos e implementações mesmo quando são identificados como dispositivos USB HID padrão.

## Aviso

O ProCon é um projeto homebrew experimental gratuito fornecido no estado em que se encontra.

Alguns tipos de dispositivos suportados não foram testados por mim por não possuir todo o hardware necessário.

A presença do suporte no ProCon **não garante que todos os controles, mouses, teclados ou modelos de dispositivos funcionarão corretamente**.

Os testes ficam atualmente a critério dos usuários que possuem hardware compatível.

Relatórios de bugs, compatibilidade e feedback técnico são bem-vindos.

Não há garantia de futuras atualizações, recursos adicionais ou compatibilidade com todos os jogos de PS4, versões de firmware, versões do GoldHEN ou dispositivos USB.
