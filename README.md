Solução desenvolvida para melhorar a movimentação da camera GRUA durante a transmissão de cultos da igreja.

Foi utilizando o IDE Arduino com o microcontrolador ESP32 (DOIT ESP32 DEVKIT V1) fabricado pela espressif

As direções vertical e horizontal não podem ser invertidas (fisicamente).

Os sentidos podem ser parametrizados na instalação.

A parametrização se dá posicionando o joystick em um dos quatro sentidos (NORDESTE, SUDESTE, SUDOESTE ou NOROESTE) e ligando o equipamento, mantendo a posição do joystick até que haja movimento no mecanismo (camera). Estes passos devem ser repetidos até que o joystick movimente o mecanismo (camera) corretamente.

GITHUB Espressif:
	https://github.com/espressif/arduino-esp32
	
DOIT ESP32 DEVKIT V1 (docs):
	https://olddocs.zerynth.com/r2.6.2/official/board.zerynth.doit_esp32/docs/index.html
	
Arduino IDE:
	https://www.arduino.cc/en/software
