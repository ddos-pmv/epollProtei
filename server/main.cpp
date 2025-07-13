#include <sys/socket.h>

#include <iostream>

#include "server.h"

int main(int argc, char *argv[])
{
  if (argc < 0)
  {
    std::exit(2);
  }
  try
  {
    protei::Server server(9641);
  }
  catch (...)
  {
  }

  // server.start();

  return 0;
}