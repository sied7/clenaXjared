#ifndef __LOGGER_H__
#define __LOGGER_H__

#define SEV_EMERG   0
#define SEV_ALERT   1
#define SEV_CRIT    2
#define SEV_ERR     3
#define SEV_WARNING 4
#define SEV_NOTICE  5
#define SEV_INFO    6
#define SEV_DEBUG   7

#define LOGD(msg, ...) printlog(SEV_DEBUG, PSTR("[%s:%d] " msg), __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define LOGI(msg, ...) printlog(SEV_INFO, PSTR("[%s:%d] " msg), __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define LOGW(msg, ...) printlog(SEV_WARNING, PSTR("[%s:%d] " msg), __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define LOGE(msg, ...) printlog(SEV_ERR, PSTR("[%s:%d] " msg), __FUNCTION__, __LINE__, ##__VA_ARGS__)

void logger_init(const char *processName);
void printlog(uint16_t severity, const char *msg, ...);

#endif /* __LOGGER_H__ */