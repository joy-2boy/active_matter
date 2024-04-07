#include"mathe.h"
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h> //这个库的作用是什么

static const gsl_rng_type * T;
static gsl_rng * r; 

void rng_init_mt19937(unsigned long seed) {
    gsl_rng_env_setup(); // 这句话用于设定默认的 gsl_rng_type 和 seed ,在当前情境下不必要使用，但是一个好的编程习惯
    T = gsl_rng_mt19937; // 显式设置为MT19937
    r = gsl_rng_alloc(T); //  r的主要作用是维护记录随机数生成过程的状态，包括种子、当前位置等内部信息。通过操作r，你可以生成随机数、重置种子、复制生成器状态等
    gsl_rng_set(r, seed);
}
// 高斯和[0,1)分布都是基于 r产生的序列（即mt19937算法）来进一步转换的
double uniform_random() {
    return gsl_rng_uniform(r);
}

double standard_gaussian_random() {
    return gsl_ran_gaussian(r, 1.0);
}

// 调用这个函数来释放随机数生成器资源，可以在程序结束前或不再需要随机数时调用
void random_free() {
    gsl_rng_free(r);
}



/*  下面是自己手动用线性同余和box-muller实现的，现在直接用gsl库


#define a__ 16807
#define b__ 0		//b小于m
#define m__ 2147483647 //2^31-1

// 静态变量存储当前种子
static unsigned int current_seed = 1; // 设置一个默认种子值，以防未调用初始化函数

// 初始化随机数生成器
void rng_init_Lehmer(unsigned int seed) {
    current_seed = seed ? seed : 1; // 防止种子为0
}


int schrage(int z)//返回(a*z+b)%m的值

{
	int q,r,z1,z2,s;
	q=m__/a__;	r=m__%a__; //m=a*q+r
	z1=z/q;   z2=z%q; //z=z1*q+z2
	s=a__*z2-r*z1;
	if(s<0)
		s=s+m__;
	//此时s=(a*z)%m
	if(m__-b__<=s)
		s=(b__-m__)+s;//这样避免了s+b的溢出;
	else
		s=s+b__;
	//此时s=(a*z+b)%m
	return s;
}


double uniform_random(void)//产生0，1之间的随机数,更新产生随机数用的种子
{
	current_seed=schrage(current_seed);//作为下一次随机数产生的种子
	return (1.0*current_seed)/m__;
}


int seed(void)//用当前时间产生一个种子
{
	int I;
	time_t t;
	struct tm* lt;
	time(&t);
	lt=localtime(&t);
	I=lt->tm_year+70*(lt->tm_mon+12*(lt->tm_mday+31*(lt->tm_hour+23*(lt->tm_min+59*lt->tm_sec))));
	I=I%m__;
	return I;
}


//*************高斯抽样*************

static double __z0, __z1;
static bool generate = false;

double standard_gaussian_random(void) {
    // 每次调用时，交替生成两个随机数和返回一个随机数
    generate = !generate;

    // 如果不需要生成，直接返回上一次生成的另一个数
    if (!generate)
        return __z1;

    double u1, u2;
    double epsilon = 1e-15; // 防止u1为0

    // 生成两个(0, 1)之间的随机数
    u1 = randnum_Lehmer(); 
    u2 = randnum_Lehmer();

// 确保不为0
    u1 = u1 < epsilon ? epsilon : u1;

    // Box-Muller变换
    __z0 = sqrt(-2.0 * log(u1)) * cos(2.0 * PI * u2);
    __z1 = sqrt(-2.0 * log(u1)) * sin(2.0 * PI * u2);


    return __z0;
}


*/