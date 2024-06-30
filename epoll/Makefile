build:
# https://github.com/smaugx/epoll_examples
	g++ epoll_client.cc  -o epoll_client -std=c++11 -lpthread
	g++ epoll_server.cc  -o epoll_server -std=c++11 -lpthread

buildsimple:
	g++ simple_server.cc -o simple_server -std=c++11 -lpthread

sever:
	./epoll_server

client:
	./epoll_client
