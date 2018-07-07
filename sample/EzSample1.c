#include <EzGraph.h>

int main(){
    int i;

    /* Set Color To Gray */
    EzSetColorByName("gray");

    /* Fill BOX */
    EzFillBox(200, 100, 500, 200);

    /* Fill BOX */
    EzFillBox(500, 100, 510, 500);

    /* Set Color To Blue */
    EzSetColorByName("blue");

    /* Fill Circle */
    EzFillCircle(250, 150, 40);

    /* Set Color To Yellow */
    EzSetColorByName("yellow");

    /* Fill Circle */
    EzFillCircle(350, 150, 40);

    /* Set Color To Red */
    EzSetColorByName("red");

    /* Fill Circle */
    EzFillCircle(450, 150, 40);

    EzEventLoop();
    return 0;
}


