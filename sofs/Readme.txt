Nesta vers�o existe apenas a directoria root no SOFS e n�o � prevista a cria��o de subdirectorias.
Os ficheiros podem ser re-escritos.

Apos a instala��o do FUSE no LINUX, Testar a montagem de "sofs" executando os seguintes passos:
===============================================================================================
1) Na directoria Executar o comando "make" para a cria��o da aplica��o "sofs" ;
2) Mudar para a directoria "sofs" e criar uma <subdirectoria> onde o "sofs" ser� montado 
  (pode utilizar a directoria "my_dir"  ja' existente);
3) Escrever na linha de comandos:
            ./sofs  <subdirectoria>
   alternativamente para a obten��o de informa��o de debbuging:
             /sofs -d <subdirectoria>  

  caso se pretenda o funcionamento uni-tarefa, incluir a op��o -s. Por exemplo:
            ./sofs  -s -d <subdirectoria>
4) Executar comandos que envolvam a <subdirectoria> , como por exemplo: 
            ls -alt > <subdirectoria>/dir.txt
            ls -alt <subdirectoria>
            etc...
   programas que utilizem as fun��es de sistema call e write e read...

5) Para desmontar o "sofs" do Fuse, executar:
            fusermount -u <subdirectoria>

6) para verificar se a desmontagem teve sucesso, confirmar que o "sofs" n�o se encontra na lista dos sistemas
	que montados que resulta da execu��o do comando:
           mount

