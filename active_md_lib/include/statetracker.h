#ifndef STATETRACKER_H
#define STATETRACKER_H
#include <stdbool.h>

bool isTimeToRecordConfig(int step);
void dumpMDInfo(char* filename,int timestep);

bool isTimeToRecordMSD(int step);// 判断当前步数是否写入 msd
void MSD_output(char* filePath,int step);

double count_V(const char* mode);
double get_average_V();// 获取平均模式下的平均值的函数
void spV_output(char* filePath);

double computeKineticEnergy(const char* mode);
double getAverageKineticEnergy();
void Kinetic_output(char* filePath);

#endif // STATETRACKER_H
