
#include <iostream>

//configuration
#define ST_ASIO_SERVER_PORT		9527
#define ST_ASIO_FORCE_TO_USE_MSG_RECV_BUFFER //force to use the msg recv buffer
#define ST_ASIO_CUSTOM_LOG
#define ST_ASIO_DEFAULT_UNPACKER non_copy_unpacker
//#define ST_ASIO_DEFAULT_UNPACKER stream_unpacker

//the following three macros demonstrate how to support huge msg(exceed 65535 - 2).
//huge msg will consume huge memory, for example, if we want to support 1M msg size, because every st_tcp_socket has a
//private unpacker which has a fixed buffer with at lest 1M size, so just for unpackers, 1K st_tcp_socket will consume 1G memory.
//if we consider the send buffer and recv buffer, the buffer's default max size is 1K, so, every st_tcp_socket
//can consume 2G(2 * 1M * 1K) memory when performance testing(both send buffer and recv buffer are full).
//generally speaking, if there are 1K clients connected to the server, the server can consume
//1G(occupied by unpackers) + 2G(occupied by msg buffer) * 1K = 2049G memory theoretically.
//please note that the server also need to define at least ST_ASIO_HUGE_MSG and ST_ASIO_MSG_BUFFER_SIZE macros too.

//#define ST_ASIO_HUGE_MSG
//#define ST_ASIO_MSG_BUFFER_SIZE (1024 * 1024)
//#define ST_ASIO_MAX_MSG_NUM 8 //reduce msg buffer size to reduce memory occupation
//configuration

//demonstrate how to use custom log system:
//use your own code to replace the following all_out_helper2 macros, then you can record logs according to your wishes.
//custom log should be defined(or included) before including any st_asio_wrapper header files except st_asio_wrapper_base.h
//notice: please don't forget to define the ST_ASIO_CUSTOM_LOG macro.
#include "../include/st_asio_wrapper_base.h"
using namespace st_asio_wrapper;

class unified_out
{
public:
	static void fatal_out(const char* fmt, ...) {all_out_helper2("fatal");}
	static void error_out(const char* fmt, ...) {all_out_helper2("error");}
	static void warning_out(const char* fmt, ...) {all_out_helper2("warning");}
	static void info_out(const char* fmt, ...) {all_out_helper2("info");}
	static void debug_out(const char* fmt, ...) {all_out_helper2("debug");}
};

#include "../include/ext/st_asio_wrapper_client.h"
using namespace st_asio_wrapper::ext;

#define QUIT_COMMAND	"quit"
#define RESTART_COMMAND	"restart"
#define RECONNECT_COMMAND "reconnect"
#define SUSPEND_COMMAND	"suspend"
#define RESUME_COMMAND	"resume"

int main(int argc, const char* argv[])
{
	printf("usage: asio_client [<port=%d> [ip=%s]]\n", ST_ASIO_SERVER_PORT + 100, ST_ASIO_SERVER_IP);
	if (argc >= 2 && (0 == strcmp(argv[1], "--help") || 0 == strcmp(argv[1], "-h")))
		return 0;
	else
		puts("type " QUIT_COMMAND " to end.");

	st_service_pump service_pump;
	st_tcp_sclient client(service_pump);
	//there is no corresponding echo client, because echo server with echo client will cause dead loop, and occupy almost all the network resource

//	argv[2] = "::1" //ipv6
//	argv[2] = "127.0.0.1" //ipv4
	if (argc > 2)
		client.set_server_addr(atoi(argv[1]), argv[2]);
	else if (argc > 1)
		client.set_server_addr(atoi(argv[1]), ST_ASIO_SERVER_IP);
	else
		client.set_server_addr(ST_ASIO_SERVER_PORT + 100, ST_ASIO_SERVER_IP);

	service_pump.start_service();
	while(service_pump.is_running())
	{
		std::string str;
		std::cin >> str;
		if (QUIT_COMMAND == str)
			service_pump.stop_service();
		else if (RESTART_COMMAND == str)
		{
			service_pump.stop_service();
			service_pump.start_service();
		}
		else if (RECONNECT_COMMAND == str)
			client.graceful_close(true);
		//the following two commands demonstrate how to suspend msg sending, no matter recv buffer been used or not
		else if (SUSPEND_COMMAND == str)
			client.suspend_send_msg(true);
		else if (RESUME_COMMAND == str)
			client.suspend_send_msg(false);
		else
			client.safe_send_msg(str);
	}

	return 0;
}

//restore configuration
#undef ST_ASIO_SERVER_PORT
#undef ST_ASIO_FORCE_TO_USE_MSG_RECV_BUFFER
#undef ST_ASIO_CUSTOM_LOG
#undef ST_ASIO_DEFAULT_UNPACKER

//#undef ST_ASIO_HUGE_MSG
//#undef ST_ASIO_MAX_MSG_LEN
//#undef ST_ASIO_MAX_MSG_NUM
//restore configuration
