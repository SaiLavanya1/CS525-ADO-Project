
CS 525 Assignment 1 - Storage Manager

      
Group 30 Members:


Name - Varun Anavatti
CWID - A20526745
Email- vanavatti@hawk.iit.edu


Name - Narthu Sailavanya
CWID - A20516764
Email- snarthu@hawk.iit.edu


Name - Venkata Sai Neeharika Koniki
CWID -  A20527105
Email- vkoniki@hawk.iit.edu


FILE MANUPILATION FUNCTIONS--------- Implemented by Neeharika

	
These methods' primary function is to manage the page files that the read and write methods use. 
The total number of pages in the file needed to be stored so that it could be retrieved by the appropriate read and write methods was one of the primary aspects of our proposal.

initStorageManager(void)
=========================
1. The storage manager is initialized using this function.
2. This method initializes the global object created for the file stream to NULL.

createPageFile (char *fileName)
===============================
1. The file name is created by the function given the input.
2. In C, the function fopen is utilized to create files with two arguments: the file name and the mode.
3. Here, the file is opened in read/write mode and truncated to zero length using the command "w+"
4. fwrite - Using the integral data type size t, this method returns the total number of correctly returned elements.
5. The result is placed in the destination string after the strcat() method concatenates the source and destination strings.

openPageFile(char *fileName, SM_FileHandle *fHandle)
====================================================
1. Opens the file with the fopen() function in read mode.
2. The file handle's filename is assigned to the current file name and the current position is set to the beginning of the page if the file pointer is not equal to null.
3. The file position of the stream is set to the specified offset using the same fseek function as before. However, the openPage function changes the inputs to "fseek(filePage,0,SEEK END)" 
where "0" is the offset and SEEK END, which tends to the file's end, is the starting point.
Using (ftell(filePage) / PAGE SIZE), where ftell(filePage) provides the current file position of the specified stream, one can determine the total number of pages in a file handle.
4. "RC OK" is given back as a success acknowledgement indicator.
5. The error "RC FILE NOT FOUND" is returned if the file pointer is equal to null (otherwise known as the otherwise case).

closePageFile (SM_FileHandle *fHandle)
=======================================
1. opens the currently open file pointer and verifies the file's status.
2. The initialized file pointer is closed by being assigned to NULL if the file pointer is initialized (not equal to NULL).
3. Returns the acknowledgment RC OK at the conclusion.

destroyPageFile (char *fileName)
================================
1. The error message "RC FILE NOT FOUND" is passed if the file reference is "NULL."
2. Otherwise, if the file is present, it is deleted using the remove() function and the message "RC OK" is provided.

---------------------------------------------------------------------------------------------------------------------------------

READ FUNCTIONS ------ Implemented by Sailavanya

Blocks of data are read from the page file into the disk using read-related functions (memory). It also moves easily through the blocks.C functions like  fseek(..) and fread(..) are used.

readBlock()

1. We determine whether the page number is accurate. The number of pages should be greater than 0 and less than the number of pages overall.
2. We use fseek() to find the specified place using the valid file reference. 
3. Verify if there is a pointer to the page file.
4. If fseek() succeeds, the data is read from the page specified in the parameter and stored in the memPage parameter.

     
getBlockPos(...) - This function returns the current value of current page position which is retrieved from FileHandle's curPagePos.

readFirstBlock(...) - By passing 0 as the pageNum argument while using readBlock(...)
      
readPreviousBlock(....) -  The pageNum argument is provided when we call the readBlock(...) function (current page position - 1)

readCurrentBlock(....) - The pageNum argument is provided when we call the readBlock(...) function (current page position)

readNextBlock(....) - The pageNum argument is (current page position + 1) when the readBlock(...) is called.

readLastBlock(..._ - The pageNum argument is provided when we call the readBlock(...) function (total number of pages - 1)

------------------------------------------------------------------------------------------------------------------------------------------

WRITE FUNCTIONS ------- Implemented by Varun
	  
writeBlock()		: It is used to write a page to a disk in a file using page Number.

writeCurrentBlock()	: It is used to write data on page's current postion.

appendEmptyBlock()	: It is used to append an empty page at the end of the file.

ensureCapacity()	: It is used to make sure that the number of pages are as defined. If the file is not found, then it would append an empty block at the end.

 
