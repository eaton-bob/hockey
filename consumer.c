#include <malamute.h>


static const char *CLIENT_ENDPOINT = "tcp://192.168.2.160:7042";
static const char *BROKER_ENDPOINT = "tcp://*:7042";

int main () {

    zactor_t *server = zactor_new (mlm_server, "Malamute");
    zstr_sendx (server, "VERBOSE", NULL);
    zstr_sendx (server, "BIND", BROKER_ENDPOINT, NULL);

    
    //client part
    mlm_client_t *client = mlm_client_new ();
    mlm_client_connect (client, CLIENT_ENDPOINT, 5000, "consumer");
    mlm_client_set_consumer (client, "HOCKEY", ".*");

    while (!zsys_interrupted) {
        zmsg_t *msg = mlm_client_recv (client);
        zmsg_print (msg);
        zmsg_destroy (&msg);
    }

    mlm_client_destroy (&client);
    //client part END

    zactor_destroy (&server);


}
