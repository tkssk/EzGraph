#include <EzGraph.h>

typedef enum {
    Blue, Yellow, Red
} TrafficSignal;

TrafficSignal current_signal = Blue;

void mouse_event(int, int, unsigned int);
void draw_traffic_signal(TrafficSignal);

int main(){
    draw_traffic_signal(current_signal);

    EzSetMouseHandler(mouse_event);
    EzEventLoop();
    return 0;
}

void mouse_event(int x, int y, unsigned int n){
    if((x-250)*(x-250)+(y-150)*(y-150) < 40*40){
	current_signal = Blue;
    }
    if((x-350)*(x-350)+(y-150)*(y-150) < 40*40){
	current_signal = Yellow;
    }
    if((x-450)*(x-450)+(y-150)*(y-150) < 40*40){
	current_signal = Red;
    }
    draw_traffic_signal(current_signal);
}

void draw_traffic_signal(TrafficSignal sig){
    /* Set Color To Gray */
    EzSetColorByName("Gray");

    /* Fill BOX */
    EzFillBox(200, 100, 500, 200);

    /* Fill BOX */
    EzFillBox(500, 100, 510, 500);

    if(sig == Blue){
	/* Set Color To Blue */
	EzSetColorByName("Blue");
    }else{
	EzSetColorByName("DarkGray");
    }

    /* Fill Circle */
    EzFillCircle(250, 150, 40);

    if(sig == Yellow){
	/* Set Color To Yellow */
	EzSetColorByName("Yellow");
    }else{
	EzSetColorByName("DarkGray");
    }

    /* Fill Circle */
    EzFillCircle(350, 150, 40);

    if(sig == Red){
	/* Set Color To Red */
	EzSetColorByName("Red");
    }else{
	EzSetColorByName("DarkGray");
    }

    /* Fill Circle */
    EzFillCircle(450, 150, 40);
}
