#ifndef __CUSTOMDEF_H__
#define __CUSTOMDEF_H__

#define LOG_SERIAL(fmt, ...) do { \
    char _buf[256]; \
    snprintf(_buf, sizeof(_buf), fmt, ##__VA_ARGS__); \
    Serial.println(_buf); \
} while (0)


#endif /* __CUSTOMDEF_H__ */