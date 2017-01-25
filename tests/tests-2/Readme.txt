Os testes II incluem os testes à cache e ao sistema sofs/logsofs.

Para cada teste, fazer a montagem de "sofs" inicialmente na directoria testsII/testeII, desmontando no final. Procedimento para montar:
Entrar em testsII
Fazer ../sofs/sofs -d testeII
Abrir outra shell. Ir para a directoria testsII.
As directorias perf_dir e fs-flush-dir contêm ficheiros que irão ser utilizados nos testes.
A directoria res é uma directoria auxiliar.
===============================================================================================
1) executar ./cache-perf.sh
Este teste avalia se o ficheiro/leitura funcionam correctamente, comparando o resultado da leitura 
com a de um ficheiro original escrito no sofs/logsofs. Não deverá haver diferenças no resultado.

Fazer teste para sofs sem cache, sofs com cache e logsofs com cache. 
O tempo calculado para sofs sem cache deverá ser o menor.

2) Testar trincos da cache, com dois processos a escrever, ler, e escrever & ler simultaneamente nesta
executar ./cache-lock-writes.sh
Dois processos escrevem na cache.
executar ./cache-lock-reads.sh
Dois processos lêm da cache.
executar ./cache-lock-wr.sh
Um processo escreve na cache enquanto o outro lê dssta.

3) Executar ./fs-flush.sh
Teste à correcção da gestão da cache
- Deverá dar, antes da limpeza começar:
Clean blocks/registers: 18 (blocos de ficheiro)
Dirty blocks/registers: 0...10 (blocos do directório e inode), 19...36 (blocos do ficheiro)
Flushed: 8
- Depois da limpeza 
Clean blocks/registers: 0...10, 18...36
Flushed: 37 (8+11+18)

Fazer teste para sofs com cache e logsofs com cache. 

4) Executar ./fs-perf.sh
Teste de performance do logsofs vs sofs com cache.
O tempo no final do cache-flush deverá ser menor para o logsofs.



