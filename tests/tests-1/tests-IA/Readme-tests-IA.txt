Os testes I-A incluem os testes parte IA do projecto.

Pode executar cada um dos 2 testes descritos a seguir individualmente, ou executar 
./tests-logrw-script 
que corre todos os testes executando estes dois programas com parameterizações pré-definidas.
Todos os testes apresentados são feitos para array de registos de tamanho 3 registos, 
e cada array de blocos com 12 blocos

===============================================================================================

Descrição de test-logrw-ops_on_registers
Programa que lança tarefas (todas lêm ou todas escrevem).

Primeiro argumento indica se as tarefas a serem lançadas fazem leituras ou escritas.
Assim, ./test-logrw-ops_on_registers w lança tarefas que escrevem,
./test-logrw-ops_on_registers r lança tarefas que lêm. O valor de defeiro é r.

Segundo argumento indica se as tarefas a serem lançadas lêm ou escrevem no mesmo registo (parametro =) 
ou em registos diferentes qualquer outro caractér, tal como d. Valor por defeito é =.

Terceiro argumento indica o nº de ticks em multiplos de 10microseg para o io_delay. Assim, para obter 1seg,
deve-se usar 100000. O valor por defeito é 1 (10 microsegundos).
 
Quarto argumento indica o número de tarefas a serem lançadas. O valor por defeito é 2.

Quinto argumento indica o número de iterações do ciclo de escritas ou leituras. O valor por defeito é NUMBLK.

O sexto argumento só é utilizado se o segundo argumento fôr =
ou seja, se fôr para escrever/ler no mesmo registo, então este argumento indica qual o registo. 
Por defeito é o registo 0.


===============================================================================================

Descrição de test-logrw-wclean
Programa que lança tarefas n tarefas que só escrevem em ciclo de n iterações, escrevendo nos 3 registos 
em cada iteração, e m tarefas  que lêm e que entram em ciclo e faz o dump de um registo sempre que 
detecta que há uma nova versão deste.

Primeiro argumento indica o número de tarefas que lêm a serem lançadas. O valor por defeito é 1.

Segundo argumento indica o número de tarefas que escrevem a serem lançadas. O valor por defeito é 1.

Terceiro argumento indica o nº de ticks em multiplos de 10microseg para o io_delay. Assim, para obter 1seg,
deve-se usar 100000. O valor por defeito é 0.

Quarto argumento indica o numero de iterações do ciclo de escritas. O valor por defeito é 4.


===============================================================================================
Descrição dos testes em tests-logrw-script
Terminologia:
Array:0 (o array onde no inicio do programa se começa a escrever)
Array:1 aquele que no arranque do programa está inactivo.
Todos os indices são dados com referência ao 0. Assim, a ultima posição do array de registos é 
a posição 2 (este contém 3 registos).
NextWrBlk: o próximo bloco livre (que pode tomar valores entre 0 e 11 para cada array)
REG_IDX: [0i]=x indica a posição x do array onde se encontra o registo i.

--------

TESTE 1 - teste com duas escritas por uma tarefa no registo 0
Executar ./test-logrw-ops_on_registers w = 1 1 2

Para passar o teste 1 com SUCESSO, o resultado deverá ser 
Array:0 NextWrBlk:005 ThreadsWaiting:000
REG_IDX: [00]=004 [01]=001 [02]=002 
00:01:00:00:00:00:00:00:	........
<registo não escrito, qualquer conteúdo inicial é correcto>
<registo não escrito, qualquer conteúdo inicial é correcto>

----------

TESTE 2 - teste com leituras do mesmo registo, o registo 0, feito por 4 tarefas, com atrasos de 
1 seg em cada leitura"
./test-logrw-ops_on_registers r = 100000 4

Para passar teste 2 com SUCESSO, o tempo total e o tempo despendido por tarefa deverá ser por 
volta dos 12 segs"

----------

TESTE 3 - teste com escritas do mesmo registo, o registo 0, feito por 4 tarefas, com atrasos de
1 seg por cada escrita"
./test-logrw-ops_on_registers w = 100000 4

Para passar teste 3 com SUCESSO, o tempo total e por tarefa deverá ser por volta dos 12-25 segs"

-----------

TESTE 4 - teste que lança uma tarefa que efectua sem atrasos um ciclo de 12 iterações, em que em 
cada uma escreve nos três registos, no block[0] o valor do registo, e no block[1] o valor da iteração i 
(de 0 a 11, sendo 11 o hexadecimal 0B).
Portanto, sem feitas um total de 36 escritas.
./test-logrw-wclean 0 1 0 12

Para passar teste 4 com SUCESSO, o resultado deverá ser
NextWrBlk:numa posição de 1 a 12
REG_IDX: [0i]=x, x=1..12 , x é a i-ésima posição no array activo (a 1ª corresponde ao 1ª elemento do array, o 0)
Conteúdo dos blocos dos registos:
00:0B:00:00:00:00:00:00: ........
01:0B:00:00:00:00:00:00: ........
02:0B:00:00:00:00:00:00: ........"

-----------

TESTE 5 - teste que lança uma tarefa de escrita, a qual efectua sem atrasos um ciclo de 4 iterações, em que em 
cada uma escreve nos três registos. Portanto, um total de 12 escritas. Lança também uma tarefa de leitura, que entra em ciclo e faz o dump de um 
registo sempre que detecta que há uma nova versão deste.
./test-logrw-wclean 1 1 10000 4

Para passar teste 5 com SUCESSO
Array activo :1
NextWrBlk: uma posição entre a 4ª e a 7ª do array1.
REG_IDX: [0i]=x, x=1..6 , é uma das 6 primeiras posições do 2ªarray (podem estar trocados. e.g. 0,4 e 2).
Conteúdo dos blocos dos registos:
00:03:00:00:00:00:00:00: ........
01:03:00:00:00:00:00:00: ........
02:03:00:00:00:00:00:00: ........"

------------

TESTE 6 - teste que lança 4 tarefas de escrita, a qual efectua sem atrasos um ciclo de 2 iterações, em que em 
cada tarefa em cada iteração escreve nos três registos. Temos assim um total de (4 x 2 x 3 = 24 escritas). 
Lança também uma tarefa de leitura, que entra em ciclo e faz o dump de um registo sempre que detecta que há 
uma nova versão deste.
./test-logrw-wclean 1 4 10000 2

Para passar teste 6 com SUCESSO, o resultado deverá ser
Array activo :0
NextWrBlk: uma posição entre a 4ª e a 10ª do array0.
REG_IDX: [0i]=x, x=1..9 , ou seja, numa das 9 primeiras posições do array 1
00:01:00:00:00:00:00:00: ........
01:01:00:00:00:00:00:00: ........
02:01:00:00:00:00:00:00: ........"
