#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <EzGraph.h>

double bx, by, dx, dy;

void repaint(void){
    /* ����(���������̥��֥������Ȥ��طʤǾ��) */
    EzSetColorByRGB(255,255,0);
    EzFillBox(30,80,570,520);
    /* ����(̵���϶���:��) */
    EzSetColorByRGB(0,255,0);
    EzFillBox(40,90,300,510);
    /* ����(���϶���:��) */
    EzSetColorByRGB(0,128,255);
    EzFillBox(300,90,560,510);

    /* ��ɸ�׻� */
    bx += dx;
    by += dy;

    /* ���� */
    if(bx >= 300){
	dy += 0.1;
    }

    /* �ɾ��ͽ��� */
    if(bx >= 500){
	bx = 500;
	dx *= -1;
    }
    if(bx <= 0){
	bx = 0;
	dx *= -1;
    }
    if(by >= 400){
	by = 400;
	dy *= -1;
    }
    if(by <= 0){
	by = 0;
	dy *= -1;
    }

    /* �ܡ������� */
    EzSetColorByRGB(255,0,0);
    EzFillCircle(bx+50, by+100, 10);
}

int main(){
    /* ����� */
    bx = 0; dx = 1.8;
    by = 0; dy = 1.0;

    /* �����ޡ����٥�Ȥ���Ͽ */
    EzSetTimerHandler(repaint, 0);

    /* ���٥���Ԥ� */
    EzEventLoop();
}
