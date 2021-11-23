#include <btp.h>

std::map<std::string, node> mac_t {
    {"A4:CF:12:54:DD:D4", node_a},
    {"3C:71:BF:EA:B6:A8", node_b},
    {"30:AE:A4:15:1C:0C", node_c},
    {"A4:CF:12:9F:06:08", node_d}};

std::map<node, String> node_names = {
    {root, "root"},
    {node_a, "node A"},
    {node_b, "node B"},
    {node_c, "node C"},
    {node_d, "node D"}};

std::map<node, std::vector<node>> tree_init {
    {node_a, {root, node_b, node_c}},
    {node_b, {node_a, node_d}},
    {node_c, {node_a}},
    {node_d, {node_b}}};

std::map<instruc, String> instructions {
    {LED, "Change LED state"},
    {HELLO, "Hello"},
    {HELLO_BACK, "Hello to you too!"},
    {REQUEST, "Request"},
    {REQUEST_OK, "Request_OK"},
    {NOT_YOUR_SON, "I'm not your son anymore!"},
    {YOUR_SON, "I'm your son now"},
    {NOT_YOUR_SON_OK,"Okey"},
    {YOUR_SON_OK,"Okey,son"}
    };

bool Node::connectWifi()
{
    uint8_t count = 0;
    Serial.print("Connecting to WiFi.");
    vTaskDelay(1000);

    WiFi.begin(NET, PASS);

    while (WiFi.status() != WL_CONNECTED)
    {
        if (count < 14)
            Serial.print(".");
        else if (count == 14)
            return false;
        count++;
        vTaskDelay(1000);
    }

    ip_id = mac_t.find(WiFi.macAddress().c_str())->second;

    return true;
}

bool Node::setIP()
{
    IPAddress ip(172, 20, 10, ip_id);
    IPAddress gateway(172, 20, 10, 14);
    IPAddress subnet(255, 255, 255, 240);

    if (!WiFi.config(ip, gateway, subnet))
        return false;

    return true;
}

bool Node::setTree()
{
    monkey = (tree_init.find((node)ip_id)->second)[MONKEY];

    for (int i = 1; i < (tree_init.find((node)ip_id)->second).size(); i++)
        banana.push_back((node)(tree_init.find((node)ip_id)->second)[i]);
    return true;
}

void Node::startUDP()
{
    udp.begin(PORT);
}

int8_t Node::getID()
{
    return ip_id;
}

String Node::getIP()
{
    return (WiFi.localIP().toString());
}

node Node::getMonkey()
{
    return monkey;
}

void Node::setMonkey(node newMonkey)
{
    monkey = newMonkey;
}


std::vector<node> Node::getBananas()
{
    return banana;
}

void Node::deleteBanana(node oldBanana){


for(int i = 0;i < banana.size();i++){


    if(this->getBananas().at(i) == oldBanana){
    this->banana.erase(banana.begin()+i);  
    }
     
}

}
void Node::newBanana(node newBanana){
    this->banana.push_back(newBanana);
}

String Node::getNames(node n)
{
    return node_names.find(n)->second;
}

String Node::getSSID()
{
    return WiFi.SSID();
}

String Node::getInstruction(instruc i)
{
    return instructions.find(i)->second;
}

void Node::createPacket(uint8_t *packet, node to, node from, node tarzan, instruc instruction)
{
    memset(packet,0,5);
    packet[FROM] = from;
    packet[TO] = to;
    packet[MESSAGE] = instruction;
    packet[TARZAN] = tarzan;
}

bool Node::checkTree(node c)
{
    for (int i = 0; i < this->getBananas().size(); i++)
    {
        if (c == this->getBananas().at(i))
            return true;
    }
    if (c == this->getMonkey())
        return true;
    return false;
}

bool Node::checkTree(char c)
{
    return this->checkTree((node)(c - 96));
}

void Node::sendPacket(node from, node to, node tarzan, instruc instruction)
{
    uint8_t packet[5], pac[5];

    pac[FROM] = from;
    pac[MESSAGE] = instruction;
    pac[TO] = to;
    pac[TARZAN] = tarzan;

    DATA_S.push_front(pac);

    if (!DATA_S.empty())
    {
        uint8_t *data = DATA_S.back();
        DATA_S.pop_back();

        packet[FROM] = data[FROM] + 48;
        packet[MESSAGE] = data[MESSAGE] + 48;
        packet[TO] = data[TO] + 48;
        packet[TARZAN] = ip_id + 48;
        Serial.println("****************************************");
        Serial.println("Created packet.");
        Serial.print("From ");
        Serial.println(getNames((node)(packet[FROM] - 48)));
        Serial.print("To ");
        Serial.println(getNames((node)(packet[TO] - 48)));
        Serial.print("Message:  ");
        Serial.println(getInstruction((instruc)(packet[MESSAGE] - 48)));
        Serial.print("Tarzan:  ");
        Serial.println(getNames((node)(packet[TARZAN] - 48)));
        Serial.println("");

        
        
        if (this->checkTree((node)data[TO]))
        {
            IPAddress ip(172, 20, 10, data[TO]);
            udp.beginPacket(ip, PORT);
            udp.write(packet, 5);
            udp.endPacket();

            if (data[MESSAGE] == NOT_YOUR_SON_OK)
                deleteBanana((node)(data[TO]));
        }
        else
        {
            for (int i = 0; i < this->getBananas().size(); i++)
            {
                if (this->getBananas().at(i) != data[TARZAN])
                {
                    
                    IPAddress ip(172, 20, 10, getBananas().at(i));
                    udp.beginPacket(ip, PORT);
                    udp.write(packet, 5);
                    udp.endPacket();
                }
            }
            if (this->getMonkey() != root && this->getMonkey() != data[TARZAN])
            {
                
                IPAddress ip(172, 20, 10, this->getMonkey());
                udp.beginPacket(ip, PORT);
                udp.write(packet, 5);
                udp.endPacket();
            }
        }
    }
}

void Node::sendPacket(uint8_t *data)
{
    this->sendPacket((node)data[FROM], (node)data[TO], (node)data[TARZAN], (instruc)data[MESSAGE]);
}

WiFiUDP Node::getUDP()
{
    return udp;
}

ret_t Node::readPacket(uint8_t *par)
{
    uint8_t data[5] = {0}, size = 0;
    size = udp.parsePacket();

    if (size > 0)
    {
        udp.read(data, 5);
        strcpy((char *)par, (char *)data);
        udp.flush();
        if ((node)(data[TO] - 48) == ip_id)
            return KEEP;
        else if (banana.size()> 0)
            return FOWARD;
        else
            return IGNORE;
    }
    return EMPTY;
}

void Node::scanNetworks()
{
    int n = WiFi.scanNetworks();
    Serial.println("scan done");
    if (n == 0)
    {
        Serial.println("no networks found");
    }
    else
    {
        Serial.print(n);
        Serial.println(" networks found");
        for (int i = 0; i < n; ++i)
        {
            // Print SSID and RSSI for each network found
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.print(WiFi.SSID(i));
            Serial.print(" (");
            Serial.print(WiFi.RSSI(i));
            Serial.print(")");
            Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
            delay(10);
        }
    }
    Serial.println("");
}

int8_t Node::calcTree()
{

    int8_t route_c[N_NODES];

    srand(time(NULL));

    /* generate secret number between 1 and 10: */

    for (int i = 0; i < N_NODES; i++)
    {
        if (i + 1 != getID())
        {
            route_c[i] = rand() % 10 + 1;
        }
        else
        {
            route_c[i] = 0;
        }
    }

    if (std::distance(route_c, std::max_element(route_c, route_c + N_NODES)) + 1 == getMonkey())
    {
        return -1;
    }
    else
    {
        return (std::distance(route_c, std::max_element(route_c, route_c + N_NODES)) + 1);
    }
}