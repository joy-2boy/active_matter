#include <stdlib.h>
#include "sys.h"
#include "init.h"
#include "mathe.h"
#include "cell_md.h"
// 关于static 作用于全局变量时，可以避免被其他源文件通过extern关键字访问。作用于局部变量，可以该变量的存储期限
static int cell_n;//每行（列）的元胞数目
static double d;//元胞边长
static int ***cell;//元胞数组指针

void init_cell(void) {
    cell_n = fmin(size.L/(size.r_2*2), sqrt(sys.Nparticle));
    d = size.L / cell_n;

	int i,j,m,n;

    // 使用 calloc 为 cell 数组分配内存，并自动初始化为零
    cell = (int ***)calloc(cell_n, sizeof(int **));
    if (cell == NULL) {
        perror("Failed to allocate memory for cell array");
        exit(EXIT_FAILURE); // 使用标准的退出码
    }

    for (i = 0; i < cell_n; i++) {
        cell[i] = (int **)calloc(cell_n, sizeof(int *));
        if (cell[i] == NULL) {
            perror("Failed to allocate memory for cell row");
            // free_cell(cell, i); // 释放已分配的内存并退出
            exit(EXIT_FAILURE);
        }

        for (j = 0; j < cell_n; j++) {
            cell[i][j] = (int *)calloc(sys.Nparticle + 1, sizeof(int));
            if (cell[i][j] == NULL) {
                perror("Failed to allocate memory for cell column");
                // free_cell(cell, i+1); // 注意这里传 i+1，因为需要释放第i行
                exit(EXIT_FAILURE);
            }
        }
    }

    // 根据初始位形初始化元胞数组
    for (i = 0; i < sys.Nparticle; i++) {
        m = Par[i].coord_x / d;
        n = Par[i].coord_y / d;
        cell[m][n][++cell[m][n][0]] = i;
    }
}

void reset_cell(void)
{
    int i,j;
	for(i=0;i<cell_n;i++)
		for(j=0;j<cell_n;j++)
			cell[i][j][0]=0;//用每一行的第一个来标志属于元胞的粒子数
}

void intforce(double *f_x,double*f_y,double x_1, double y_1,double R_1, double x_2, double y_2, double R_2)
//1粒子感受到2粒子的x方向作用力,type=0返回x方向的力，否则返回y方向的力
{
	// 计算粒子间的距离差分，避免重复计算
    double dx = x_1 - x_2;
    double dy = y_1 - y_2;
    // 计算粒子间的直线距离
    double D = sqrt(dx * dx + dy * dy);
    // 计算重叠距离
    double delta = R_1 + R_2 - D;
    // 如果没有重叠，则力为0
    if (delta <= 0) {
        *f_x = 0;
        *f_y = 0;
        return; // 直接返回
    }
    // 优化k的计算，如果可能的话
    double k = sys.e_0 / ((R_1 + R_2) * (R_1 + R_2));
    // 计算力
    *f_x = k * dx * delta / D;
    *f_y = k * dy * delta / D;
}

void cell_intforce(int i, int j) // 计算i，j元胞中的粒子与周围元胞的相互作用
{
    int i_next, j_next, k, l, m, n, p, q;
    double f_x, f_y, adjust_x, adjust_y;

    k = cell[i][j][0]; // 本元胞内的粒子数量

    // 计算本元胞内的粒子相互作用
    for (m = 1; m <= k; m++) {
        for (n = m + 1; n <= k; n++) {
            p = cell[i][j][m];
            q = cell[i][j][n];
            intforce(&f_x, &f_y, Par[p].coord_x, Par[p].coord_y, Par[p].R, Par[q].coord_x, Par[q].coord_y, Par[q].R);
            Par[p].intforce_x += f_x;
            Par[p].intforce_y += f_y;
            Par[q].intforce_x -= f_x;
            Par[q].intforce_y -= f_y;
        }
    }

    // 处理与周围元胞的相互作用
    int dirs[4][2] = {{1, 0}, {1, 1}, {0, 1}, {-1, 1}}; // 右，右上，上，左上
	int dir;
    for (dir = 0; dir < 4; dir++) {
        adjust_x = adjust_y = 0;
        i_next = i + dirs[dir][0];
        j_next = j + dirs[dir][1];

        // 处理周期性边界条件
        if (i_next >= cell_n) {
            i_next = 0;
            adjust_x = size.L;
        } else if (i_next < 0) {
            i_next = cell_n - 1;
            adjust_x = -size.L;
        }
        if (j_next >= cell_n) {
            j_next = 0;
            adjust_y = size.L;
        } else if (j_next < 0) {
            j_next = cell_n - 1;
            adjust_y = -size.L;
        }     //可以考虑进一步优化

        l = cell[i_next][j_next][0]; // 目标元胞内的粒子数量
        for (m = 1; m <= k; m++) {
            for (n = 1; n <= l; n++) {
                p = cell[i][j][m];
                q = cell[i_next][j_next][n];
                intforce(&f_x, &f_y, Par[p].coord_x, Par[p].coord_y, Par[p].R, Par[q].coord_x + adjust_x, Par[q].coord_y + adjust_y, Par[q].R);
                Par[p].intforce_x += f_x;
                Par[p].intforce_y += f_y;
                Par[q].intforce_x -= f_x;
                Par[q].intforce_y -= f_y;
            }
        }
    }
}

void count_intforce(void)//计算每个粒子受到的其他粒子的力
{
	int i,j;
    for(i=0;i<sys.Nparticle;i++)
        Par[i].intforce_x=Par[i].intforce_y=0;//作用力清零
	
	for(i=0;i<cell_n;i++)
		for(j=0;j<cell_n;j++)
			cell_intforce(i,j);//算出作用力
	
}


void flu_force(void)//赋予粒子热扰动力
{
	int i;
	double sigma=sqrt(2*sys.Tem*sys.xi/sys.delta_t);
	for(i=0;i<sys.Nparticle;i++)
    {
        Par[i].flu_x=sigma*standard_gaussian_random();
        Par[i].flu_y=sigma*standard_gaussian_random();
    }
}


void count_velocity(void)
{	
	double centroid_f_x=0,centroid_f_y=0; //去除质心力
	int i;
	for(i=0;i<sys.Nparticle;i++)
	{
		centroid_f_x += Par[i].flu_x+sys.spf*Par[i].sp_x;
		centroid_f_y += Par[i].flu_y+sys.spf*Par[i].sp_y;
	}
	centroid_f_x *= sys.Nparticle_inv;
	centroid_f_y *= sys.Nparticle_inv;
	for(i=0;i<sys.Nparticle;i++)
	{
		Par[i].velocity_x=(Par[i].intforce_x + Par[i].flu_x + sys.spf*Par[i].sp_x - centroid_f_x )/sys.xi;
		Par[i].velocity_y=(Par[i].intforce_y + Par[i].flu_y + sys.spf*Par[i].sp_y - centroid_f_y )/sys.xi;
	}
}


void count_coord(void)//计算下一时刻的坐标，并更新元胞信息
{
	int i,m,n;
	double deltaX, deltaY;
	reset_cell();//对元胞信息清零;
	for (i=0; i<sys.Nparticle; i++)
	{
		deltaX = Par[i].velocity_x * sys.delta_t;
        deltaY = Par[i].velocity_y * sys.delta_t;
		Par[i].cum_coord_x += deltaX;
        Par[i].cum_coord_y += deltaY;

		Par[i].coord_x += deltaX;
        Par[i].coord_y += deltaY;

		while(Par[i].coord_x>=size.L)
			Par[i].coord_x=Par[i].coord_x-size.L;
		while(Par[i].coord_x<0)
			Par[i].coord_x=Par[i].coord_x+size.L;
		while(Par[i].coord_y>=size.L)
			Par[i].coord_y=Par[i].coord_y-size.L;
		while(Par[i].coord_y<0)
			Par[i].coord_y=Par[i].coord_y+size.L;

		m=Par[i].coord_x/d;
		n=Par[i].coord_y/d;
		cell[m][n][++cell[m][n][0]]=i;
	}
}


void rotation(void)
{
	int i;
	double sigma=sqrt(2*sys.delta_t/sys.t_p);
	
	for(i=0;i<sys.Nparticle;i++)
    {
		Par[i].theta = Par[i].theta + sigma*standard_gaussian_random();
		Par[i].sp_x = cos(Par[i].theta);
		Par[i].sp_y = sin(Par[i].theta);
    }
}

void md_step(void)
{
	flu_force();//赋予高斯噪声
	count_intforce();//算出粒子相互作用力
	count_velocity();//算出速度（做了质心速度修正）
	count_coord();//算下一时刻的坐标并更新元胞信息;
	rotation();//转动自驱动方向
}