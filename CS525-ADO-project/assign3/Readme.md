
RUNNING THE SCRIPT 
===========================================

1) Using Terminal, navigate to the Project root (assign3).

2) Enter ls to list the files and make sure we're in the right directory.

3) Run "make clean" to remove previously compiled.o files.

4) Execute "make" to compile all project files, including "test assign3 1.c."

5) Enter "./recordmgr" to execute the "test assign3 1.c" file.


DESCRIPTION OF THE SOLUTION ===========================================

MakeFile was created using the tutorial found here: http://mrbook.org/blog/tutorials/make/.

We maintained appropriate memory management when creating this record manager by freeing up any reserved space and minimizing the use of variables as much as needed. We implemented the Tombstone mechanism in addition to the functions described in record mgr.h.










1. TABLE AND RECORD MANAGER FUNCTIONS
=======================================

The record manager-related functions are used to start and stop the record manager. Table-related functions are used to create, open, close, and remove tables. We use Buffer Manager from Assignment 2 to access pages via Buffer Pool applying a page replacement strategy. In addition, the Storage Manager from Assignment 1 is used indirectly to deploy troops on disk page files.

initRecordManager (...) 
--> This method is used to initialize the record manager.
--> To initialize the storage manager, we use Storage Manager's initStorageManager(...) function.

shutdownRecordManager(...)
--> It removes all Record Manager resources/memory space.
--> This function terminates the record manager and de-allocates all of the record manager's resources.
--> To de-allocate memory space, we set the recordManager data structure pointer to NULL and run the C function free().

createTable(...) 
--> This function creates the table with the name provided by the parameter 'name.
--> It sets the table's attributes such as name, datatype, and size as well as initializes all of its values.
--> It calls initBufferPool to start the Buffer Pool (...). We apply an LRU page replacement policy.
--> It then creates a page file, opens it, writes the table block to the page file, after that it closes the page file.

openTable(...) 
--> This function creates a table with the name provided in the 'name' variable in the schema specified in the schema' parameter.
--> This function allows for faster turning and smoother running.

closeTable(...) 
--> It implements this by calling the BUffer Manager function shutdownBufferPool (...)
--> This function closes the table given by the parameters rel'.
--> Before closing the buffer pool, the buffer manager copies the alterations to the table to the page file.
deleteTable(...) 
--> This function deletes the table provided by the 'name' parameter and It runs the Storage Manager's destroyPageFile method (...).
--> The destroyPageFile(...) function deletes the page from disk and frees up the memory space reserved for that mechanism.

getNumTuples(...) 
--> This function returns the number of tuples in the table given by the parameters rel'.
-->  This function used to return a Float value from the Tuples.
--> It returns the value of the variable the tuplesCount given in our  data structure for storing table meta-data.


2. RECORD FUNCTIONS
=======================================

These functions are used to obtain a record with a specific RID, delete a record with a specific RID, insert a new record, and update an existing record with new values.

insertRecord(...)
--> This function adds a record into the table and updates the record' argument with the Record ID supplied in the insertRecord() function. We set the Record ID for the record that is being inserted.
--> We also make the page dirty so that the Buffer Manager can write the page's content back to disk.
--> We pin the page with an empty slot. When we find an empty slot, we locate the data pointer and add a '+' to indicate that this is a newly added entry.
--> We use the memcpy() C function to copy the record's data which is provided through parameter record' into the new record, and then unpin the page.

deleteRecord(...) 
--> This function deletes a record from the table identified by the parameters' that has the Record ID 'id' given by the parameter.
--> We pin the page and browse to the record's data pointer, where we set the first character to '-', indicating that the record has been removed and is no longer required.
--> We set the meta-data freePage in our database to the Page ID of the page whose record is being deleted, so that this space can be utilized by a new record later.
--> At last, we mark the page unclean so that the BUffer Manager can store the page's contents to disk, and then we unpin it.

updateRecord(...)
--> This method modifies a record in the table specified by the parameter "rel" that is referred to by the parameter "record."
--> It creates the Record ID and navigates to the site where the record's data is stored.
-->It uses table meta-data to find the page where the record is placed and pins that page to the buffer pool.
--> We use the memcpy() C function to copy the record's data which is provided through parameter record' into the new record, then mark the page dirty and unpin the page.

getRecord(....) 
--> This function retrieves a record in the table referenced by "rel," which is also passed in the parameter, with the Record ID "id" passed in the parameter. 
--> It detects the page where the record is located using the table's meta-data and pins that page in the buffer pool using the record's 'id'.
--> It replaces the record' parameter's Record ID with the id of the existing record on the page and transfers the data as well and The page is then unpinned.


3. SCAN FUNCTIONS
==============================================

The Scan related functions are used to retrieve all tuples from a table that meet a specific criterion which is represented as an Expr. Initiating a scan produces the RM ScanHandle data structure, which is supplied as an argument to startScan. Following that, calls to the next function are performed, which returns the next tuple that fulfills the scan condition. If NULL is supplied as a scan condition, it returns RC SCAN CONDITION NOT FOUND. If the scan is complete, it returns RC RM NO MORE TUPLES; otherwise, it returns RC OK (unless an error occurs).

startScan(...)
--> This function initiates a scan by reading data from the RM ScanHandle data structure, which is supplied as an argument to the function startScan() and We set up the scan-related variables in our specific data structure.
--> If the condition is NULL, the error code is returned. RC SCAN CONDITION NOT FOUND





next(...) 
--> This function returns the next tuple that meets the requirement (test expression).
--> If the condition is NULL, we return the error code RC SCAN CONDITION NOT FOUND and If there are no tuples in the table, we return the error code RC RM NO MORE TUPLES.
--> We loop through the tuples in the table. Pin the page that contains that tuple, browse to the site where data is stored, copy data into a temporary buffer, and then evaluate the test expression by calling eval (....)
--> If the test expression's result (v.boolV) is TRUE, it signifies the tuple meets the requirement. We then unpin the page and return. RC OK\s
--> If none of the tuples meet the criterion, we return the error code RC RM NO MORE TUPLES.

closeScan(...)
--> Closes the scan operation.
--> We check the scanCount value in the table's metadata to see if the scan was complete. If it is more than zero, the scan was insufficient.
--> If the scan failed, we unpin the page and reset all scan mechanism-related variables in our table's meta-data and The space allocated by the metadata is then released.


4. SCHEMA FUNCTIONS
=========================================

These functions are used to return the number of records in a given schema in bytes and to construct a new schema.

getRecordSize(...) 
--> The size of a record in the given schema is returned by this function.
--> We iterate through the schema's attributes. 
--> We add the size (in bytes) required by each attribute to the variable size' repeatedly.
--> The value of the variable size' represents the record's size.


freeSchema(...)
--> The schema specified by the parameters' schema' is removed from memory by this function.
--> Each page frame's variable refNum provides this purpose. refNum keeps a record of how many page frames the client is accessing.
--> We use the C function free(...) to de-allocate the schema's memory space, deleting it from memory.

createSchema(...)
--> In memory, this function creates a new schema using the provided parameters.
--> We construct a schema object and assign memory space to it. Finally, we set the schema's parameters to the ones supplied in the createSchema function (...)
--> numAttr defines the number of parameters. attrNames defines the name of the attributes. datatypes defines the datatype of the characteristics. typeLength provides the length of the attribute (example: length of STRING).



5. ATTRIBUTE FUNCTIONS
=========================================

These functions are used to acquire or set a record's attribute values and to generate a new record for a specific schema. 

createRecord(...)
--> This function produces a new record in the schema specified by the argument schema' and delivers the new record to the createRecord() function's record parameter.
--> We provide appropriate memory space for the new record. We can provide memory space for the record's data, which is the record size.
--> After that, we assign this new record to the parameters record'.
--> We also add a '-' to the first position and append '0', which is NULL in C. '-' specifies that this is a new blank record.
-->This event shows a page where you can create a record for the supplied entityApiName.


attrOffset(...) 
--> This function inserts the offset (in bytes) from the initial position to the specified attribute of the record into the result' parameter passed by the function.
--> We iterate through the schema's attributes until we reach the supplied attribute number. We repeatedly add the size (in bytes) required by each property to the pointer *result.
--> Every function must be developed to return a particular type of information and to accept specific types of data (parameters).




freeRecord(...) 
--> This function frees the memory space assigned to the parameters record'.
--> We use the C function free() to free up the memory space used by the record.
--> The function free in C is used to de-allocate or free up the space allocated by functions such as malloc() and **calloc()**. free(ptr).
--> The free() method in C does not return any value. It has a return type of void.



getAttr(...) 
--> This function retrieves an attribute from the provided record in the specified schema.
The parameter contains the record, schema, and attribute number for which data is to be received. The attribute details are saved back to the place specified by the parameter's value.
--> The getattr() function is used to retrieve the value of an object's attribute; if no such property is discovered, the default value is returned.
--> We use the attrOffset(...) function to go to the position of the attribute. Next, depending on the datatype of the attribute, we copy the data type and value of the attribute to the '*value' parameter.


setAttr(...)
--> This function changes the attribute value in the supplied schema's record. The parameter contains the record, schema, and attribute number about which data is to be received.
--> Using the attrOffset(...) function, we get to the attribute's location. The data in the '*value' parameter is then moved to the attribute's data type and value, depending on the data type.
--> The setattr() method is used to change the value of an object's or instance's attribute.
--> The 'value' parameter contains the data that will be placed in the attribute.
