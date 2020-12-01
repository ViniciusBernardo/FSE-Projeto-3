# FSE-Projeto-3

## Configurando variáveis de ambiente

Na pasta raiz do projeto, execute o comando:
* `idf.py menuconfig`

Em seguida, mova o cursor para a opção `Configuração do Wifi Configuration  --->` e preencha os campos de SSID e password do seu roteador com acesso a internet. Caso deseje, preencha o número máximo de tentativas de reconexão com o roteador wi-fi.

Por fim, volte para o menu princial do menuconfig e mova o cursor para a opção `Configuração das chaves de API  --->` e preencha os campos com sua chave de API da ipstack.com e do openweatherdata.com.

## Executando o projeto

Conecte a esp32 no seu computador através de um cabo USB.

Na pasta raiz do projeto, execute os seguintes comandos na sequência que aparecem:
* `idf.py build`
* `idf.py -p /dev/ttyUSB0 flash monitor`. Obs: em alguns dispositivos a porta onde é possível acessar o file descriptor da esp32 pode mudar. Verifique se no seu computador a porta é '/dev/ttyUSB0' indicada no comando. Caso não seja, substitua pela porta correta de acordo com seu computador.
