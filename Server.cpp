#include <iostream>
#include <zmq.h>
#include <stdio.h>
#include <queue>
#include <windows.h>
#include <cassert>


int main(int argc, char** argv) {

    setlocale(LC_ALL, "rus");

    void* context = zmq_ctx_new();
    void* receiver = zmq_socket(context, ZMQ_PULL);
    zmq_bind(receiver, "tcp://*:5050");

    zmq_pollitem_t items[2];
    /* First item refers to 0MQ socket 'socket' */
    items[0].socket = receiver;
    items[0].events = ZMQ_POLLIN;
    /* Second item refers to standard socket 'fd' */
    items[1].socket = NULL;
    items[1].events = ZMQ_POLLIN;
    /* Poll for events indefinitely */
    int rc = zmq_poll(items, 1, 10000); //ожидаем 10 секунд
    if (rc <= 0) {
        std::cout << "Сообщений не получено";
        return 0;
    }

    std::queue<std::string> paths;

    int count = 0;

    for (;;) {

        rc = zmq_poll(items, 1, 1000); //ожидаем 10 секунд
        if (rc <= 0)
            break;

        zmq_msg_t reply;

        char buffer[256];
        int size = zmq_recv(receiver, buffer, 255, 0);

        if (size == -1)
            return NULL;
        if (size > 255)
            size = 255;
        buffer[size] = '\0';

        paths.push(buffer);

        zmq_msg_close(&reply);
    }

    zmq_close(receiver);


    void* sender = zmq_socket(context, ZMQ_PUSH);
    zmq_connect(sender, "tcp://localhost:4040"); //клиент 1

    void* sender2 = zmq_socket(context, ZMQ_PUSH);
    zmq_connect(sender2, "tcp://localhost:6060"); //клиент 2

    count = 0;
    while (!paths.empty())
    {
        zmq_msg_t message;
        const char* ssend = paths.front().c_str();
        int t_length = strlen(ssend);
        zmq_msg_init_size(&message, t_length);
        memcpy(zmq_msg_data(&message), ssend, t_length);

        if (count % 2 == 0)
            zmq_msg_send(&message, sender, 0);
        else
            zmq_msg_send(&message, sender2, 0);

        zmq_msg_close(&message);
        paths.pop();
        count++;
    }

    zmq_msg_t message, message1;
    const char* ssend = "0";
    int t_length = strlen(ssend);
    zmq_msg_init_size(&message, t_length);
    memcpy(zmq_msg_data(&message), ssend, t_length);
    zmq_msg_init_size(&message1, t_length);
    memcpy(zmq_msg_data(&message1), ssend, t_length);
    memcpy(zmq_msg_data(&message1), ssend, t_length);

    zmq_msg_send(&message, sender, 0);
    zmq_msg_send(&message1, sender2, 0);

    zmq_msg_close(&message);
    zmq_msg_close(&message1);

    zmq_close(sender);
    zmq_close(sender2);

    zmq_ctx_destroy(context);

    return 0;
}