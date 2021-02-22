#pragma  once 
#include<iostream>
#include<string>
#include<cstring>



#define REGISTER 0
#define LOGIN 1
#define LOGINOUT 2
//约定双方发送udp数据包的最大值
#define  MES_MAX_SIZE 1024

//注册信息
struct reg_info
{
  char name_[15];
  char from_[15];
  char passwd_[15];
};

//登录信息
struct login_info
{
  uint32_t user_id;//返回给用户的id号
  char passwd_[15];
};

//用户状态
enum U_S
{
  REGISTER_F = 0, //注册失败
  REGISTER_T,     //注册成功
  LOGIN_F,        //登录失败
  LOGIN_T         //登录成功
};
//应答信息
struct reply_info
{
  //当前状态，注册完成，登录完成
  int status;
  uint32_t user_id;

};


class login_connect
{
  public:
    login_connect(int sock,void* server)
    {
      sock_ = sock;
      server_ = server;
    }
    int get_tcp_sock()
    {
      return sock_;
    }
    void* get_server()
    {
      return server_;
    }
  private:
    int sock_;
    //可以保存chat_server类的实例化指针
    void* server_;
};






