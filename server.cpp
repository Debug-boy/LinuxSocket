#include "socket.hpp"
using namespace infinity;
using namespace std;

int main(int argc,char *argv[]){

    std::cout<<"this is server!"<<std::endl;

    sockaddr_in reg_sockaddr{};
    reg_sockaddr.sin_family = AF_INET;
    reg_sockaddr.sin_addr.s_addr = INADDR_ANY;
    reg_sockaddr.sin_port = htons(6666);

    socket::CServer m_server(reg_sockaddr);
    std::cout<<"--------server all information--------"<<endl;
    std::cout<<"address :"<<m_server.get_reg_addr_cstr()<<endl;
    std::cout<<"port    :"<<m_server.get_reg_port()<<endl;
    std::cout<<"--------------------------------------"<<endl;

    try{
        m_server.init_server();
        std::cout<<"client:["<<m_server.get_client_addr_cstr()<<"] connect."<<std::endl;
        socket::CServer::data_callback f_callback = [&](void*buffer,size_t length)->bool{
            auto *local_pack = reinterpret_cast<socket::pack_message*>(buffer);
            std::cout<<"client["<<m_server.get_client_addr_cstr()<<"]:"<<local_pack->buffer<<endl;
            return local_pack->flag != 65535;
        };
        m_server.recv_callback<socket::pack_message>(f_callback);
        m_server.close_server();

    }catch(const socket::socket_error &e){
        std::cout<<e.what()<<std::endl;
        m_server.close_server();
    }
    std::cout<<"main thread is normal exit!"<<std::endl;
    return 0;
}