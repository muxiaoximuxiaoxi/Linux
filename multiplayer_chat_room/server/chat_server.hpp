#pragma once 
#include<iostream>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<unistd.h>
#include<pthread.h>
#include<string>


#include"message.hpp"
#include"connect_info.hpp"
#include"msg_pool.hpp"
#include"log_svr.hpp"
#include"user_manager.hpp"
#define  TCP_PORT 10001
#define  UDP_PORT 10000
#define  PRODUCER_COUNT 2
//服务端的一个聊天类
//实现：1.接收客户端数据
//      2.发送数据给客户端
//依赖udp协议实现
class chat_server
{
  public:
    chat_server()
    {
      udp_sock = -1;
      udp_port = UDP_PORT;
      msg_pool_ = NULL;
      tcp_port = TCP_PORT;
      tcp_sock = -1;
      user_man = NULL;
    }
    ~chat_server()
    {
      if(msg_pool_)
      {
        delete msg_pool_;
        msg_pool_ = NULL;
      }
      if(user_man)
      {
        delete user_man;
        user_man = NULL;
      }
    }

    //上层调用init_server函数初始化udp
    void init_server()
    {
      //1.创建udp套接字
      udp_sock = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
      if(udp_sock < 0)
      {
        LOG(FATAL,"socket")<<std::endl;
        exit(1);
      }
      //2.绑定udp地址信息
      struct sockaddr_in addr;
      addr.sin_family = AF_INET;
      addr.sin_port = htons(udp_port);
      addr.sin_addr.s_addr = inet_addr("0.0.0.0");

      int ret = bind(udp_sock,(struct sockaddr*)&addr,sizeof(addr));
      if(ret < 0)
      {
        LOG(FATAL,"bind addr")<<std::endl;
        exit(2);
      }
     //初始化数据池
      msg_pool_ = new msg_pool;
      if(!msg_pool_)
      {
        LOG(FATAL,"msg_pool")<<std::endl;
        exit(3);
      }
      LOG(INFO,"udp init_server success")<<std::endl;
      
      user_man = new user_manager();
      if(!user_man)
      {
        LOG(FATAL,"user_manager ")<<std::endl;
        exit(4);
      }
      //创建tcp-socket 
      tcp_sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
      if(tcp_sock < 0)
      {
        LOG(FATAL,"tcp_sock")<<std::endl;
        exit(4);
      }

      struct sockaddr_in tcp_addr;
      tcp_addr.sin_family = AF_INET;
      tcp_addr.sin_port = htons(tcp_port);
      //tcp_addr.sin_addr.s_addr = inet_addr("0.0.0.1");

      tcp_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
      ret = bind(tcp_sock,(struct sockaddr*)&tcp_addr,sizeof(tcp_addr));
      if(ret < 0)
      {
        LOG(FATAL,"bind tcp addrinfo")<<std::endl;
        exit(5);
      }

      // ret = listen(tcp_sock,5);
      ret = listen(tcp_sock,6);
      if(ret < 0)
      {
        LOG(FATAL,"tcp listen")<<std::endl;
        exit(6);
      }
      LOG(INFO,"tcp listen 0.0.0.1:10001")<<std::endl;
    
    }
    //初始化程序当中生产和消费线程
    void start()
    {
      pthread_t tid;
      for(int i = 0;i < PRODUCER_COUNT;i++)
      {
        int ret = pthread_create(&tid,NULL,product_msg_start,(void*)this);
          if(ret < 0)
          {
            LOG(FATAL,"pthread_create product_msg_start")<<std::endl;
            exit(4);
          }
          //printf("product_msg_start success\n");
        ret = pthread_create(&tid,NULL,consume_msg_start,(void*)this);
          if(ret < 0)
          {
            LOG(FATAL,"pthread_create consume_msg_start")<<std::endl;
            exit(4);
          }
          //printf("consume_msg_start success\n");
      }
      LOG(INFO,"udp start success")<<std::endl;
      while(1)
      {
        struct sockaddr_in cliaddr;
        socklen_t cliaddrlen = sizeof(cliaddr);
        int new_sock = accept(tcp_sock,(struct sockaddr*)&cliaddr,&cliaddrlen);
        
        if(new_sock <0)
        {
          LOG(ERROR,"accept new connect failed")<<std::endl;
          continue;
        }


        login_connect* lc = new login_connect(new_sock,(void*)this);
        if(!lc)
        {
          LOG(ERROR,"create login_connect failed")<<std::endl;
          continue;
        }
        //创建线程，处理登录和注册请求
       pthread_t tid;
       int ret = pthread_create(&tid,NULL,Login_Reg_Start,(void*)lc);
       if(ret < 0)
       {
         LOG(ERROR,"create user login connect thread failed")<<std::endl;
         continue; 
       }
      LOG(INFO,"create tcp connect thread success")<<std::endl;
      }
    }

    static void* product_msg_start(void* arg)
    {
      pthread_detach(pthread_self());
      chat_server* cs = (chat_server*)arg;
      while(1)
      {
        //将接收到的消息缓存到数据池中
        cs->recv_msg();
       //printf("recv_msg\n");
      }
      return NULL;
    }

    static void* consume_msg_start(void* arg)
    {
      pthread_detach(pthread_self());
      
      chat_server* cs  = (chat_server*)arg;
      while(1)
      {
        //从数据池中获取数据
       // printf("many_send_msg\n");
        cs->many_send_msg();
      }
      return NULL;
    }

    static void* Login_Reg_Start(void* arg)
    {
      pthread_detach(pthread_self());
      login_connect* lc = (login_connect*)arg;
      chat_server* cs = (chat_server*)lc->get_server();
      //注册，请求登录
      //  请求从客户端来，recv(sock,buf,size,0)
      char rues_type;
      ssize_t recv_size = recv(lc->get_tcp_sock(),&rues_type,1,0);
      if(recv_size < 0)
      {
        LOG(ERROR,"recv rues_type failed")<<std::endl;
        return NULL;
      }
      else if(recv_size == 0)
      {
        LOG(ERROR,"client shutdowan connect")<<std::endl;
        return NULL;
      }

      uint32_t userId = -1;
      int userStatus = -1;
      //正常接收到一个请求标识
      switch(rues_type)
      {
          case REGISTER:
            //用户管理模块--注册
            userStatus = cs->deal_register(lc->get_tcp_sock(),&userId);
            break;
          
          case LOGIN:
            //用户管理模块--登录
            userStatus = cs->deal_login(lc->get_tcp_sock());
            break;

          case LOGINOUT:
            //用户管理模块--退出登录
            userStatus = cs->deal_login_out();
            break;
          
          default:
            LOG(ERROR,"recv request typt default")<<std::endl;
            break;
      }
      
      //  响应send（sock，buf，size，0)
      reply_info ri;
      ri.status = userStatus;
      ri.user_id = userId; 
      ssize_t send_size = send(lc->get_tcp_sock(),&ri,sizeof(ri),0);
      if(send_size < 0)
      {
        //如果发送数据失败，是否考虑应用层重新发送
        LOG(ERROR,"send F")<<std::endl;
      }
      LOG(INFO,"send success")<<std::endl;

      //将tcp连接释放掉
      close(lc->get_tcp_sock());
      delete lc;
      return NULL;
    }
 
    int deal_register(int sock,uint32_t* userId)
    {
      //接收注册请求，
      reg_info ri;
      ssize_t recv_size = recv(sock,&ri,sizeof(ri),0);
      if(recv_size < 0)
      {
        LOG(ERROR,"recv rues_type failed")<<std::endl;
        return ON_LINE;
      }
      else if(recv_size == 0)
      {
        LOG(ERROR,"client shutdowan connect")<<std::endl;
        //特殊处理，对端关闭情况
      }
      //调用用户管理模块，进行注册请求的处理
      int ret = user_man->Register(ri.name_,ri.from_,ri.passwd_,userId);
      //返回注册成功之后，用户的userid
      if( ret == -1 )
      {
        return REGISTER_F;
      }
      return REGISTER_T;
      //返回当前状态
    }
    int deal_login(int sock)
    {
      struct login_info li;
      ssize_t recv_size = recv(sock,&li,sizeof(li),0);

      if(recv_size < 0)
      {
        LOG(ERROR,"recv rues_type failed")<<std::endl;
        return OFF_LINE;
      }
      else if(recv_size == 0)
      {
        LOG(ERROR,"client shutdowan connect")<<std::endl;
        //需要处理
      }
      int ret = user_man->Login(li.user_id,li.passwd_);
     if(ret == -1)
     {
       return LOGIN_F;
     }
     return LOGIN_T;
    }
    int deal_login_out()
    {

    }
  private:
    //接收数据
    void recv_msg()
    {
      char buf[10240]={0};
      struct sockaddr_in cli_addr;
      socklen_t cli_addr_len = sizeof(struct sockaddr_in);
      int recv_size = recvfrom(udp_sock,buf,sizeof(buf)-1,0,(struct sockaddr*)&cli_addr,&cli_addr_len);
      if(recv_size < 0)
      {
        LOG(ERROR,"recvfrom msg")<<std::endl;
      }
      else 
      {
        //正常逻辑
        std::string msg;
        msg.assign(buf,recv_size);
        LOG(INFO,msg)<<std::endl;
        //需要将发送的数据将json格式的数据转化成我们可以识别的格式
        message json_msg;
        json_msg.deseruakuze(msg);
        //增加用户管理，只有注册且登录的人才可以发送消息
        //1.查看当前消息是否是注册用户或者老用户
        //  1.1不是，非法消息
        //  1.2 是，是否是第一次发送消息
        //         是，保存地址信息，并更新状态为在线，将数据放到数据池中
        //         否，直接将数据放到数据池中
        //
        //2.校验，则需要和用户管理交互
        //
       bool ret = user_man->is_login(json_msg.get_user_id(),cli_addr,cli_addr_len);

       //if(ret == true)
       //按道理是非真
       if(ret == true)
         
       {
       LOG(ERROR,"discarded the msg")<<" "<<msg<<std::endl;
       return;
       }
       //printf("push_msg_pool success\n"); 
       LOG(INFO,"push_msg_pool ")<<std::endl;
       msg_pool_->push_msg_pool(msg);
      }
    }

    //给一个客户端发送单个消息
    void send_msg(const std::string& msg,struct sockaddr_in& cli_addr,socklen_t& len)
    {
      ssize_t send_size = sendto(udp_sock,msg.c_str(),msg.size(),0,(struct sockaddr*)& cli_addr,len);
      LOG(INFO,"---------------\n");
      if( send_size < 0 )
      {
        
        //printf("send_msg failed\n");
        LOG(ERROR,"sendto msg")<<std::endl;
        //没有发送成功，考虑是否需要缓存没有发送成功的信息，还有客户端地址
      }
      else 
      {
        //printf("send_msg success\n");
        //成功
        LOG(INFO,"send_msg success")<<"【"<<inet_ntoa(cli_addr.sin_addr)<<":"<<":"<<ntohs(cli_addr.sin_port)<<"】"<<msg<<std::endl;
      }
    }

    // 
    void many_send_msg()
    {
      //1.获取给哪个用户发送
      std::string msg;
      msg_pool_->pop_msg_pool(&msg);
      //2.获取发送内容
      //3.调用send_msg()接口

      LOG(INFO,"----many_send_msg----\n");
      //用户管理系统提供在线的用户列表
      std::vector<user_info> online_user_v;
      user_man->get_user_vectot_info(&online_user_v);
      //int i=1;
      auto it = online_user_v.begin();
      for(;it != online_user_v.end();++it)
      {
        //send_msg()接口
        //send_msg(msg,it->get_cli_addr(),it->get_cli_addr_len());
        //++i;
        //printf("many_send_msg success %d\n",i);

        LOG(INFO,"---for---\n");
        send_msg(msg,it->get_cli_addr(),it->get_cli_addr_len());
      }
    }
  private:
    int udp_sock;//保存udp端口
    int udp_port;
    //数据池
    msg_pool* msg_pool_;
    
    //tcp处理注册，登录
    int tcp_port;
    int tcp_sock;
    
    //用户管理
    user_manager* user_man;

};

