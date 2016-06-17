#include <malamute.h>

// this is fully abstract name
// inproc - transport is available only within this process
static const char *ENDPOINT = "inproc://@/eaton-bob-counter";
static const char *STREAM = "HOCKEY";

#define COUNT 5
static const char *DEVICES [COUNT] = {"UPS1", "UPS2", "EPDU1", "EPDU2", "EPDU3"};

typedef struct _device_t{
  zhashx_t *device_zhash; 

} device_t;


device_t * 
device_new()
{
  device_t *self = (device_t *) zmalloc (1 * sizeof(device_t));
  assert(self);
  self -> device_zhash =  zhashx_new();
  assert(self -> device_zhash);
  return self;
}

void
device_count_up (device_t *self, char *device){
    assert(self);
    zhashx_update (self->device_zhash, device , "");

    zsys_info ("result=%zu", zhashx_size (self->device_zhash));

}

void 
device_destroy(device_t **self_p){

  assert(self_p);
  
  if (*self_p){
    device_t * self = *self_p;
    zhashx_destroy(&self->device_zhash);
    free(self);
    *self_p = NULL;
  }
  
}

static void
s_free (void **x_p) {
    if (*x_p) {
        free ((uint32_t*) *x_p);
        *x_p = NULL;
    }
}

void device_test (zhashx_t *device_map, char *dev)
{
    
    //zhashx_update (device_map, dev, (void*) count_p);
    //void *value_p = zhashx_lookup (device_map, dev);

    uint32_t *uvalue_p = (uint32_t*) zhashx_lookup (device_map, dev);

    
    zsys_debug("device: %s", dev);

    // add
    if (uvalue_p){
        *uvalue_p += 1;
        zhashx_update (device_map, dev, (void*) uvalue_p);
        zsys_info ("uvalue_p (if): -->, uvalue_p=%"PRIu32, *uvalue_p);
    }
    else {
        uint32_t *count_p = (uint32_t*) malloc (sizeof (uint32_t));
        *count_p = 1;

        zhashx_update (device_map, dev, (void*) count_p);
	uint32_t *uvalue_p = (uint32_t*) zhashx_lookup (device_map, dev);

	zsys_info ("uvalue_p (else): -->, uvalue_p=%"PRIu32, *uvalue_p);
    }      
    
    /*
    for (void *it = zhashx_first (device_map); //it ..iterace
               it != NULL;                  // dokud neni null        
	       it = zhashx_next (device_map)) // dalsi
      {
       zsys_debug ("zhash_cursor --> \t%s : %"PRIu32, zhashx_cursor (device_map), *(uint32_t*) it);
       }*/
    

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
        const char *device = DEVICES [random () % COUNT];
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
    //device_test ();
    //    return 0;
    
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

    zhashx_t * device_map = zhashx_new ();
    zhashx_set_destructor (device_map, s_free);

    while (!zsys_interrupted) {
        zmsg_t *msg = mlm_client_recv (consumer);

        if (!msg)
            break;

	zmsg_print(msg);

	// split msg 
	char *pwrval = zmsg_popstr(msg);
	char *pwrstr = zmsg_popstr(msg);
	char *dev = zmsg_popstr(msg);

	device_test(device_map,dev);

	device_count_up(pocet,dev);

	zstr_free(&dev);
	zstr_free(&pwrstr);
	zstr_free(&pwrval);
        zmsg_destroy (&msg);

    } //while

    zhashx_destroy (&device_map);  
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
