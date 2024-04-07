#ifndef SYS_H
#define SYS_H



typedef struct {
    int    Nparticle;
    double Nparticle_inv;
    double e_0;//能量尺度epsilon
    double xi;//粘滞系数
    double delta_t;//系统演化时间间隔
    double phi;
    double Tem;
    double spf;//自驱动力的大小
    double t_p;//转动的特征时间
    int seed;//随机数种子
}tpsys;

extern tpsys sys;//注意这里只是声明了sys为全局变量，并没有定义它，也就是说还没有为它分配内存

typedef struct {        
    double r_1,r_2;//二维双分散系统的粒子半径
    double m_1,m_2;//粒子质量
    double L;//地图边长
}tpsize;

extern tpsize size;

#endif