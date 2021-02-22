#pragma  once 
#include<iostream>
#include<string>
#include<unistd.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<stdio.h>
#include<string.h>
#include<stdio.h>
#include<netinet/in.h>

#include"../server/log_svr.hpp"
#include"../server/message.hpp"
#include"../server/connect_info.hpp"


#define MES_MAX_SIZE 1024
#define UDP_PORT 10000
#define TCP_PORT 10001

struct my_info
{
  std::string Name_;
  std::string From_;
  std::string Passwd_;

  uint32_t Uer_ID;
};


class chat_client
{

  public:
    chat_client(std::string s_ip = "127.0.0.1")
    {
      udp_sock = -1;
      udp_port = UDP_PORT;

      tcp_sock = -1;
      tcp_port = TCP_PORT;

      server_ip = s_ip;
    }
    ~chat_client()
    {
      if(udp_sock > 0)
      {
        close(udp_sock);
      }
    }

    void init_socket()
    {
      udp_sock = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
      if(udp_sock < 0)
      {
        LOG(ERROR,"client create udp_sock failed")<<std::endl;
        exit(1);
      }

    }

    bool connect_server()
    {
      tcp_sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

      if(tcp_sock < 0)
      {
        LOG(ERROR,"client create tcp_sock failed")<<std::endl;
        exit(2);
      }
      struct sockaddr_in addr;
      addr.sin_family = AF_INET;
      addr.sin_port = htons(tcp_port);
      addr.sin_addr.s_addr = inet_addr(server_ip.c_str());

      int ret = connect(tcp_sock,(struct sockaddr*)& addr,sizeof(addr));
      if (ret < 0)
      {
        perror("conect");
        LOG(ERROR,"connect server failed")<<"  server_ip:"<<server_ip<<"  tcp_port:"<<tcp_port<<std::endl;
        return false;
      }
      return true;
    }
    bool Register()
    {
      if(!connect_server())
      {
        return false;
      }

      //1.发送注册标识
      char type = REGISTER;
      ssize_t send_size = send(tcp_sock,&type,1,0);

      if(send_size < 0)
      {
        LOG(ERROR,"send Register type ")<<std::endl;
        return false;
      }
      //2.发送注册内容
      struct reg_info ri;
      std::cout<<"please enter your name:";
      std::cin>>ri.name_;

      std::cout<<"please enter your from:";
      std::cin>>ri.from_;

      while(1)
      {

        std::string p_a1;//第一次输入的密码
        std::cout<<"please enter your passwd:";
        std::cin>>p_a1;
        std::cout<<"please enter your passwd again :";
        std::string p_a2;//第二次输入的密码
        std::cin>>p_a2;
        if(p_a1 == p_a2)
        {
          strcpy(ri.passwd_,p_a1.c_str());
          break;
        }
        else 
        {
          printf("The password is not the same\n");
        }
      }

      send_size = send(tcp_sock,&ri,sizeof(ri),0);

      if(send_size < 0)
      {
        LOG(ERROR,"send Register type ")<<std::endl;
        return false;
      }
      //3.解析应答状态和获取用户id
      struct reply_info re_i;
      ssize_t re_i_size = recv(tcp_sock,&re_i,sizeof(re_i),0);
      if(re_i_size < 0)
      {
        LOG(ERROR,"re_i_size ")<<std::endl;
        return false;
      }
      else if(re_i_size == 0)
      {
        LOG(ERROR,"peer shutdown connect ")<<std::endl;
        return false;
      }

      if(re_i.status != REGISTER_T)
      {
        LOG(ERROR,"Register failed")<<std::endl;
        printf("注册失败\n");
        return false;
      }
      LOG(INFO,"Register success user_id = ")<<re_i.user_id<<std::endl;
      //printf("注册成功,user_id = %lu\n",re_i.user_id);
      //4.注册成功之后保存用户信息
      me.Name_ = ri.name_;
      me.From_ = ri.from_;
      me.Passwd_ = ri.passwd_;
      me.Uer_ID = re_i.user_id;
      close(tcp_sock);
      return true;
    }
    bool Login()
    {
      if(!connect_server())
      {
        return false;
      }

      //1.发送登录标识
      char type = LOGIN;
      ssize_t send_size = send(tcp_sock,&type,1,0);

      if(send_size < 0)
      {
        LOG(ERROR,"send login type ")<<std::endl;
        return false;
      }
      //2.发送登录数据
      struct login_info li;
      li.user_id = me.Uer_ID;
      strcpy(li.passwd_,me.Passwd_.c_str()); 
      send_size = send(tcp_sock,&li,sizeof(li),0);

      if(send_size < 0)
      {
        LOG(ERROR,"send login type ")<<std::endl;
        return false;
      }
      //3.解析登录状态
      struct reply_info re_i;
      ssize_t re_i_size = recv(tcp_sock,&re_i,sizeof(re_i),0);
      
      if(re_i_size < 0)
      {
        LOG(ERROR,"re_i_size ")<<std::endl;
        return false;
      }
      else if(re_i_size == 0)
      {
        LOG(ERROR,"peer shutdown connect ")<<std::endl;
        return false;
      }
      if(re_i.status != LOGIN_T)
      {
        LOG(ERROR,"login failed status = ")<<re_i.status<<std::endl;
        //rintf("登录失败\n");
        //printf("login status = %d\n",re_i.status);
        return false;
      }
      //printf("登录成功\n");
      //printf("login status = %d\n",re_i.status);
      LOG(INFO,"login success status = ")<<re_i.status<<std::endl;
      return true;
    }


    //udp数据收发
    bool send_msg(const std::string& msg)
    {
      struct sockaddr_in p_addr;
      p_addr.sin_family = AF_INET;
      p_addr.sin_port = htons(udp_port);
      p_addr.sin_addr.s_addr = inet_addr(server_ip.c_str());
      ssize_t send_size = sendto(udp_sock,msg.c_str(),msg.size(),0,(struct sockaddr*)&p_addr,sizeof(p_addr));

      if(send_size < 0)
      {
       // LOG(ERROR,"send msg to server failed\n");
        return false;
      }

      //LOG(INFO,"send_msg to server success\n");
      return true;
    }

    bool recv_msg(std::string* msg)
    {
      //LOG(INFO,"--------------\n");
      char buf[MES_MAX_SIZE];
      memset(buf,'\0',sizeof(buf));
      struct sockaddr_in svr_addr;
      //struct sockaddr svr_addr;
      socklen_t svr_addr_len = sizeof(svr_addr);
      // LOG(INFO,"**************\n");
      //LOG(INFO,"22222222222222\n");
      //ssize_t recv_size = recvfrom(udp_sock,buf,sizeof(buf)-1,0,(struct sockaddr*)&svr_addr,&svr_addr_len);
      ssize_t recv_size = recvfrom(udp_sock,buf,sizeof(buf)-1,0,(struct sockaddr*)&svr_addr,&svr_addr_len);
     
      
      //LOG(INFO,"000000000000\n");
     //LOG(INFO,"ssssssssssss\n");
      if(recv_size < 0)
     {
      // LOG(ERROR,"recv msg from sevser failed\n");
       return false;
     }
      //LOG(INFO,"11111111111111\n");
     (*msg).assign(buf,recv_size);
     //LOG(INFO,"recv_msg from server success\n");
     return true;
    }

    my_info& get_my_info()
    {
      return me;
    }

    std::vector<std::string>& get_online_user()
    {
      return online_user;
    }
    void push_user(std::string& user_info)
    {
      auto it = online_user.begin();
      for(;it !=online_user.end();++it)
      {
        if(*it == user_info)
          return;
      }
      online_user.push_back(user_info);
    }
  private:
    //udp正常交互
    int udp_sock;
    int udp_port;
    //tcp处理登录注册请求
    int tcp_sock;
    int tcp_port;

    //服务器ip
    std::string server_ip;

    //客户端信息
    my_info me;    
    
    //保存在线用户
    std::vector<std::string> online_user;
};
