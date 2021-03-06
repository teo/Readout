#include "RdhUtils.h"
RdhHandle::RdhHandle(void *data) {
  rdhPtr=(o2::Header::RAWDataHeader *)data;
}

RdhHandle::~RdhHandle(){
}

void RdhHandle::dumpRdh() {
  printf("RDH @ 0x%p\n",(void *)rdhPtr);
  printf("Version       = 0x%02X\n",(int)getHeaderVersion());
  printf("Header size   = %d\n",(int)getHeaderSize());
  printf("Block length (link) = %d bytes\n",(int)getBlockLength());
  printf("Block length (memory) = %d bytes\n",(int)getMemorySize());
  printf("FEE Id        = %d\n",(int)getFeeId());
  printf("Link Id       = %d\n",(int)getLinkId());
  printf("Next block    = %d\n",(int)getOffsetNextPacket());
  //printf("%04X %04X %04X %04X\n",rdhPtr->word3,rdhPtr->word2,rdhPtr->word1,rdhPtr->word0);
}

int RdhHandle::validateRdh(std::string &err) {
  int retCode=0;
  // expecting RDH V3
  if (getHeaderVersion()!=3) {
    err+="Wrong header version\n";
    retCode++;
  }
  // expecting 16*32bits=64 bytes for header
  if (getHeaderSize()!=64) {
    err+="Wrong header size\n";
    retCode++;  
  }
  // expecting linkId 0-31
  if (getLinkId()>31) {
    err+="Wrong link ID\n";
    retCode++;  
  }
  // expecting block length <= 8kB
  if (getBlockLength()>8*1024) {
    err+="Wrong block length\n";
    retCode++;  

  }
  // check FEE Id ?  
  return retCode;
}


RdhBlockHandle::RdhBlockHandle(void *ptr, size_t size) : blockPtr(ptr), blockSize(size) {
}

RdhBlockHandle::~RdhBlockHandle() {
}

int RdhBlockHandle::printSummary() {
  printf("\n\n************************\n");
  printf("Start of page %p (%d bytes)\n\n",blockPtr,blockSize);
  
  // intialize start of block
  uint8_t *ptr=(uint8_t *)(blockPtr);
  size_t bytesLeft=blockSize;
  
  int rdhcount=0;
  
  for (;;) {
 
    // check enough space for RDH
    if (bytesLeft<sizeof(o2::Header::RAWDataHeader)) {
      printf("page too small, %d bytes left! need at least %d bytes for RDH\n",bytesLeft,(int)sizeof(o2::Header::RAWDataHeader));
      return -1;
    }

    rdhcount++;
    int offset=ptr-(uint8_t*)blockPtr;
    printf("*** RDH #%d @ 0x%04X = %d\n",rdhcount,offset,offset);

    // print raw bytes
    //printf("Raw bytes dump (32-bit words):\n");
    for (int i=0;i<sizeof(o2::Header::RAWDataHeader)/sizeof(int32_t);i++) {
      if (i%8==0) {printf("\n");}
      printf("%08X ",(int)(((uint32_t*)ptr)[i]));
    }  
    printf("\n\n");

    RdhHandle rdh(ptr);
    rdh.dumpRdh();
    printf("\n");

    int next=rdh.getOffsetNextPacket(); // next RDH
    if (next==0) {
      break;
    }
    
    // check enough space to go to next offset
    if (bytesLeft<next) {
      printf("page too small, %d bytes left! need at least %d bytes for next offset\n",bytesLeft,(int)rdh.getOffsetNextPacket());
      return -1;     
    }
    

    bytesLeft-=next;
    ptr+=next;
    if (bytesLeft==0) {
      break;
    }
  }
  
  printf("End of page %p (%d bytes)",blockPtr,blockSize);
  printf("\n************************\n\n");
    
  return 0;
}
