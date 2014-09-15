#include "stmacwebview.h"
#include <Cocoa/Cocoa.h>
#include <WebKit/WebView.h>
void *STMacWebViewCreate(){
    @autoreleasepool{
        NSRect rt=NSMakeRect(0.f, 0.f, 800.f, 600.f);
        WebView *view=[[WebView alloc] initWithFrame:rt frameName:nil groupName:nil];
        [view setNeedsDisplay:YES];
        return view;
    }
}

void STMacWebViewRelease(void *v){
    [(NSView *)v release];
}

void STMacWebViewSetUrl(void *v, const char *utf8Url){
    @autoreleasepool{
        WebView *view=(WebView *)v;
        [view setMainFrameURL:[NSString stringWithUTF8String:utf8Url]];
    };
}
