#pragma once 
#include<iostream>
#include<sys/time.h>
#include<cstdio>
#include<cstdlib>
#include<cstring>

//[时间 info/warning/error/fatal/debug 文件 行号] 具体的错误信息

const char* level[] = { "INFO","WARNING","ERROR","FATAL","DEBUG"};

enum log_level
{
  INFO=0,
  WARNING,
  ERROR,
  FATAL,
  DEBUG
};

class log_time
{
  public:
    static int64_t Get_TimeStamp()
    {
      //第一个参数是一个结构体，struct timeval
      //第二个参数是时区,一般传NULL即默认系统时区
      struct timeval tv;
      gettimeofday(&tv,NULL);
      return tv.tv_sec;
    }
    
    static void Get_TimeStamp(std::string& timestamp)
    {
      //返回年月日时分秒
      time_t sys_time;
      time(&sys_time);

      struct tm* ST = localtime(&sys_time);
      //格式化字符串
      char time_now[50] = {'\0'};
      snprintf(time_now,sizeof(time_now)-1,"%04d-%02d-%02d %02d:%02d:%02d",ST->tm_year+1900,ST->tm_mon+1,ST->tm_mday,ST->tm_hour,ST->tm_min,ST->tm_sec);
      timestamp.assign(time_now,strlen(time_now));
    }

};

inline std::ostream& Log(log_level lev,const char* file,int line,const std::string& log_msg)
{
  std::string level_info = level[lev];
  std::string timer_stamp;
  log_time::Get_TimeStamp(timer_stamp);
  
  //[时间 info/warning/error/fatal/debug 文件 行号] 具体的错误信息
  //std::cout<<"["<<timer_stamp<<" "<<level_info<<" "<<file<<":"<<line<<"]"<<log_msg<<std::endl;
  std::cout<<"["<<timer_stamp<<" "<<level_info<<" "<<file<<":"<<line<<"]"<<log_msg;
  return std::cout;
}

#define LOG(lev,msg) Log(lev,__FILE__,__LINE__,msg)
