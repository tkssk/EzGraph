#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <EzGraph.h>

double bx, by, dx, dy;

void repaint(void){
    /* 外壁(以前の描写オブジェクトを背景で上書き) */
    EzSetColorByRGB(255,255,0);
    EzFillBox(30,80,570,520);
    /* 下地(無重力空間:左) */
    EzSetColorByRGB(0,255,0);
    EzFillBox(40,90,300,510);
    /* 下地(重力空間:右) */
    EzSetColorByRGB(0,128,255);
    EzFillBox(300,90,560,510);

    /* 座標計算 */
    bx += dx;
    by += dy;

    /* 重力 */
    if(bx >= 300){
	dy += 0.1;
    }

    /* 壁衝突処理 */
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

    /* ボール描写 */
    EzSetColorByRGB(255,0,0);
    EzFillCircle(bx+50, by+100, 10);
}

int main(){
    /* 初期値 */
    bx = 0; dx = 1.8;
    by = 0; dy = 1.0;

    /* タイマーイベントを登録 */
    EzSetTimerHandler(repaint, 0);

    /* イベント待ち */
    EzEventLoop();
}
