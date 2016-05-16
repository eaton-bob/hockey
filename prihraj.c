#include <malamute.h>

static const char *CLIENT_ENDPOINT = "tcp://192.168.2.160:7042";
static const char *stream = "HOCKEY";

int main(){

	// producer
	mlm_client_t *myProducer = mlm_client_new();
	mlm_client_connect(myProducer, CLIENT_ENDPOINT, 5000, "myProducer");
	mlm_client_set_producer(myProducer,stream);

	// send
	while (!zsys_interrupted){
		for (int i = 0; i < 1000; i = i + 1){
			mlm_client_sendx(myProducer,"prihraj","prohraj","prihraj","pozde", NULL);	
			zclock_sleep(1000);
		
		}
	

	mlm_client_destroy(&myProducer);
	}
}
