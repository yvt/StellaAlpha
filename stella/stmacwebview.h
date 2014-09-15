#ifndef STMACWEBVIEW_H
#define STMACWEBVIEW_H

#ifdef __cplusplus
extern "C"{
#endif

    void *STMacWebViewCreate();
    void STMacWebViewRelease(void *v);
    void STMacWebViewSetUrl(void *, const char *utf8Url);

#ifdef __cplusplus
};
#endif

#endif // STMACWEBVIEW_H
