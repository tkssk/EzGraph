#include <EzGraph.h>

int main(){
    /* Set Color To Gray */
    EzSetColorByName("Gray");

    /* Fill BOX */
    EzFillBox(200, 100, 500, 200);

    /* Fill BOX */
    EzFillBox(500, 100, 510, 500);

    /* Set Color To Blue */
    EzSetColorByName("Blue");

    /* Fill Circle */
    EzFillCircle(250, 150, 40);

    /* Set Color To Yellow */
    EzSetColorByName("Yellow");

    /* Fill Circle */
    EzFillCircle(350, 150, 40);

    /* Set Color To Red */
    EzSetColorByName("Red");

    /* Fill Circle */
    EzFillCircle(450, 150, 40);

    EzEventLoop();
    return 0;
}


