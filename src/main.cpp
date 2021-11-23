#include <FreeRTOS.h>
#include <WiFiUdp.h>
#include <freertos/task.h>
#include <freertos/StackMacros.h>
#include <btp.h>


#define BAUD 9600

Node n;

instruc instruction;
node destination;
node targetMonkey = n.getMonkey();
TaskHandle_t t;

uint8_t led_tog = 1;

void get_info() {
    Serial.println("+--------------------------------------+");

    Serial.println("|   Node info:");
    Serial.println("+--------------------------------------+");
    Serial.print("|   Node id: ");
    Serial.println(n.getNames((node)n.getID()));
    Serial.println("+--------------------------------------+");
    Serial.print("|   Monkey: ");
    Serial.println(n.getNames(n.getMonkey()));
    Serial.println("+--------------------------------------+");
    Serial.print("|   Bananas: ");
    for(int i = 0; i < n.getBananas().size(); i++) {
        Serial.print(n.getNames(n.getBananas()[i]));
        if(i < n.getBananas().size() - 1)
            Serial.println(", ");
    }
    Serial.println("\n+--------------------------------------+");
    Serial.print("|   IP address: ");
    Serial.println(n.getIP());
    Serial.println("+--------------------------------------+\n\n");
}

void send_task(void* pvParameters) {
    char data[5];
    data[FROM] = ((char*)pvParameters) [FROM];
    data[TO] = ((char*)pvParameters) [TO];
    data[MESSAGE] = ((char*)pvParameters) [MESSAGE];
    data[TARZAN] = ((char*)pvParameters) [TARZAN];
    
    n.sendPacket((uint8_t*)data);
    vTaskDelete(NULL);
}

void read_task(void* pvParameters) {
    ret_t ret;
    TickType_t xLastWakeTime;
    TickType_t freq_ticks = 7;  // Periodic task of 50ms
    xLastWakeTime = xTaskGetTickCount();
    uint8_t data[5];
    uint8_t packet[5];
    while(1){
        ret = n.readPacket(data);
        if(ret != EMPTY) {
            Serial.println("*************************************************");
            Serial.println("Received packet.");
            Serial.print("From ");
            Serial.println(n.getNames((node)(data[FROM] - 48)));
            Serial.print("To ");
            Serial.println(n.getNames((node)(data[TO] - 48)));
            Serial.print("Message:  ");
            Serial.println(n.getInstruction((instruc)(data[MESSAGE]- 48)));
            Serial.print("Tarzan:  ");
            Serial.println(n.getNames((node)(data[TARZAN]- 48)));
           

            if(ret == KEEP) {
                /* Process all received reliable data */
                Serial.println("Process message.");
                if((data[MESSAGE] - 48) == HELLO) {
                    Serial.println("Replying...");
                    n.createPacket(packet, (node)(data[FROM] - 48), (node)(n.getID()), (node)(n.getID()), (instruc)(HELLO_BACK));

                    xTaskCreate(send_task, "Send UDP packets", 10000, (void*)packet, configMAX_PRIORITIES - 1, NULL);
                }
                else if((data[MESSAGE] - 48) == LED) {
                    digitalWrite(LED_PIN, led_tog);
                    led_tog = !led_tog; 
                }else if((data[MESSAGE] - 48) == REQUEST){
                    n.createPacket(packet, (node)(data[FROM] - 48), (node)(n.getID()), (node)(n.getID()), (instruc)(REQUEST_OK));
                    xTaskCreate(send_task, "Send UDP packets", 10000, (void*)packet, configMAX_PRIORITIES - 1, NULL);

                }else if((data[MESSAGE] - 48) == REQUEST_OK){
                    n.createPacket(packet, n.getMonkey(), (node)(n.getID()), (node)(n.getID()), (instruc)(NOT_YOUR_SON));
                    xTaskCreate(send_task, "Send UDP packets", 10000, (void*)packet, configMAX_PRIORITIES - 1, NULL);
                    targetMonkey = (node)(data[FROM] - 48);
                }
                else if((data[MESSAGE] - 48) == NOT_YOUR_SON){

                    n.createPacket(packet, (node)(data[FROM] - 48), (node)(n.getID()), (node)(n.getID()), (instruc)(NOT_YOUR_SON_OK));
                    xTaskCreate(send_task, "Send UDP packets", 10000, (void*)packet, configMAX_PRIORITIES - 1, NULL);
                    //n.deleteBanana((node)(data[FROM] - 48));

                } else if(data[MESSAGE] - 48 == NOT_YOUR_SON_OK) {

                    n.setMonkey(targetMonkey);
                    n.createPacket(packet, n.getMonkey(), (node)(n.getID()), (node)(n.getID()), (instruc)(YOUR_SON));
                    xTaskCreate(send_task, "Send UDP packets", 10000, (void*)packet, configMAX_PRIORITIES - 1, NULL);
                }
                else if((data[MESSAGE] - 48) == YOUR_SON)
                {
                    n.newBanana((node)(data[FROM] - 48));
                }
                
            } else if(ret == FOWARD) {
                Serial.println("Foward message");
                 data[FROM] = data[FROM] - 48;
                 data[MESSAGE] = data[MESSAGE] - 48;
                 data[TO] = data[TO] - 48;
                 data[TARZAN] = data[TARZAN] - 48;
                xTaskCreate(send_task, "Send UDP packets", 10000, (void*)data, configMAX_PRIORITIES - 1, NULL);
            } else if(ret == IGNORE) {
                Serial.println("Ignore message");
            }
         Serial.println("*************************************************\n");

        }
        vTaskDelayUntil(&xLastWakeTime, freq_ticks);
    }
}

void check_serial(void* pvParameters) {
    TickType_t xLastWakeTime;
    TickType_t freq_ticks = 20;  // Periodic task of 100ms
    xLastWakeTime = xTaskGetTickCount();

    while(1) {
        
        if(Serial.available() > 0) {
            vTaskResume(t);
            vTaskDelete(NULL);
        }
        
        vTaskDelayUntil(&xLastWakeTime, freq_ticks);
    }
}

void control_task(void *pvParameters) {
    uint8_t i = 0;
    char cmd[10], c = 0;
    uint8_t packet[5];

    memset(cmd, 0, 10);
    memset(packet, 0, 5);

    while(1) {
        xTaskCreate(check_serial, "Check Serial Port", 1000, NULL, configMAX_PRIORITIES - 1, NULL);
        vTaskSuspend(NULL);
        
        i = 0;
        memset(cmd, 0, 10);

        while(Serial.available()) {
            c = Serial.read();
            if(c != '.') {
                cmd[i++] = c;
                Serial.print(c);
                xTaskCreate(check_serial, "Check Serial Port", 1000, NULL, configMAX_PRIORITIES - 1, NULL);
                vTaskSuspend(NULL);
            } 
        } 
        
        c = 0;
        Serial.println();
        memset(packet, 0, 5);

        if(!strncmp(cmd, "hello ", 6)) {
            if((cmd[6]-96) >= 1 && (cmd[6]-96) <= N_NODES && (cmd[6]-96) != n.getID()) {
                packet[FROM] = n.getID();
                packet[TO] = cmd[6] - 96; // because nodes start at 1
                packet[MESSAGE] = HELLO;
                packet[TARZAN] = n.getID();
                xTaskCreate(send_task, "Send UDP packets", 10000, (void*)packet, configMAX_PRIORITIES - 1, NULL);
            } else 
                Serial.println("Node is not in the network!");
        } else if(!strcmp(cmd, "info")) {
            get_info();

        } else if(!strncmp(cmd, "reconf ", 7)) {
             if(cmd[7] - 96 >= 1 && cmd[7] - 96 <= N_NODES && 
               (cmd[7] - 96) != n.getID() && (cmd[7]-96) != n.getMonkey() ){
                if(n.getBananas().size() == 0){

                    int8_t r = n.calcTree();

                    if(r == -1){
                        Serial.println("Tree with minimal cost");
                    } else {

                        packet[FROM] = n.getID();
                        packet[TO] = r; 
                        packet[MESSAGE] = REQUEST;
                        packet[TARZAN] = n.getID();
                        xTaskCreate(send_task, "Send UDP packets", 10000, (void*)packet, configMAX_PRIORITIES - 1, NULL);

                    }

                }else{
                Serial.println("You have no permission.");
                }
             }else {
                 Serial.println("Node not valid.");
             }
            
            
        } else if (!strncmp(cmd, "led ",4)) {
            /* between node 1 and 3 */
            if(cmd[4]-96 >= 1 && cmd[4]-96 <= N_NODES && (cmd[4]-96) != n.getID()) {
                packet[FROM] = n.getID();
                packet[TO] = cmd[4] - 96; 
                packet[MESSAGE] = LED; // test led.builtin on
                packet[TARZAN] = n.getID();
                xTaskCreate(send_task, "Send UDP packets", 10000, (void*)packet, configMAX_PRIORITIES - 1, NULL);
            } else 
                Serial.println("Node is not in the network!");
        }
    }
}

void setup() {
    // Start code
	Serial.begin(BAUD);
    Serial.println("\n***************************************");
    Serial.println("Routine start.");
    
    if(!n.connectWifi()) {
        Serial.println("\nCannot connect to WiFi. Rebooting... ");
        ESP.restart();
    }

    Serial.println("\nConnected to WiFi network.");

    if(n.setIP())
        Serial.println("=> IP well set.");
    else
        Serial.println("=> IP not well set.");

    if(n.setTree())
        Serial.println("=> Tree well set.");
    else
        Serial.println("=> Tree not well set.\n\n");

    pinMode(LED_PIN,OUTPUT);

    n.startUDP();
    
    get_info();
    
    xTaskCreate(read_task, "Read UDP packets", 10000, NULL, configMAX_PRIORITIES - 1, NULL);
    xTaskCreate(control_task, "Control", 10000, NULL, configMAX_PRIORITIES - 1, &t);
}

void loop() {

}
