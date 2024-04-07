#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>
#include<stdbool.h>
#include"sys.h"
#include"init.h"


// config
/* void config_output(char* f)//输出当前位形保存到文件f内
{
    int i;
	FILE* fp;
	if(!(fp=fopen(f,"w")))
	{
		printf("cannot open the file %s",f);
		exit(1);
	}
    fprintf(fp,"box_size:%lf\nnum_particals:%d\n",size.L,sys.Nparticle);//输出一些基本参数
    fprintf(fp,"partical_id\tradius\tx\ty\tsp_x\tsp_y\n");//格式控制
    for(i=0;i<sys.Nparticle;i++)
    {
        fprintf(fp,"%d\t%lf\t%lf\t%lf\t%lf\t%lf\n",i,
		Par[i].R,Par[i].coord_x,Par[i].coord_y,Par[i].sp_x,Par[i].sp_y);//输出粒子坐标和半径
    }
    fclose(fp);
}
*/

bool isTimeToRecordConfig(int step)// 判断当前时间是否写入位形
{
    // 对于 step 大于或等于 10000 的情况，直接检查是否为 10000 的倍数
    if (step >= 10000) {
        return step % 10000 == 0;
    }

    // 当 step 小于 10000 时，使用原有逻辑判断
    double y = step / (pow(10, (int)log10(step)));

    // 使用一个紧凑的条件表达式来替代多个if分支
    return y == 1 || y == 2 || y == 4 || y == 6 || y == 8;
}

void dumpMDInfo(char* filename,int timestep)
{
    static int first_call = 1; // 静态变量，记录是否为第一次调用
    FILE *fp;
    if (first_call) {
        fp = fopen(filename, "w");
        first_call = 0; // 设置为非第一次调用
    } else {
        fp = fopen(filename, "a");
    }

    if (!fp) {
        printf("cannot open the file %s\n", filename);
        exit(1);
    }
    // 输出文件头部
    fprintf(fp, "ITEM: TIMESTEP\n");
    fprintf(fp, "%d\n", timestep);
    fprintf(fp, "ITEM: NUMBER OF ATOMS\n");
    fprintf(fp, "%d\n", sys.Nparticle);
    fprintf(fp, "ITEM: BOX BOUNDS pp pp pp\n");
    fprintf(fp,"%lf %lf\n%lf %lf\n%lf %lf\n",0.0,size.L,0.0,size.L,-1.0,1.0);
    fprintf(fp, "ITEM: ATOMS %3s %5s %16s %26s %26s %26s %26s %26s %26s %26s\n", "id", "type", "x", "y" ,"vx" , "vy" ,"spx" , "spy" ,"cx", "cy");

    // 输出每个原子的信息
    int i;
    for (i = 0; i < sys.Nparticle; ++i) {
        fprintf(fp, "%15d %3d %26.16e %26.16e %26.16e %26.16e %26.16e %26.16e %26.16e %26.16e\n", 
                i, i*2/sys.Nparticle+1, Par[i].coord_x, Par[i].coord_y, Par[i].velocity_x ,Par[i].velocity_y ,Par[i].sp_x ,Par[i].sp_y, Par[i].cum_coord_x, Par[i].cum_coord_y);
    }
    fclose(fp);
}
// msd

bool isTimeToRecordMSD(int step)// 判断当前时间是否写入 msd
{
    double y;
	y=step/(pow(10,(int)log10(step)));
    if(y==1||y==2||y==4||y==6||y==8)
        return true;
    else
        return false;
}

void MSD_output(char* filePath,int step) //写入msd到文件   如果文件是第一次打开，则将文件清零，否则追加，用static 实现
{
    static bool isFirstTime = true; // 静态变量跟踪是否第一次打开文件

    FILE* fp;
    if (isFirstTime) {
        fp = fopen(filePath, "w"); // "w"模式打开文件，清零内容
        isFirstTime = false; // 更新标志，表示文件已经被打开过
    } else {
        fp = fopen(filePath, "a"); // "a"模式打开文件，追加内容
    }
    
    if (fp == NULL) {
        printf("cannot open the file %s",filePath);
		exit(1);
    }

    int j;
	double msd=0;
    for(j=0;j<sys.Nparticle;j++)
        	msd=msd+(Par[j].cum_coord_x-inti_coord_x[j])*(Par[j].cum_coord_x-inti_coord_x[j])+
        		(Par[j].cum_coord_y-inti_coord_y[j])*(Par[j].cum_coord_y-inti_coord_y[j]);

    fprintf(fp,"%.2lf\t\t%.20lf\n",step*sys.delta_t,msd/sys.Nparticle);
    fclose(fp); // 关闭文件
}

// spV

typedef struct {
    double velocity_sum;
    int count;
} Accumulator;

static Accumulator acc = {0, 0}; // 初始化累加器


double count_V(const char* mode) {
    double sum = 0;
    int i;
    for (i = 0; i < sys.Nparticle; i++)
        sum += (Par[i].velocity_x * Par[i].sp_x + Par[i].velocity_y * Par[i].sp_y);

    if (strcmp(mode, "monitor") == 0) {
        // 监控模式：直接返回当前计算结果
        return sum * sys.Nparticle_inv;
    } else if(strcmp(mode, "average") == 0) {
        // 平均模式：累加结果并更新计数，但不立即计算平均
        acc.velocity_sum += sum;
        acc.count += 1;
        return 0; // 在累加模式下，可以返回0或者当前累加值，根据需求决定
    }
    return 0; // 默认返回0
}

// 获取平均模式下的平均值的函数
double get_average_V() {
    if (acc.count == 0) return 0; // 防止除以0
    double average = acc.velocity_sum * sys.Nparticle_inv / (acc.count);
    acc.velocity_sum = 0; // 重置累加器，以便新的计算
    acc.count = 0;
    return average;
}

void spV_output(char* filePath)
{
    FILE* fp;
	if(!(fp=fopen(filePath,"w")))
	{
		printf("cannot open the file %s",filePath);
		exit(1);
	}

    fprintf(fp,"%lf\t%.15lf\t%.15lf\n",sys.phi,get_average_V(),sys.spf);

    fclose(fp);
}


// kinetic

typedef struct {
    double kinetic_energy_sum;
    int count;
} KEAccumulator;

static KEAccumulator ke_acc = {0, 0}; // Initialize the kinetic energy accumulator.

// Function to compute the system's kinetic energy or monitor its average.
double computeKineticEnergy(const char* mode) {
    double kineticEnergy = 0;
    int i;
    for (i = 0; i < sys.Nparticle; i++) {
        // Compute the kinetic energy for each particle.
        kineticEnergy += 0.5 * Par[i].mass * (Par[i].velocity_x * Par[i].velocity_x + Par[i].velocity_y * Par[i].velocity_y)*sys.delta_t;  //注意这里要乘delta_t,这样定义似乎才是非病态的，不然结果依赖delta_t的选取
    }

    if (strcmp(mode, "monitor") == 0) {
        // Monitor mode: directly return the current kinetic energy.
        return kineticEnergy/sys.Nparticle;
    } else if (strcmp(mode, "average") == 0) {
        // Average mode: accumulate kinetic energy and update the count, without immediately computing the average.
        ke_acc.kinetic_energy_sum += kineticEnergy/sys.Nparticle;
        ke_acc.count += 1;
        return 0; // In accumulation mode, returning 0 or the current accumulated value is optional, based on requirements.
    }
    return 0; // Default return value is 0 for unspecified modes.
}

// Function to get the average kinetic energy in 'average' mode.
double getAverageKineticEnergy() {
    if (ke_acc.count == 0) return 0; // Prevent division by zero.
    double averageKE = ke_acc.kinetic_energy_sum / ke_acc.count;
    // Reset the kinetic energy accumulator for new computations.
    ke_acc.kinetic_energy_sum = 0;
    ke_acc.count = 0;
    return averageKE;
}

void Kinetic_output(char* filePath)
{
    FILE* fp;
	if(!(fp=fopen(filePath,"w")))
	{
		printf("cannot open the file %s",filePath);
		exit(1);
	}

    fprintf(fp,"%lf\t%.15lf\t%.15lf\n",sys.phi,sys.spf,getAverageKineticEnergy());

    fclose(fp);
}
