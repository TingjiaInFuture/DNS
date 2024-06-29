#ifndef CONFIG_H
#define CONFIG_H

void config_load(const char* filename);
const char* config_get_dns_db_file();
int config_get_cache_size();
const char* config_get_log_level();
int config_get_server_port();
const char* config_get_external_dns_server();

#endif // CONFIG_H
