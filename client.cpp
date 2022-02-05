#include "socket.hpp"
using namespace std;
using namespace infinity;

int main(int arg_count,char* arg_values[]){

    std::cout<<"this is client!"<<endl;

    sockaddr_in reg_server_info{};
    reg_server_info.sin_family = AF_INET;
    reg_server_info.sin_addr.s_addr = INADDR_ANY;
    reg_server_info.sin_port = htons(6666);

    socket::CClient client(reg_server_info);
    socket::pack_message local_pack{};

    try{
        client.init();
        client.connect();

        for(;;){
            auto reRecv = client.recv((void*)(&local_pack),sizeof(socket::pack_message));
            if(local_pack.flag == 32767){
                std::cout<<"server["<<inet_ntoa(reg_server_info.sin_addr)<<"]:"<<local_pack.buffer<<endl;
                continue;
            }else if(local_pack.flag == 16384){
                std::cout<<"server["<<inet_ntoa(reg_server_info.sin_addr)<<"]:"<<local_pack.buffer<<endl;
                break;
            }
        }

        //Session with server....
        do{
            std::cout<<"in flag and buffer:";
            std::cin>>local_pack.flag;
            std::cin>>local_pack.x1>>local_pack.y1>>local_pack.x2>>local_pack.y2;
            std::getchar();
            std::cin.getline(local_pack.buffer,1024);

            auto reSend = client.send<socket::pack_message>(local_pack);

            if(!reSend.has_value()){
                throw socket::socket_error("send is error!");
            }

        } while (local_pack.flag != 65535);

        client.close();

    }catch (const socket::socket_error &error){
        std::cout<<"[exception]"<<error.what()<<std::endl;
    }

    std::cout<<"close is socket_fd"<<endl;
    return 0;
}