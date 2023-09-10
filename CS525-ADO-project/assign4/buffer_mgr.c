#include<stdio.h>
#include<stdlib.h>
#include "buffer_mgr.h"
#include "storage_mgr.h"
#include <math.h>

// A page frame in the buffer pool
typedef struct {
    SM_PageHandle data;  // The actual data of the page
    PageNumber pageNum;  // An identifier given to each page
    int isDirty;         // Indicates whether the page has been modified by a client
    int numUsers;        // The number of clients currently using the page
    int lastAccessTime;  // The time the page was last accessed (used by LRU algorithm)
    int accessCount;     // The number of times the page has been accessed (used by LFU algorithm)
} PageFrame;

// The size of the buffer pool
int bufferPoolSize = 0;

// The index of the last page read from disk
int rearPageIndex = 0;

// The number of pages written to disk
int writeCount = 0;

// The number of page frames added to the buffer pool
int numHits = 0;

// The index of the last page added to the buffer pool (used by CLOCK algorithm)
int clockIndex = 0;

// The index of the least frequently used page frame (used by LFU algorithm)
int lfuIndex = 0;


// Replaces a page frame in the buffer pool using the First-In-First-Out (FIFO) algorithm
extern void FIFO(BM_BufferPool *const bm, PageFrame *page)
{
    //printf("FIFO Started");
    PageFrame *Pageframes = (PageFrame *) bm->mgmtData;
    
    // Find the index of the page frame that was loaded into the buffer pool the longest time ago
    int oldestIndex = -1;
    int oldestTimestamp = rearPageIndex % bufferPoolSize;
    for (int i = 0; i < bufferPoolSize; i++) {
        
		if(Pageframes[oldestTimestamp].numUsers!=0){
			oldestTimestamp=oldestTimestamp+1;
			if(oldestTimestamp%bufferPoolSize==0)
				oldestTimestamp=0;
		}
		
			
	

		else {
			if( Pageframes[oldestTimestamp].isDirty == 1) {
				SM_FileHandle fh;
				openPageFile(bm->pageFile,&fh);
				writeBlock(Pageframes[oldestTimestamp].pageNum,&fh,Pageframes[oldestTimestamp].data);
				writeCount++;
			}
				Pageframes[oldestTimestamp].data=page->data;
				Pageframes[oldestTimestamp].isDirty=page->isDirty;
				Pageframes[oldestTimestamp].numUsers=page->numUsers;
				Pageframes[oldestTimestamp].pageNum=page->pageNum;
				break;
        
		}

    }
    
    // If the oldest page frame is dirty, write it back to disk
	/*
    if (oldestIndex != -1 && Pageframes[oldestIndex].isDirty == 1) {
        SM_FileHandle fh;
        openPageFile(bm->pageFile, &fh);
        writeBlock(Pageframes[oldestIndex].pageNum, &fh, Pageframes[oldestIndex].data);
        writeCount++;
    }
    
    // Replace the oldest page frame with the new page
    if (oldestIndex != -1) {
        Pageframes[oldestIndex].data = page->data;
        Pageframes[oldestIndex].pageNum = page->pageNum;
        Pageframes[oldestIndex].isDirty = page->isDirty;
        Pageframes[oldestIndex].numUsers = page->numUsers;
        Pageframes[oldestIndex].lastAccessTime = ++numHits;
    } */
}

extern void LFU(BM_BufferPool *const bm, PageFrame *page)
{
	//printf("LFU Started");
	PageFrame *pageFrame = (PageFrame *) bm->mgmtData;

	// Find the page frame with the lowest frequency of use (i.e. the least frequently used page)
	int leastFreqIndex = 0;
	for (int i = 1; i < bufferPoolSize; i++)
	{
		if (pageFrame[i].accessCount < pageFrame[leastFreqIndex].accessCount)
		{
			leastFreqIndex = i;
		}
	}

	// If the least frequently used page is currently in use by a client, try to find a different page
	if (pageFrame[leastFreqIndex].numUsers > 0)
	{
		for (int i = 0; i < bufferPoolSize; i++)
		{
			if (pageFrame[i].numUsers == 0 && pageFrame[i].accessCount < pageFrame[leastFreqIndex].accessCount)
			{
				leastFreqIndex = i;
			}
		}
	}

	// If the least frequently used page is dirty, write it to disk
	if (pageFrame[leastFreqIndex].isDirty == 1)
	{
		SM_FileHandle fh;
		openPageFile(bm->pageFile, &fh);
		writeBlock(pageFrame[leastFreqIndex].pageNum, &fh, pageFrame[leastFreqIndex].data);

		// Increase the numPages which records the number of writes done by the buffer manager.
		writeCount++;
	}

	// Set the least frequently used page's content to the new page's content
	pageFrame[leastFreqIndex].data = page->data;
	pageFrame[leastFreqIndex].pageNum = page->pageNum;
	pageFrame[leastFreqIndex].isDirty = page->isDirty;
	pageFrame[leastFreqIndex].numUsers = page->numUsers;
	pageFrame[leastFreqIndex].accessCount = 1;
	
	// Increment the reference count for all other pages to reflect their relative frequency of use
	for (int i = 0; i < bufferPoolSize; i++)
	{
		if (i != leastFreqIndex && pageFrame[i].numUsers == 0)
		{
			pageFrame[i].accessCount++;
		}
	}
}


extern void LRU(BM_BufferPool *const bm, PageFrame *page)
{	
	PageFrame *pageFrame = (PageFrame *)bm->mgmtData;
	int i, leastRecentIndex;
	int leastRecentTime = 0;
	// Iterate through all the page frames in the buffer pool.
	for(i = 0; i < bufferPoolSize; i++)
	{
		// If a page frame is not pinned, check if it was accessed less recently than the current least recent page frame.
		if(pageFrame[i].numUsers == 0)
		{
			leastRecentIndex = i;
			leastRecentTime = pageFrame[i].lastAccessTime;
			break;
		}
	}	

	// If page in memory has been modified (isDirty = 1), then write page to disk
	for(i=leastRecentIndex+1;i<bufferPoolSize;i++)
	{

	if(leastRecentTime > pageFrame[i].lastAccessTime )
	{
		SM_FileHandle fh;
		leastRecentTime=pageFrame[i].lastAccessTime;
		leastRecentIndex=i;
	}
	}

	if(pageFrame[leastRecentIndex].isDirty==1)
	{
		SM_FileHandle fh;
		openPageFile(bm->pageFile,&fh);
		writeBlock(pageFrame[leastRecentIndex].pageNum,&fh,pageFrame[leastRecentIndex].data);
		
	}
	
	// Set the content of the page frame to the content of the new page.
	pageFrame[leastRecentIndex].data = page->data;
	pageFrame[leastRecentIndex].pageNum = page->pageNum;
	pageFrame[leastRecentIndex].isDirty = page->isDirty;
	pageFrame[leastRecentIndex].numUsers = page->numUsers;
	pageFrame[leastRecentIndex].lastAccessTime=page->lastAccessTime;

}

extern void CLOCK(BM_BufferPool *const bm, PageFrame *page)
{
    PageFrame *pageFrame = (PageFrame *) bm->mgmtData;
    int found = 0;

    while (!found) {
        // Check if the current page frame has been referenced
        if (pageFrame[clockIndex].lastAccessTime == 1) {
            // Mark the page frame as unreferenced and move the clock pointer to the next frame
            pageFrame[clockIndex].lastAccessTime = 0;
            clockIndex = (clockIndex + 1) % bufferPoolSize;
        } else {
            // If page in memory has been modified (isDirty = 1), then write page to disk
            if (pageFrame[clockIndex].isDirty == 1) {
                SM_FileHandle fh;
                openPageFile(bm->pageFile, &fh);
                writeBlock(pageFrame[clockIndex].pageNum, &fh, pageFrame[clockIndex].data);
                // Increase the numPages which records the number of writes done by the buffer manager.
                writeCount++;
            }

            // Setting page frame's content to new page's content
            pageFrame[clockIndex].data = page->data;
            pageFrame[clockIndex].pageNum = page->pageNum;
            pageFrame[clockIndex].isDirty = page->isDirty;
            pageFrame[clockIndex].numUsers = page->numUsers;
            pageFrame[clockIndex].lastAccessTime = page->lastAccessTime;
            clockIndex = (clockIndex + 1) % bufferPoolSize;
            found = 1;
        }
    }
}

extern RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName, 
		  const int numPages, ReplacementStrategy strategy, 
		  void *stratData)
{
	bm->pageFile = (char *)pageFileName;
	bm->numPages = numPages;
	bm->strategy = strategy;

	// Memory space for reserved objects is calculated as follows: pages x page size
	PageFrame *pageBuffer = malloc(sizeof(PageFrame) * numPages);

    /// The total amount of pages in the buffer pool is known as the bufferPoolSize.
 bufferPoolSize = numPages;	

	//All pages in the buffer pool are universalized. The fields (variables) on the page have values of 0 or NULL.
	for(int idx = 0; idx < bufferPoolSize; idx++)
	{
		pageBuffer[idx].data        = NULL;
		pageBuffer[idx].pageNum     = -1;
		pageBuffer[idx].isDirty    = 0;
		pageBuffer[idx].numUsers    = 0;
		pageBuffer[idx].lastAccessTime      = 0;	
		pageBuffer[idx].accessCount      = 0;
	}

	bm->mgmtData    = pageBuffer;
	writeCount      = 0;
    clockIndex    = 0;
    lfuIndex      = 0;
	return RC_OK;
		
}

// shut the buffer pool, releasing all resources and some memory space by deleting all pages from memory and closing the buffer pool.
extern RC shutdownBufferPool(BM_BufferPool *const bm)
{
	PageFrame *pageFrame = (PageFrame *)bm->mgmtData;
	// Restore the disk with all modified or dirty pages.
	forceFlushPool(bm);

	for(int idx = 0; idx < bufferPoolSize; idx++)
	{
		// If numUsers!= 0, it signifies that a client modified the page's text, and the changes were not saved to disk.
		if(pageFrame[idx].numUsers != 0)
		{
			return RC_PINNED_PAGES_IN_BUFFER;
		}
	}
	free(pageFrame);
	bm->mgmtData    = NULL;
	return RC_OK;
}

// forceFlushPool function saves to disk all the unclean pages with numUsers = 0.
extern RC forceFlushPool(BM_BufferPool *const bm)
{
	PageFrame *pageFrame = (PageFrame *)bm->mgmtData;
	
	// Keep all modified pages (dirty pages) in memory and add them to the page file on disk.	
	for(int idx = 0; idx < bufferPoolSize; idx++)
	{
		if(pageFrame[idx].numUsers == 0 && pageFrame[idx].isDirty == 1)
		{
			SM_FileHandle fileHandle;
			// opening a disk-based page file
			openPageFile(bm->pageFile, &fileHandle);
			// Adding a block of data to the disk's page file
			writeBlock(pageFrame[idx].pageNum, &fileHandle, pageFrame[idx].data);
			pageFrame[idx].isDirty = 0;
			// Increment the numPages
			writeCount++;
		}
	}	
	return RC_OK;
}


extern RC markDirty(BM_BufferPool* const bm, BM_PageHandle* const page) {
    PageFrame* frames = (PageFrame*)bm->mgmtData;
    bool pageFound = false;

    for (int i = 0; i < bm->numPages && !pageFound; i++) {
        PageFrame* frame = &frames[i];
        if (frame->pageNum == page->pageNum) {
            pageFound = true;
            frame->isDirty = 1;
        }
    }

    return pageFound ? RC_OK : RC_ERROR;
}

extern RC unpinPage(BM_BufferPool* const bm, BM_PageHandle* const page) {
    PageFrame* frames = (PageFrame*)bm->mgmtData;
    bool pageFound = false;

    for (int i = 0; i < bufferPoolSize; i++) {
        
        if (frames[i].pageNum == page->pageNum) {
            pageFound = true;
            if (frames[i].numUsers > 0) {
                frames[i].numUsers--;
				break;
            } 
            
        }
    }

    return RC_OK;
}

extern RC forcePage(BM_BufferPool* const bm, BM_PageHandle* const page) {
    PageFrame* frames = (PageFrame*)bm->mgmtData;
    bool pageFound = false;

    for (int i = 0; i < bm->numPages && !pageFound; i++) {
        PageFrame* frame = &frames[i];
        if (frame->pageNum == page->pageNum) {
            pageFound = true;
            if (frame->isDirty) {
                SM_FileHandle fileHandle;
                openPageFile(bm->pageFile, &fileHandle);
                writeBlock(frame->pageNum, &fileHandle, frame->data);
                frame->isDirty = 0;
                bm->numPages++;

            }
        }
    }

    return pageFound ? RC_OK : RC_ERROR;
}

extern RC pinPage (BM_BufferPool *const bm, BM_PageHandle *const page, 
	    const PageNumber pageNum)
{
	PageFrame *pageFrame = (PageFrame *)bm->mgmtData;
	SM_FileHandle fHandle;
	// Checking if buffer pool is empty and this is the first page to be pinned
	if(pageFrame[0].pageNum == -1)
	{
		// Reading page from disk and initializing page frame's content in the buffer pool
		SM_FileHandle fh;
		openPageFile(bm->pageFile, &fh);
		pageFrame[0].data = (SM_PageHandle) malloc(PAGE_SIZE);
		ensureCapacity(pageNum,&fh);
		readBlock(pageNum, &fh, pageFrame[0].data);
		pageFrame[0].pageNum = pageNum;
		pageFrame[0].numUsers++;
		rearPageIndex = numHits = 0;
		pageFrame[0].numUsers = numHits;	
		pageFrame[0].accessCount = 0;
		page->pageNum = pageNum;
		page->data = pageFrame[0].data;
		
		return RC_OK;		
	}
	else
	{	
		int i;
		bool isBufferFull = true;
		
		for(i = 0; i < bufferPoolSize; i++)
		{
			if(pageFrame[i].pageNum != -1)
			{	
				// Checking if page is in memory
				if(pageFrame[i].pageNum == pageNum)
				{
					// Increasing numUsers i.e. now there is one more client accessing this page
					pageFrame[i].numUsers++;
					isBufferFull = false;
					numHits++; // Incrementing numHits (numHits is used by LRU algorithm to determine the least recently used page)

					if(bm->strategy == RS_LRU)
						// LRU algorithm uses the value of numHits to determine the least recently used page	
						pageFrame[i].lastAccessTime = numHits;
					else if(bm->strategy == RS_CLOCK)
						// numUsers = 1 to indicate that this was the last page frame examined (added to the buffer pool)
						pageFrame[i].numUsers = 1;
					else if(bm->strategy == RS_LFU)
						// Incrementing acessCount to add one more to the count of number of times the page is used (referenced)
						pageFrame[i].accessCount++;
					
					page->pageNum = pageNum;
					page->data = pageFrame[i].data;

					clockIndex++;
					break;
				}				
			} else {
				SM_FileHandle fh;
				openPageFile(bm->pageFile, &fh);
				pageFrame[i].data = (SM_PageHandle) malloc(PAGE_SIZE);
				readBlock(pageNum, &fh, pageFrame[i].data);
				pageFrame[i].pageNum = pageNum;
				pageFrame[i].numUsers = 1;
				pageFrame[i].accessCount = 0;
				rearPageIndex++;	
				numHits++; // Incrementing numHits (numHits is used by LRU algorithm to determine the least recently used page)

				if(bm->strategy == RS_LRU)
					// LRU algorithm uses the value of numHits to determine the least recently used page
					pageFrame[i].lastAccessTime = numHits;				
				else if(bm->strategy == RS_CLOCK)
					// numUsers = 1 to indicate that this was the last page frame examined (added to the buffer pool)
					pageFrame[i].numUsers = 1;
						
				page->pageNum = pageNum;
				page->data = pageFrame[i].data;
				
				isBufferFull = false;
				break;
			}
		}
		
		// If isBufferFull = true, then it means that the buffer is full and we must replace an existing page using page replacement strategy
		if(isBufferFull == true)
		{
			// Create a new page to store data read from the file.
			PageFrame *newPage = (PageFrame *) malloc(sizeof(PageFrame));		
			
			// Reading page from disk and initializing page frame's content in the buffer pool
			SM_FileHandle fh;
			openPageFile(bm->pageFile, &fh);
			newPage->data = (SM_PageHandle) malloc(PAGE_SIZE);
			readBlock(pageNum, &fh, newPage->data);
			newPage->pageNum = pageNum;
			newPage->isDirty = 0;		
			newPage->numUsers = 1;
			newPage->accessCount = 0;
			rearPageIndex++;
			numHits++;

			if(bm->strategy == RS_LRU)
				// LRU algorithm uses the value of numHits to determine the least recently used page
				newPage->lastAccessTime = numHits;				
			else if(bm->strategy == RS_CLOCK)
				// numUsers = 1 to indicate that this was the last page frame examined (added to the buffer pool)
				newPage->numUsers = 1;

			page->pageNum = pageNum;
			page->data = newPage->data;			

			// Call appropriate algorithm's function depending on the page replacement strategy selected (passed through parameters)
			switch(bm->strategy)
			{			
				case RS_FIFO: // Using FIFO algorithm
					FIFO(bm, newPage);
					break;
				
				case RS_LRU: // Using LRU algorithm
					LRU(bm, newPage);
					break;
				
				case RS_CLOCK: // Using CLOCK algorithm
					CLOCK(bm, newPage);
					break;
  				
				case RS_LFU: // Using LFU algorithm
					LFU(bm, newPage);
					break;
  				
				case RS_LRU_K:
					printf("\n LRU-k algorithm not implemented");
					break;
				
				default:
					printf("\nAlgorithm Not Implemented\n");
					break;
			}
						
		}		
		return RC_OK;
	}	
}


// This function returns a list of page numbers as an array
extern PageNumber *getFrameContents (BM_BufferPool *const bm)
{
		PageNumber *frameContents = malloc(sizeof(PageNumber) * bufferPoolSize);
		PageFrame *pageFrame = (PageFrame *) bm->mgmtData;
		int i;
// Looping through all the pages in the buffer pool and setting the frameContents value to the pageNum.
		for (int i = 0; i < bufferPoolSize; i++)  {
			if (pageFrame[i].pageNum != NO_PAGE) {
            				frameContents[i] = pageFrame[i].pageNum;
			}
			else {  

          				  frameContents[i] = NO_PAGE;
      			  }
			
    		
		}
		return frameContents;
}




// This function returns a bool array, with each element representing the dirtyBit of the corresponding page.
extern bool *getDirtyFlags (BM_BufferPool *const bm)
{
		bool *dirtyFlags = malloc(sizeof(bool) * bufferPoolSize);
		PageFrame *pageFrame = (PageFrame *)bm->mgmtData;
		int i=0;
// Iterating through all the pages in the buffer pool and changing dirtyFlags to TRUE if the page is dirty otherwise. FALSE
while(i<bufferPoolSize){
if(pageFrame[i].isDirty ==1){
dirtyFlags[i] = true;
}
else{
dirtyFlags[i]=false;
}
i++;
}
return dirtyFlags;
}
extern int *getFixCounts (BM_BufferPool *const bm)
{
		int *fixCounts = malloc(sizeof(int) * bufferPoolSize);
		PageFrame *pageFrame= (PageFrame *)bm->mgmtData;
		int i;
// Looping through all pages in the buffer pool and setting fixCounts to the page's fixCount
		for(int i=0;i<bufferPoolSize;i++){
			if(pageFrame[i].numUsers != 0){
				fixCounts[i] =pageFrame[i].numUsers;
}
else{
	fixCounts[i] = 0;
}
		}
		return fixCounts;
}



// This method returns the number of pages read from disk since a buffer pool was started.
extern int getNumReadIO (BM_BufferPool *const bm)
{
int rearValue=rearPageIndex+1;
		return (rearValue);
}


// The number of pages written to the page file since the buffer pool has been initialized is returned by this method.
extern int getNumWriteIO (BM_BufferPool *const bm)
{
int writeValue=writeCount;
		return writeValue;
}
