#include <czmq.h>


typedef struct _device_t {
    zhashx_t *device_zhash;
} device_t;

static void
s_free (void **x_p) {
    if (*x_p) {
        free ((uint32_t*) *x_p);
        *x_p = NULL;
    }
}


device_t * 
device_new()
{
  device_t *self = (device_t *) zmalloc (1 * sizeof(device_t));
  assert(self);
  self -> device_zhash =  zhashx_new();
  zhashx_set_destructor (self->device_zhash, s_free);
  assert(self -> device_zhash);
  return self;
}

void 
device_destroy(device_t **self_p)
{

  assert(self_p);
  
  if (*self_p){
    device_t * self = *self_p;
    zhashx_destroy(&self->device_zhash);
    free(self);
    *self_p = NULL;
  }
  
}

uint32_t device_add (device_t *self, const char *dev)
{
    assert (self);

    uint32_t *uvalue_p = (uint32_t*) zhashx_lookup (self->device_zhash, dev);

    if (!uvalue_p) {
        uvalue_p = (uint32_t*) malloc (sizeof (uint32_t));
        *uvalue_p = 1;
        zhashx_update (self->device_zhash, dev, (void*) uvalue_p);
    }
    else {
        *uvalue_p += 1;
    }

    return *uvalue_p;
}


//TODO
//
// // Vrat aktualni pocet zarizeni, 0 kdyz zariezni neni
// uint32_t device_get (device_t *self, const char *dev);

void device_test ()
{
    device_t * self = device_new ();

    /*
    zsys_info ("add dev1: %"PRIu32, device_add (self, "dev1"));
    zsys_info ("add dev1: %"PRIu32, device_add (self, "dev1"));
    zsys_info ("add dev1: %"PRIu32, device_add (self, "dev1"));
    zsys_info ("add dev1: %"PRIu32, device_add (self, "dev1"));
    zsys_info ("add dev1: %"PRIu32, device_add (self, "dev1"));
    
    zsys_info ("add dev2: %"PRIu32, device_add (self, "dev2"));
    zsys_info ("add dev2: %"PRIu32, device_add (self, "dev2"));
    zsys_info ("add dev2: %"PRIu32, device_add (self, "dev2"));
    zsys_info ("add dev2: %"PRIu32, device_add (self, "dev2"));
    
    zsys_info ("add dev1: %"PRIu32, device_add (self, "dev1"));
    */
    assert (device_add (self, "dev1") == 1);
    assert (device_add (self, "dev1") == 2);
    assert (device_add (self, "dev2") == 1);
    assert (device_add (self, "dev2") == 2);
    assert (device_add (self, "dev2") == 3);


    /*
    assert (device_get (self, "dev1") == 2);
    assert (device_get (self, "dev2") == 3);
    assert (device_get (self, "dev3") == 0);
     */

    device_destroy (&self);
}

int main () {
    device_test ();
    
    /*
    zhashx_t *m = zhashx_new ();
    assert (m);

    uint32_t * count_p = (uint32_t*) malloc (sizeof (uint32_t));
    *count_p = 13;

    zhashx_update (m, "DEV1", (void*) count_p);

    count_p = zhashx_lookup (m, "DEV1");
    *count_p += 1;
    zhashx_update (m, "DEV1", (void*) count_p);

    count_p = zhashx_lookup (m, "DEV1");
    zsys_debug ("res=%"PRIu32, *count_p);

    zhashx_destroy (&m);
    */

}
