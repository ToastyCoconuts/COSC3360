#include <iostream>
#include <string>

int main()
{
  std::string str;
  std::cout << "Please enter a string to find its memory address: ";

  std::cin >> str;

  for (int i = 0; i < str.length(); ++i)

    std::cout << "Address of " << str[i] << " is :" << (void *)&str[i] << std::endl;

  std::cout << std::endl;
  std::string str2;
  std::cout << "Please enter a string to find its memory address: ";
  std::cin >> str2;
  std::cout << std::endl;
  for (int i = 0; i < str2.length(); ++i)

    std::cout << "Address of " << str2[i] << " is :" << (void *)&str2[i] << std::endl;
  std::string temp = str;
  str = str2;
  std::cout << std::endl;
  for (int i = 0; i < str.length(); ++i)

    std::cout << "Address of " << str[i] << " is :" << (void *)&str[i] << std::endl;

  str2 = temp;
  std::cout << std::endl;
  for (int i = 0; i < str.length(); ++i)

    std::cout << "Address of " << str2[i] << " is :" << (void *)&str2[i] << std::endl;

  return 0;
}