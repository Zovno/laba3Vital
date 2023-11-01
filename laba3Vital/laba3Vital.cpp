#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

using namespace std;

struct student
{
    std::string name;
    std::string secondName;
    std::string lastName;
    int group;
    double money;
    int marks[4];
};

std::vector<student> students;

void handle_client(int client_socket) {
    int request;

    while (true)
    {
        request = 0;
        recv(client_socket, (char*)&request, sizeof(int), 0);

        if (request == 1) 
        {
            std::cout << "1";
            std::vector<student> studentsToSend;
            for (const auto& student : students) {
                bool hasThree = false;
                for (int i = 0; i < 4; i++) {
                    if (student.marks[i] == 3) {
                        hasThree = true;
                        break;
                    }
                }
                if (!hasThree) {
                    studentsToSend.push_back(student);
                }
            }

            // Отправка количества структур
            int count = studentsToSend.size();
            send(client_socket, (char*)&count, sizeof(int), 0);
            // Отправка самих структур
            for (int i = 0; i < studentsToSend.size(); i++) 
            {
                send(client_socket, (char*)&studentsToSend[i], sizeof(student), 0);
            }
        }
        if (request == 2)
        {
            std::cout << "2";
            student* newStudent = new student;
            recv(client_socket, (char*)newStudent, sizeof(student), 0);
            students.push_back(*newStudent);
        }
        if (request == 3)
        {
            std::cout << "3";
            // Отправка всех студентов
            int count = students.size();
            send(client_socket, (char*)&count, sizeof(int), 0);
            for (const auto& student : students) {
                send(client_socket, (char*)&student, sizeof(student), 0);
            }

            // Получение индекса студента для изменения
            int index;
            recv(client_socket, (char*)&index, sizeof(int), 0);

            // Получение новых данных студента
            student* newStudent = new student;
            recv(client_socket, (char*)newStudent, sizeof(student), 0);

            // Замена старого студента новым
            if (index >= 0 && index < students.size()) 
            {
                students[index] = *newStudent;
            }
        }
        if (request == 0)
            return;
        
    }
}



int main() {
    WSADATA wsaData;
    setlocale(LC_ALL, "ru");
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Ошибка инициализации Winsock" << std::endl;
        return 1;
    }
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    int addr_len = sizeof(struct sockaddr_in);

    // Создание сокета
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Не удалось создать сокет" << std::endl;
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Привязка сокета к адресу
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Привязка сокета не удалась" << std::endl;
        return 1;
    }

    // Прослушивание сокета
    if (listen(server_socket, 3) < 0) {
        std::cerr << "Ошибка прослушивания" << std::endl;
        return 1;
    }

    std::vector<std::thread> threads;

    while (true) {
        std::cout << "Ожидание подключений..." << std::endl;

        // Принятие нового подключения
        if ((client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &addr_len)) < 0) { // Используйте &addr_len вместо (socklen_t*)&addr_len
            std::cerr << "Ошибка принятия подключения" << std::endl;
            return 1;
        }

        // Создание нового потока для обработки подключения
        threads.push_back(std::thread(handle_client, client_socket));
    }

    for (auto& thread : threads) {
        thread.join();
    }

    WSACleanup();

    return 0;
}
