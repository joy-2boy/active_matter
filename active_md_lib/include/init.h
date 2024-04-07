#ifndef INIT_H
#define INIT_H

typedef struct{
    double R;//粒子半径
	double mass;//粒子质量
	double coord_x,coord_y;// 盒子坐标
	double cum_coord_x,cum_coord_y; // 累计坐标
	double velocity_x,velocity_y;//粒子的速度
	double intforce_x,intforce_y;//粒子受到其他粒子的相互作用力
	double theta;//自驱动角度
    double sp_x,sp_y;//自驱动力的方向 ，看似多余，其实是为了存储，避免对此计算sp_x,sp_y，牺牲空间，换取时间
	double flu_x,flu_y;//热涨落带来的力
}tp_particle;  //注意，这里使用及结构体数组（SoA）并不是一个好的方案，相比于数组结构体AoS，在计算坐标和速度的时候， cache的命中率不高,不过这种小优化就不值当了，因为主要的复杂度是O(n^2),优化这种O(n)的意义不大

extern tp_particle *Par;//粒子数组指针

extern double *inti_coord_x;
extern double *inti_coord_y;//用来记录初始位形的指针

void gen_rand_con(void);//初始化一个随机位形
//也可以再写一个从给定文件读取位形的函数
void record_init_con(void);


#endif