//
// Created by subhojeet on 6/17/21.
// CAN functions exposed on the development branch
//

#ifndef AMP_CHALLENGE_05_CAN_LOG_ENCRYPTION_CAN_J1939_H
#define AMP_CHALLENGE_05_CAN_LOG_ENCRYPTION_CAN_J1939_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <linux/can.h>
#include <linux/can/raw.h>
#include <stdint.h>
#include <assert.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>
#include "conf.h"

#define PRIORITY_MASK  0x1C000000
#define EDP_MASK       0x02000000
#define DP_MASK        0x01000000
#define PF_MASK        0x00FF0000
#define PS_MASK        0x0000FF00
#define SA_MASK        0x000000FF
#define PDU1_PGN_MASK  0x03FF0000
#define PDU2_PGN_MASK  0x03FFFF00

/* Globals */

//Externs
struct can_frame read_frame;
//struct can_frame *write_frame;
uint8_t priority, da, sa;
uint32_t pgn;

//Tmmps
char *iface;
int nbytes = 0;
int can_socket = -1;

/* Supporting functions */
int init_can( int *sock, char *interface ) {
    struct ifreq ifr;
    struct sockaddr_can addr = {0};
    struct can_filter rfilter[2];
    int ro = LOGSELF;


    strcpy(ifr.ifr_name, interface);
    ioctl(*sock, SIOCGIFINDEX, &ifr);  // Retrieve the interface index of the interface into ifr_ifindex

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    // Filter setup : With integrated login -- we dont need filters any more
//    // Challenge 03 filters
//    rfilter[0].can_id   = 0x00FFFD00;
//    rfilter[0].can_mask = 0x00FFFF00;
//    // Challenge 04 filters
//    rfilter[1].can_id   = 0x00EA0000;
//    rfilter[1].can_mask = 0x00FF0000;

    // setsockopt(*sock, SOL_CAN_RAW, CAN_RAW_JOIN_FILTERS, &rfilter, sizeof(rfilter));
    setsockopt(*sock, SOL_CAN_RAW, CAN_RAW_RECV_OWN_MSGS, &ro, sizeof(ro));


    // We're going to be listening
    if (bind(*sock,(struct sockaddr *)&addr, sizeof(addr)) < 0) {
        return -1;
    }
    return 0;
}

/* Main functions */
void can_setup(char *iface_type, char* channel){
    iface = malloc(sizeof (iface_type) + sizeof (channel));
    sprintf(iface,"%s%s", iface_type, channel);
    can_socket = socket (PF_CAN, SOCK_RAW, CAN_RAW);
    //TODO error frame counts to be logged
    while (init_can(&can_socket, iface) != 0) {
        printf("Failed to create socket\n");
        sleep(3);
    }
    printf("Socket created\n");
}

void can_takedown(){
    free(iface);
    close(can_socket);
}

int can_write(struct can_frame *write_frame, int block){
    nbytes = 0;
    write_frame->can_id = write_frame->can_id | CAN_EFF_FLAG;
    while (block == 1 && nbytes != sizeof(struct can_frame)){
        nbytes = write(can_socket, write_frame, sizeof(struct can_frame));
        if (nbytes < 0 && errno != EINTR) // Interrupted system calls at the beginning are skipped
            error(0, "Error while writting");
    }
    return nbytes;
}

int can_read(int block){
    //int read_bytes = 0;
    nbytes = 0;
    while (block == 1 && nbytes == 0){
        nbytes = read(can_socket, &read_frame, sizeof(struct can_frame));
        if (nbytes < 0 && errno != EINTR) // Interrupted system calls at the beginning are skipped
            error(0, "Error while reading");
    }
    return nbytes;
}

void parse_J1939(){//uint32_t can_id, uint8_t *priority, uint32_t *pgn, uint8_t *da, uint8_t *sa) {
    // Parse J1939
    int pf, ps;
    // Priority
    priority = (PRIORITY_MASK & read_frame.can_id) >> 26;
    // Protocol Data Unit (PDU) Format
    pf = (PF_MASK & read_frame.can_id) >> 16;
    // Protocol Data Unit (PDU) Specific
    ps = (PS_MASK & read_frame.can_id) >> 8;
    // Determine the Parameter Group Number and Destination Address
    if (pf >= 0xF0) {
        // PDU 2 format, include the PS as a group extension
        da = 255;
        pgn = (read_frame.can_id & PDU2_PGN_MASK) >> 8;
    } else {
        da = ps;
        pgn = (read_frame.can_id & PDU1_PGN_MASK) >> 8;
    }
    // source address
    sa = (read_frame.can_id & SA_MASK);
}

#endif //AMP_CHALLENGE_05_CAN_LOG_ENCRYPTION_CAN_J1939_H
