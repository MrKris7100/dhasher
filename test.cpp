#include <iostream>

using namespace std;

//do not compile this when run dhasher.cpp, this will cause compile error with duplicate main function

string username, password;

void setup(string a, string b)
{
    if (a=="-u")
    {
        username=b;
    }
    else if (a=="-p")
    {
        password=b;
    }
}

int main(int argc, char** argv)
{
    std::cout << "Have " << argc << " arguments:" << std::endl;
    for (int i = 0; i < argc; ++i)
    {
        std::cout << argv[i] << std::endl;
    }

    string arg;
    string inp;
    for (int i=1;i<argc;i+=2)
    {
        arg=argv[i];
        inp=argv[i+1];
        setup(arg, inp);
    }

    cout << "Username is: " <<username << " " <<"password is: "<<password<<endl;

    system("pause");
}
