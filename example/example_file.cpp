#include <iostream>
#include <fstream>

using namespace std;
int main (int){

    string s;

    ifstream inf;
    inf.open("/Users/linchengzhong/MyDocument/myCode/cppBasicTool/example/in.txt");

    //打开输出文件
    ofstream outf;
    outf.open("/Users/linchengzhong/MyDocument/myCode/cppBasicTool/example/out.txt");


    //从in.txt　文件中读入数据，并输出到out.txt中
    while( getline(inf,s ) ){
        outf << s  << '\n';
    }

    //out.txt中，跟着读下一行
    inf.close();
    outf.close();
    return 0;
}

