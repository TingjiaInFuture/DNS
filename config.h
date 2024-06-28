#ifndef CONFIG_H
#define CONFIG_H

void config_load(const char* filename);
const char* config_get_upstream_dns_ip();
int config_get_cache_size();
const char* config_get_log_level();
int config_get_server_port();

#endif // CONFIG_H
