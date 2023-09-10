--------------------------------
   Steps to run the code :
--------------------------------
$ make clean
This will delete all the previous complied files

$ make 
This will compile all project files and creates an executable file "test_assign1". Use the below command to run the executable.

$ ./test_assign1 or make run_test
This will execute the project.


If the code needs to be run again, we can clean the files and then Redo from Step 1. To clean use below command.

$ make clean



PAGE REPLACEMENT ALGORITHM FUNCTIONS
=========================================

The page replacement strategy functions implement FIFO, LRU, LFU, CLOCK algorithms which are used while pinning a page. If the buffer pool is full and a new page has to be pinned, then a page should be replaced from the buffer pool. These page replacement strategies determine which page has to be replaced from the buffer pool.

FIFO(...)
--First In First Out (FIFO) is the most basic page replacement strategy used.
--FIFO is generally like a queue where the page which comes first in the buffer pool is in front and that page will be replaced first if the buffer pool is full.
--Once the page is located, we write the content of the page frame to the page file on disk and then add the new page at that location.

LFU(...)
--Least Frequently Used (LFU) removes the page frame which is used the least times (lowest number of times) amongst the other page frames in the buffer pool.
--The variable (field) refNum in each page frame serves this purpose. refNum keeps a count of of the page frames being accessed by the client.
--So when we are using LFU, we just need to find the position of the page frame having the lowest value of refNum.
--We then write the content of the page frame to the page file on disk and then add the new page at that location.
--Also, we store the position of the least frequently used page frame in a variable "lfuPointer" so that is useful next time when we are replacing a page in the buffer pool. It reduces the number of iterations from 2nd page replacement onwards.

LRU(...)
--Least Recently Used (LRU) removes the page frame which hasn't been used for a long time (least recent) amongst the other page frames in the buffer pool.
--The variable (field) hitNum in each page frame serves this purpose. hitNum keeps a count of of the page frames being accessed and pinned by the client. Also a global variable "hit" is used for this purpose.
--So when we are using LRU, we just need to find the position of the page frame having the lowest value of hitNum.
--We then write the content of the page frame to the page file on disk and then add the new page at that location.

CLOCK(...)
--CLOCK algorithm keeps a track of the last added page frame in the buffer pool. Also, we use a clockPointer which is a counter to point the page frames in the buffer pool.
--When a page has to be replaced we check the "clockPointer"s position. If that position's page's hitNum is not 1 (i.e. it wasn't the last page added), then replace that page with the new page.
--In case, hitNum = 1, then we set it's hitNum = 0, increment clockPointer i.e. we go to the next page frame to check the same thing. This process goes on until we find a position to replace the page. We set hitNum = 0 so that we don't enter into an infinite loop.



BUFFER POOL FUNCTIONS------------------

For an already-existing page file on disk, a buffer pool is created using the functions related to buffer pools. While the page file is on disk, the buffer pool is formed in memory. To operate just on page file on disk, we use Storage Manager (Assignment 1).
 
RC initBufferPool();
-- This function builds a fresh memory buffer pool.
--The maximum of page frames that can be maintained in the buffer is determined by the option numPages.
--PageFileName keeps track of which page file contains the pages that are being cached in memory.
--This buffer pool's page replacement strategy (FIFO, LRU, LFU, and CLOCK) is represented by strategy.
--If there are any parameters, they are passed to the page replacement method using stratData.
 
RC shutdownBufferPool();
--This function destroys the buffer pool in order to shut down.
--It releases all of the memory and resources that the buffer manager was using for the buffer pool.
--We use forceFlushPool(...) to flush the buffer pool of all dirty pages (pages that have been changed) before destroying it.
--Any page that is currently being accessed by a client generates the error RC PINNED PAGES IN BUFFER.
 
RC forceFlushPool();
--The dirty pages (updated pages with dirtyBit = 1) are all written to the disk by this function.
The page frame is written to the page file on disk if both of the following conditions are met: dirtyBit = 1 (which indicates that the content of the page frame has been altered by some client) and fixCount = 0 (which suggests no user is using that page Frame). Otherwise, it checks all the page frames in the buffer pool.


STATISTIC FUNCTIONS---------------------------------------

The statistics-related functions are used to obtain information about the buffer pool. As a result, it provides various statistical information regarding the buffer pool.

getFrameContents(...)
--An array of PageNumbers is returned by this function. The array size is equal to the buffer size (numPages).
--We iterate through all of the page frames in the buffer pool to acquire the pageNum value of the page frames in the buffer pool.
--The page number of the page stored in the "nth page frame" is represented by the "n"th element.


getDirtyFlags(...)
--This function returns a bool array. The array size equals the buffer size (numPages)
--We iterate through all of the page frames in the buffer pool to retrieve the dirtyBit value of each page frame in the buffer pool.
--If the page saved in the "nth page" frame is dirty, the "n"th element returns TRUE.


getFixCounts(...)
--The result of this function is an array of ints. The array size is equal to the buffer size (numPages).
--We loop through all of the page frames in the buffer pool to find the fixCount value of the page frames in the buffer pool.
--The fixCount of the page contained in the "n"th page frame is the "n"th element.


getNumReadIO(...)
--This function returns the total number of IO reads executed by the buffer pool, i.e. the number of disk pages read.
--The rearIndex variable is used to keep track of this information.


getNumWriteIO(...)
--This function returns the total number of IO writes performed by the buffer pool, i.e. the number of disk pages written.
--The writeCount variable is used to keep track of this information. When we start the buffer pool, we set writeCount to 0 and increment it whenever a page frame is written to disk.
