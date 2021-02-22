
#include"chat_server.hpp"

int main()
{
  chat_server cs;
  cs.init_server();
  cs.start();
  while(1)
  {
    sleep(1);
  }
}
