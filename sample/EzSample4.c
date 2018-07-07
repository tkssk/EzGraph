#include <EzGraph.h>

void mouse_handler(int x, int y, unsigned int b);
void mouse_motion_handler(int x, int y);

int main(){
    EzSetColorByName("blue");
    EzFillBox(  0, 0, 199, 50);
    EzSetColorByName("yellow");
    EzFillBox(200, 0, 399, 50);
    EzSetColorByName("red");
    EzFillBox(400, 0, 599, 50);

    EzSetMouseHandler(mouse_handler);
    EzSetMouseMotionHandler(mouse_motion_handler);
    EzEventLoop();
}

void mouse_handler(int x, int y, unsigned int b){
    if(y >= 0 && y <= 50){
	x /= 200;
	switch(x){
	  case 0:
	    EzSetColorByName("blue");
	    break;
	  case 1:
	    EzSetColorByName("yellow");
	    break;
	  case 2:
	    EzSetColorByName("red");
	    break;
	}
    }
}

void mouse_motion_handler(int x, int y){
    EzFillCircle(x, y, 5);
}
