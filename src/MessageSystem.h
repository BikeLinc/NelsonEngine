#ifndef MESSAGE_SYSTEM_H
#define MESSAGE_SYSTEM_H

#include "Nelson.h"

const unsigned int WINDOW_EVENT = 0;
const unsigned int CONSOLE_EVENT = 1;
const unsigned int RENDER_EVENT = 2;
const unsigned int EDITOR_EVENT = 3;
const unsigned int INPUT_EVENT = 4;
const unsigned int ENGINE_EVENT = 256;

class Message {
public:
        Message(std::vector<unsigned int> types, const std::string event) {
                messageTypes = types;
                messageEvent = event;
        }

        std::string getEvent() {
                return messageEvent;
        }

        std::vector<unsigned int> getTypes() {
                return messageTypes;
        }
private:
        std::vector<unsigned int> messageTypes;
        std::string messageEvent;
};

class MessageBus {
public:
        MessageBus() {};
        ~MessageBus() {};

        void addReceiver(std::function<void(Message)> messageReceiver) {
                receivers.push_back(messageReceiver);
        }

        void sendMessage(Message message) {
                messages.push(message);
        }

        void notify() {
                while (!messages.empty()) {
                        for (auto iter = receivers.begin(); iter != receivers.end(); iter++) {
                                (*iter)(messages.front());
                        }

                        messages.pop();
                }
        }
private:
        std::vector<std::function<void(Message)>> receivers;
        std::queue<Message> messages;
};

class System {
public:
        System(std::vector<unsigned int> types, MessageBus* messageBus) {
                this->types = types;
                this->messageBus = messageBus;
                this->messageBus->addReceiver(this->getNotifyFunc());
        }
        virtual void update() abstract;
protected:
        MessageBus* messageBus;

        std::function<void(Message)> getNotifyFunc() {
                auto messageListener = [=](Message message) -> void {
                        if (doesCommonElementExits(this->types, message.getTypes())) {
                                this->onNotify(message);
                        }
                };
                return messageListener;
        }

        void postMessage(Message message) {
                messageBus->sendMessage(message);
        }

        virtual void onNotify(Message message) {
                std::cout << "onNotify() is not implemented for: " << message.getEvent();
        }

        bool doesCommonElementExits(std::vector<unsigned int> const& inVectorA, std::vector<unsigned int> const& nVectorB)
        {
                return std::find_first_of(inVectorA.begin(), inVectorA.end(),
                        nVectorB.begin(), nVectorB.end()) != inVectorA.end();
        }
private:
        std::vector<unsigned int> types;

        
};

#endif // MESSAGE_SYSTEM_H
