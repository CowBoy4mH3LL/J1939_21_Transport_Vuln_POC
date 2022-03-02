// /* Handles the system state machine and makes related calls to modular challenge problem functions */

// #include "can_j1939.h"
// #include "transport.h"
// #include "common.h"
// #include "conf.h"

// void terminate_gracefully(int sig, siginfo_t *info, void *vp){
//     if (sig == SIGINT || sig == SIGABRT || sig == SIGTERM){
//         printf("Done. Entering terminal state!!\n");
//         state = STATE_TERMINAL;
//     }
// }

// void setup(char *iface_type, char *iface_num){

//     initiate_signals(terminate_gracefully);

//     //Setup CAN
//     can_setup(iface_type, iface_num);

//     //Setup everything else
//     transport_setup();
// }

// void loop() {
//     if (can_read(0) > 0){
//         parse_J1939();
//         transport_handler();
//     }
// }

int main(int argc, char * argv[]){
    // Check parameters
    if (argc < 3) {
        printf("Usage %s [can interface type] [can interface number]\n", argv[0]);
        return 1;
    }

    setup(argv[1], argv[2]);

//     while (state != STATE_TERMINAL){
//         loop();
//     }

//     can_takedown();
//     transport_takedown();

    printf("Exiting!!");
    return 0;
}