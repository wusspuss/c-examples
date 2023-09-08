all: main lib

main:
	gcc  `pkg-config --cflags  --libs json-glib-1.0` -ldatachannel media_receiver.c -o media_receiver

lib:
	sed 's/int main()/int test()/' media_receiver.c | gcc  `pkg-config --cflags  --libs json-glib-1.0` -ldatachannel -shared -fPIC -x c - -o media_receiver.so
