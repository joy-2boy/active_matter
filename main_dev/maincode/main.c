#include<stdio.h>
#include"sys.h"
#include"init.h"
#include"mathe.h"
#include"genfilename.h"
#include"cell_md.h"
#include"statetracker.h"

char dirname[200];

void SetParam_test(void)
{
    size.m_1 = size.m_2 = 1;
    sys.xi = 1;

    size.r_1 = 0.5 , size.r_2=0.7; //这些也都可以从外界输入
    sys.e_0 = 1;
    sys.delta_t = 1e-1;

    //下面的应该被scanf
    sys.Nparticle = 128;
    sys.spf = 1e-2;
    sys.t_p = HUGE_VAL;
    sys.phi = 0.82;
    sys.Tem = 0;

    sys.Nparticle_inv = 1.0/sys.Nparticle;

    sys.seed = 1;
    rng_init_mt19937(sys.seed);
    printf("seed : %d\n",sys.seed);

    #include <string.h>
    strcpy(dirname,"/data2/xsli/active_matter/main_dev/data/test");

}

void SetParam(void)
{
    size.m_1 = size.m_2 = 1;
    size.r_1 = 0.5 , size.r_2=0.7;
    sys.e_0 = 1;
    sys.xi = 1;
    sys.delta_t = 1e-1;

    scanf("%d",&sys.Nparticle);
    sys.Nparticle_inv = 1.0/sys.Nparticle;
    scanf("%lf",&sys.Tem);
    scanf("%lf",&sys.t_p);
    scanf("%lf",&sys.phi);
    scanf("%lf",&sys.spf);
    scanf("%d",&sys.seed);
    scanf("%s",dirname);

    rng_init_mt19937(sys.seed);

}

void spps_cell_md()
{
    char* filepath_msd = gen_filename(dirname,"msd");
    char* filepath_fv = gen_filename(dirname,"fv");
    char* filepath_Ek = gen_filename(dirname,"Ek");
    char* filepath_dump = gen_filename(dirname,"dump");

    int i=0;
	int preheat_num=1e4/(sys.delta_t),time_num=1e4/(sys.delta_t);
    int monitorInterval = 1e3;
    gen_rand_con(); //分配粒子数组空间，初始化随机位形和方向，记录初始位形
    init_cell(); // 分配元胞数组空间，根据初始位形初始化元胞数组
    while(i<preheat_num)
	{		
		md_step(); // md
    	if(((i+1)%monitorInterval)==0) // 监视
    	    printf("%d\t%lf\t%lf\t%lf\n",i,sys.spf,count_V("monitor"),computeKineticEnergy("monitor"));
		i++;
	}
	record_init_con();  //记录预热后的初始位置,确保msd计算的正确性  。记录坐标的时候还干了一件事，就是把累计坐标重置成盒子内部坐标，必要性不大，但是感觉上更舒服
    dumpMDInfo(filepath_dump,0); //输出预热后的初始位形
    // 主循环
	while(i<(preheat_num+time_num))
	{

		md_step();  // md
        if(((i+1)%monitorInterval)==0) // 监视
        {
            printf("%d\t%lf\t%lf\t%lf\n",i,sys.spf,count_V("monitor"),computeKineticEnergy("monitor"));
            //dumpMDInfo(filepath_dump,i-preheat_num+1);
        }
        //记录位形
        if(isTimeToRecordConfig(i-preheat_num+1))
            dumpMDInfo(filepath_dump,i-preheat_num+1);
        // 记录msd
        if(isTimeToRecordMSD(i-preheat_num+1))
            MSD_output(filepath_msd,i-preheat_num+1);
        //记录V
        count_V("average");
        computeKineticEnergy("average");
        i++;
    }

    spV_output(filepath_fv);
    Kinetic_output(filepath_Ek);
}



int main()
{
    SetParam();
    spps_cell_md();
    return 0;
}