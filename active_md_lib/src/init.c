#include <stdio.h>
#include <stdlib.h>
#include "init.h"
#include "sys.h"
#include "mathe.h"

tp_particle *Par;
double *inti_coord_x;
double *inti_coord_y;

void count_box_size(void)//计算盒子边长
{
    size.L=sqrt((sys.Nparticle*PI*(size.r_1*size.r_1+size.r_2*size.r_2))/(2*sys.phi));
}

void alloc_space(void)//为粒子结构体数组和初始位形数组分配空间
{
    Par = (tp_particle *)malloc(sys.Nparticle * sizeof(tp_particle));
}

void rescale() // 将自驱动方向化为和为零
{
	int i,j=0;
	double sum_x=0,sum_y=0;
	double tol=1e-15,l; //tol 太小了会有不收敛的情况 种子 512029013 tol 1e-17 就不收敛了，来回振荡

	for(i=0;i<sys.Nparticle;i++)
	{
			sum_x=sum_x+Par[i].sp_x;
			sum_y=sum_y+Par[i].sp_y;
	}
	sum_x=sum_x*sys.Nparticle_inv;
	sum_y=sum_y*sys.Nparticle_inv;
	while( ( (fabs(sum_x)>tol)||(fabs(sum_y)>tol) ) && j<5000 )
	{
		for(i=0;i<sys.Nparticle;i++)
		{
			Par[i].sp_x=Par[i].sp_x-sum_x;
			Par[i].sp_y=Par[i].sp_y-sum_y;
			l=sqrt(Par[i].sp_x*Par[i].sp_x+Par[i].sp_y*Par[i].sp_y);
			Par[i].sp_x=Par[i].sp_x/l;
			Par[i].sp_y=Par[i].sp_y/l;
		}
		sum_x=sum_y=0;
		for(i=0;i<sys.Nparticle;i++)
		{
			sum_x=sum_x+Par[i].sp_x;
			sum_y=sum_y+Par[i].sp_y;
		}
		sum_x=sum_x*sys.Nparticle_inv;
		sum_y=sum_y*sys.Nparticle_inv;
		j++;
	}
	printf("%.20lf\t%.20lf\n",sum_x,sum_y);
	for(i=0;i<sys.Nparticle;i++)
	{
		Par[i].theta = atan(Par[i].sp_y/Par[i].sp_x);
		if(Par[i].sp_x<0)
			Par[i].theta=Par[i].theta + PI;
	}
	//	for(i=0;i<N;i++)
//		printf("%lf\n",Par[i].sp_x*Par[i].sp_x+Par[i].sp_y*Par[i].sp_y);
}

void record_init_con(void)//记录初始位形;
{
	inti_coord_x = (double *)malloc(sys.Nparticle * sizeof(double));
    inti_coord_y = (double *)malloc(sys.Nparticle * sizeof(double));

    int i;
    for(i=0;i<sys.Nparticle;i++)
    {
        inti_coord_x[i]=Par[i].cum_coord_x = Par[i].coord_x;
	    inti_coord_y[i]=Par[i].cum_coord_y = Par[i].coord_y;
    }
}

void gen_rand_con(void)//初始化一个随机位形
{
	alloc_space();//为粒子结构体数组分配空间
	count_box_size();//计算盒子边长
    int i;
    //初始化半径和质量
    for(i=0;i<sys.Nparticle/2;i++)  // 也可以考虑将这一块函数化，来应对连续分散的情况
    {
        Par[i].R=size.r_1;
		Par[i+sys.Nparticle/2].R=size.r_2;
		Par[i].mass=size.m_1;
		Par[i+sys.Nparticle/2].mass=size.m_2;
    }
    //初始化自驱动方向，位置坐标
    for(i=0;i<sys.Nparticle;i++)
    {
        Par[i].coord_x=size.L*uniform_random();
        Par[i].coord_y=size.L*uniform_random();
		if(Par[i].coord_x>=size.L)//防止坐标等于L,有时候随机数足够接近1，可能会导致
			Par[i].coord_x=Par[i].coord_x-size.L;
		if(Par[i].coord_y>=size.L)
			Par[i].coord_y=Par[i].coord_y-size.L;

		Par[i].cum_coord_x = Par[i].coord_x;
		Par[i].cum_coord_y = Par[i].coord_y;

        Par[i].theta=2*PI*uniform_random(); //随机自驱动力方向
        Par[i].sp_x=cos(Par[i].theta);
        Par[i].sp_y=sin(Par[i].theta);
    }
    rescale();
}
