#ifndef __CUSTOMDEF_H__
#define __CUSTOMDEF_H__

#define LOG_SERIAL(fmt, args) do { \
    char _buf[256]; \
    vsnprintf(_buf, sizeof(_buf), fmt, args); \
    Serial.println(_buf); \
} while (0)


#endif /* __CUSTOMDEF_H__ */