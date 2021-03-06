#ifndef NODE_MQTT_H
#define NODE_MQTT_H

#include <IPAddress.h>
#include <functional>

typedef std::function<void(char *payload)> mqtt_callback_t;

void mqtt_topic(char *buf, int buf_len, const char *name);
const char * mqtt_tmp_topic(const char *name);
void mqtt_setup(void);
void mqtt_update_server(const IPAddress& new_mqtt_server, int new_mqtt_port);
void mqtt_disconnect(void);
bool mqtt_connected(void);
void mqtt_loop(void);
void mqtt_publish_float(const char *name, float val);
void mqtt_publish_str(const char *name, const char *val);
void mqtt_publish_bool(const char *name, bool val);
void mqtt_publish_int(const char *name, int val);
void mqtt_subscribe(const char *name, mqtt_callback_t cb);

#endif
