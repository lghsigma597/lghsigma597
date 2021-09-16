#include <bits/stdc++.h>
#include "test.h"

int main()
{
  auto str = "TEST";
  std::cout << "hello world: " << str << std::endl;

  auto v = test::A(1);
  std::cout << "test: " << v.get() << std::endl;

  auto x = (int *)malloc(0);
  std::cout << "malloc(0): " << x << std::endl;

  *x = 1;
  std::cout << "x: " << *x << std::endl;
  free(x);

  return 0;
}
