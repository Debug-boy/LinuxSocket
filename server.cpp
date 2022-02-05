#include "socket.hpp"
#include "touch.hpp"
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

        socket::pack_message local_pack{};
        local_pack.flag = 32767;
        strcpy(local_pack.buffer,"please touch screen!");
        m_server.send((void*)(&local_pack),sizeof(socket::pack_message));

        //wait client touch screen........
        if(touch::touch_init()){

            local_pack.flag = 16384;
            strcpy(local_pack.buffer,"get touch devices is successful!");
            m_server.send((void*)(&local_pack),sizeof(socket::pack_message));
        }

        socket::CServer::data_callback f_callback = [&](void*buffer,size_t length)->bool{
            auto *local_pack = reinterpret_cast<socket::pack_message*>(buffer);
            std::cout<<"client["<<m_server.get_client_addr_cstr()<<"]:"<<local_pack->buffer;
            std::cout<< local_pack->x1 <<" "<< local_pack->y1 <<" "<< local_pack->x2 <<" "<< local_pack->y2 <<endl;
            if(local_pack->flag == 1024){
                touch::touch_down(9,local_pack->x1,local_pack->y1);
                touch::touch_move(9,local_pack->x2,local_pack->y2);
                touch::touch_up(9);
            }
            return local_pack->flag != 65535;
        };
        m_server.recv_callback<socket::pack_message>(f_callback);
        m_server.close();
    }catch(const socket::socket_error &e){
        std::cout<<e.what()<<std::endl;
    }

    std::cout<<"main thread is normal exit!"<<std::endl;
    return 0;
}