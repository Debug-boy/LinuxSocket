#include <iostream>
#include <functional>
#include <memory>
#include <optional>
#include <exception>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

namespace infinity::socket{
        
        constexpr signed int SOCKET_ERROR = 0xffffffff;
        constexpr signed int SOCKET_CLOSE = 0x7fffffff;

        [[maybe_unused]] ssize_t send(int32_t socket_fd,void* buffer,size_t length){
            auto *local_buffer = (unsigned char*)buffer;
            while(length){
                /*
                 * linux下当连接断开，还发数据的时候，不仅send()的返回值会有反映，而且还会像系统发送一个异常消息，如果不作处理，系统会出BrokePipe，程序会退出。
                 * 为此，send()函数的最后一个参数可以设MSG_NOSIGNAL，禁止send()函数向系统发送异常消息。
                 * 11111
                 */
                ssize_t real_length = ::send(socket_fd,local_buffer,length,MSG_NOSIGNAL);
                if(real_length < 0)
                    return socket::SOCKET_ERROR;//"unknow exception!"
                else if(real_length == 0)
                    return socket::SOCKET_CLOSE;//target is close scoket!
                local_buffer = local_buffer + real_length;
                length = length - real_length;
            }
            return local_buffer - (unsigned char*)buffer;
        }

        [[maybe_unused]] ssize_t recv(int32_t socket_fd,void* buffer,size_t length){
            auto *local_buffer = (unsigned char*)buffer;
            while(length){
                size_t real_length = ::recv(socket_fd,local_buffer,length,0);
                if(real_length < 0)
                    return socket::SOCKET_ERROR;//"unknow exception!"
                else if(real_length == 0)
                    return socket::SOCKET_CLOSE;//"target is close scoket!
                local_buffer = local_buffer + real_length;
                length = length - real_length;
            }
            return local_buffer - (unsigned char*)buffer;
        }

        struct pack_message{
            signed int flag;
            char buffer[1024];
        };

        struct client_information{
            signed int fd{};
            struct{
                sockaddr_in address{};
                socklen_t address_len{};
            }net;
        };

        class socket_error : public std::exception{
            private:
                std::string m_error_msg;
            public:

                explicit socket_error(const std::string &_error_msg) noexcept(true){
                    this->m_error_msg = _error_msg;
                }

                explicit socket_error(const char* _error_msg) noexcept(true){
                    this->m_error_msg = _error_msg;
                }

            public:
                [[nodiscard]] const char *what() const noexcept(true) override{
                    return this->m_error_msg.c_str();
                }
        };
    
        class CServer{
        private:
                std::optional<signed int>m_fd;
                ::sockaddr_in m_sockaddr{};
                socket::client_information m_target_client{};

            public:
                using data_callback = std::function<bool(void* buffer,size_t length)>;

            public:

                CServer(){
                    m_sockaddr.sin_family = AF_INET;
                    m_sockaddr.sin_addr.s_addr = INADDR_ANY;
                    m_sockaddr.sin_port = htons(6666);
                }

                explicit CServer(const sockaddr_in &_sock_addr) noexcept(true){
                    m_sockaddr = _sock_addr;
                }

                //@parameter _protocol:AF_INET or AF_INET6
                //@parameter _address: please use htonl transformation 32bit address!
                //@parameter _port: please usee htons transformation 16bit port
                CServer(unsigned short _protocol,unsigned int _address,unsigned short _port) noexcept(true){
                    m_sockaddr.sin_family = _protocol;
                    m_sockaddr.sin_addr.s_addr = _address;
                    m_sockaddr.sin_port = _port;
                }

                //no need transformation!
                CServer(unsigned short _protocol,const char* normal_address,unsigned short normal_port) noexcept(true){
                    m_sockaddr.sin_family = _protocol;
                    m_sockaddr.sin_addr.s_addr = inet_addr(normal_address);
                    m_sockaddr.sin_port = htons(normal_port);
                }

                ~CServer(){
                    this->close_server();
                };

            public:
                void init_server() noexcept(false){

                    if((m_fd = ::socket(AF_INET,SOCK_STREAM,IPPROTO_IP)) == SOCKET_ERROR){
                        throw socket_error(std::string("server init create socket() error code:-1"));
                    }
                
                    if(::bind(m_fd.value(),(const sockaddr*)(&m_sockaddr),sizeof(sockaddr_in)) == SOCKET_ERROR){
                        throw socket_error(std::string("server bind error code:-1"));
                    }

                    if(::listen(m_fd.value(),0x10) == SOCKET_ERROR){
                        throw socket_error(std::string("server listen error code:-1"));
                    }

                    std::cout<<"wait client handshake is successful!"<<std::endl;
                    m_target_client.fd = ::accept(m_fd.value(),(sockaddr*)(&m_target_client.net.address),(&m_target_client.net.address_len));
                    if(m_target_client.fd == SOCKET_ERROR){
                        throw socket::socket_error("cient connect socket is error!");
                    }
                }

                ssize_t send(void *buffer,size_t length) const noexcept(false){
                    auto reSize = socket::send(m_target_client.fd,buffer,length);
                    if(reSize == SOCKET_ERROR){
                        throw socket::socket_error(std::string("server send is SOKCET_ERROR!"));
                    }
                    return reSize;
                }

                ssize_t recv(void*buffer,size_t length) const noexcept(false){
                    auto reSize = socket::recv(m_target_client.fd,buffer,length);
                    if(reSize == socket::SOCKET_ERROR){
                        throw socket::socket_error(std::string("server recv data is SOKCET_ERROR!"));
                    }else if(reSize == socket::SOCKET_CLOSE){
                        throw socket::socket_error(std::string("client socket is close!"));
                    }
                    return reSize;
                }
                
                template <typename T>
                void recv_callback(const CServer::data_callback& callback) noexcept(false){
                    std::unique_ptr<T>wise_ptr(new T());
                    while(true){
                        auto reSize = socket::recv(m_target_client.fd,(void*)(wise_ptr.get()),sizeof(T));
                        if(reSize == SOCKET_CLOSE){
                            wise_ptr.release();
                            throw socket::socket_error(std::string("client socket is close!"));
                        }

                        if(!callback((void*)(wise_ptr.get()),reSize)){
                            wise_ptr.release();
                            break;
                        }
                    }
                }

                [[nodiscard]] bool online() const{
                    signed int hert_value = INT32_MAX;
                    return socket::send(m_target_client.fd,(void*)(&hert_value),sizeof(int32_t)) != 0;
                }

                void close_server() const noexcept(true){
                    close(m_target_client.fd);
                    close(m_fd.value());
                }
            
            public:

                [[nodiscard]] unsigned int get_reg_addr()const noexcept(true){
                    return m_sockaddr.sin_addr.s_addr;
                }

                [[nodiscard]] const char* get_reg_addr_cstr()const noexcept(true){
                    return inet_ntoa(m_sockaddr.sin_addr);
                }

                [[nodiscard]] unsigned short get_reg_port()const noexcept(true){
                    return m_sockaddr.sin_port;
                }

                [[nodiscard]] unsigned int get_client_addr()const noexcept(true){
                    return m_target_client.net.address.sin_addr.s_addr;
                }

                [[nodiscard]] const char* get_client_addr_cstr()const noexcept(true){
                    return inet_ntoa(m_target_client.net.address.sin_addr);
                }

        };

        class CClient{
        private:
            signed int m_fd{};
            ::sockaddr_in m_reg_server{};

        public:
            CClient(unsigned int server_address,unsigned short normal_port) noexcept(true){
                m_reg_server.sin_addr.s_addr = server_address;
                m_reg_server.sin_port = htons(normal_port);
            }

            CClient(const char *server_address,unsigned short normal_port) noexcept(true){
                m_reg_server.sin_addr.s_addr = inet_addr(server_address);
                m_reg_server.sin_port = htons(normal_port);
            }

            explicit CClient(const sockaddr_in &sock_address) noexcept(true){
                m_reg_server = sock_address;
            }

        public:
            void init() noexcept(false){
                m_fd = ::socket(AF_INET,SOCK_STREAM,IPPROTO_IP);
                if(m_fd == SOCKET_ERROR){
                    throw socket::socket_error("client socket create is failed!");
                }
            }

            void connect() noexcept(false){
                if(::connect(m_fd,(sockaddr*)(&m_reg_server),sizeof(sockaddr_in)) == SOCKET_ERROR){
                    throw socket::socket_error("client connect server is failed,error code:-1");
                }
            }

            ssize_t send(void *buffer,size_t length) const noexcept(false){
                auto reSize = socket::send(m_fd,buffer,length);
                if(reSize == SOCKET_ERROR){
                    throw socket::socket_error(std::string("server send is SOKCET_ERROR!"));
                }
                return reSize;
            }

            ssize_t recv(void*buffer,size_t length) const noexcept(false){
                auto reSize = socket::recv(m_fd,buffer,length);
                if(reSize == SOCKET_ERROR){
                    throw socket::socket_error(std::string("server recv data is SOKCET_ERROR!"));
                }else if(reSize == SOCKET_CLOSE){
                    throw socket::socket_error(std::string("client socket is close!"));
                }
                return reSize;
            }

            template <typename T>
            std::optional<ssize_t>send(T &buffer)const noexcept(true){
                ssize_t reSize = socket::send(m_fd,(void*)(&buffer),sizeof(T));
                if(reSize > 0)return reSize;
                return {};
            }
        };
    }