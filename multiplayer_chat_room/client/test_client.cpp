#include"chat_client.hpp"
#include"chat_window.hpp"

void menu()
{
  std::cout<<"********************************"<<std::endl;

  std::cout<<"1:register               2.login"<<std::endl;

  std::cout<<"                                "<<std::endl;

  std::cout<<"3.logout                 4.exit "<<std::endl;

  std::cout<<"********************************"<<std::endl;
}

int main(int argc,char* argv[])
{
  if(argc != 2)
  {
    printf("./out_client [ip]\n");
    exit(1);
  }

  chat_client* cc = new chat_client(argv[1]);
  //初始化
  cc->init_socket();

  while(1)
  {
    menu();
    int cur = -1;
    printf("please enter your choose\n");
    fflush(stdout);
    std::cin>>cur;
    if(cur == 1)
    {
      //cc->Register();
      //注册
      if(!cc->Register())
      {
        std::cout<<"Register failed please try again"<<std::endl;
      }
      else 
      {
        std::cout<<"Register success please login"<<std::endl;
      }
    }   
    else if(cur == 2)
    {
      //登录
      if(!cc->Login())
      {
        std::cout<<"login failed please try again"<<std::endl;
      }
      else 
      {
        std::cout<<"login success please chat "<<std::endl;
        while(1)
        {
          std::cout<<"please enter#";
          fflush(stdout);
          std::string stdin_msg;
          std::cin>>stdin_msg;
          Json::Value v;
          v["name_"] = "1";
          v["from_"] = "1";
          v["msg"] = stdin_msg;
          v["user_id"] = 0;
          Json::FastWriter w;
          std::string msg;
          //strcpy(msg,stdin_msg.c_str());
          //msg = stdin_msg; 
          msg = w.write(v);
          cc->send_msg(msg);
          cc->recv_msg(&msg);
         
          cc->recv_msg(&msg);
          cc->send_msg(msg);
          printf("%s\n",msg.c_str());
      
        }
    
      }   
    }
    else if(cur == 3)
    {

      //退出登录
    }

    else if(cur == 4)
    {
      break;
      //退出
    }  
  }
  delete cc;
  return 0;
}
