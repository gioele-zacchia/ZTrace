#include <ZTR/ZTrace.h>

void funzioneCheNonFunziona(char* a){
    *a = 0x41;
}


int main(){
    ZTInstall();
    //printf(\e[1;34mThis is a blue text.\e[0m\n");

    char* aho = 0x41414141;
    funzioneCheNonFunziona(aho);


    ZTClean();
    return 0;
}