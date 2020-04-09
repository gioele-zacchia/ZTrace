# ZTrace
C Stack Trace

Per installare 
```
git clone https://github.com/gioele-zacchia/ZTrace.git
make && sudo make install
```

L'API Espone 2 funzioni
```
#include<ZTR/ZTrace.h>

extern void ZTInstall();
extern void ZTClean();

```

Chiamare *ZTInstall()* all'avvio dell'applicazione e *ZTClean()* alla chiusura per rilasciare le risorse

Per compilare con ZTrace è necessario specificare a gcc il parametro `-lZTrace` e, se si vuole avere le indicazioni di riga, con `-g`

```
gcc -g test.c -o test -lZTrace

```

E' presente anche un un sorgente di esempio (compilabile con `make test`) che va volontariamente in segfault per verificare l'effettivo funzionamento della libreria

test.c
```
#include <ZTR/ZTrace.h>

void funzioneCheNonFunziona(char* a){
    *a = 0x41;
}


int main(){
    ZTInstall();

    char* aho = 0x41414141;
    funzioneCheNonFunziona(aho);


    ZTClean();
    return 0;
}
```

