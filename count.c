#include <malamute.h>

// this is fully abstract name
// inproc - transport is available only within this process
static const char *ENDPOINT = "inproc://@/eaton-bob-counter";
static const char *STREAM = "HOCKEY";

static const char *DEVICES[] = {"UPS1", "UPS2", "EPDU1", "EPDU2", "EPDU3"};

static void
s_producer (zsock_t *pipe, void *args)
{
    //producer part
    mlm_client_t *producer = mlm_client_new ();
    mlm_client_connect (producer, ENDPOINT, 5000, "producer");
    mlm_client_set_producer (producer, STREAM);

    zsock_signal (pipe, 0);
    while (!zsys_interrupted)
    {
        zmsg_t *msg = zmsg_new ();
        const char *device = DEVICES [random () % 4];
        zmsg_pushstr (msg, device);                          // name of device
        zmsg_pushstr (msg, "realpower");                     // type of metric
        zmsg_pushstrf (msg, "%"PRIu32, (uint32_t) random () % 100); // value

        char *subject;
        asprintf (&subject, "realpower@%s", device);
		mlm_client_send (producer, subject, &msg);
        zstr_free (&subject);
		zclock_sleep (1000);
    }

    mlm_client_destroy (&producer);

}


int main () {

    srandom (time (NULL));

    // create Malamute broker
    zactor_t *server = zactor_new (mlm_server, "Malamute");
    zstr_sendx (server, "BIND", ENDPOINT, NULL);

    // create producer actor
    zactor_t *producer = zactor_new (s_producer, NULL);

    // consumer part
    mlm_client_t *consumer = mlm_client_new ();
    mlm_client_connect (consumer, ENDPOINT, 5000, "consumer");
    mlm_client_set_consumer (consumer, STREAM, ".*");

    while (!zsys_interrupted) {
        zmsg_t *msg = mlm_client_recv (consumer);

        if (!msg)
            break;

        zmsg_print (msg);
        zmsg_destroy (&msg);
    }

    mlm_client_destroy (&consumer);
    zactor_destroy (&producer);
    zactor_destroy (&server);

}
