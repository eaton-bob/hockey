#include <malamute.h>

// this is fully abstract name
// inproc - transport is available only within this process
static const char *ENDPOINT = "inproc://@/eaton-bob-counter";
static const char *STREAM = "HOCKEY";

static const char *DEVICES[] = {"UPS1", "UPS2", "EPDU1", "EPDU2", "EPDU3"};

typedef struct _device_t{
  uint64_t ups1c;
  uint64_t ups2c;
  uint64_t epdu1c;
  uint64_t epdu2c;
  uint64_t epdu3c;
} device_t;


device_t * 
device_new()
{
  device_t *self = (device_t *) zmalloc (1 * sizeof(device_t));
  assert(self);
  //self -> ups1c = ups1c;
  return self;
}

void
device_count_up (device_t *self, char *device){
  //assert(self);
  int temp;

  if (streq(device,"UPS1")) {
    temp = self -> ups1c;
    self -> ups1c = temp + 1;
  }
  if (streq(device,"UPS2")) {
    temp = self -> ups2c;
    self -> ups2c = temp + 1;
  }
  if (streq(device,"EPDU1")) {
    temp = self -> epdu1c;
    self -> epdu1c = temp + 1;
  }

  if (streq(device,"EPDU2")) {
    temp = self -> epdu2c;
    self -> epdu2c = temp + 1;
  }

  if (streq(device,"EPDU3")) {
    temp = self -> epdu3c;
    self -> epdu3c = temp + 1;
  }

}

void 
device_destroy(device_t **self_p){
  assert(self_p);
  if (*self_p){
    device_t * self = *self_p;
    free(self);
    *self_p = NULL;
  }
}

void
device_print (device_t *self){
  assert(self);
  printf("UPS1: %i \n UPS2: %i \n EPDU1: %i \n EPDU2: %i \n EPDU3: %i \n", self -> ups1c, self -> ups2c, self -> epdu1c, self -> epdu2c, self -> epdu3c);
}

void device_test ()
{
    //zhashx example
    zhashx_t * device_map = zhashx_new ();
    zhashx_update (device_map, "UPS1", "");
    zhashx_update (device_map, "UPS2", "");
    zhashx_update (device_map, "UPS3", "");
    zhashx_update (device_map, "UPS3", "");
    zhashx_update (device_map, "UPS3", "");
    zhashx_update (device_map, "UPS3", "");
    zhashx_update (device_map, "UPS3", "");

    zsys_info ("result=%zu", zhashx_size (device_map));

    zsys_debug ("device_map:");
    for (void *it = zhashx_first (device_map);
               it != NULL;
               it = zhashx_next (device_map))
    {
        zsys_debug ("\t%s", zhashx_cursor (device_map));
    }
    
    zhashx_destroy (&device_map);
}

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
    } //while

    mlm_client_destroy (&producer);

} // producer_s

int main () {

    puts ("Running unit tests");
    device_test ();
    return 0;
    
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

    device_t *pocet = device_new();

    while (!zsys_interrupted) {
        zmsg_t *msg = mlm_client_recv (consumer);

        if (!msg)
            break;

	zmsg_print(msg);

	// split msg 
	char *pwrval = zmsg_popstr(msg);
	char *pwrstr = zmsg_popstr(msg);
	char *dev = zmsg_popstr(msg);
	
	device_count_up(pocet,dev);

	zstr_free(&dev);
	zstr_free(&pwrstr);
	zstr_free(&pwrval);
        zmsg_destroy (&msg);

    } //while

    device_print(pocet);

    device_destroy(&pocet);
    mlm_client_destroy (&consumer);
    zactor_destroy (&producer);
    zactor_destroy (&server);

} //main

    /**
     * print number of distinc devices here
     * HINT: the program never ends, type Ctrl+C to break the producer/consumer and get the result
     *       alternativelly you might want to add counter, so both will break up later
     * HINT: take a look on zhash class, which will help you to get the result
     *
     * zhash_t *count = zhash_new ();
     * zhash_autofree (count);
     *
     */

        /**
         * HINT - see zmsg_popstr and zstr_free to unpack the message
         */
