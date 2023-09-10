#include "storage_mgr.h"
#include<stdio.h>
#include <stdlib.h>
#include "string.h"
#include "dberror.h"

FILE *page_fl;
extern void initStorageManager (void) {
	// Initialising file pointer to NULL for storage manager.
	page_fl = NULL;
}

RC createPageFile (char *fileName) {
    page_fl = fopen(fileName, "w");
    
    SM_PageHandle firstPage;

    if (page_fl == NULL)
    {
        return RC_FILE_NOT_FOUND;
    }
    else
    {
        firstPage = (SM_PageHandle) calloc(PAGE_SIZE, sizeof(char));   /* considered as actual first page for the data */

        if (firstPage == NULL)
        {
                printf("Cannot allocate enough memory to create page file");
        }
        else {
            fwrite(firstPage, sizeof(char), PAGE_SIZE, page_fl);
            free(firstPage);
        }
        fclose(page_fl);
    }
    return RC_OK;
}

RC openPageFile (char *fileName, SM_FileHandle *fHandle) 
{

	page_fl = fopen(fileName, "rb+");
	
    if (page_fl == NULL)
	{
	    return RC_FILE_NOT_FOUND;
	}
	else
    {
		fHandle->fileName = fileName;
		fHandle->curPagePos = 0;
		fseek(page_fl, 0, SEEK_END);
		fHandle->totalNumPages = (ftell(page_fl) / PAGE_SIZE );
		fclose(page_fl);
		return RC_OK;
		
	}
}

RC closePageFile (SM_FileHandle *fHandle) {
	// Initialised file pointer is closed.
	
	page_fl = NULL;	
	return RC_OK; 
}

// deleting from the disk
RC destroyPageFile (char *fileName) {
	// Opening file stream in read mode 'r' 
	page_fl = fopen(fileName, "r");
	
	if(page_fl == NULL)
		return RC_FILE_NOT_FOUND; 
	
	// Deleting the given filename 
	remove(fileName);
	return RC_OK;
}


RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
	int totalNumPages = fHandle->totalNumPages;
	//Checking to see if the pageNumber parameter satisfies the Total number of   pages and returning the appropriate error code
	if (pageNum < 0 || pageNum > totalNumPages)
		return RC_READ_NON_EXISTING_PAGE;

 //Opening the read-only file stream. The "r" option creates a blank file intended only for reading.
	page_fl = fopen(fHandle->fileName, "r");
	//Checking to see if the file was opened successfully.
	if(page_fl  == NULL)
		return RC_FILE_NOT_FOUND;

	/*Setting the file stream's cursor (pointer) location. Position is determined by page number multiplied by page size. 
If fseek() returns 0 then the seek has been successful */

	else{
	int isAccomplish = fseek(page_fl , (pageNum * PAGE_SIZE), SEEK_SET);
	if(isAccomplish != 0) {
		return RC_READ_NON_EXISTING_PAGE;
	} else {
	//We're reading the text and putting it where memPage included to place it.
	fHandle->curPagePos = ftell(page_fl);

		fread(memPage, sizeof(char), PAGE_SIZE, page_fl);
		
	}
	//Setting the file stream's cursor (pointer) location as the current page position
	fHandle->curPagePos = ftell(page_fl);
	// Closing the file stream to clear all of the buffers.
	fclose(page_fl);
	return RC_OK;
	}
}



//Assigning the current page position retrieved from the file handle to currentValue
 //Return the currentValue retrieved from the file handle
int getBlockPos (SM_FileHandle *fHandle) {
	int currentValue;
	currentValue =  fHandle->curPagePos;
	return (int)currentValue;
}




// Because this is an integer value, we can't return RC FILE HANDLE NOT INIT; -1 implies that there is no fHandle
RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
	
	return readBlock(0, fHandle, memPage);
}



 //using the read-current-block method
// Read the block preceding the one at the current position.
RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
	//int currentValue = fHandle->curPagePos 
	int currentData = readCurrentBlock(fHandle, memPage);
	int currentPageNum = currentData - 1 ;
	return readBlock(currentPageNum, fHandle, memPage);
}




// By dividing the page size by the current page size, the current value number is computed.
// Passing the pageNum argument to the readBlock(...) function
RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
	int currentValue = fHandle->curPagePos;
	int currentPageNum = currentValue/ PAGE_SIZE;
	return readBlock(currentPageNum, fHandle, memPage);
}


/* writing blocks to a page file */

/*
 * This method is used to write onto the block specified in the pageNum field
 * If the block is not present it gives an error RC_WRITE_FAILED
 */
RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	//check if pageNum is valid
	if(pageNum > fHandle->totalNumPages - 1 || pageNum < 0)
	{
		return RC_WRITE_FAILED;
	}
	else
	{
		page_fl = fopen(fHandle -> fileName, "w");
		//seek to specified page number
		fseek(page_fl,(pageNum)*PAGE_SIZE,SEEK_SET);

		//write to the page block
		fwrite(memPage,PAGE_SIZE,sizeof(char),page_fl);

		//update the current page block position
		fHandle->curPagePos = pageNum;
		fHandle->mgmtInfo = page_fl;

		return RC_OK;
	}

}

/*
 * This method is used to write to the block which is pointed currently.
 */
RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    //writes on current block using page position passed in the function
    return writeBlock(fHandle->curPagePos,fHandle,memPage);
	
		
}

/*
 * This method is used to append an Empty block into the pageFile.
 */
RC appendEmptyBlock (SM_FileHandle *fHandle)
{
     createPageFile(fHandle->fileName);
     //Increment total number of pages to allocate the empty block
		fHandle->totalNumPages = fHandle->totalNumPages + 1;
    //Point the current page position
		fHandle->curPagePos = fHandle->totalNumPages - 1;
		return RC_OK;		
	
}

/*
 * This method is used to ensure the capacity of pageFile.
 * The pageFile must have numberOfPages that is specified,
 * if not then add those many number of pages to achieve that capacity
 */
RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle)
{
	//Check the capacity of pagefile
	if(fHandle->totalNumPages >= numberOfPages)
	{
		return RC_OK;
	}
	else	//if less capacity
	{
		int i, pagesToAdd;
		//calculate the pages to be appended
		pagesToAdd = numberOfPages - fHandle->totalNumPages;

        //Append empty blocks until the required capacity is fulfilled
		for(i=0; i < pagesToAdd ; i++)
		{
			//call appendEmptyBlock()
			appendEmptyBlock(fHandle);
		}
		return RC_OK;
	}
}