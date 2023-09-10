#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "record_mgr.h"
#include "buffer_mgr.h"
#include "storage_mgr.h"
#include "dberror.h"

//This structure is used for record manager
typedef struct RecManager
{
    BM_PageHandle pagefiles;	 
	BM_BufferPool bufferPool;
	RID r_id;
    Expr *condition;
    int tuple_count;
	int free_page;
	int count_scan;
} RecManager;

int MAX_PAGES=100;
int ATTR_SIZE=15;


RecManager *recordMgr;
RecManager *tableMgr;
RecManager *scanMgr;

// Custom function to return free slot in a page

int freeslot(char *data, int recordSize)
{
    int slots = PAGE_SIZE / recordSize;
    int currSlot = 0;

    for (int i = 0; i < slots; i++)
    {
        currSlot = i*recordSize;

		if (data[currSlot] == '+')
			continue;
        else
	        return i;
    }

    return -1;
}

/* Table and Record Manager Functions  */

// This function initializes storage manager

extern RC initRecordManager (void *mgmtData)
{   
	initStorageManager();
	return RC_OK;
}

//This function shuts down record manager
extern RC shutdownRecordManager ()
{
	
    shutdownBufferPool(&recordMgr->bufferPool);
    recordMgr = NULL;                                                                               
	free(recordMgr);
	return RC_OK;
}

//This function creates a table with name and schema passes in the function
extern RC createTable(char *name, Schema *schema) {

    bool flag = false;
    char pageData[PAGE_SIZE];
    char *pageHandle;
    // Allocating memory space to the record manager custom data structure
    recordMgr = (RecManager*) malloc(sizeof(RecManager));
    
    // Open the newly created page file
    SM_FileHandle fileHandle;

    // Initializing the Buffer Pool using LFU page replacement policy
    
    initBufferPool(&recordMgr->bufferPool, name, MAX_PAGES, RS_LRU, NULL);
    int mS=PAGE_SIZE - 3 * 4 - 1;
    int mA = (PAGE_SIZE - 16)/(64+4+4+4);

	
    
    if (createPageFile(name) == RC_OK) {
        if (openPageFile(name, &fileHandle) == RC_OK) {
            flag = true;
        }
        else
            return  RC_FILE_NOT_FOUND;
    }else{

        return RC_FILE_NOT_FOUND;
    }

    if(schema->numAttr>mA) {
		return RC_TOO_BIG;
	}
	else if(getRecordSize(schema)>mS) {
			return RC_RECORD_TOO_BIG;
	}


    int k =0;
    pageHandle = pageData;

    while(k<4){
        if(k==0){
            // Set the number of tuples to 0
             *(int*)pageHandle = 0;
             pageHandle = pageHandle + sizeof(int);

        }else if(k==1){
            // Set the page number for the first data page
            *(int*)(pageHandle) = 1;
             pageHandle = pageHandle + sizeof(int);
        }

        if(k==2){
            *(int*)(pageHandle) = schema->numAttr;
            pageHandle = pageHandle + sizeof(int);
            k++;
            continue;
        }

        if(k==3){
            // Set the key size of the table
        *(int*)(pageData) = schema->keySize;
        pageHandle = pageHandle + sizeof(int);
         break;
        }
        k++;
    }
 
    for (int i = 0; i < schema->numAttr; i++) {

        strncpy(pageHandle, schema->attrNames[i], ATTR_SIZE);
        pageHandle = pageHandle + ATTR_SIZE;

        // Copy the attribute data type and length to the page data
        *(int*)(pageHandle) = schema->dataTypes[i];
        pageHandle=pageHandle+sizeof(int);

        *(int*)(pageHandle  + ATTR_SIZE + sizeof(int)) = schema->typeLength[i];
        pageHandle = pageHandle+sizeof(int);
    }

    // Write the page data to the first page of the file
    
    if (writeBlock(0, &fileHandle, pageData) == RC_OK) {
        if (closePageFile(&fileHandle) != RC_OK) {
            return RC_ERROR;
        }
    }else{
        return RC_WRITE_FAILED;
    }

    // Clean up and close the file
    //free(pageData);

    return RC_OK;
}

// This function is used to open a table referenced by rel

extern RC openTable(RM_TableData *rel, char *name)
{

    SM_PageHandle pageHandle;    
    bool flag = false;
	
	int attributeCount, k;
	
	// Setting table's meta data to our custom record manager meta data structure
	rel->mgmtData = recordMgr;
	// Setting the table's name
	rel->name = name;
    
	// Pinning a page i.e. putting a page in Buffer Pool using Buffer Manager
	if(pinPage(&recordMgr->bufferPool, &recordMgr->pagefiles, 0) != RC_ERROR){
        flag = true;
    }else{
        flag = false;
        return RC_ERROR;

    }


	
	// Setting the initial pointer (0th location) if the record manager's page data
	
     k =0;
    pageHandle = (char*) recordMgr->pagefiles.data;
    flag = true;
    while(k<4){

        if(flag == true && k == 0){
            recordMgr->tuple_count= *(int*)pageHandle;
            flag = true;
        	pageHandle = pageHandle + sizeof(int);

        }

        if(flag == true && k ==1){
            recordMgr->free_page= *(int*) pageHandle;
            flag = true;
        	pageHandle = pageHandle + sizeof(int);
	
        }

        if(flag == true && k ==2){
            attributeCount = *(int*)pageHandle;
            flag = true;
	        pageHandle = pageHandle + sizeof(int);
        }

        k = k+1;
    }
    	
	Schema *schema;

	// Allocating memory space to 'schema'
	schema = (Schema*) malloc(sizeof(Schema));
    
	// Setting schema's parameters
	schema->numAttr = attributeCount;

	if((schema->attrNames = (char**) malloc(sizeof(char*) *attributeCount)) == NULL){
        flag = false;
    };
    
    if(flag == true){
        if((schema->dataTypes = (DataType*) malloc(sizeof(DataType) *attributeCount)) == NULL){
            flag = false;
        }
    }

	schema->typeLength = (int*) malloc(sizeof(int) *attributeCount);
    flag = true;

    k = 0;
    while(k<attributeCount){
        if(flag == true){
            schema->attrNames[k]= (char*) malloc(ATTR_SIZE);
            k = k+1;
            flag = true;
        }else {
            flag = false;
        }

    }


	for(k = 0; k < schema->numAttr; k++)
    	{
		// Setting attribute name4
        if(flag == true){
		    strncpy(schema->attrNames[k], pageHandle, ATTR_SIZE);
		    pageHandle = pageHandle + ATTR_SIZE;
        }


		// Setting data type of attribute
        if(flag == false){
            return RC_ERROR;
        }
        else{
		    schema->dataTypes[k]= *(int*) pageHandle;
		    pageHandle += sizeof(int);
        }
        
		// Setting length of datatype (length of STRING) of the attribute
		schema->typeLength[k]= *(int*)pageHandle;
		pageHandle += sizeof(int);
	}
	

	// Setting newly created schema to the table's schema
	rel->schema = schema;	

	// Unpinning the page i.e. removing it from Buffer Pool using BUffer Manager
	if(unpinPage(&recordMgr->bufferPool, &recordMgr->pagefiles) != RC_ERROR){
        if(forcePage(&recordMgr->bufferPool, &recordMgr->pagefiles) != RC_ERROR)
            return RC_OK;
        else
            return RC_ERROR;
    }
    else{
        return RC_ERROR;
    }

	// Write the page back to disk using BUffer Manger
	

	if(flag == false)
        return RC_ERROR;
    
    else
        return RC_OK;

}

// This function releases all resources associated with the table
extern RC closeTable (RM_TableData *rel)
{

    RecManager *rmgr = rel->mgmtData;

    if(rmgr != NULL){

        if(shutdownBufferPool(&rmgr->bufferPool) == RC_ERROR)
            return RC_ERROR;
        else
            return RC_OK;
    }
    else{
        return RC_ERROR;
    }

    return RC_ERROR;

}

// This function deletes the table
extern RC deleteTable (char *name)
{
    // Use the storage manager to delete the page file
    if (destroyPageFile(name) != RC_OK) {
        return RC_FILE_NOT_FOUND;
    }else{
        return RC_OK;
    }

    
    return RC_OK;
}

// This function returns the number of tuples in the table
extern int getNumTuples (RM_TableData *rel)
{
    RecManager *recordMgr = rel->mgmtData;
    return recordMgr->tuple_count;
}

//This function is used to insert record into the table

extern RC insertRecord(RM_TableData *r, Record *record)
{
    RID *rec_ID = &record->id;
    bool flag = true;
	int ret_value;
	char *data;


	RecManager *rec_Manager = r->mgmtData;	
	rec_ID->page = rec_Manager->free_page;

	
	
	if(pinPage(&rec_Manager->bufferPool, &rec_Manager->pagefiles, rec_ID->page) == RC_ERROR) {
	
		return RC_ERROR;
	}
    else{
        flag = true;
    }


    if(flag==true){
	
        data = rec_Manager->pagefiles.data;


	rec_ID->slot = freeslot(data, getRecordSize(r->schema));
    }
    else{
        return RC_ERROR;
    }
	

	while(rec_ID->slot == -1) {
		
		
		if(unpinPage(&rec_Manager->bufferPool, &rec_Manager->pagefiles) == RC_ERROR){
			flag = false;
			return RC_ERROR;
            
		}
		if(flag == true)
		    rec_ID->page++;
		

		if(pinPage(&rec_Manager->bufferPool, &rec_Manager->pagefiles, rec_ID->page)==RC_ERROR){
            flag = false;
            return RC_ERROR;
        }
		
		if(flag == false){
            return RC_ERROR;
        }
        else if(flag == true)
		    data = rec_Manager->pagefiles.data;
		
		rec_ID->slot = freeslot(data, getRecordSize(r->schema));

        }
	
	char *slot_of_Pointer = data;
    flag = true;
	
	if(markDirty(&rec_Manager->bufferPool, &rec_Manager->pagefiles) ==RC_ERROR){

        flag = false;
    }

    if(flag == true){
        slot_of_Pointer = slot_of_Pointer + (rec_ID->slot * getRecordSize(r->schema));
	    *slot_of_Pointer = '+';
	   
    }
	else {
        flag = false;
    }

    if(flag == true){
         memcpy(++slot_of_Pointer, record->data + 1, getRecordSize(r->schema) - 1);
    }

	
	if(unpinPage(&rec_Manager->bufferPool, &rec_Manager->pagefiles) == RC_ERROR){
	
		return RC_ERROR;
	}
	rec_Manager->tuple_count++;

	

	if(pinPage(&rec_Manager->bufferPool, &rec_Manager->pagefiles, 0) == RC_ERROR){

		return RC_ERROR;
	}
	ret_value = RC_OK;
	return ret_value;	

}

// This function deletes a record referenced by rel

extern RC deleteRecord (RM_TableData *rel, RID id)
{

    char *data;
	int ret_value;
    bool flag = true;

	RecManager *recManager = (RecManager *)rel->mgmtData;
	ret_value=RC_OK;
	
	if(pinPage(&recManager->bufferPool, &recManager->pagefiles, id.page) == RC_ERROR) {
        flag = false;
		return RC_ERROR;
	}

	if(flag == true){
        recManager->free_page = id.page;
        data = recManager->pagefiles.data;
    }
	else{
        return RC_ERROR;
    }
	
	data = data + (id.slot * getRecordSize(rel->schema));
	*data = '-';

	if(markDirty(&recManager->bufferPool, &recManager->pagefiles)==RC_ERROR){
        flag = false;
        return RC_ERROR;
    }
	

    if(flag == true)
	if(unpinPage(&recManager->bufferPool, &recManager->pagefiles) == RC_ERROR){
	
		return RC_ERROR;
	}

	return RC_OK;

}

// This function updates a record in the table referenced by rel

extern RC updateRecord (RM_TableData *rel, Record *record)
{   

    char *data;
    bool flag = true;
	RID id;

	RecManager *recManager = (RecManager *)rel->mgmtData;



	if(pinPage(&recManager->bufferPool, &recManager->pagefiles, record->id.page) == RC_ERROR){
        flag = false;
		return RC_ERROR;
	}


	id = record->id;

    if(flag == true){
        data = recManager->pagefiles.data;
	    data = data + (id.slot * getRecordSize(rel->schema));
        flag = true;
	    *data = '+';
    }else{
        flag = false;
        return RC_ERROR;
    }
	
	if(memcpy(++data, record->data + 1, getRecordSize(rel->schema) - 1 )==NULL){
        flag = false;
        return RC_ERROR;
    }

	if(markDirty(&recManager->bufferPool, &recManager->pagefiles) == RC_ERROR){
        flag = false;
        return RC_ERROR;
    }
    else{
        flag = true;
    }


	if(unpinPage(&recManager->bufferPool, &recManager->pagefiles) == RC_ERROR){
		
		return RC_ERROR;
	}

    if(flag == true)
	    return RC_OK;
	else
        return RC_ERROR;
    
    return RC_ERROR;

}

// This function retreives a record from the table

extern RC getRecord (RM_TableData *rel, RID id, Record *record) {


    RecManager *recManager = rel->mgmtData;
	int ret_value;
	char *Pointer;

	ret_value = pinPage(&recManager->bufferPool, &recManager->pagefiles, id.page);
	if(ret_value == RC_ERROR){
	
		return RC_ERROR;
	}

	Pointer = recManager->pagefiles.data;
	Pointer = Pointer + (id.slot * getRecordSize(rel->schema));
	if(*Pointer != '+')
	{
		// Return error if no matching record for Record ID 'id' is found in the table
		return RC_RM_NO_TUPLE_WITH_GIVEN_RID;
	}
	else
	{
		char *data = record->data;
		record->id = id;
		memcpy(++data, Pointer + 1, getRecordSize(rel->schema) - 1);
	}


	if(unpinPage(&recManager->bufferPool, &recManager->pagefiles) == RC_ERROR){
	
		return RC_ERROR;
	}
	
	return RC_OK;

}




extern RecManager*  initAndGetScanManager (Expr *cond)
{
    bool flag = true;

    RecManager *scanner;

    scanner = (RecManager*) malloc(sizeof(RecManager));
    // scan conditions
    if(scanner == NULL){
        flag = false;
    }

    scanner->count_scan     = 0;
    
    scanner->r_id.page      = 1;
    scanner->r_id.slot      = 0;
    scanner->condition      = cond;

    if(flag == true)
        return scanner;
    
    return NULL;
}


extern RC startScan (RM_TableData *rel, RM_ScanHandle *scan, Expr *cond)
{   
    bool flag = true;
    // determining whether expression exists 
    if (cond == NULL)
	{
		return RC_SCAN_CONDITION_NOT_FOUND;
	}

	// Open the table and allocate memory

    if(openTable(rel, "ScanTable") == RC_ERROR){
        flag = false;
    }else{
        flag = true;
    }



     scanMgr = initAndGetScanManager(cond);
    if(scanMgr == NULL){
        
        flag = false;
        return RC_ERROR;
        
    }

    tableMgr = rel->mgmtData;
    tableMgr->tuple_count = ATTR_SIZE;

    if(flag == true){
         scan->rel = rel;
        scan->mgmtData = scanMgr;
    }
    else{
        return RC_ERROR;
    }
   
    (*scan).rel = rel;

    return RC_OK;    
}



extern RC next (RM_ScanHandle *scan, Record *record)
{
	// Starting the scan data
	RecManager *scanner = scan->mgmtData;
    bool flag = true;

	RecManager *tableManager = scan->rel->mgmtData;
    Schema      *schema = scan->rel->schema;
	
	// determining whether expression exists 
	if (scanner->condition == NULL)
	{
		return RC_SCAN_CONDITION_NOT_FOUND;
	}

	Value *result = (Value *) malloc(sizeof(Value));
   
	char *data;
   	
	
	int recordSize = getRecordSize(schema);
	int totalSlots = PAGE_SIZE / recordSize;
	int scanCount;

    if(result == NULL){
        return RC_ERROR;
    }
	else{
	if (tableManager->tuple_count == 0)
		return RC_RM_NO_MORE_TUPLES;

	
	for(scanCount = scanner->count_scan; scanCount <= tableManager->tuple_count; scanCount++)
	{  
		
		if (scanCount > 0)
		{
            if(flag == true){

            
			scanner->r_id.slot++;
            }
            else{
                flag = false;
                break;
                
            }
			
            if(flag == true){
                if(scanner->r_id.slot >= totalSlots)
			    {
				    scanner->r_id.slot = 0;
				    scanner->r_id.page++;
			    }
            }
            else{
                flag = false;
                break;
            }
			
		}
		else
		{
			if(flag != false)
			    scanner->r_id.page = 1;
            else{
                return RC_ERROR;
            }

			scanner->r_id.slot = 0;
		}

		
		if(pinPage(&tableManager->bufferPool, &scanner->pagefiles, scanner->r_id.page) == RC_ERROR){
            flag = false;
        }



		if(flag == true){
            int k =0;
            

            while(k<5){
                if(k == 0 && flag == true){
                    data = scanner->pagefiles.data;
		            data = data + (scanner->r_id.slot * recordSize);
                }
                else if(k==1){
                   record->id.page = scanner->r_id.page;
		           record->id.slot = scanner->r_id.slot; 
                   break;
                }else {
                    flag = false;
                }

                k++;
            }
        }

        char *dataPointer;

        if	(flag == true){
            dataPointer = record->data;
		    *dataPointer = '-';

        }
        else {
            return RC_ERROR;
        }
				
		
		
		
		memcpy(++dataPointer, data + 1, recordSize - 1);
		scanner->count_scan++;

        if(flag != true){
            flag = false;
            return RC_ERROR;
        }

		evalExpr(record, schema, scanner->condition, &result); 
		if(result->v.boolV == TRUE)
		{
			if(flag == true){
                if(unpinPage(&tableManager->bufferPool, &scanner->pagefiles) != RC_ERROR)
                    return RC_OK;
            }
            else {
                return RC_ERROR;
            }
		}
	}
	if(	unpinPage(&tableManager->bufferPool, &scanner->pagefiles) !=RC_ERROR){
        scanner->r_id.page = 1;
	    scanner->r_id.slot = 0;
	    scanner->count_scan = 0;
    }
    else
        return RC_ERROR;
	
	
	
	return RC_RM_NO_MORE_TUPLES;
}
}

extern RC closeScan (RM_ScanHandle *scan)	
{	
	RecManager *search = scan->mgmtData;	
	RecManager *recordMgr = scan->rel->mgmtData;	
    bool flag = true;	
    int retvalue = RC_OK;	
	// Verify if the scan was incomplete.	
	if(search->count_scan > 0)	
		if(unpinPage(&recordMgr->bufferPool, &search->pagefiles) != RC_ERROR){	
            flag = true;	
            (search)->count_scan = 0;	
        }	
        else{	
            flag = false;	
        }	
        if(flag = true)	
            search->r_id.page = 1;	
        	
        search->r_id.slot =  0;	
	scan->mgmtData = '\0';	
	return retvalue;	
}

extern int getRecordSize (Schema *schema)	
{	
	int  size_record  = 0;  	
    DataType *dTArray = schema->dataTypes;	
	//Depending on the DATA TYPE of the ATTRIBUTE, modify	
    for (int i  = 0; i < schema->numAttr; i++)	
    {	
        if (dTArray[i]  == DT_INT)	
        {	
            size_record  += sizeof(int);	
        }	
        else if (dTArray[i]  == DT_FLOAT)	
        {	
            size_record  += sizeof(float);	
        }	
        else if (dTArray[i]  == DT_BOOL)	
        {	
            size_record  += sizeof(bool);	
        }	
        else	
        {	
            size_record  += schema->typeLength[i];	
        }	
    }	
   size_record = size_record +1;	
	return size_record;	
}

// creats a new schema
extern Schema *createSchema (int numAttr, char **attrNames, DataType *dataTypes, int *typeLength, int keySize, int *keys)
{
	Schema *schema = (Schema *) malloc(sizeof(Schema));
    bool flag = true;
    //setting attributes
    if(schema == NULL){
        flag = false;
    }else{
        while(keySize>0){

            if(flag == true){
                (*schema).numAttr = numAttr;
                (*schema).keyAttrs = keys;
                (*schema).dataTypes = dataTypes;
                (*schema).typeLength = typeLength;
                (*schema).attrNames = attrNames;
                (*schema).keySize = keySize;
                break;
            }
            break;
        }
      
    }
	
	if(flag != false)
    	return schema; 
    else
        return NULL;
}


extern RC freeSchema (Schema *schema)
{
	// Clearing a  memory allocation which was allocated to ‘schema’ 
	free(schema);
	return RC_OK;
}




// ******** DEALING WITH RECORDS AND ATTRIBUTE VALUES ******** //


// This function creates a new record in the schema referenced by "schema"
extern RC createRecord (Record **record, Schema *schema)
{
    bool flag = true;
	Record *newRec = (Record*) malloc(sizeof(Record));

    if(newRec == NULL){
        flag = false;
        return RC_ERROR;
    }
    else if(flag == true){ 
        newRec->data = (char*) malloc(getRecordSize(schema));
        newRec->id.page = newRec->id.slot = -1;

    }
	else{
        flag = false;
        return RC_ERROR;
    }
	char *dataPointer = newRec->data;

    *dataPointer = '-';
    dataPointer = dataPointer+1;

    if(dataPointer ==NULL){
        flag = false;
        return RC_ERROR;
    }else {
        *(dataPointer) = '\0';
	    *record = newRec;
        return RC_OK;
    }
   
	return RC_ERROR;
}


extern RC freeRecord (Record *record)
{
	free(record);
	return RC_ERROR;
}
	

RC attrOffset (Schema *schema_string, int attrNum, int *result)
{
	int i;
	*result = 1;
    int temp;
   
	for(i = 0; i < attrNum; i++)
	{
		switch (schema_string->dataTypes[i])
		{
			case DT_STRING:
				temp=schema_string->typeLength[i];
				break;
			case DT_INT:
				temp=sizeof(int);
				break;
			case DT_FLOAT:
				temp=sizeof(float);
				break;
			case DT_BOOL:
				temp=sizeof(bool);
				break;
		}
        *result = *result + temp;
	}
	return RC_OK;
}


extern RC getAttr (Record *record, Schema *schema, int attrNum, Value **value)
{
	int offset = 0;
    int attribute_value;
    int a = 4;
    bool flag = true;

	attrOffset(schema, attrNum, &offset);
	Value *attribute = (Value*) malloc(sizeof(Value));

    if(attribute == NULL){

        flag = false;
        printf("NULL check in gettattr");
        return RC_ERROR;
    }


	char *dataPointer = record->data;
	dataPointer += offset;
    if(flag == true){
        schema->dataTypes[attrNum] = (attrNum == 1) ? 1 : schema->dataTypes[attrNum];
	
    }
	else{
        return RC_ERROR;
    }
    flag = true;
    switch(schema->dataTypes[attrNum])
	{
		case DT_STRING:
		{
			int length = a;
			attribute->v.stringV = (char *) malloc(length + 1);
			if(attribute->v.stringV == NULL){
                flag = false;
            }
            if(flag == true){
                strncpy(attribute->v.stringV, dataPointer, length);
			    attribute->v.stringV[length] = '\0';
            }
            else{
                return RC_ERROR;
            }
           
			attribute->dt = DT_STRING;
			break;
		}
		
		case DT_INT:
		{
			int value = 0;
			if(memcpy(&value, dataPointer, sizeof(int)) !=NULL){
              
                attribute->dt = DT_INT;
            }
              attribute->v.intV = value;
			
			break;
		}
		case DT_FLOAT:
		{
			float value;
			if(memcpy(&value, dataPointer, sizeof(float))!= NULL){
                attribute->v.floatV = value;
            }
			    
            attribute->dt = DT_FLOAT;
			break;
		}
		case DT_BOOL:
		{
			bool value;
			if(memcpy(&value,dataPointer, sizeof(bool))!=NULL){
                
			    attribute->v.boolV = value;
            }

			attribute->dt = DT_BOOL;
			break;
		}
		default:
				printf("Serializer not defined for the given datatype. \n");
				break;
		}
		*value = attribute;
        if(flag == true)
		    attribute_value=RC_OK;
		
        return attribute_value;
;
}

extern RC setAttr (Record *record, Schema *schema, int attrNum, Value *value)	
{	
    char *dataPoint = record->data;	
    int bal = 0;	
	attrOffset(schema, attrNum, &bal);	
	dataPoint = bal + dataPoint;	
	switch(schema->dataTypes[attrNum])	
	{	
		case DT_STRING:	
		{	
			int length = schema->typeLength[attrNum];	
            if(attrNum >= 0){	
                strncpy(dataPoint, (*value).v.stringV, length);	
                dataPoint += schema->typeLength[attrNum];	
            }	
            break;	
		}	
		case DT_INT:	
		{	
			*(int *)dataPoint = (*value).v.intV;	
            if(attrNum >= 0){	
                dataPoint += sizeof(int);	
            }	
				
			break;	
		}	
		case DT_FLOAT:	
		{	
			*(float *) dataPoint = (*value).v.floatV;	
            if(attrNum >=0){	
                dataPoint += sizeof(float);	
            }	
				
			break;	
		}	
		case DT_BOOL:	
		{	
			*(bool *) dataPoint = (*value).v.boolV;	
            if(attrNum>=0){	
                dataPoint += sizeof(bool);	
            }	
				
			break;	
		}	
		default:{	
				printf("Serializer not defined for the given datatype. \n");	
				break;	
        }	
	}	
			
		return RC_OK;	
}

























