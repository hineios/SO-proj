Os testes I-A incluem os testes parte IA do projecto.

Pode executar cada um dos 2 testes descritos a seguir individualmente, ou executar 
./tests-logrw-script 
que corre todos os testes executando estes dois programas com parameteriza��es pr�-definidas.
Todos os testes apresentados s�o feitos para array de registos de tamanho 3 registos, 
e cada array de blocos com 12 blocos

===============================================================================================

Descri��o de test-logrw-ops_on_registers
Programa que lan�a tarefas (todas l�m ou todas escrevem).

Primeiro argumento indica se as tarefas a serem lan�adas fazem leituras ou escritas.
Assim, ./test-logrw-ops_on_registers w lan�a tarefas que escrevem,
./test-logrw-ops_on_registers r lan�a tarefas que l�m. O valor de defeiro � r.

Segundo argumento indica se as tarefas a serem lan�adas l�m ou escrevem no mesmo registo (parametro =) 
ou em registos diferentes qualquer outro caract�r, tal como d. Valor por defeito � =.

Terceiro argumento indica o n� de ticks em multiplos de 10microseg para o io_delay. Assim, para obter 1seg,
deve-se usar 100000. O valor por defeito � 1 (10 microsegundos).
 
Quarto argumento indica o n�mero de tarefas a serem lan�adas. O valor por defeito � 2.

Quinto argumento indica o n�mero de itera��es do ciclo de escritas ou leituras. O valor por defeito � NUMBLK.

O sexto argumento s� � utilizado se o segundo argumento f�r =
ou seja, se f�r para escrever/ler no mesmo registo, ent�o este argumento indica qual o registo. 
Por defeito � o registo 0.


===============================================================================================

Descri��o de test-logrw-wclean
Programa que lan�a tarefas n tarefas que s� escrevem em ciclo de n itera��es, escrevendo nos 3 registos 
em cada itera��o, e m tarefas  que l�m e que entram em ciclo e faz o dump de um registo sempre que 
detecta que h� uma nova vers�o deste.

Primeiro argumento indica o n�mero de tarefas que l�m a serem lan�adas. O valor por defeito � 1.

Segundo argumento indica o n�mero de tarefas que escrevem a serem lan�adas. O valor por defeito � 1.

Terceiro argumento indica o n� de ticks em multiplos de 10microseg para o io_delay. Assim, para obter 1seg,
deve-se usar 100000. O valor por defeito � 0.

Quarto argumento indica o numero de itera��es do ciclo de escritas. O valor por defeito � 4.


===============================================================================================
Descri��o dos testes em tests-logrw-script
Terminologia:
Array:0 (o array onde no inicio do programa se come�a a escrever)
Array:1 aquele que no arranque do programa est� inactivo.
Todos os indices s�o dados com refer�ncia ao 0. Assim, a ultima posi��o do array de registos � 
a posi��o 2 (este cont�m 3 registos).
NextWrBlk: o pr�ximo bloco livre (que pode tomar valores entre 0 e 11 para cada array)
REG_IDX: [0i]=x indica a posi��o x do array onde se encontra o registo i.

--------

TESTE 1 - teste com duas escritas por uma tarefa no registo 0
Executar ./test-logrw-ops_on_registers w = 1 1 2

Para passar o teste 1 com SUCESSO, o resultado dever� ser 
Array:0 NextWrBlk:005 ThreadsWaiting:000
REG_IDX: [00]=004 [01]=001 [02]=002 
00:01:00:00:00:00:00:00:	........
<registo n�o escrito, qualquer conte�do inicial � correcto>
<registo n�o escrito, qualquer conte�do inicial � correcto>

----------

TESTE 2 - teste com leituras do mesmo registo, o registo 0, feito por 4 tarefas, com atrasos de 
1 seg em cada leitura"
./test-logrw-ops_on_registers r = 100000 4

Para passar teste 2 com SUCESSO, o tempo total e o tempo despendido por tarefa dever� ser por 
volta dos 12 segs"

----------

TESTE 3 - teste com escritas do mesmo registo, o registo 0, feito por 4 tarefas, com atrasos de
1 seg por cada escrita"
./test-logrw-ops_on_registers w = 100000 4

Para passar teste 3 com SUCESSO, o tempo total e por tarefa dever� ser por volta dos 12-25 segs"

-----------

TESTE 4 - teste que lan�a uma tarefa que efectua sem atrasos um ciclo de 12 itera��es, em que em 
cada uma escreve nos tr�s registos, no block[0] o valor do registo, e no block[1] o valor da itera��o i 
(de 0 a 11, sendo 11 o hexadecimal 0B).
Portanto, sem feitas um total de 36 escritas.
./test-logrw-wclean 0 1 0 12

Para passar teste 4 com SUCESSO, o resultado dever� ser
NextWrBlk:numa posi��o de 1 a 12
REG_IDX: [0i]=x, x=1..12 , x � a i-�sima posi��o no array activo (a 1� corresponde ao 1� elemento do array, o 0)
Conte�do dos blocos dos registos:
00:0B:00:00:00:00:00:00: ........
01:0B:00:00:00:00:00:00: ........
02:0B:00:00:00:00:00:00: ........"

-----------

TESTE 5 - teste que lan�a uma tarefa de escrita, a qual efectua sem atrasos um ciclo de 4 itera��es, em que em 
cada uma escreve nos tr�s registos. Portanto, um total de 12 escritas. Lan�a tamb�m uma tarefa de leitura, que entra em ciclo e faz o dump de um 
registo sempre que detecta que h� uma nova vers�o deste.
./test-logrw-wclean 1 1 10000 4

Para passar teste 5 com SUCESSO
Array activo :1
NextWrBlk: uma posi��o entre a 4� e a 7� do array1.
REG_IDX: [0i]=x, x=1..6 , � uma das 6 primeiras posi��es do 2�array (podem estar trocados. e.g. 0,4 e 2).
Conte�do dos blocos dos registos:
00:03:00:00:00:00:00:00: ........
01:03:00:00:00:00:00:00: ........
02:03:00:00:00:00:00:00: ........"

------------

TESTE 6 - teste que lan�a 4 tarefas de escrita, a qual efectua sem atrasos um ciclo de 2 itera��es, em que em 
cada tarefa em cada itera��o escreve nos tr�s registos. Temos assim um total de (4 x 2 x 3 = 24 escritas). 
Lan�a tamb�m uma tarefa de leitura, que entra em ciclo e faz o dump de um registo sempre que detecta que h� 
uma nova vers�o deste.
./test-logrw-wclean 1 4 10000 2

Para passar teste 6 com SUCESSO, o resultado dever� ser
Array activo :0
NextWrBlk: uma posi��o entre a 4� e a 10� do array0.
REG_IDX: [0i]=x, x=1..9 , ou seja, numa das 9 primeiras posi��es do array 1
00:01:00:00:00:00:00:00: ........
01:01:00:00:00:00:00:00: ........
02:01:00:00:00:00:00:00: ........"
