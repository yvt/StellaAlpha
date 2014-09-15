#include "stthumbnailmanager.h"

STThumbnailManager::STThumbnailManager() :
    STRemoteImageManager()
{
    m_unusedTicketsLimit=64;
}


STThumbnailManager *STThumbnailManager::sharedManager(){
    static STThumbnailManager *manager=NULL;
    if(!manager){
        manager=new STThumbnailManager();
    }
    return manager;
}

