//
// Created by subhojeet on 7/6/21.
//

#ifndef TRANSPORTLAYERISSUES_TRANSPORT_H
#define TRANSPORTLAYERISSUES_TRANSPORT_H

#include <byteswap.h>
#include <math.h>
#include "can_j1939.h"
#include "conf.h"

/* Globals */
struct ConnectionInfo{
    int type; //0 -> sending, 1 -> receiving
    int state; //1 -> Sent RTS, 2 -> Sent CTS, 3 -> Connection established
    uint32_t pgn;
    uint16_t size;
    uint8_t num_packets;
    uint8_t recv_num_packets;
    uint8_t* data;
    uint16_t data_pos;
    uint8_t sender_src;
};
struct ConnectionInfo* connection_infos[NUM_DEVS];
uint8_t next_transport_send_check = 0;
uint8_t src = SRC;
struct can_frame CTS;
struct can_frame RTS;
struct can_frame DUMMY_DATA;
uint16_t size;
uint8_t num_packets;
uint32_t pgn;


/* Support functions */

void print_session(src){
    printf ("------------ New Connection with %x at 0x%x -------------\n", src, connection_infos[src]);
    printf("connection_infos[%d]->type = %d\n", src, connection_infos[src]->type);
    printf("connection_infos[%d]->state = %d\n", src, connection_infos[src]->state);
    printf("connection_infos[%d]->pgn = %x\n", src, connection_infos[src]->pgn);
    printf("connection_infos[%d]->size = %d\n", src, connection_infos[src]->size);
    printf("connection_infos[%d]->num_packets = %d\n", src, connection_infos[src]->num_packets);
    printf("connection_infos[%d]->recv_num_packets = %d\n", src, connection_infos[src]->recv_num_packets);
    printf("connection_infos[%d]->sender_src = %x\n", src, connection_infos[src]->sender_src);
    printf("connection_infos[%d]->data at 0x%x is %x\n", src, connection_infos[src]->data, *connection_infos[src]->data);
    printf("connection_infos[%d]->data_pos = %d\n", src, connection_infos[src]->data_pos);
    printf ("--------------------------------------------------------------\n");
}

void delete_connection(src){
      if (connection_infos[src]->data != NULL){
            free(connection_infos[src]->data);
            connection_infos[src]->data = NULL;
        }
    free(connection_infos[src]);
    connection_infos[src] = NULL;
}

uint8_t** RTS_alarm_queue[((int)1250/BITWIDTH*88) + 1]; //A request is worth 88 bits
long RTS_alarm_queue_index = -1;
void alarm_handle(){
    // J1939 specifies scheduled timers here but the max limit there is 1250 ms. 
    // This means that an attacker will full CAN access i.e. able to send appx 4 frames per ma will send appx. 1250*4 = 5000 requests
    // If the data is close to 44 bytes this will result in reserving 44*1250*4 = 220 KB of heap data.
    // For systems with 256 KB of memory, and more than half reserved for program, stack frames etc. this may simply crash the system anyways
    // This alarm function works but is not a defense really.

    free(RTS_alarm_queue[RTS_alarm_queue_index]);
    RTS_alarm_queue[RTS_alarm_queue_index] = NULL;
    RTS_alarm_queue_index++;
}
void schedule_RTS_alarm(uint8_t** data_pointer){
    RTS_alarm_queue_index++;
    if (RTS_alarm_queue_index >= ((int)1250/BITWIDTH*88) + 1)
        RTS_alarm_queue_index = 0;
    RTS_alarm_queue[RTS_alarm_queue_index] = data_pointer;
}

void create_or_update_session(int state, int type){
    if (connection_infos[src] == NULL){
        connection_infos[src] = malloc(sizeof(struct ConnectionInfo));
        connection_infos[src]->state = 0;
    }

    if (connection_infos[src]->state == 3) /* Vuln 3. */
        return;

    size = (int)ceil((float)size/7)*7; //scale size so we can fit the FF's and buffer overflows cannot be caused by sending less than 7 bytes of data in a single frame
    if (connection_infos[src]->state == 2){
        /* Vuln 2. */
        connection_infos[src]->data = realloc(connection_infos[src]->data, size);
    }
    else{
        // if (connection_infos[src]->data != NULL) // Patch for Vuln 1.
        connection_infos[src]->data = malloc(size); /* Vuln 1. */
    }

    memset(connection_infos[src]->data , 0, size);

    connection_infos[src]->type = type;
    connection_infos[src]->data_pos = 0;
    connection_infos[src]->state = state;
    connection_infos[src]->pgn = pgn;
    connection_infos[src]->size = size;
    connection_infos[src]->num_packets = num_packets;
    connection_infos[src]->recv_num_packets = 0;
    connection_infos[src]->sender_src = src;
    print_session(src);

}

uint16_t check_pgn_size(){ //Returns the size if the PGN is available, else 0
    return 44; //TODO change the default later
}

/* Main functions */
void transport_setup(){

    memset(connection_infos, 0, NUM_DEVS*sizeof(struct ConnectionInfo));
    //Init default RTS
    RTS.can_dlc = 8;
    RTS.can_id = 0x18ECFF00 | SRC;
    memset(RTS.data, 0xff, 8);
    RTS.data[0] = 0x10;

    //Init default CTS
    CTS.can_dlc = 8;
    CTS.can_id = 0x18ECFF00 | SRC;
    memset(CTS.data, 0xff, 8);
    CTS.data[0] = 0x11;
    CTS.data[2] = 1;

    //Init dummy data
    DUMMY_DATA.can_dlc = 8;
    DUMMY_DATA.can_id = 0x18EBFF00 | SRC;
    DUMMY_DATA.data[0] = 0x1;
    memcpy(&DUMMY_DATA.data[1], "ABCDEFG", 7);
}

void transport_handler(int new_frame){
    if (new_frame == 0){
        if (connection_infos[next_transport_send_check]->state == 3 && connection_infos[next_transport_send_check]->type == 0){ 
            DUMMY_DATA.data[0] = (uint8_t)(connection_infos[next_transport_send_check]->data_pos/7) + 1;
            can_write(&DUMMY_DATA, 1);
            next_transport_send_check++;
            if (next_transport_send_check == NUM_DEVS)
                next_transport_send_check = 0;
        }
        return;
    }
    //Get the sender
    src = read_frame.can_id & 0x000000ff;

    if (((read_frame.can_id & 0x0000ff00) >> 8) == SRC) { //TODO add broadcast support later
        switch (read_frame.can_id & 0x00ff0000){
            case 0x00ec0000: //TP.CM
                //Get the PGN
                memcpy (&pgn, &read_frame.data[5], 3); 
                pgn = bswap_32(pgn) >> 8;

                switch (read_frame.data[0]) {
                    case 0x10: //RTS recv, CTS send
                        //Get size and num_packets
                        memcpy(&size, &read_frame.data[1], 2);
                        size = bswap_16(size);
                        num_packets = read_frame.data[3]; /* Vuln 2. */
                        /* Patch of vuln 2.0 */
                        // num_packets = (int)ceil((float)size/7);

                        // Create a session
                        create_or_update_session(2, 1);

                        if (connection_infos[src]->state == 3){
                            printf("Duplicate connection request from SA = 0x%x. Not responding!!", src);
                            break;
                        }

                        CTS.can_id = (CTS.can_id & 0xffff00ff) | (src << 8);
                        CTS.data[1] = read_frame.data[4];
                        memcpy (&CTS.data[5], &read_frame.data[5], 3); 
                        can_write(&CTS, 1);
                        break;
                    case 0x11: //CTS recieve, connection established
                        if (connection_infos[src]->state == 1)
                            connection_infos[src]->state = 3;
                        
                        connection_infos[src]->data_pos = (read_frame.data[1] - 1)*7;
                        break;
                }
                break;
            case 0x00eb0000: //Data recieve
                if (connection_infos[src] == NULL)
                    break;
                if (connection_infos[src]->state == 2)
                        connection_infos[src]->state = 3;
                
                if (connection_infos[src]->state == 3){
                    connection_infos[src]->recv_num_packets++;
                    printf ("Recieved packet %d for connection with %d\n", connection_infos[src]->recv_num_packets, src);
                   
                    memcpy(&connection_infos[src]->data[(read_frame.data[0] -1)*7], &read_frame.data[1], 7);

                    if (connection_infos[src]->recv_num_packets >= connection_infos[src]->num_packets){
                        //Presumably pass the data to the application layer providing the size along with the data, something like this callback(connection_infos[src]->data, size) so that layer
                         printf ("Recieved all packets for connection with %d. Closing connection!!\n", connection_infos[src]->recv_num_packets, src);
                        delete_connection(src);
                    }
                    
                    /* Possible patch of Vuln 2.*/
                    // connection_infos[src]->data_pos = connection_infos[src]->data_pos + 7;
                    // if (connection_infos[src]->data_pos >= connection_infos[src]->size){
                    //     free(connection_infos[src]->data);
                    //     free(connection_infos[src]);
                    // }
                }
                break;

            case 0x00ea0000: //Request recieve; RTS sent
                //Get the PGN
                memcpy (&pgn, &read_frame.data[0], 3); 
                pgn = bswap_32(pgn) >> 8;

                //Check if PGN is available
                if ((size = check_pgn_size()) != -1){
                    // Create a session
                    create_or_update_session(1, 0); /* Vuln 1. */
                    schedule_RTS_alarm(connection_infos[src]->data);

                    //Send RTS
                    RTS.can_id = (RTS.can_id & 0xffff00ff) | (src << 8);
                    RTS.data[3] = (int)ceil((float)size/7);
                    size = bswap_16(size);
                    memcpy(&RTS.data[1], &size, 2);
                    memcpy(&RTS.data[5], &read_frame.data[0], 3);
                    can_write(&RTS, 1);
                }
                //TODO add else to send negative ack on unavailibility of PGN
                break;
        }
    }
    
}

void transport_takedown(){
    for (int i = 0; i < NUM_DEVS; i++){
        if (connection_infos[i] != NULL){
            delete_connection(i);
        }
    }
}

#endif //TRANSPORTLAYERISSUES_TRANSPORT_H
