#include <EzGraph.h>

typedef enum {
    Blue, Yellow, Red
} TrafficSignal;

TrafficSignal current_signal = Blue;

void timer_event(void);
void draw_traffic_signal(TrafficSignal);

int main(){
    draw_traffic_signal(current_signal);

    EzSetTimerHandler(timer_event, 1000);
    EzEventLoop();
    return 0;
}

void timer_event(void){
    current_signal++;
    if(current_signal > Red){
	current_signal = Blue;
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
   
