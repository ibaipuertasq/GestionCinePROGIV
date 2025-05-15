// protocol.cpp
#include "protocol.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <cstring>
#include <vector>

Message::Message(OperationCode code, const std::string& content) : opCode(code), data(content) {}

void Message::addString(const std::string& str) {
    data += str + SEPARATOR;
}

void Message::addInt(int value) {
    data += std::to_string(value) + SEPARATOR;
}

void Message::addDouble(double value) {
    data += std::to_string(value) + SEPARATOR;
}

void Message::addBool(bool value) {
    data += (value ? "1" : "0") + std::string(1, SEPARATOR);
}

std::string Message::getString() {
    size_t pos = data.find(SEPARATOR);
    if (pos == std::string::npos) {
        std::string result = data;
        data.clear();
        return result;
    }
    
    std::string result = data.substr(0, pos);
    data = data.substr(pos + 1);
    return result;
}

int Message::getInt() {
    return std::stoi(getString());
}

double Message::getDouble() {
    return std::stod(getString());
}

bool Message::getBool() {
    return getString() == "1";
}

std::string Message::serialize() const {
    return std::to_string(static_cast<int>(opCode)) + SEPARATOR + data + END_MESSAGE;
}

Message Message::deserialize(const std::string& serialized) {
    size_t pos = serialized.find(SEPARATOR);
    if (pos == std::string::npos) {
        return Message(OP_ERROR, "Malformed message");
    }
    
    OperationCode opCode = static_cast<OperationCode>(std::stoi(serialized.substr(0, pos)));
    std::string content;
    
    if (pos + 1 < serialized.length()) {
        content = serialized.substr(pos + 1);
        // Eliminar el END_MESSAGE si existe
        if (!content.empty() && content.back() == END_MESSAGE) {
            content.pop_back();
        }
    }
    
    return Message(opCode, content);
}

OperationCode Message::getOpCode() const {
    return opCode;
}

std::string Message::getData() const {
    return data;
}

void Message::clear() {
    data.clear();
}

bool Message::hasMoreData() const {
    return !data.empty();
}

bool sendMessage(int socket, const Message& msg) {
    std::string serialized = msg.serialize();
    int total = 0;
    int bytesLeft = serialized.length();
    int n;

    while (total < serialized.length()) {
        n = send(socket, serialized.c_str() + total, bytesLeft, 0);
        if (n == -1) { break; }
        total += n;
        bytesLeft -= n;
    }

    return n != -1;
}

Message receiveMessage(int socket) {
    char buffer[BUFFER_SIZE];
    std::string receivedData;
    bool messageComplete = false;
    
    while (!messageComplete) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytesReceived = recv(socket, buffer, BUFFER_SIZE - 1, 0);
        
        if (bytesReceived <= 0) {
            return Message(OP_ERROR, "Connection closed or error");
        }
        
        receivedData.append(buffer, bytesReceived);
        
        // Comprobar si hemos recibido el mensaje completo
        if (receivedData.find(END_MESSAGE) != std::string::npos) {
            messageComplete = true;
        }
    }
    
    return Message::deserialize(receivedData);
}