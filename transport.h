#ifndef TRANSPORTLAYERISSUES_TRANSPORT_H
#define TRANSPORTLAYERISSUES_TRANSPORT_H

#include <stdlib.h>
#include <stdint.h>


#define TPCM 0x00ec0000
#define TPCM_MSG 0x18ECFF00
#define TPDT 0x00eb0000
#define RQST 0x00ea0000
#define RTS 0x10
#define CTS 0x11
#define SRC_MASK 0x000000ff
#define DST_MASK 0x0000ff00
#define PF_MASK  0x00ff0000

/*
For sender:: 
    free --[sent rts]--> initiated --[recieved CTS]--> established + (send data) --[sent packets/bytes or recv eoma]--> free;
For reciever::
    free --[recv rts]--> (allocate/reallocate, send cts) + established + (recv data) --[recf packets/bytes]--> free;
*/
typedef enum {
    IDLE,
    INIT,
    SNT_EST,
    RCV_EST
} State;
State curr_state = IDLE;

typedef enum {
    SND_RTS,
    RCV_RTS,
    RCV_CTS,
    TERM
} Action;

struct ConnectionInfo{
    // enum state state; //1 -> Sent RTS, 2 -> Sent CTS, 3 -> Connection established
    // uint32_t pgn;
    // uint16_t size;
    // uint8_t num_packets;
    // uint8_t recv_num_packets;
    uint8_t* data;
    uint16_t data_pos;
    uint8_t sender_src;
};

uint8_t num_devices; //to be set by user arg
struct ConnectionInfo** connection_infos; //init in setup with num_devices

void delete_conn_obj(uint8_t src){
    if (connection_infos[src]->data != NULL){
        free(connection_infos[src]->data);
        connection_infos[src]->data = NULL;
    }
    free(connection_infos[src]);
    connection_infos[src] = NULL;
}

void create_conn_obj(uint8_t other, uint16_t num_bytes){
    connection_infos[other] = (ConnectionInfo*)malloc(sizeof(struct ConnectionInfo));
    connection_infos[other]->data = (uint8_t *)malloc(num_bytes);
}

void create_or_update_session(Action act, uint8_t other){ //other can be sender or to be reciever, depending on the what this device is
    switch (act){
        case SND_RTS:
            break;
        case RCV_RTS:
            switch (curr_state){
                case IDLE:
                    break;
                case INIT:
                    connection_infos[src]->data = realloc(connection_infos[src]->data, size);
                    break;
                default:
                    break;
            }
            break;
        case RCV_CTS:
            switch (curr_state){
                case INIT:
                    curr_state = SNT_EST;
                    break;
                case SNT_EST:
                    //reset data pos in conn info to requested position
                    break;
                default:
                    break;
            }
            break;
        case TERM:
            delete_conn(other);
            curr_state = IDLE;
            break;
        default:
            break;
    }

    // if (connection_infos[src] == NULL){
    //     connection_infos[src] = malloc(sizeof(struct ConnectionInfo));
    //     connection_infos[src]->state = RCVD_RQST;
    // }

    // if (connection_infos[src]->state == CONN_ESTABLISHED) /* Vuln 3. */
    //     return;

    // size = (int)ceil((float)size/7)*7; //scale size so we can fit the FF's and buffer overflows cannot be caused by sending less than 7 bytes of data in a single frame
    // if (connection_infos[src]->state == SENT_CTS){
    //     /* Vuln 2. */
    //     connection_infos[src]->data = realloc(connection_infos[src]->data, size);
    // }
    // else{
    //     // if (connection_infos[src]->data != NULL) // Patch for Vuln 1.
    //     connection_infos[src]->data = malloc(size); /* Vuln 1. */
    // }

    // memset(connection_infos[src]->data , 0, size);

    // connection_infos[src]->type = type;
    // connection_infos[src]->data_pos = 0;
    // connection_infos[src]->state = state;
    // connection_infos[src]->pgn = pgn;
    // connection_infos[src]->size = size;
    // connection_infos[src]->num_packets = num_packets;
    // connection_infos[src]->recv_num_packets = 0;
    // connection_infos[src]->sender_src = src;
    // print_session(src);

}

void clean(){
    for (uint8_t i = 0; i < num_devices; i++){
        delete_conn(i);
    }
    free(connection_infos);
    connection_infos = NULL;
}

#endif //TRANSPORTLAYERISSUES_TRANSPORT_H

