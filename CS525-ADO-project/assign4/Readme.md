RUNNING THE SCRIPT 
===========================================

1) Using Terminal, navigate to the Project root (assign4).

2) Enter ls to list the files and make sure we're in the right directory.

3) Run "make clean" to remove previously compiled.o files.

4) Execute "make" to compile all project files, including "test assign4 1.c."

5) Enter "./recordmgrtest_assign4" to execute the "test assign4 1.c" file. 
 
 ACCESS INFORMATION ABOUT OUR B+ TREE
=================================================
These functions enable us to obtain information about our B+ Tree, such as the number of nodes and keys in our tree.

getNumNodes(...) 
--> This function retrieves the number of nodes in our B+ Tree.
--> In B-trees, you usually start at the root node and visit the tree recursively, counting each node as you approach.
--> This information is stored in our TreeManager structure in the "numNodes" variable. As a result, we just return this data.


getNumEntries(...) 
--> Returns the number of records/entries/keys in our B+ Tree.
--> This information is recorded in the "numEntries" variable of our TreeManager structure. So we simply return this data.
-->To begin, the method counts the keys in the current node. It then loops through each child node, calling getNumEntries() recursively on each child and adding the outcome to the count.


getKeyType(...) 
--> Returns the datatype of the keys in our B+ Tree.
--> In our TreeManager structure, we save this information in the "keyType" variable. So we simply return this information.
--> In order to use getKeyType(...) in B-trees, the type of keys must usually be defined when the B-tree is created. This is due to the need that all keys in one node have the same data type in B-trees.


4. ACCESSING B+ TREE FUNCTIONS
=========================================
These functions are used to search for, insert, and delete keys in/from a specified B+ Tree. We can also employ the nextEntry,openTreeScan,  and closeTreeScan methods to iterate through all entries of a B+ Tree in sorted order.

findKey(...) 
--> This method searches the B+ Tree for the key provided in the parameter.
--> We use the findRecord(..) function to accomplish this. If findRecord(..) returns NULL, it means the key was not found in the B+ Tree, and we provide the error number RC_IM_KEY_NOT_FOUND.
--> If an entry with the provided key is found, we store the RID - value for that key in the storage device solution specified by the "result" option.
--> findKey() searches for a given key in the B-tree. The method starts at the root node of the tree and drops recursively through the tree, evaluating the search key to the keys at each node it reaches.


insertKey(...) 
--> This function creates a new entry or record with the given key and RID.
--> We begin by looking for the supplied key in the B+ Tree. If it is discovered, the error code RC_IM_KEY_ALREADY_EXISTS is returned.
--> If the RID does not exist, we create a NodeData structure to hold it.
--> We determine whether the tree's root is empty. If it's not empty, we call createNewTree(..), which generates a new B+ Tree and adds this entry to it.
--> If our tree already has a root element, then we find the leaf node where this entry can be put.
--> If the tree is empty, the function inserts a new root node with the key. If the tree is not empty, it checks to see if the root node is full and, if so, splits it and generates a new root node with the middle key.


deleteKey(...) 
--> This function eliminates the entry in the B+ Tree with the given "key" from the tree.
--> As previously said, our B+ Tree technique is called remove(...). This function removes the entry or key from the tree while altering it to keep the B+ Tree attributes.
--> If the tree is empty, the procedure returns without doing anything. It then executes the deleteKey() method on the root node, handing in the key to be deleted. 


openTreeScan(...) 
--> This function initiates the scan, which is used to scan the entries in the B+ Tree in the sorted key order.
--> This function initializes our ScanManager structure, which keeps extra data for performing the scan operation. 
--> If the root node of the B+ Tree is NULL, we return an error result. RC_NO_RECORDS_TO_SCAN.
--> The method checks to see if the tree is empty and returns an empty list if it is. Otherwise, it executes the root node's scan() method, which climbs the tree recursively, accessing each node to analyze the keys in chronological order.


nextEntry(...) 
--> This function is utilized to traverse the B+ Tree entries.
--> It records the record details, mainly the RID, in the memory address specified by the "result" parameter.
--> If all of the data has been thoroughly examined and no more entries remain, we return the error code. RC_IM_NO_MORE_ENTRIES;
--> The method checks to see if the tree is empty and returns None if it is. Otherwise, it executes the nextEntry(key) method on the root node, which descends the tree iteratively in search of the node with the input key. 


closeTreeScan(...) 
--> This function stops the scan process and relieves resources.
--> The closeTreeScan(scan) method simply executes the close() method on the scan object to free up the scan's resources.
--> When the scan is finished or no longer needed, use the closeTreeScan(scan) method to close it and free up the related resources.
