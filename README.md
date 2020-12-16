# FSE - Trabalho Final
Trabalho Final da Disciplina de Fundamentos de Sistemas Embarcados.

Aluno: Felipe de Oliveira Hargreaves - 15/0009313

## Compilação e Execução
### Servidor Central
- Dentro da pasta `central_server` executar `pip3 install -r requirements.txt` para instalar as dependências do projeto (a maioria já deve estar instalada por padrão na Rasp, à exceção de uma lib do Flask).
- Executar com `python3 main.py`.
- O projeto roda um servidor Flask na porta 10103, onde é possível acessar a interface. Para expor a porta fora da Rasp, executar `ssh -L 10103:localhost:10103 <user>@3.tcp.ngrok.io -p 23900`, no computador onde se deseja acessar a interface. Então, acessar `localhost:10103` em qualquer navegador.
### Cliente ESP32
- Dentro da pasta `esp_client` executar `idf.py menuconfig` e adicionar as credenciais corretas de Wifi no menu correspondente.
- Realizar a build com `idf.py -p <path da esp> build flash monitor`

## Funcionamento

Na primeira execução, deve-se conectar a ESP apenas após abrir a interface do servidor central no navegador, para que a tela de cadastro do dispositivo apareça corretamente. Para dispositivos já cadastrados, a ordem de inicialização não importa. Os dados de cada dispositivo salvo no servidor central são armazenados em `central_server/data/devices.csv`. Deve-se apagar esse arquivo, bem como executar `idf.py <porta da esp> erase_flash` em todos os clientes cadastrados, caso deseje-se resetar o sistema.

Os logs de atividades no servidor central ficam armazenados em `central_server/log.txt`.


## Outros

O áudio do alarme foi retirado de [https://freesound.org/people/o_ultimo/sounds/324394/](https://freesound.org/people/o_ultimo/sounds/324394/), licenciado através da [Creative Commons 3.0](https://creativecommons.org/licenses/by/3.0/). 