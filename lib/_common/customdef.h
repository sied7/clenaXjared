#ifndef __CUSTOMDEF_H__
#define __CUSTOMDEF_H__

#define LOG_SERIAL(fmt, args) do { \
    char _buf[256]; \
    vsnprintf(_buf, sizeof(_buf), fmt, args); \
    Serial.println(_buf); \
} while (0)

typedef enum {
    RET_STATUS_OK = 0,
    RET_STATUS_ERROR = -1,
    RET_STATUS_INVALID_PARAM = -2,
    RET_STATUS_ALREADY_EXISTS = -3,
    RET_STATUS_NOT_FOUND = -4,
    RET_STATUS_OUT_OF_BOUNDS = -5,
} ret_status_t;


#endif /* __CUSTOMDEF_H__ */