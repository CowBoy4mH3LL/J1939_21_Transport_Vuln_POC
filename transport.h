// /*
// For sender:: 
//     free --[sent rts]--> initiated --[recieved CTS]--> established + (send data) --[sent packets/bytes or recv eoma]--> free;
// For reciever::
//     free --[recv rts]--> (allocate/reallocate, send cts) + established + (recv data) --[recf packets/bytes]--> free;
// */
// enum state{FREE, INITIATED, ESTABLISHED};
// enum action{SENT_RTS, RECV_RTS, RECV_CTS, TERM}
// // /*
// // free -
// // */
// // enum conn_state{FREE, SENT_RTS, SENT_CTS, RECV_CTS};

// #define TPCM 0x00ec0000
// #define TPCM_MSG 0x18ECFF00
// #define TPDT 0x00eb0000
// #define RQST 0x00ea0000
// #define RTS 0x10
// #define CTS 0x11
// #define SRC_MASK 0x000000ff
// #define DST_MASK 0x0000ff00
// #define PF_MASK  0x00ff0000

// void create_or_update_session(action act){

//     if (connection_infos[src] == NULL){
//         connection_infos[src] = malloc(sizeof(struct ConnectionInfo));
//         connection_infos[src]->state = RCVD_RQST;
//     }

//     if (connection_infos[src]->state == CONN_ESTABLISHED) /* Vuln 3. */
//         return;

//     size = (int)ceil((float)size/7)*7; //scale size so we can fit the FF's and buffer overflows cannot be caused by sending less than 7 bytes of data in a single frame
//     if (connection_infos[src]->state == SENT_CTS){
//         /* Vuln 2. */
//         connection_infos[src]->data = realloc(connection_infos[src]->data, size);
//     }
//     else{
//         // if (connection_infos[src]->data != NULL) // Patch for Vuln 1.
//         connection_infos[src]->data = malloc(size); /* Vuln 1. */
//     }

//     memset(connection_infos[src]->data , 0, size);

//     connection_infos[src]->type = type;
//     connection_infos[src]->data_pos = 0;
//     connection_infos[src]->state = state;
//     connection_infos[src]->pgn = pgn;
//     connection_infos[src]->size = size;
//     connection_infos[src]->num_packets = num_packets;
//     connection_infos[src]->recv_num_packets = 0;
//     connection_infos[src]->sender_src = src;
//     print_session(src);

// }
