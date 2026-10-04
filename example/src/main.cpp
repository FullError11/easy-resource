#include "erc/erc.h"
#include <iostream>
#include <string>

ERC_IMPORT(example_txt, "example.txt");

int main() {
    std::cout << "easy resource: "
        << "\n\tname: " << erc::example_txt.name()
        << "\n\tsize: " << erc::example_txt.size()
        << "\n\tpath: " << erc::example_txt.path()
        << "\n\tcontent: "
        << "\n" << std::string(20, '=')
        << "\n" << std::string(reinterpret_cast<const char*>(erc::example_txt.data().data()), erc::example_txt.data().size())
        << "\n" << std::string(20, '=')
        << std::endl;
}
