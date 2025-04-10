#include <iostream>
#include <zmq.h>
#include <stdio.h>
#include <queue>
#include <windows.h>


int main(int argc, char** argv) {

    setlocale(LC_ALL, "rus");

    void* context = zmq_ctx_new();
    void* receiver = zmq_socket(context, ZMQ_PULL);
    zmq_bind(receiver, "tcp://*:5050");

    std::queue<std::string> paths;

    int count = 0;

    for (;;) {

        zmq_msg_t reply;

        char buffer[256];
        int size = zmq_recv(receiver, buffer, 255, 0);
        if (size == -1)
            return NULL;
        if (size > 255)
            size = 255;
        buffer[size] = '\0';

        if (buffer[0] == '0')
            count++;
        else
            paths.push(buffer);

        if (count == 2)
            break;        

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