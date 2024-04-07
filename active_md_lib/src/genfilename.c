#include "genfilename.h"
#include "sys.h"
#include <stdio.h>
#include <stdlib.h>
// 定义路径分隔符
#ifdef _WIN32
#define PATH_SEPARATOR "\\"
#else
#define PATH_SEPARATOR "/"
#endif


char* gen_filename(char* dirname, char* marker)
{
    // 计算所需字符串长度，并为路径分隔符、文件名格式留出足够空间
    int neededSize = snprintf(NULL, 0, "%s" PATH_SEPARATOR "%s_N%d_dt%.0e_T%.1e_phi%.2f_tp%.1e_spf%.2e_iseed%d.txt", 
                                         dirname,marker, sys.Nparticle, sys.delta_t,sys.Tem,sys.phi,sys.t_p,sys.spf, sys.seed) + 1;
    char* filePath = (char*)malloc(neededSize);

    if (filePath == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL; // 处理内存分配失败
    }

    // 根据操作系统使用适当的路径分隔符创建文件路径字符串
    snprintf(filePath, neededSize, "%s" PATH_SEPARATOR "%s_N%d_dt%.0e_T%.1e_phi%.2f_tp%.1e_spf%.2e_iseed%d.txt", 
                                         dirname,marker, sys.Nparticle, sys.delta_t,sys.Tem,sys.phi,sys.t_p,sys.spf, sys.seed);

    return filePath;
}

