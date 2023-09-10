#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "storage_mgr.h"
#include "dberror.h"

FILE *fl;
RC ret_value;

void initStorageManager (void)
{
    /*First call made toi initStorage*/
    fl = NULL;
    ret_value = -1;
    printf("Initializing the Storage Manager");
	printf("\nStorage Manager Initialized");
}


void setNull (char *memory){

    int i;
    for (i=0; i < PAGE_SIZE; i++){
        memory[i] = '\0';
    }
    return;
}


extern RC createPageFile ( char * fileName )
{
    fl = fopen(fileName,"w+"); 
	
    //opening the file with write w+ mode, hence creating a empty file with both reading and writing mode.
	//checking the existence of file through file pointer
    
    if (fl == NULL)
	    return RC_FILE_NOT_FOUND;

	else 
    {
        SM_PageHandle firstPage= (SM_PageHandle)malloc(PAGE_SIZE * sizeof(char)); 
        setNull(firstPage);
	    // creating empty page using calloc
	    //checking if write operation is possible on the empty page.
	    if(fwrite(firstPage, sizeof(char), PAGE_SIZE, fl))
	    {
		    printf("zero page is written to file");
	    }
    	else
	    {
		    printf("zero page cannot be written to file");
	    }
	    
        free(firstPage);// freeing the memory allocated for zero page
	    fclose(fl);// closing the file to make sure buffers are flushed
	    
        return RC_OK;
	}
	
}


extern RC openPageFile(char* fileName, SM_FileHandle *fHandle)
{   
    int totalNumPages = 0;
	
    fl = fopen(fileName,"r+"); // opening file in r mode to create an empty page in read only

    if(fl != NULL)
    {
        fHandle->curPagePos = 0;// initializing current position to zero.
		fHandle->fileName = fileName;// initializing the filename of the file
		
        fseek(fl, 0, SEEK_END);// finding the end point of the file
		totalNumPages = ftell(fl) ;// calculating number of pages using the filepointer end location and page size given.
		fHandle->totalNumPages = totalNumPages / PAGE_SIZE;// initializing total number of pages
		fclose(fl);// closing the file

		return RC_OK;
    }
    return RC_FILE_NOT_FOUND;
	
	
    
}


extern RC closePageFile(SM_FileHandle *fHandle)
{

    if (fHandle != NULL)
    {
        fl = fopen(fHandle->fileName, "r");

        if (fl != NULL)
            if (fclose(fl) == 0) // checking if file exists 
                return RC_OK;
            
    }
    else 
        return RC_FILE_NOT_FOUND;

    
}


extern RC destroyPageFile(char *fileName)
{
	
    fl = fopen(fileName, "r");// opening the file in read mode to get the file pointer 
	
    if(fl != NULL)
    {
	    remove(fileName);// removing the file if the file is found
	    return RC_OK;
	}
	else
	    return RC_FILE_NOT_FOUND;	
}

extern RC readBlock (int pgNum, SM_FileHandle *fileHandle, SM_PageHandle mrPg)
		{
			if (pgNum < 0 || pgNum > (*fileHandle).totalNumPages ) 
				return RC_READ_NON_EXISTING_PAGE; //If the total number of pages is more than the page number it will throw error of Non Exisitng Page.
			
			//FILE *f; : used as a global variable in common.h
			
			fl = fopen(fileHandle->fileName, "r"); //open file

			if(fl == NULL)
				{  //If file doesn't exists
					return RC_FILE_NOT_FOUND; 
				}
			else
				{
					int offset = pgNum * PAGE_SIZE;
					if (fseek(fl, offset, SEEK_SET))
					{
						return EXIT_FAILURE;
					}
					fread(mrPg, sizeof(char), PAGE_SIZE, fl);
					fileHandle->curPagePos = ftell(fl); // Current page position update to pagenum 
				}
			fclose(fl);
			return RC_OK; 
		}
    		

extern RC getBlockPos (SM_FileHandle *fHandle)
{
    /*Current Position of the pointer has to be returned */

    int curPos = 0;

	if(fHandle == NULL)                                                     
    {
       
       return RC_FILE_NOT_FOUND;
                                        
    }
    else
    {
        curPos = fHandle->curPagePos;
        ret_value = curPos;
    }	

    return ret_value;
}


extern RC readFirstBlock (SM_FileHandle *fileHandle, SM_PageHandle mrPg)
{
    
    fl = fopen(fileHandle->fileName, "r");

    if(fl == NULL)
        {  //If file doesn't exists
            printf("file does not exist");
            return RC_FILE_NOT_FOUND; 
        }
    else if (ferror(fl)) //ferror() will detect error in file pointer stream
        {   
            printf("Error in reading from file!");
        
            clearerr(fl);      //clearerr() will clear error-indicators from the file stream
            ferror(fl);       //No error will be detected now
            printf("Error again in reading from file!");
            return EXIT_FAILURE;
        }
    else
        {
            if(fseek(fl, 0, SEEK_SET))
                {
                    fprintf(stderr, "fseek() failed in file %s at line # %d\n", __FILE__, __LINE__ - 2);
                    fclose(fl);
                    return EXIT_FAILURE;
                }
            fread(mrPg, sizeof(char), PAGE_SIZE, fl); //This return the first block of PAGE_SIZE
            if(feof(fl))
                {       
                    //On detecting the end-of-file, feof() function will return non-zero value
                    printf("Error reading: unexpected end of file\n");   
                    //hence, it will break the loop
                    return EXIT_FAILURE;
                }
            (*fileHandle).curPagePos = ftell(fl); //First page of the file has index 0
        }
    fclose(fl);
    //Successful exit from the program
    return RC_OK;
}


extern RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle mrPg)
{
    /* Using the present blocks poosition, we find the previous block and read it's comtents*/
	if(fHandle != NULL)                                                     
    {
	    return(readBlock (fHandle->curPagePos-1, fHandle, mrPg));                                   
    }
    else
    {
        return RC_FILE_NOT_FOUND; 
    }	
}

extern RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle mrPg)
{
    /*Using the present blocks poosition, we read it's comtents*/
    if(fHandle != NULL)                                                     
    {
	    return(readBlock (fHandle->curPagePos, fHandle, mrPg));                               
    }
    else
    {
        return RC_FILE_NOT_FOUND; 
    }	

}

extern RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle mrPg)
{
    /*Using the present blocks poosition, we find the next block and read it's comtents*/
    if(fHandle != NULL)                                                     
    {
        return(readBlock (fHandle->curPagePos+1, fHandle, mrPg));                                 
    }
    else
    {
        return RC_FILE_NOT_FOUND; 
    }	        
}


extern RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle mrPg)
{
    /*Read contents of the last page in the block*/	
    if(fHandle != NULL)                                                     
    {
	    return(readBlock (fHandle->totalNumPages-1, fHandle, mrPg));                              
    }
    else
    {
        return RC_FILE_NOT_FOUND; 
    }	
}

extern RC writeBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle mrPg) {

    /*Write contents to into the specified page.*/

    int Offset = 0;

    if (pageNum >= 0 && pageNum <= fHandle->totalNumPages){
            Offset = PAGE_SIZE * pageNum;
            fl = fopen(fHandle->fileName, "r+");

            if (fl != NULL)
            {   //Initially the filepointer has to moved the mentioned page number.
                if(pageNum ==0){
                    fseek(fl, Offset, SEEK_SET);
                    fwrite(mrPg, sizeof(char), PAGE_SIZE, fl);
                    fHandle->curPagePos = ftell(fl);
                }
                else {
                    appendEmptyBlock(fHandle);
                    fseek(fl, Offset, SEEK_SET);
                    fwrite(mrPg, sizeof(char), strlen(mrPg), fl);
                    fHandle->curPagePos = ftell(fl);
                }

                fclose(fl);
                return  RC_OK;  
            }  
            fclose(fl);
            return  RC_FILE_NOT_FOUND;                         
        }
        
    return RC_WRITE_FAILED;
            	
    
    
}


extern RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle mrPg)
{
    /* Writing the mrPg data into the file by passing the curPagePos to the writeblock */

    int currentblock = 0;

    if(fHandle == NULL)                                                     
    {
        return RC_FILE_NOT_FOUND; 
                                     
    }
    
    currentblock=fHandle->curPagePos;
	return(writeBlock (currentblock, fHandle, mrPg));  
    
        	
}

extern RC appendEmptyBlock(SM_FileHandle *fHandle) 
{
    // To append an empty block the file poniter has to be seek to eof and 
    // new page has to added to the file. Later the total number of pages is increased by 1.

    
    if(fHandle != NULL)                                                     
    {
        // Creating an empty page

        SM_PageHandle emptyBlock = (SM_PageHandle)malloc(PAGE_SIZE*sizeof(char));
        setNull(emptyBlock); 

        //fl = fopen(fHandle->fileName, "w+");

        if(fl != NULL) 
        {
            // Moving file pointer to eof.

            fseek(fl, 0, SEEK_END);

            //writing the empty page into the file.

            if(fwrite(emptyBlock, 1, PAGE_SIZE, fl) == 0) 
            {   
                free(emptyBlock);            
             //   fclose(fl);
                return RC_WRITE_FAILED;                               
            }
            else
            {
                fHandle->totalNumPages++; //Incrementing the total number of pages by 1.       
                //fHandle->curPagePos = fHandle->totalNumPages - 1;       
                free(emptyBlock);   
                //fclose(fl);                                        
                return RC_OK;         
            } 

        } 
        //fclose(fl);
                               
    }
    
    return RC_FILE_NOT_FOUND; 
    	
}


extern RC ensureCapacity(int numberOfPages, SM_FileHandle *fHandle) 
{
    
    
    fl = fopen(fHandle->fileName, "a");
    int pages;
    if (fHandle != NULL){

        pages = numberOfPages - fHandle->totalNumPages;
        
        while(pages > 0){
            appendEmptyBlock(fHandle);
            pages--;
        }

        fclose(fl);
         return RC_OK;   
	}
    else
    {                                
        return RC_FILE_HANDLE_NOT_INIT;
    }
 
}